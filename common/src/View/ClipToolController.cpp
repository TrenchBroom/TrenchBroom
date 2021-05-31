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

#include "ClipToolController.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/HitType.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/ClipTool.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/segment.h>
#include <vecmath/distance.h>
#include <vecmath/intersection.h>

#include <memory>

namespace TrenchBroom {
    namespace View {
        namespace {
            class PartDelegateBase {
            protected:
                ClipTool* m_tool;
            public:
                explicit PartDelegateBase(ClipTool* tool) :
                m_tool{tool} {}

                virtual ~PartDelegateBase() = default;

                ClipTool* tool() const {
                    return m_tool;
                }

                bool addClipPoint(const InputState& inputState, vm::vec3& position) {
                    if (!doGetNewClipPointPosition(inputState, position)) {
                        return false;
                    }

                    if (!m_tool->canAddPoint(position)) {
                        return false;
                    }

                    m_tool->addPoint(position, getHelpVectors(inputState, position));
                    return true;
                }

                bool setClipFace(const InputState& inputState) {
                    const Model::Hit& hit = inputState.pickResult().query().type(Model::BrushNode::BrushHitType).occluded().first();
                    if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                        m_tool->setFace(*faceHandle);
                        return true;
                    } else {
                        return false;
                    }
                }

                virtual DragRestricter* createDragRestricter(const InputState& inputState, const vm::vec3& initialPoint) const = 0;
                virtual DragSnapper* createDragSnapper(const InputState& inputState) const = 0;
                virtual std::vector<vm::vec3> getHelpVectors(const InputState& inputState, const vm::vec3& clipPoint) const = 0;

                void renderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                    if (inputState.anyToolDragging()) {
                        return;
                    }

                    vm::vec3 position;
                    if (!doGetNewClipPointPosition(inputState, position)) {
                        return;
                    }

                    if (!m_tool->canAddPoint(position)) {
                        return;
                    }

                    m_tool->renderFeedback(renderContext, renderBatch, position);
                }
            private:
                virtual bool doGetNewClipPointPosition(const InputState& inputState, vm::vec3& position) const = 0;
            };

            class PartDelegate2D : public PartDelegateBase {
            public:
                explicit PartDelegate2D(ClipTool* tool) :
                PartDelegateBase{tool} {}

                DragRestricter* createDragRestricter(const InputState& inputState, const vm::vec3& initialPoint) const override {
                    const auto& camera = inputState.camera();
                    const auto camDir = vm::vec3(camera.direction());
                    return new PlaneDragRestricter(vm::plane3(initialPoint, vm::get_abs_max_component_axis(camDir)));
                }

                DragSnapper* createDragSnapper(const InputState&) const override {
                    return new AbsoluteDragSnapper(m_tool->grid());
                }

                std::vector<vm::vec3> getHelpVectors(const InputState& inputState, const vm::vec3& /* clipPoint */) const override {
                    return std::vector<vm::vec3>{vm::vec3(inputState.camera().direction())};
                }

                bool doGetNewClipPointPosition(const InputState& inputState, vm::vec3& position) const override {
                    const auto& camera = inputState.camera();
                    const auto viewDir = vm::get_abs_max_component_axis(vm::vec3(camera.direction()));

                    const auto& pickRay = inputState.pickRay();
                    const auto defaultPos = m_tool->defaultClipPointPos();
                    const auto distance = vm::intersect_ray_plane(pickRay, vm::plane3(defaultPos, viewDir));
                    if (vm::is_nan(distance)) {
                        return false;
                    } else {
                        const auto& grid = m_tool->grid();
                        position = grid.snap(vm::point_at_distance(pickRay, distance));
                        return true;
                    }
                }
            };


            std::vector<const Model::BrushFace*> selectIncidentFaces(const Model::BrushNode* brushNode, const Model::BrushFace& face, const vm::vec3& hitPoint) {
                static const auto MaxDistance = vm::constants<FloatType>::almost_zero();

                // First, try to see if the clip point is almost equal to a vertex:
                FloatType closestVertexDistance = MaxDistance;
                const Model::BrushVertex* closestVertex = nullptr;
                for (const auto* vertex : face.vertices()) {
                    const auto distance = vm::distance(vertex->position(), hitPoint);
                    if (distance < closestVertexDistance) {
                        closestVertex = vertex;
                        closestVertexDistance = distance;
                    }
                }

                if (closestVertex != nullptr) {
                    const Model::Brush& brush = brushNode->brush();
                    return brush.incidentFaces(closestVertex);
                }

                // Next, try the edges:
                FloatType closestEdgeDistance = MaxDistance;
                const Model::BrushEdge* closestEdge = nullptr;
                for (const auto* edge : face.edges()) {
                    const auto distance = vm::distance(vm::segment<FloatType,3>(edge->firstVertex()->position(), edge->secondVertex()->position()), hitPoint).distance;
                    if (distance < closestEdgeDistance) {
                        closestEdge = edge;
                        closestEdgeDistance = distance;
                    }
                }

                if (closestEdge != nullptr) {
                    const auto firstFaceIndex = closestEdge->firstFace()->payload();
                    const auto secondFaceIndex = closestEdge->secondFace()->payload();
                    
                    if (firstFaceIndex && secondFaceIndex) {
                        return {
                            &brushNode->brush().face(*firstFaceIndex),
                            &brushNode->brush().face(*secondFaceIndex)
                        };
                    }
                }

                return { &face };
            }

