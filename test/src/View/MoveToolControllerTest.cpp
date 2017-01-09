/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Renderer/PerspectiveCamera.h"
#include "View/Grid.h"
#include "View/MoveToolController.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace View {
        class MockMoveToolController : public MoveToolController<NoPickingPolicy, NoMousePolicy> {
        private:
            class MyTool : public Tool {
            public:
                MyTool() : Tool(true) {}
            };
            
            MyTool m_tool;
        public:
            MockMoveToolController(const Grid& grid) :
            MoveToolController(grid),
            m_tool() {}
        protected:
            MoveInfo doStartMove(const InputState& inputState) {
                return mockDoStartMove(inputState);
            }
            
            DragResult doMove(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) {
                return mockDoMove(inputState, lastHandlePosition, nextHandlePosition);
            }
            
            void doEndMove(const InputState& inputState) {
                mockDoEndMove(inputState);
            }
            
            void doCancelMove() {
                mockDoCancelMove();
            }
            
            Tool* doGetTool() { return &m_tool; }
            bool doCancel() { return false; }
        public:
            MOCK_METHOD1(mockDoStartMove, MoveInfo(const InputState&));
            MOCK_METHOD3(mockDoMove, DragResult(const InputState&, const Vec3&, const Vec3&));
            MOCK_METHOD1(mockDoEndMove, void(const InputState&));
            MOCK_METHOD0(mockDoCancelMove, void());
        public:
            using MoveToolController::MoveInfo;
            using RestrictedDragPolicy::DragResult;
            using RestrictedDragPolicy::DR_Continue;
        };
        
        TEST(MoveToolControllerTest, testMoveWithSnapUp) {
            using namespace ::testing;
            using ::testing::InSequence;

            const Renderer::Camera::Viewport viewport(-200, -200, 400, 400);
            Renderer::PerspectiveCamera camera(90.0f, 0.1f, 500.0f, viewport, Vec3f(0.0f, 0.0f, 100.0f), Vec3f::NegZ, Vec3f::PosY);
            
            const Grid grid(4); // Grid size 16
            MockMoveToolController controller(grid);
            
            InputState inputState(0, 0);
            inputState.mouseDown(MouseButtons::MBLeft);
            
            const Vec3 origin = Vec3(camera.position());
            inputState.setPickRequest(PickRequest(Ray3(origin, Vec3::NegZ), camera));

            EXPECT_CALL(controller, mockDoStartMove(Ref(inputState))).Times(1).WillOnce(Return(MockMoveToolController::MoveInfo(Vec3::Null)));
            controller.startMouseDrag(inputState);
            
            inputState.mouseMove(9, 0, 9, 0);
            inputState.setPickRequest(PickRequest(Ray3(origin, (Vec3(9.0, 0.0, 0.0) - origin).normalized()), camera));
            
            EXPECT_CALL(controller, mockDoMove(Ref(inputState), Vec3(0.0, 0.0, 0.0), Vec3(16.0, 0.0, 0.0))).Times(1).WillOnce(Return(MockMoveToolController::DR_Continue));
            controller.mouseDrag(inputState);
            
            inputState.mouseUp(MouseButtons::MBLeft);
            EXPECT_CALL(controller, mockDoEndMove(Ref(inputState)));
            controller.endMouseDrag(inputState);
        }
        
        TEST(MoveToolControllerTest, testMoveAfterZeroVerticalMove) {
            // see https://github.com/kduske/TrenchBroom/issues/1529
            
            using namespace ::testing;
            using ::testing::InSequence;
            
            const Renderer::Camera::Viewport viewport(-200, -200, 400, 400);
            Renderer::PerspectiveCamera camera(90.0f, 0.1f, 500.0f, viewport, Vec3f(0.0f, 0.0f, 100.0f), Vec3f::NegZ, Vec3f::PosY);
            
            const Grid grid(4); // Grid size 16
            MockMoveToolController controller(grid);
            
            InputState inputState(0, 0);
            inputState.mouseDown(MouseButtons::MBLeft);
            
            const Vec3 origin = Vec3(camera.position());
            inputState.setPickRequest(PickRequest(Ray3(origin, Vec3::NegZ), camera));
            
            EXPECT_CALL(controller, mockDoStartMove(Ref(inputState))).Times(1).WillOnce(Return(MockMoveToolController::MoveInfo(Vec3::Null)));
            controller.startMouseDrag(inputState);
            
            // nothing will happen due to grid snapping
            inputState.mouseMove(1, 0, 1, 0);
            inputState.setPickRequest(PickRequest(Ray3(origin, (Vec3(1.0, 0.0, 0.0) - origin).normalized()), camera));
            controller.mouseDrag(inputState);
            
            // trigger switch to vertical move mode
            inputState.setModifierKeys(ModifierKeys::MKAlt);
            controller.modifierKeyChange(inputState);

            // and back
            inputState.setModifierKeys(ModifierKeys::MKNone);
            controller.modifierKeyChange(inputState);
            
            // must not trigger an actual move
            inputState.mouseMove(2, 0, 1, 0);
            inputState.setPickRequest(PickRequest(Ray3(origin, (Vec3(2.0, 0.0, 0.0) - origin).normalized()), camera));
            controller.mouseDrag(inputState);
            
            inputState.mouseUp(MouseButtons::MBLeft);
            EXPECT_CALL(controller, mockDoEndMove(Ref(inputState)));
            controller.endMouseDrag(inputState);
        }
    }
}
