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

#include "RotateObjectsToolController.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/HitQuery.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Circle.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "View/RotateObjectsTool.h"
#include "View/InputState.h"
#include "View/MoveToolController.h"

#include <vecmath/intersection.h>
#include <vecmath/mat_ext.h>
#include <vecmath/quat.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>

#include <sstream>

namespace TrenchBroom {
    namespace View {
        class RotateObjectsToolController::RotateObjectsBase : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            RotateObjectsTool* m_tool;
        private:
            RotateObjectsHandle::HitArea m_area;
            vm::vec3 m_center;
            vm::vec3 m_start;
            vm::vec3 m_axis;
            FloatType m_angle;
        protected:
            explicit RotateObjectsBase(RotateObjectsTool* tool) :
            m_tool(tool),
            m_area(RotateObjectsHandle::HitArea::None),
            m_angle(FloatType(0.0)) {
                ensure(m_tool != nullptr, "tool is null");
            }
        private:
            Tool* doGetTool() override {
                return m_tool;
            }

            const Tool* doGetTool() const override {
                return m_tool;
            }

            bool doMouseClick(const InputState& inputState) override {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
                    return false;
                }

                const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
                if (!hit.isMatch()) {
                    return false;
                }

                const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
                if (area == RotateObjectsHandle::HitArea::Center) {
                    return false;
                }

                m_tool->updateToolPageAxis(area);
                return true;
            }

            DragInfo doStartDrag(const InputState& inputState) override {
                if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                    inputState.modifierKeys() != ModifierKeys::MKNone) {
                    return DragInfo();
                }

                const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
                if (!hit.isMatch()) {
                    return DragInfo();
                }

                const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
                if (area == RotateObjectsHandle::HitArea::Center) {
                    return DragInfo();
                }

                // We cannot use the hit's hitpoint because it is on the surface of the handle torus, whereas our drag snapper expects it to
                // be on the plane defined by the rotation handle center and the rotation axis.
                const auto handleArea = hit.target<RotateObjectsHandle::HitArea>();
                const auto rotationCenter = m_tool->rotationCenter();
                const auto rotationAxis = m_tool->rotationAxis(handleArea);
                const auto startPointDistance = vm::intersect_ray_plane(inputState.pickRay(), vm::plane3(rotationCenter, rotationAxis));
                if (vm::is_nan(startPointDistance)) {
                    return DragInfo();
                }

                m_tool->beginRotation();

