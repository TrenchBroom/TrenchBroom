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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Renderer/PerspectiveCamera.h"
#include "View/Grid.h"
#include "View/MoveToolController.h"
#include "View/Tool.h"

#include <kdl/vector_utils.h>

#include <variant>

namespace TrenchBroom {
    namespace View {
        class MockMoveToolController : public MoveToolController<NoPickingPolicy, NoMousePolicy> {
        public:
            enum class CheckMoveArgs { No,Yes };
            struct DoStartMove { MoveInfo moveInfoToReturn; };
            struct DoMove { CheckMoveArgs checkArgs; vm::vec3 expectedLastHandlePosition; vm::vec3 nextHandlePosition; DragResult dragResult; };
            struct DoEndMove {};
            struct DoCancelMove {};

            using ExpectedCall = std::variant<
                DoStartMove,
                DoMove,
                DoEndMove,
                DoCancelMove>;
        private:
            mutable std::vector<ExpectedCall> m_expectedCalls;
        private:
            template <class T>
            T popCall() const {
                ASSERT_FALSE(m_expectedCalls.empty());
                const ExpectedCall variant = kdl::vec_pop_front(m_expectedCalls);
                const T call = std::get<T>(variant);
                return call;
            }
        public:
            /**
             * Sets an expectation that the given member function will be called.
             *
             * The expectations set this way are all mandatory and must be called in the order they are set.
             */
            void expectCall(ExpectedCall call) {
                m_expectedCalls.push_back(std::move(call));
            }
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

            ~MockMoveToolController() {
                ASSERT_TRUE(m_expectedCalls.empty());
            }
        protected:
            MoveInfo doStartMove(const InputState&) override {
                return popCall<DoStartMove>().moveInfoToReturn;
            }

            DragResult doMove(const InputState&, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override {
                const DoMove expectedCall = popCall<DoMove>();

                // Only validate the arguments if requested when the expectation was set
                if (expectedCall.checkArgs == CheckMoveArgs::Yes) {
                    ASSERT_EQ(expectedCall.expectedLastHandlePosition, lastHandlePosition);
                    ASSERT_EQ(expectedCall.nextHandlePosition, nextHandlePosition);
                }

                return expectedCall.dragResult;
            }

            void doEndMove(const InputState&) override {
                popCall<DoEndMove>();
            }

            void doCancelMove() override {
                popCall<DoCancelMove>();
            }
            Tool* doGetTool() override { return &m_tool; }
            const Tool* doGetTool() const override { return &m_tool; }
            bool doCancel() override { return false; }
        public:
            using MoveToolController::MoveInfo;
            using RestrictedDragPolicy::DragResult;
            using RestrictedDragPolicy::DR_Continue;
        };

        TEST_CASE("MoveToolControllerTest.testMoveWithSnapUp", "[MoveToolControllerTest]") {
            const Renderer::Camera::Viewport viewport(-200, -200, 400, 400);
            Renderer::PerspectiveCamera camera(90.0f, 0.1f, 500.0f, viewport, vm::vec3f(0.0f, 0.0f, 100.0f), vm::vec3f::neg_z(), vm::vec3f::pos_y());

            const Grid grid(4); // Grid size 16
            MockMoveToolController controller(grid);

            InputState inputState(0, 0);
            inputState.mouseDown(MouseButtons::MBLeft);

            const vm::vec3 origin = vm::vec3(camera.position());
            inputState.setPickRequest(PickRequest(vm::ray3(origin, vm::vec3::neg_z()), camera));

            controller.expectCall(MockMoveToolController::DoStartMove{MockMoveToolController::MoveInfo(vm::vec3::zero())});
            controller.startMouseDrag(inputState);

            inputState.mouseMove(9, 0, 9, 0);
            inputState.setPickRequest(PickRequest(vm::ray3(origin, normalize(vm::vec3(9.0, 0.0, 0.0) - origin)), camera));

            controller.expectCall(MockMoveToolController::DoMove{MockMoveToolController::CheckMoveArgs::Yes,
                                                                 vm::vec3(0.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 0.0), MockMoveToolController::DR_Continue});
            controller.mouseDrag(inputState);

            inputState.mouseUp(MouseButtons::MBLeft);
            controller.expectCall(MockMoveToolController::DoEndMove{});
            controller.endMouseDrag(inputState);
        }

