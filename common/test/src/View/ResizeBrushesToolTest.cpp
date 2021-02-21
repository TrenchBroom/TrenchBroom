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

#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/GameImpl.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "View/ResizeBrushesTool.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "TestLogger.h"
#include "MapDocumentTest.h"
#include "Model/TestGame.h"

#include <memory>

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
         * Test for 
         * Issue where two planes with almost identical distance, but different normals.
         */
        TEST_CASE("ResizeBrushesToolTest.findCoplanarFacesTest", "[ResizeBrushesToolTest]") {
            // FIXME: deduplicate with EntityModelTest
            TestLogger logger;
            const auto configPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/games/Quake/GameConfig.cfg");
            const auto gamePath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Game/Quake");
            const auto configStr = IO::Disk::readTextFile(configPath);
            auto configParser = IO::GameConfigParser(configStr, configPath);
            Model::GameConfig config = configParser.parse();

            auto game = std::make_shared<Model::GameImpl>(config, gamePath, logger);

            // create document
            auto document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(Model::MapFormat::Valve, vm::bbox3(8192.0), game);

            const auto mapPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/View/ResizeBrushesToolTest/findCoplanarFacesTest.map");
            document->loadDocument(document->world()->mapFormat(), document->worldBounds(), game, mapPath);

            document->selectAllNodes();

            auto brushes = document->selectedNodes().brushes();
            REQUIRE(brushes.size() == 2);

            // The two faces of interest
            const Model::BrushFace& largerTopFace = brushes.at(0)->brush().face(brushes.at(0)->brush().findFace("larger_top_face").value());
            const Model::BrushFace& smallerTopFace = brushes.at(1)->brush().face(brushes.at(1)->brush().findFace("smaller_top_face").value());

            // Find the entities defining the camera position and look at point for our test
            Model::EntityNode* cameraEntity = kdl::vec_filter(document->selectedNodes().entities(),
                                                             [](const Model::EntityNode* node){ return node->entity().classname() == "trigger_relay"; }).at(0);

            const auto pickRay = vm::ray3(cameraEntity->entity().origin(),
                                          vm::normalize(largerTopFace.center() - cameraEntity->entity().origin()));

            ResizeBrushesTool tool(document);

            Model::PickResult pickResult = Model::PickResult::byDistance(document->editorContext());
            const Model::Hit hit = tool.pick3D(pickRay, pickResult);

            CHECK(hit.type() == ResizeBrushesTool::Resize3DHitType);
            CHECK(!vm::is_nan(hit.hitPoint()));

            std::cout << pickRay.get_origin() << "\n";
            std::cout << hit.hitPoint() << "\n";
            const Model::BrushFaceHandle hitTarget = hit.target<Model::BrushFaceHandle>();
            std::cout << hitTarget.face().boundary().normal << "\n";

            REQUIRE(hit.isMatch());
            pickResult.addHit(hit);

            // this will find the faces that we're going to drag
            REQUIRE(!tool.hasDragFaces());
            tool.updateDragFaces(pickResult);

            REQUIRE(tool.hasDragFaces());
            CHECK(tool.dragFaces().size() == 1);

            std::cout << "drag face normals\n";
            for (auto& dragFaceHandle : tool.dragFaces()) {
                std::cout << dragFaceHandle.face().boundary().normal << "\n";
            }
        }
    }
}
