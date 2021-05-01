/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapDocumentTest.h"

#include "Exceptions.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "IO/WorldReader.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EmptyPropertyKeyIssueGenerator.h"
#include "Model/EmptyPropertyValueIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/ModelUtils.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/TestGame.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/PasteType.h"
#include "View/SelectionTool.h"

#include <kdl/result.h>
#include <kdl/overload.h>
#include <kdl/vector_utils.h>
#include <kdl/zip_iterator.h>

#include <vecmath/approx.h>
#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/scalar.h>

#include "Catch2.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        MapDocumentTest::MapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Standard) {}

        MapDocumentTest::MapDocumentTest(const Model::MapFormat mapFormat) :
        m_mapFormat(mapFormat),
        m_pointEntityDef(nullptr),
        m_brushEntityDef(nullptr) {
            SetUp();
        }

        void MapDocumentTest::SetUp() {
            game = std::make_shared<Model::TestGame>();
            document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(m_mapFormat, vm::bbox3(8192.0), game);

            // create two entity definitions
            m_pointEntityDef = new Assets::PointEntityDefinition("point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});
            m_brushEntityDef = new Assets::BrushEntityDefinition("brush_entity", Color(), "this is a brush entity", {});

            document->setEntityDefinitions(std::vector<Assets::EntityDefinition*>{ m_pointEntityDef, m_brushEntityDef });
        }

        MapDocumentTest::~MapDocumentTest() {
            m_pointEntityDef = nullptr;
            m_brushEntityDef = nullptr;
        }

        Model::BrushNode* MapDocumentTest::createBrushNode(const std::string& textureName, const std::function<void(Model::Brush&)>& brushFunc) const {
            const Model::WorldNode* world = document->world();
            Model::BrushBuilder builder(world->mapFormat(), document->worldBounds(), document->game()->defaultFaceAttribs());
            Model::Brush brush = builder.createCube(32.0, textureName).value();
            brushFunc(brush);
            return new Model::BrushNode(std::move(brush));
        }

        Model::PatchNode* MapDocumentTest::createPatchNode(const std::string& textureName) const {
            return new Model::PatchNode{Model::BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, textureName}};
        }

        ValveMapDocumentTest::ValveMapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Valve) {}

        Quake3MapDocumentTest::Quake3MapDocumentTest() :
        MapDocumentTest{Model::MapFormat::Quake3} {}

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
            CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
        }

        TEST_CASE("MapDocumentTest.detectValveFormatMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/valveFormatMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
            CHECK(document->world()->defaultLayer()->childCount() == 1);
        }

        TEST_CASE("MapDocumentTest.detectStandardFormatMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/standardFormatMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            CHECK(document->world()->mapFormat() == Model::MapFormat::Standard);
            CHECK(document->world()->defaultLayer()->childCount() == 1);
        }

        TEST_CASE("MapDocumentTest.detectEmptyMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/emptyMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            // an empty map detects as Valve because Valve is listed first in the Quake game config
            CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
            CHECK(document->world()->defaultLayer()->childCount() == 0);
        }

        TEST_CASE("MapDocumentTest.mixedFormats", "[MapDocumentTest]") {
            // map has both Standard and Valve brushes
            CHECK_THROWS_AS(View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/mixedFormats.map"),
                                                  "Quake", Model::MapFormat::Unknown), IO::WorldReaderException);
        }
    }
}