                m_area = handleArea;
                m_center = rotationCenter;
                m_start = vm::point_at_distance(inputState.pickRay(), startPointDistance);
                m_axis = rotationAxis;
                m_angle = 0.0;
                const auto radius = m_tool->majorHandleRadius(inputState.camera());
                return DragInfo(new CircleDragRestricter(m_center, m_axis, radius), new CircleDragSnapper(m_tool->grid(), m_tool->angle(), m_start, m_center, m_axis, radius));
            }

            DragResult doDrag(const InputState&, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                const vm::vec3 ref = vm::normalize(m_start - m_center);
                const vm::vec3 vec = vm::normalize(nextHandlePosition - m_center);
                m_angle = vm::measure_angle(vec, ref, m_axis);
                m_tool->applyRotation(m_center, m_axis, m_angle);
                return DR_Continue;
            }

            void doEndDrag(const InputState&) override {
                m_tool->commitRotation();
            }

            void doCancelDrag() override {
                m_tool->cancelRotation();
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                if (thisToolDragging()) {
                    doRenderHighlight(inputState, renderContext, renderBatch, m_area);
                    renderAngleIndicator(renderContext, renderBatch);
                    renderAngleText(renderContext, renderBatch);
                } else {
                    const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
                    if (hit.isMatch()) {
                        const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
                        if (area != RotateObjectsHandle::HitArea::Center)
                            doRenderHighlight(inputState, renderContext, renderBatch, hit.target<RotateObjectsHandle::HitArea>());
                    }
                }
            }

            class AngleIndicatorRenderer : public Renderer::DirectRenderable {
            private:
                vm::vec3 m_position;
                Renderer::Circle m_circle;
            public:
                AngleIndicatorRenderer(const vm::vec3& position, const float radius, const vm::axis::type axis, const vm::vec3& startAxis, const vm::vec3& endAxis) :
                m_position(position),
                m_circle(radius, 24, true, axis, vm::vec3f(startAxis), vm::vec3f(endAxis)) {}
            private:
                void doPrepareVertices(Renderer::VboManager& vboManager) override {
                    m_circle.prepare(vboManager);
                }

                void doRender(Renderer::RenderContext& renderContext) override {
                    glAssert(glDisable(GL_DEPTH_TEST))

                    glAssert(glPushAttrib(GL_POLYGON_BIT))
                    glAssert(glDisable(GL_CULL_FACE))
                    glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))

                    Renderer::MultiplyModelMatrix translation(renderContext.transformation(),vm::translation_matrix(vm::vec3f(m_position)));
                    Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
                    shader.set("Color", Color(1.0f, 1.0f, 1.0f, 0.2f));
                    m_circle.render();

                    glAssert(glEnable(GL_DEPTH_TEST))
                    glAssert(glPopAttrib())
                }
            };

            void renderAngleIndicator(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                const auto handleRadius = static_cast<float>(m_tool->majorHandleRadius(renderContext.camera()));
                const auto startAxis = normalize(m_start - m_center);
                const auto endAxis = vm::quat3(m_axis, m_angle) * startAxis;

                renderBatch.addOneShot(new AngleIndicatorRenderer(m_center, handleRadius, vm::find_abs_max_component(m_axis), startAxis, endAxis));
            }

            void renderAngleText(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                Renderer::RenderService renderService(renderContext, renderBatch);

                renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
                renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
                renderService.renderString(angleString(vm::to_degrees(m_angle)), vm::vec3f(m_center));
            }

            std::string angleString(const FloatType angle) const {
                std::stringstream str;
                str.precision(2);
                str.setf(std::ios::fixed);
                str << angle;
                return str.str();
            }

            bool doCancel() override {
                return false;
            }
        private:
            virtual void doRenderHighlight(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) = 0;
        };

        class RotateObjectsToolController::MoveCenterBase : public MoveToolController<NoPickingPolicy, NoMousePolicy> {
        protected:
            RotateObjectsTool* m_tool;
        protected:
            explicit MoveCenterBase(RotateObjectsTool* tool) :
            MoveToolController(tool->grid()),
            m_tool(tool) {
                ensure(m_tool != nullptr, "tool is null");
            }

            Tool* doGetTool() override {
                return m_tool;
            }

            const Tool* doGetTool() const override {
                return m_tool;
            }

            MoveInfo doStartMove(const InputState& inputState) override {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.checkModifierKeys(ModifierKeyPressed::MK_No, ModifierKeyPressed::MK_DontCare, ModifierKeyPressed::MK_No)) {
                    return MoveInfo();
                }

                const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
                if (!hit.isMatch()) {
                    return MoveInfo();
                }

                if (hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea::Center) {
                    return MoveInfo();
                }

                return MoveInfo(m_tool->rotationCenter());
            }

            DragResult doMove(const InputState&, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                m_tool->setRotationCenter(nextHandlePosition);
                return DR_Continue;
            }

            void doEndMove(const InputState&) override {}

            void doCancelMove() override {
                m_tool->setRotationCenter(initialHandlePosition());
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                MoveToolController::doRender(inputState, renderContext, renderBatch);
                if (thisToolDragging()) {
                    doRenderHighlight(inputState, renderContext, renderBatch, RotateObjectsHandle::HitArea::Center);
                } else if (!anyToolDragging(inputState)) {
                    const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
                    if (hit.isMatch() && hit.target<RotateObjectsHandle::HitArea>() == RotateObjectsHandle::HitArea::Center) {
                        doRenderHighlight(inputState, renderContext, renderBatch, RotateObjectsHandle::HitArea::Center);
                    }
                }
            }

            bool doCancel() override {
                return false;
            }
        private:
            virtual void doRenderHighlight(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) = 0;
        };

        RotateObjectsToolController::RotateObjectsToolController(RotateObjectsTool* tool) :
        m_tool(tool) {}

        RotateObjectsToolController::~RotateObjectsToolController() = default;

        Tool* RotateObjectsToolController::doGetTool() {
            return m_tool;
        }

        const Tool* RotateObjectsToolController::doGetTool() const {
            return m_tool;
        }

        void RotateObjectsToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            const Model::Hit hit = doPick(inputState);
            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }

        void RotateObjectsToolController::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            const Model::Hit& hit = inputState.pickResult().query().type(RotateObjectsHandle::HandleHitType).occluded().first();
            if (thisToolDragging() || hit.isMatch()) {
                renderContext.setForceShowSelectionGuide();
            }
        }

        void RotateObjectsToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            doRenderHandle(renderContext, renderBatch);
            ToolControllerGroup::doRender(inputState, renderContext, renderBatch);
        }

        bool RotateObjectsToolController::doCancel() {
            return false;
        }

        class RotateObjectsToolController2D::MoveCenterPart : public MoveCenterBase {
        public:
            explicit MoveCenterPart(RotateObjectsTool* tool) :
            MoveCenterBase(tool) {}
        private:
            void doRenderHighlight(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) override {
                m_tool->renderHighlight2D(renderContext, renderBatch, area);
            }
        };

        class RotateObjectsToolController2D::RotateObjectsPart : public RotateObjectsBase {
        public:
            explicit RotateObjectsPart(RotateObjectsTool* tool) :
            RotateObjectsBase(tool) {}
        private:
            void doRenderHighlight(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) override {
                m_tool->renderHighlight2D(renderContext, renderBatch, area);
            }
        };

        RotateObjectsToolController2D::RotateObjectsToolController2D(RotateObjectsTool* tool) :
        RotateObjectsToolController(tool) {
            addController(new MoveCenterPart(tool));
            addController(new RotateObjectsPart(tool));
        }

        Model::Hit RotateObjectsToolController2D::doPick(const InputState& inputState) {
            return m_tool->pick2D(inputState.pickRay(), inputState.camera());
        }

        void RotateObjectsToolController2D::doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->renderHandle2D(renderContext, renderBatch);
        }


        class RotateObjectsToolController3D::MoveCenterPart : public MoveCenterBase {
        public:
            explicit MoveCenterPart(RotateObjectsTool* tool) :
            MoveCenterBase(tool) {}
        private:
            void doRenderHighlight(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) override {
                m_tool->renderHighlight3D(renderContext, renderBatch, area);
            }
        };

        class RotateObjectsToolController3D::RotateObjectsPart : public RotateObjectsBase {
        public:
            explicit RotateObjectsPart(RotateObjectsTool* tool) :
            RotateObjectsBase(tool) {}
        private:
            void doRenderHighlight(const InputState&, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, RotateObjectsHandle::HitArea area) override {
                m_tool->renderHighlight3D(renderContext, renderBatch, area);
            }
        };

        RotateObjectsToolController3D::RotateObjectsToolController3D(RotateObjectsTool* tool) :
        RotateObjectsToolController(tool) {
            addController(new MoveCenterPart(tool));
            addController(new RotateObjectsPart(tool));
        }

        Model::Hit RotateObjectsToolController3D::doPick(const InputState& inputState) {
            return m_tool->pick3D(inputState.pickRay(), inputState.camera());
        }

        void RotateObjectsToolController3D::doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->renderHandle3D(renderContext, renderBatch);
        }
    }
}
