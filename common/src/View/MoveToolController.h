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

#pragma once

#include "View/ToolController.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>

namespace TrenchBroom {
    namespace View {
        class ToolActivationDelegate;

        template <class PickingPolicyType, class MousePolicyType>
        class MoveToolController : public ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            using Super = ToolControllerBase<PickingPolicyType, KeyPolicy, MousePolicyType, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>;
        private:
            typedef enum {
                MT_Default,
                MT_Vertical,
                MT_Restricted
            } MoveType;

            MoveType m_lastMoveType;
            vm::vec3 m_moveTraceOrigin;
            vm::vec3 m_moveTraceCurPoint;
            bool m_restricted;
        protected:
            struct MoveInfo {
                bool move;
                vm::vec3 initialPoint;

                MoveInfo() :
                move(false) {}

                explicit MoveInfo(const vm::vec3& i_initialPoint) :
                move(true),
                initialPoint(i_initialPoint) {}
            };
        protected:
            const Grid& m_grid;
        public:
            explicit MoveToolController(const Grid& grid) :
            m_lastMoveType(MT_Default),
            m_restricted(false),
            m_grid(grid) {}

            ~MoveToolController() override {}
        protected:
            virtual void doModifierKeyChange(const InputState& inputState) override {
                if (Super::thisToolDragging()) {
                    const vm::vec3& currentPosition = RestrictedDragPolicy::currentHandlePosition();

                    const MoveType nextMoveType = moveType(inputState);
                    if (nextMoveType != m_lastMoveType) {
                        if (m_lastMoveType != MT_Default) {
                            RestrictedDragPolicy::setRestricter(inputState, doCreateDefaultDragRestricter(inputState, currentPosition), m_lastMoveType == MT_Vertical);
                            if (m_lastMoveType == MT_Vertical)
                                m_moveTraceOrigin = m_moveTraceCurPoint = currentPosition;
                        }
                        if (nextMoveType == MT_Vertical) {
                            RestrictedDragPolicy::setRestricter(inputState, doCreateVerticalDragRestricter(inputState, currentPosition), false);
                            m_moveTraceOrigin = m_moveTraceCurPoint = currentPosition;
                            m_restricted = true;
                        } else if (nextMoveType == MT_Restricted) {
                            const vm::vec3& initialPosition = RestrictedDragPolicy::initialHandlePosition();
                            RestrictedDragPolicy::setRestricter(inputState, doCreateRestrictedDragRestricter(inputState, initialPosition, currentPosition), false);
                            m_restricted = true;
                        }
                        m_lastMoveType = nextMoveType;
                    }
                }
            }

        private:
            MoveType moveType(const InputState& inputState) const {
                if (isVerticalMove(inputState)) {
                    return MT_Vertical;
                } else if (isRestrictedMove(inputState)) {
                    return MT_Restricted;
                } else {
                    return MT_Default;
                }
            }

            virtual bool isVerticalMove(const InputState& inputState) const {
                const Renderer::Camera& camera = inputState.camera();
                return camera.perspectiveProjection() && inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
            }

            virtual bool isRestrictedMove(const InputState& inputState) const {
                return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift);
            }
        protected:
            RestrictedDragPolicy::DragInfo doStartDrag(const InputState& inputState) override {
                const MoveInfo info = doStartMove(inputState);
                if (!info.move) {
                    return RestrictedDragPolicy::DragInfo();
                }

                DragRestricter* restricter = nullptr;
                if (isVerticalMove(inputState)) {
                    restricter = doCreateVerticalDragRestricter(inputState, info.initialPoint);
                    m_lastMoveType = MT_Vertical;
                    m_restricted = true;
                } else {
                    restricter = doCreateDefaultDragRestricter(inputState, info.initialPoint);
                    m_lastMoveType = MT_Default;
                    m_restricted = false;
                }

                m_moveTraceOrigin = m_moveTraceCurPoint = info.initialPoint;
                DragSnapper* snapper = doCreateDragSnapper(inputState);
                return RestrictedDragPolicy::DragInfo(restricter, snapper, info.initialPoint);
            }

            RestrictedDragPolicy::DragResult doDrag(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) override {
                const RestrictedDragPolicy::DragResult result = doMove(inputState, lastHandlePosition, nextHandlePosition);
                if (result == RestrictedDragPolicy::DR_Continue) {
                    m_moveTraceCurPoint = m_moveTraceCurPoint + (nextHandlePosition - lastHandlePosition);
                }
                return result;
            }

            void doEndDrag(const InputState& inputState) override {
                doEndMove(inputState);
            }

            void doCancelDrag() override {
                doCancelMove();
            }

            void doRender(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                if (Super::thisToolDragging()) {
                    renderMoveTrace(renderContext, renderBatch);
                }
            }

            void renderMoveTrace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                const auto& start = m_moveTraceOrigin;
                const auto& end = m_moveTraceCurPoint;
                if (end != start) {
                    const auto vec = end - start;

                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setShowOccludedObjects();
                    if (m_restricted) {
                        renderService.setLineWidth(2.0f);
                    }

                    std::vector<vm::vec3> stages(3);
                    stages[0] = vec * vm::vec3::pos_x();
                    stages[1] = vec * vm::vec3::pos_y();
                    stages[2] = vec * vm::vec3::pos_z();

                    std::vector<Color> colors(3);
                    colors[0] = pref(Preferences::XAxisColor);
                    colors[1] = pref(Preferences::YAxisColor);
                    colors[2] = pref(Preferences::ZAxisColor);

                    auto lastPos = start;
                    for (size_t i = 0; i < 3; ++i) {
                        const auto& stage = stages[i];
                        const auto curPos = lastPos + stage;

                        renderService.setForegroundColor(colors[i]);
                        renderService.renderLine(vm::vec3f(lastPos), vm::vec3f(curPos));
                        lastPos = curPos;
                    }

                }
            }
        protected: // subclassing interface
            virtual MoveInfo doStartMove(const InputState& inputState) = 0;
            virtual RestrictedDragPolicy::DragResult doMove(const InputState& inputState, const vm::vec3& lastHandlePosition, const vm::vec3& nextHandlePosition) = 0;
            virtual void doEndMove(const InputState& inputState) = 0;
            virtual void doCancelMove() = 0;

            virtual DragRestricter* doCreateDefaultDragRestricter(const InputState& inputState, const vm::vec3& curPoint) const {
                const auto& camera = inputState.camera();
                const auto axis = camera.perspectiveProjection() ? vm::vec3::pos_z() : vm::vec3(vm::get_abs_max_component_axis(camera.direction()));
                return new PlaneDragRestricter(vm::plane3(curPoint, axis));
            }

            virtual DragRestricter* doCreateVerticalDragRestricter(const InputState& inputState, const vm::vec3& curPoint) const {
                const auto& camera = inputState.camera();
                if (camera.perspectiveProjection()) {
                    return new LineDragRestricter(vm::line3(curPoint, vm::vec3::pos_z()));
                } else {
                    const auto axis = vm::vec3(vm::get_abs_max_component_axis(camera.direction()));
                    return new PlaneDragRestricter(vm::plane3(curPoint, axis));
                }
            }

            virtual DragRestricter* doCreateRestrictedDragRestricter(const InputState&, const vm::vec3& initialPoint, const vm::vec3& curPoint) const {
                const auto delta = curPoint - initialPoint;
                const auto axis = vm::get_abs_max_component_axis(delta);
                return new LineDragRestricter(vm::line3(initialPoint, axis));
            }

            virtual DragSnapper* doCreateDragSnapper(const InputState&) const {
                return new DeltaDragSnapper(m_grid);
            }
        };
    }
}