        TEST_CASE("MoveToolControllerTest.testMoveAfterZeroVerticalMove", "[MoveToolControllerTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/1529

            const Renderer::Camera::Viewport viewport(-200, -200, 400, 400);
            Renderer::PerspectiveCamera camera(90.0f, 0.1f, 500.0f, viewport, vm::vec3f(0.0f, 0.0f, 100.0f), vm::vec3f::neg_z(), vm::vec3f::pos_y());

            const Grid grid(4); // Grid size 16
            MockMoveToolController controller(grid);

            InputState inputState(0, 0);
            inputState.mouseDown(MouseButtons::MBLeft);

            const vm::vec3 origin = vm::vec3(camera.position());
            inputState.setPickRequest(PickRequest(vm::ray3(origin, vm::vec3::neg_z()), camera));

            controller.expectCall(MockMoveToolController::DoStartMove{MockMoveToolController::MoveInfo(vm::vec3::zero())});
            controller.startMouseDrag(inputState);

            // nothing will happen due to grid snapping
            // NOTE: if doMove were called it would automatically cause the test to fail
            inputState.mouseMove(1, 0, 1, 0);
            inputState.setPickRequest(PickRequest(vm::ray3(origin, normalize(vm::vec3(1.0, 0.0, 0.0) - origin)), camera));
            controller.mouseDrag(inputState);

            // trigger switch to vertical move mode
            inputState.setModifierKeys(ModifierKeys::MKAlt);
            controller.modifierKeyChange(inputState);

            // and back
            inputState.setModifierKeys(ModifierKeys::MKNone);
            controller.modifierKeyChange(inputState);

            // must not trigger an actual move
            inputState.mouseMove(2, 0, 1, 0);
            inputState.setPickRequest(PickRequest(vm::ray3(origin, normalize(vm::vec3(2.0, 0.0, 0.0) - origin)), camera));
            controller.mouseDrag(inputState);

            inputState.mouseUp(MouseButtons::MBLeft);
            controller.expectCall(MockMoveToolController::DoEndMove{});
            controller.endMouseDrag(inputState);
        }


        TEST_CASE("MoveToolControllerTest.testDontJumpAfterVerticalMoveWithOffset", "[MoveToolControllerTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/pull/1635#issuecomment-271460182

            const Renderer::Camera::Viewport viewport(0, 0, 400, 400);
            Renderer::PerspectiveCamera camera(90.0f, 0.1f, 500.0f, viewport, vm::vec3f(0.0f, 0.0f, 100.0f),
                                               normalize(vm::vec3f::neg_x() + vm::vec3f::neg_y() + vm::vec3f::neg_z()),
                                               normalize(vm::vec3f::neg_x() + vm::vec3f::neg_y() + vm::vec3f::pos_z()));

            const Grid grid(4); // Grid size 16
            MockMoveToolController controller(grid);

            InputState inputState(0, 0);
            inputState.mouseDown(MouseButtons::MBLeft);

            const vm::ray3 initialPickRay(camera.pickRay(200, 200));
            inputState.setPickRequest(PickRequest(initialPickRay, camera));

            const FloatType initialHitDistance = vm::intersect_ray_plane(initialPickRay, vm::plane3(vm::vec3::zero(), vm::vec3::pos_z()));
            const vm::vec3 initialHitPoint = vm::point_at_distance(initialPickRay, initialHitDistance);

            controller.expectCall(MockMoveToolController::DoStartMove{MockMoveToolController::MoveInfo(initialHitPoint)});
            controller.startMouseDrag(inputState);

            // switch to vertical move mode
            inputState.setModifierKeys(ModifierKeys::MKAlt);
            controller.modifierKeyChange(inputState);

            // We don't really care about the actual values
            controller.expectCall(MockMoveToolController::DoMove{MockMoveToolController::CheckMoveArgs::No,
                                                                 vm::vec3(), vm::vec3(), MockMoveToolController::DR_Continue});

            // drag vertically, but with a bit of an offset to the side
            inputState.mouseMove(20, 50, 20, 50);
            inputState.setPickRequest(PickRequest(vm::ray3(camera.pickRay(20, 50)), camera));
            controller.mouseDrag(inputState);

            // switch to horizontal mode, must not trigger a move, so no expectation set
            // NOTE: if doMove were called it would automatically cause the test to fail
            inputState.setModifierKeys(ModifierKeys::MKNone);
            controller.modifierKeyChange(inputState);

            inputState.mouseUp(MouseButtons::MBLeft);
            controller.expectCall(MockMoveToolController::DoEndMove{});
            controller.endMouseDrag(inputState);
        }
    }
}
