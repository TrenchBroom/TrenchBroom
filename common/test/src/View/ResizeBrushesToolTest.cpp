/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2020 Eric Wasylishen

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

#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/LayerNode.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "View/ResizeBrushesTool.h"

#include <kdl/result.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <memory>

#include "MapDocumentTest.h"
#include "TestLogger.h"
#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class ResizeBrushesToolTest : public MapDocumentTest {
        public:
            ResizeBrushesToolTest() : MapDocumentTest(Model::MapFormat::Valve) {}
        };

        static const auto PickRay = vm::ray3(vm::vec3(0.0, -100.0, 0.0), vm::normalize(vm::vec3(-1.0, 1.0, 0)));

        TEST_CASE_METHOD(ResizeBrushesToolTest, "ResizeBrushesToolTest.pickBrush") {
            ResizeBrushesTool tool(document);

            const auto bboxMax = GENERATE(vm::vec3::fill(0.01),
                                          vm::vec3::fill(8.0));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 =new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3::fill(0.0), bboxMax), "texture").value());

            addNode(*document, document->currentLayer(), brushNode1);
            document->select(brushNode1);

            const Model::Hit hit = tool.pick3D(PickRay, Model::PickResult());
            CHECK(hit.isMatch());
            CHECK(hit.type() == ResizeBrushesTool::Resize3DHitType);
            CHECK(!vm::is_nan(hit.hitPoint()));
            CHECK(!vm::is_nan(hit.distance()));

            const auto hitHandle = hit.target<ResizeBrushesTool::Resize3DHitData>();
            CHECK(hitHandle.node() == brushNode1);
            CHECK(hitHandle.faceIndex() == brushNode1->brush().findFace(vm::vec3::neg_x()).value());
        }

        /**
         * Test for https://github.com/TrenchBroom/TrenchBroom/issues/3726
         */
        TEST_CASE("ResizeBrushesToolTest.findDragFaces", "[ResizeBrushesToolTest]") {
            struct TestCase {
                IO::Path mapName;
                std::vector<std::string> expectedDragFaceTextureNames;
            };

            auto [mapName, expectedDragFaceTextureNames] = GENERATE(values<TestCase>({
                {IO::Path("findDragFaces_noCoplanarFaces.map"), {"larger_top_face"}},
                {IO::Path("findDragFaces_twoCoplanarFaces.map"), {"larger_top_face", "smaller_top_face"}}
            }));

            const auto mapPath = IO::Path("fixture/test/View/ResizeBrushesToolTest") + mapName;
            auto [document, game, gameConfig] = View::loadMapDocument(mapPath, "Quake", Model::MapFormat::Valve);

            document->selectAllNodes();

            auto brushes = document->selectedNodes().brushes();
            REQUIRE(brushes.size() == 2);

            const Model::BrushFace& largerTopFace = brushes.at(0)->brush().face(brushes.at(0)->brush().findFace("larger_top_face").value());

            // Find the entity defining the camera position for our test
            Model::EntityNode* cameraEntity = kdl::vec_filter(document->selectedNodes().entities(),
                                                             [](const Model::EntityNode* node){ return node->entity().classname() == "trigger_relay"; }).at(0);

            // Fire a pick ray at largerTopFace
            const auto pickRay = vm::ray3(cameraEntity->entity().origin(),
                                          vm::normalize(largerTopFace.center() - cameraEntity->entity().origin()));

            auto tool = ResizeBrushesTool(document);

            Model::PickResult pickResult = Model::PickResult::byDistance(document->editorContext());
            document->pick(pickRay, pickResult); // populate pickResult

            const Model::Hit hit = tool.pick3D(pickRay, pickResult);
            CHECK(hit.type() == ResizeBrushesTool::Resize3DHitType);
            CHECK(!vm::is_nan(hit.hitPoint()));

            const Model::BrushFaceHandle hitTarget = hit.target<Model::BrushFaceHandle>();
            REQUIRE(hitTarget.face() == largerTopFace);
            REQUIRE(hit.isMatch());
            pickResult.addHit(hit);

            // Find the faces that we would drag when pressing Shift
            REQUIRE(!tool.hasDragFaces());
            tool.updateDragFaces(pickResult);
            REQUIRE(tool.hasDragFaces());

            const std::vector<std::string> dragFaces =
                kdl::vec_transform(tool.dragFaces(),
                                   [](const Model::BrushFaceHandle& handle) { return handle.face().attributes().textureName(); });
            CHECK_THAT(dragFaces, Catch::UnorderedEquals(expectedDragFaceTextureNames));
        }
    }
}