            std::vector<vm::vec3> selectHelpVectors(const Model::BrushNode* brushNode, const Model::BrushFace& face, const vm::vec3& hitPoint) {
                auto result = std::vector<vm::vec3>{};
                for (const Model::BrushFace* incidentFace : selectIncidentFaces(brushNode, face, hitPoint)) {
                    const vm::vec3& normal = incidentFace->boundary().normal;
                    result.push_back(vm::get_abs_max_component_axis(normal));
                }

                return kdl::vec_sort_and_remove_duplicates(std::move(result));
            }

            class PartDelegate3D : public PartDelegateBase {
            public:
                explicit PartDelegate3D(ClipTool* tool) :
                PartDelegateBase{tool} {}

                DragRestricter* createDragRestricter(const InputState&, const vm::vec3& /* initialPoint */) const override {
                    SurfaceDragRestricter* restricter = new SurfaceDragRestricter();
                    restricter->setType(Model::BrushNode::BrushHitType);
                    restricter->setOccluded(Model::HitType::AnyType);
                    return restricter;
                }

                class ClipPointSnapper : public SurfaceDragSnapper {
                public:
                    explicit ClipPointSnapper(const Grid& grid) :
                    SurfaceDragSnapper{grid} {}
                private:
                    vm::plane3 doGetPlane(const InputState&, const Model::Hit& hit) const override {
                        const auto faceHandle = Model::hitToFaceHandle(hit);
                        ensure(faceHandle, "invalid hit type");
                        return faceHandle->face().boundary();
                    }
                };

                DragSnapper* createDragSnapper(const InputState&) const override {
                    SurfaceDragSnapper* snapper = new ClipPointSnapper(m_tool->grid());
                    snapper->setType(Model::BrushNode::BrushHitType);
                    snapper->setOccluded(Model::HitType::AnyType);
                    return snapper;
                }

                std::vector<vm::vec3> getHelpVectors(const InputState& inputState, const vm::vec3& clipPoint) const override {
                    auto hit = inputState.pickResult().query().selected().type(Model::BrushNode::BrushHitType).occluded().first();
                    if (!hit.isMatch()) {
                        hit = inputState.pickResult().query().type(Model::BrushNode::BrushHitType).occluded().first();
                    }
                    const auto faceHandle = Model::hitToFaceHandle(hit);
                    ensure(faceHandle, "hit is not a match");

                    return selectHelpVectors(faceHandle->node(), faceHandle->face(), clipPoint);
                }

