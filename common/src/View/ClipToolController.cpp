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
        ClipToolController::PartDelegateBase::PartDelegateBase(ClipTool* tool) :
        m_tool(tool) {
            ensure(m_tool != nullptr, "tool is null");
        }

        ClipToolController::PartDelegateBase::~PartDelegateBase() = default;

        ClipTool* ClipToolController::PartDelegateBase::tool() const {
            return m_tool;
        }

        bool ClipToolController::PartDelegateBase::addClipPoint(const InputState& inputState, vm::vec3& position) {
            if (!doGetNewClipPointPosition(inputState, position))
                return false;
            if (!m_tool->canAddPoint(position))
                return false;
            m_tool->addPoint(position, getHelpVectors(inputState, position));
            return true;
        }

        bool ClipToolController::PartDelegateBase::setClipFace(const TrenchBroom::View::InputState &inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                m_tool->setFace(*faceHandle);
                return true;
            } else {
                return false;
            }
        }

        void ClipToolController::PartDelegateBase::renderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (inputState.anyToolDragging())
                return;

            vm::vec3 position;
            if (!doGetNewClipPointPosition(inputState, position))
                return;

            if (!m_tool->canAddPoint(position))
                return;

            m_tool->renderFeedback(renderContext, renderBatch, position);
        }

        ClipToolController::PartBase::PartBase(PartDelegateBase* delegate) :
        m_delegate(delegate) {
            ensure(m_delegate != nullptr, "delegate is null");
        }

        ClipToolController::PartBase::~PartBase() {
            delete m_delegate;
        }

        ClipToolController::AddClipPointPart::AddClipPointPart(PartDelegateBase* delegate) :
        PartBase(delegate),
        m_secondPointSet(false) {}

        Tool* ClipToolController::AddClipPointPart::doGetTool() {
            return m_delegate->tool();
        }

        const Tool* ClipToolController::AddClipPointPart::doGetTool() const {
            return m_delegate->tool();
        }

        bool ClipToolController::AddClipPointPart::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            vm::vec3 temp;
            return m_delegate->addClipPoint(inputState, temp);
        }

        bool ClipToolController::AddClipPointPart::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            return m_delegate->setClipFace(inputState);
        }

        RestrictedDragPolicy::DragInfo ClipToolController::AddClipPointPart::doStartDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return DragInfo();

            vm::vec3 initialPoint;
            if (!m_delegate->addClipPoint(inputState, initialPoint))
                return DragInfo();

            m_secondPointSet = false;
            DragRestricter* restricter = m_delegate->createDragRestricter(inputState, initialPoint);
            DragSnapper* snapper = m_delegate->createDragSnapper(inputState);
            return DragInfo(restricter, snapper, initialPoint);
        }

        RestrictedDragPolicy::DragResult ClipToolController::AddClipPointPart::doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) {

            if (!m_secondPointSet) {
                vm::vec3 position;
                if (m_delegate->addClipPoint(inputState, position)) {
                    m_delegate->tool()->beginDragLastPoint();
                    m_secondPointSet = true;
                    return DR_Continue;
                }
            } else {
                if (m_delegate->tool()->dragPoint(nextHandlePosition, m_delegate->getHelpVectors(inputState, nextHandlePosition)))
                    return DR_Continue;
            }
            return DR_Deny;
        }

        void ClipToolController::AddClipPointPart::doEndDrag(const InputState&) {
            if (m_secondPointSet)
                m_delegate->tool()->endDragPoint();
        }

        void ClipToolController::AddClipPointPart::doCancelDrag() {
            if (m_secondPointSet) {
                m_delegate->tool()->cancelDragPoint();
                m_delegate->tool()->removeLastPoint();
            }
            m_delegate->tool()->removeLastPoint();
        }

        void ClipToolController::AddClipPointPart::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_delegate->renderFeedback(inputState, renderContext, renderBatch);
        }

        bool ClipToolController::AddClipPointPart::doCancel() {
            return false;
        }

        ClipToolController::MoveClipPointPart::MoveClipPointPart(PartDelegateBase* delegate) :
        PartBase(delegate) {}

        Tool* ClipToolController::MoveClipPointPart::doGetTool() {
            return m_delegate->tool();
        }

        const Tool* ClipToolController::MoveClipPointPart::doGetTool() const {
            return m_delegate->tool();
        }

        RestrictedDragPolicy::DragInfo ClipToolController::MoveClipPointPart::doStartDrag(const InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return DragInfo();

            vm::vec3 initialPoint;
            if (!m_delegate->tool()->beginDragPoint(inputState.pickResult(), initialPoint))
                return DragInfo();

            DragRestricter* restricter = m_delegate->createDragRestricter(inputState, initialPoint);
            DragSnapper* snapper = m_delegate->createDragSnapper(inputState);
            return DragInfo(restricter, snapper, initialPoint);
        }

        RestrictedDragPolicy::DragResult ClipToolController::MoveClipPointPart::doDrag(const InputState& inputState, const vm::vec3& /* lastHandlePosition */, const vm::vec3& nextHandlePosition) {
            if (m_delegate->tool()->dragPoint(nextHandlePosition, m_delegate->getHelpVectors(inputState, nextHandlePosition))) {
                return DR_Continue;
            } else {
                return DR_Deny;
            }
        }

        void ClipToolController::MoveClipPointPart::doEndDrag(const InputState&) {
            m_delegate->tool()->endDragPoint();
        }

        void ClipToolController::MoveClipPointPart::doCancelDrag() {
            m_delegate->tool()->cancelDragPoint();
        }

        bool ClipToolController::MoveClipPointPart::doCancel() {
            return false;
        }

        ClipToolController::ClipToolController(ClipTool* tool) :
        m_tool(tool) {}

        ClipToolController::~ClipToolController() = default;

        Tool* ClipToolController::doGetTool() {
            return m_tool;
        }

        const Tool* ClipToolController::doGetTool() const {
            return m_tool;
        }

        void ClipToolController::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }

        void ClipToolController::doSetRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const {
            if (m_tool->hasBrushes()) {
                renderContext.setHideSelection();
                renderContext.setForceHideSelectionGuide();
            }
        }

        void ClipToolController::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch, inputState.pickResult());
            ToolControllerGroup::doRender(inputState, renderContext, renderBatch);
        }

        bool ClipToolController::doCancel() {
            if (m_tool->removeLastPoint()) {
                if (!m_tool->hasPoints())
                    m_tool->reset();
                return true;
            }
            return false;
        }

        class ClipToolController2D::PartDelegate : public PartDelegateBase {
        public:
            explicit PartDelegate(ClipTool* tool) :
            PartDelegateBase(tool) {}

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

        ClipToolController2D::ClipToolController2D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(std::make_unique<AddClipPointPart>(new PartDelegate(tool)));
            addController(std::make_unique<MoveClipPointPart>(new PartDelegate(tool)));
        }

        std::vector<vm::vec3> ClipToolController3D::selectHelpVectors(const Model::BrushNode* brushNode, const Model::BrushFace& face, const vm::vec3& hitPoint) {
            std::vector<vm::vec3> result;
            for (const Model::BrushFace* incidentFace : selectIncidentFaces(brushNode, face, hitPoint)) {
                const vm::vec3& normal = incidentFace->boundary().normal;
                result.push_back(vm::get_abs_max_component_axis(normal));
            }

            return kdl::vec_sort_and_remove_duplicates(std::move(result));
        }

        std::vector<const Model::BrushFace*> ClipToolController3D::selectIncidentFaces(const Model::BrushNode* brushNode, const Model::BrushFace& face, const vm::vec3& hitPoint) {
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

        class ClipToolController3D::PartDelegate : public PartDelegateBase {
        public:
            explicit PartDelegate(ClipTool* tool) :
            PartDelegateBase(tool) {}

            DragRestricter* createDragRestricter(const InputState&, const vm::vec3& /* initialPoint */) const override {
                SurfaceDragRestricter* restricter = new SurfaceDragRestricter();
                restricter->setPickable(true);
                restricter->setType(Model::BrushNode::BrushHitType);
                restricter->setOccluded(Model::HitType::AnyType);
                return restricter;
            }

            class ClipPointSnapper : public SurfaceDragSnapper {
            public:
                explicit ClipPointSnapper(const Grid& grid) :
                SurfaceDragSnapper(grid) {}
            private:
                vm::plane3 doGetPlane(const InputState&, const Model::Hit& hit) const override {
                    const auto faceHandle = Model::hitToFaceHandle(hit);
                    ensure(faceHandle, "invalid hit type");
                    return faceHandle->face().boundary();
                }
            };

            DragSnapper* createDragSnapper(const InputState&) const override {
                SurfaceDragSnapper* snapper = new ClipPointSnapper(m_tool->grid());
                snapper->setPickable(true);
                snapper->setType(Model::BrushNode::BrushHitType);
                snapper->setOccluded(Model::HitType::AnyType);
                return snapper;
            }

            std::vector<vm::vec3> getHelpVectors(const InputState& inputState, const vm::vec3& clipPoint) const override {
                auto hit = inputState.pickResult().query().selected().type(Model::BrushNode::BrushHitType).occluded().first();
                if (!hit.isMatch()) {
                    hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
                }
                const auto faceHandle = Model::hitToFaceHandle(hit);
                ensure(faceHandle, "hit is not a match");

                return selectHelpVectors(faceHandle->node(), faceHandle->face(), clipPoint);
            }

            bool doGetNewClipPointPosition(const InputState& inputState, vm::vec3& position) const override {
                const auto hit = inputState.pickResult().query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
                if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                    const Grid& grid = m_tool->grid();
                    position = grid.snap(hit.hitPoint(), faceHandle->face().boundary());
                    return true;
                } else {
                    return false;
                }
            }
        };

        ClipToolController3D::ClipToolController3D(ClipTool* tool) :
        ClipToolController(tool) {
            addController(std::make_unique<AddClipPointPart>(new PartDelegate(tool)));
            addController(std::make_unique<MoveClipPointPart>(new PartDelegate(tool)));
        }
    }
}
