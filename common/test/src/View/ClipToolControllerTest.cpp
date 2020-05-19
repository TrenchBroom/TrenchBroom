/*
 Copyright (C) 2019 Eric Wasylishen

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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "MapDocumentTest.h"

#include "Model/BrushNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/ClipTool.h"
#include "View/ClipToolController.h"
#include "View/Grid.h"
#include "View/PasteType.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace View {

        class ClipToolControllerTest : public ValveMapDocumentTest {
        protected:
            ClipToolControllerTest() = default;
        };

        static void updatePickState(InputState& inputState, const Renderer::Camera& camera, const MapDocument& document) {
            Model::PickResult pickResult = Model::PickResult::byDistance(document.editorContext());
            const PickRequest pickRequest(vm::ray3(camera.pickRay(inputState.mouseX(), inputState.mouseY())), camera);

            document.pick(pickRequest.pickRay(), pickResult);

            inputState.setPickRequest(pickRequest);
            inputState.setPickResult(std::move(pickResult));
        }

        // https://github.com/kduske/TrenchBroom/issues/2602
        TEST_CASE_METHOD(ClipToolControllerTest, "ClipToolControllerTest.testTwoPointsCreateClipPlane") {
            const auto data = R"(
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( 20 -16 52 ) ( 20 -16 56 ) ( 20 112 56 ) d3b_door03a [ 0 1 0 -0 ] [ 0 0 -1 0.799988 ] -0 1 1
( -16 -16 52 ) ( -12 -16 48 ) ( -12 112 48 ) d3b_door03a [ 0 -1 0 0.800049 ] [ 0.707107 0 -0.707107 -0.724365 ] -0 1 1
( -16 -16 64 ) ( -16 -16 56 ) ( -16 112 56 ) d3b_door03a [ 0 -1 0 -0 ] [ 0 0 -1 0.799988 ] -0 1 1
( 384 16 72 ) ( 385.24999999999977263 16 72 ) ( 384 16 73.24999999999994316 ) d3b_door03a [ -0.8 0 0 0.200073 ] [ 0 0 -0.8 0.600006 ] -0 1 1
( -56 -16 -128 ) ( -56 -16 -126.75 ) ( -54.75000000000022737 -16 -128 ) d3b_door03a [ 1 0 0 -0 ] [ 0 0 -1 -0 ] -0 1 1
( 384 16 72 ) ( 384 17 72 ) ( 385.24999999999977263 16 72 ) d3b_door03a [ 0.8 0 0 -0.200073 ] [ 0 -1 0 -0 ] -0 1 1
}
}
            )";
            ASSERT_EQ(PasteType::Node, document->paste(data));

            ClipTool tool(document);
            ClipToolController3D controller(&tool);

            ASSERT_TRUE(tool.activate());

            document->grid().setSize(2); // 2^2, so this sets it to grid 4

            const Renderer::Camera::Viewport viewport(0, 0, 1920, 1080);

            // Camera at 0 -160 64 looking towards +y
            Renderer::PerspectiveCamera camera(90.0f, 1.0f, 8000.0f, viewport, vm::vec3f(0.0f, -160.0f, 64.0f), vm::vec3f::pos_y(), vm::vec3f::pos_z());

            // The following test places these 2 clip points
            const auto clipPoint1 = vm::vec3(-16, -16, 52);
            const auto clipPoint2 = vm::vec3( 20, -16, 52);

            auto clipPoint1ScreenSpace = vm::vec2i(vm::round(camera.project(vm::vec3f(clipPoint1))));
            auto clipPoint2ScreenSpace = vm::vec2i(vm::round(camera.project(vm::vec3f(clipPoint2))));

            // Transform the points so (0, 0) is in the upper left
            clipPoint1ScreenSpace = vm::vec2i(clipPoint1ScreenSpace.x(), viewport.height - clipPoint1ScreenSpace.y());
            clipPoint2ScreenSpace = vm::vec2i(clipPoint2ScreenSpace.x(), viewport.height - clipPoint2ScreenSpace.y());

            ASSERT_FALSE(tool.canClip());
            ASSERT_TRUE(tool.canAddPoint( clipPoint1));

            // HACK: bias the points towards the center of the screen a bit
            // There's no way around this unless the clip tool allowed the mouse to be slightly outside of the brush
            InputState inputState(clipPoint1ScreenSpace.x() + 2, clipPoint1ScreenSpace.y());
            updatePickState(inputState, camera, *document);
            ASSERT_EQ(1u, inputState.pickResult().size());

            inputState.mouseDown(MouseButtons::MBLeft);
            ASSERT_TRUE(controller.mouseClick(inputState));
            inputState.mouseUp(MouseButtons::MBLeft);

            ASSERT_FALSE(tool.canClip());
            ASSERT_TRUE(tool.canAddPoint(clipPoint2));

            // HACK: bias the points towards the center of the screen a bit
            inputState.mouseMove(clipPoint2ScreenSpace.x() - 2, clipPoint2ScreenSpace.y(), 0, 0);
            updatePickState(inputState, camera, *document);
            ASSERT_EQ(1u, inputState.pickResult().size());

            inputState.mouseDown(MouseButtons::MBLeft);
            ASSERT_TRUE(controller.mouseClick(inputState));
            inputState.mouseUp(MouseButtons::MBLeft);

            ASSERT_TRUE(tool.canClip());

            tool.performClip();

            // Check the clip result
            // TODO: would be better to check the clip plane but it's not public
            const std::vector<Model::Node*>& objects = document->world()->defaultLayer()->children();
            ASSERT_EQ(1u, objects.size());

            auto* brush = dynamic_cast<Model::BrushNode*>(objects.at(0));
            ASSERT_NE(nullptr, brush);

            ASSERT_EQ(vm::bbox3(vm::vec3(-16, -16, 52), vm::vec3(20, 16, 72)), brush->logicalBounds());
        }
    }
}