                bool doGetNewClipPointPosition(const InputState& inputState, vm::vec3& position) const override {
                    const auto hit = inputState.pickResult().query().type(Model::BrushNode::BrushHitType).occluded().first();
                    if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                        const Grid& grid = m_tool->grid();
                        position = grid.snap(hit.hitPoint(), faceHandle->face().boundary());
                        return true;
                    } else {
                        return false;
                    }
                }
            };

            class PartBase {
            protected:
                std::unique_ptr<PartDelegateBase> m_delegate;

                explicit PartBase(std::unique_ptr<PartDelegateBase> delegate) :
                m_delegate{std::move(delegate)} {
                    ensure(m_delegate != nullptr, "delegate is null");
                }
            public:
                virtual ~PartBase() = default;
            };

            class AddClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, protected PartBase {
            private:
                bool m_secondPointSet;
            public:
                explicit AddClipPointPart(std::unique_ptr<PartDelegateBase> delegate) :
                PartBase{std::move(delegate)},
                m_secondPointSet{false} {}
            private:
                Tool* doGetTool() override {
                    return m_delegate->tool();
                }

                const Tool* doGetTool() const override {
                    return m_delegate->tool();
                }

                bool doMouseClick(const InputState& inputState) override {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.modifierKeysPressed(ModifierKeys::MKNone)) {
                        return false;
                    }

                    auto temp = vm::vec3{};
                    return m_delegate->addClipPoint(inputState, temp);
                }

                bool doMouseDoubleClick(const InputState& inputState) override {
                    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                        !inputState.modifierKeysPressed(ModifierKeys::MKNone)) {
                        return false;
                    }
                    return m_delegate->setClipFace(inputState);
                }

                DragInfo doStartDrag(const InputState& inputState) override {
                    if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                        inputState.modifierKeys() != ModifierKeys::MKNone) {
                        return DragInfo();
                        }

                    auto initialPoint = vm::vec3{};
                    if (!m_delegate->addClipPoint(inputState, initialPoint)) {
                        return DragInfo();
                    }

                    m_secondPointSet = false;
                    DragRestricter* restricter = m_delegate->createDragRestricter(inputState, initialPoint);
                    DragSnapper* snapper = m_delegate->createDragSnapper(inputState);
                    return DragInfo(restricter, snapper, initialPoint);
                }

                DragResult doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                    if (!m_secondPointSet) {
                        auto position = vm::vec3{};
                        if (m_delegate->addClipPoint(inputState, position)) {
                            m_delegate->tool()->beginDragLastPoint();
                            m_secondPointSet = true;
                            return DR_Continue;
                        }
                    } else {
                        if (m_delegate->tool()->dragPoint(nextHandlePosition, m_delegate->getHelpVectors(inputState, nextHandlePosition))) {
                            return DR_Continue;
                        }
                    }
                    return DR_Deny;
                }

                void doEndDrag(const InputState&) override {
                    if (m_secondPointSet) {
                        m_delegate->tool()->endDragPoint();
                    }
                }

                void doCancelDrag() override {
                    if (m_secondPointSet) {
                        m_delegate->tool()->cancelDragPoint();
                        m_delegate->tool()->removeLastPoint();
                    }
                    m_delegate->tool()->removeLastPoint();
                }

                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                    m_delegate->renderFeedback(inputState, renderContext, renderBatch);
                }

                bool doCancel() override {
                    return false;
                }
            };

            class MoveClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy>, protected PartBase {
            public:
                explicit MoveClipPointPart(std::unique_ptr<PartDelegateBase> delegate) :
                PartBase{std::move(delegate)} {}
            private:
                Tool* doGetTool() override {
                    return m_delegate->tool();
                }

                const Tool* doGetTool() const override {
                    return m_delegate->tool();
                }

                DragInfo doStartDrag(const InputState& inputState) override {
                    if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                        inputState.modifierKeys() != ModifierKeys::MKNone) {
                        return DragInfo();
                    }

                    auto initialPoint = vm::vec3{};
                    if (!m_delegate->tool()->beginDragPoint(inputState.pickResult(), initialPoint)) {
                        return DragInfo();
                    }

                    DragRestricter* restricter = m_delegate->createDragRestricter(inputState, initialPoint);
                    DragSnapper* snapper = m_delegate->createDragSnapper(inputState);
                    return DragInfo(restricter, snapper, initialPoint);
                }

                DragResult doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) override {
                    if (m_delegate->tool()->dragPoint(nextHandlePosition, m_delegate->getHelpVectors(inputState, nextHandlePosition))) {
                        return DR_Continue;
                    } else {
                        return DR_Deny;
                    }
                }

                void doEndDrag(const InputState&) override {
                    m_delegate->tool()->endDragPoint();
                }
        
                void doCancelDrag() override {
                    m_delegate->tool()->cancelDragPoint();
                }

                bool doCancel() override {
                    return false;
                }
            };
        }

        ClipToolControllerBase::ClipToolControllerBase(ClipTool* tool) :
        m_tool{tool} {}

        ClipToolControllerBase::~ClipToolControllerBase() = default;

        Tool* ClipToolControllerBase::doGetTool() {
            return m_tool;
        }

        const Tool* ClipToolControllerBase::doGetTool() const {
            return m_tool;
        }

        void ClipToolControllerBase::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }

        void ClipToolControllerBase::doSetRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const {
            if (m_tool->hasBrushes()) {
                renderContext.setHideSelection();
                renderContext.setForceHideSelectionGuide();
            }
        }

        void ClipToolControllerBase::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch, inputState.pickResult());
            ToolControllerGroup::doRender(inputState, renderContext, renderBatch);
        }

        bool ClipToolControllerBase::doCancel() {
            if (m_tool->removeLastPoint()) {
                if (!m_tool->hasPoints()) {
                    m_tool->reset();
                }
                return true;
            }
            return false;
        }

        ClipToolController2D::ClipToolController2D(ClipTool* tool) :
        ClipToolControllerBase{tool} {
            addController(std::make_unique<AddClipPointPart>(std::make_unique<PartDelegate2D>(tool)));
            addController(std::make_unique<MoveClipPointPart>(std::make_unique<PartDelegate2D>(tool)));
        }

        ClipToolController3D::ClipToolController3D(ClipTool* tool) :
        ClipToolControllerBase{tool} {
            addController(std::make_unique<AddClipPointPart>(std::make_unique<PartDelegate3D>(tool)));
            addController(std::make_unique<MoveClipPointPart>(std::make_unique<PartDelegate3D>(tool)));
        }
    }
}
