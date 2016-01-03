/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "VertexToolAdapter.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Lasso.h"
#include "View/VertexTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexToolAdapter::MaxVertexDistance = 0.25;

        VertexToolAdapter::VertexToolAdapter(VertexTool* tool, MoveToolHelper* helper) :
        MoveToolAdapter(helper),
        m_tool(tool),
        m_lasso(NULL) {
            assert(m_tool != NULL);
        }

        VertexToolAdapter::~VertexToolAdapter() {
            delete m_lasso;
        }

        Tool* VertexToolAdapter::doGetTool() {
            return m_tool;
        }

        bool VertexToolAdapter::doMouseClick(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;
            
            const Model::Hit::List hits = firstHits(inputState.pickResult());
            if (hits.empty())
                return m_tool->deselectAll();
            else if (inputState.modifierKeysPressed(ModifierKeys::MKShift))
                return m_tool->mergeVertices(hits.front());
            else
                return m_tool->select(hits, inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd));
        }
        
        bool VertexToolAdapter::doMouseDoubleClick(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;
            
            const Model::Hit::List hits = firstHits(inputState.pickResult());
            if (hits.empty())
                return false;
            
            const Model::Hit& hit = hits.front();
            return m_tool->handleDoubleClicked(hit);
        }

        bool VertexToolAdapter::dismissClick(const InputState& inputState) const {
            return !(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                     (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd)));
        }

        typedef MoveToolAdapter<PickingPolicy, MousePolicy, RenderPolicy> MoveAdapter;
        
        bool VertexToolAdapter::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (MoveAdapter::doStartPlaneDrag(inputState, plane, initialPoint))
                return true;
            return startLasso(inputState, plane, initialPoint);
        }
        
        bool VertexToolAdapter::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            if (m_lasso == NULL)
                return MoveAdapter::doPlaneDrag(inputState, lastPoint, curPoint, refPoint);
            return updateLasso(inputState, lastPoint, curPoint, refPoint);
        }
        
        void VertexToolAdapter::doEndPlaneDrag(const InputState& inputState) {
            if (m_lasso == NULL)
                MoveAdapter::doEndPlaneDrag(inputState);
            else
                endLasso(inputState);
        }
        
        void VertexToolAdapter::doCancelPlaneDrag() {
            if (m_lasso == NULL)
                MoveAdapter::doCancelPlaneDrag();
            else
                cancelLasso();
        }
        
        bool VertexToolAdapter::startLasso(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                !inputState.checkModifierKeys(MK_DontCare, MK_No, MK_No))
                return false;

            const Renderer::Camera& camera = inputState.camera();
            const FloatType distance = 64.0f;
            plane = orthogonalDragPlane(camera.defaultPoint(distance), camera.direction());
            initialPoint = inputState.pickRay().pointAtDistance(plane.intersectWithRay(inputState.pickRay()));
            
            m_lasso = new Lasso(camera, distance, initialPoint);
            return true;
        }
        
        bool VertexToolAdapter::updateLasso(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_lasso != NULL);
            m_lasso->setPoint(curPoint);
            return true;
        }
        
        void VertexToolAdapter::endLasso(const InputState& inputState) {
            assert(m_lasso != NULL);
            m_tool->select(*m_lasso, inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd));
            delete m_lasso;
            m_lasso = NULL;
        }
        
        void VertexToolAdapter::cancelLasso() {
            assert(m_lasso != NULL);
            delete m_lasso;
            m_lasso = NULL;
        }

        bool VertexToolAdapter::doHandleMove(const InputState& inputState) const {
            if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                  (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift))))
                return false;
            
            const Model::Hit& hit = firstHit(inputState.pickResult());
            return hit.isMatch();
        }
        
        Vec3 VertexToolAdapter::doGetMoveOrigin(const InputState& inputState) const {
            const Model::Hit& hit = firstHit(inputState.pickResult());
            assert(hit.isMatch());
            return hit.hitPoint();
        }
        
        bool VertexToolAdapter::doStartMove(const InputState& inputState) {
            const Model::Hit& hit = firstHit(inputState.pickResult());
            if (!hit.isMatch())
                return false;
            return m_tool->beginMove(hit);
        }
        
        Vec3 VertexToolAdapter::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            const Model::Hit& hit = firstHit(inputState.pickResult());
            const bool shiftDown = inputState.modifierKeysDown(ModifierKeys::MKShift);
            return m_tool->snapMoveDelta(delta, hit, shiftDown);
        }
        
        MoveResult VertexToolAdapter::doMove(const InputState& inputState, const Vec3& delta) {
            return m_tool->move(delta);
        }
        
        void VertexToolAdapter::doEndMove(const InputState& inputState) {
            m_tool->endMove();
        }
        
        void VertexToolAdapter::doCancelMove() {
            m_tool->cancelMove();
        }

        void VertexToolAdapter::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
        }
        
        void VertexToolAdapter::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->renderHandles(renderContext, renderBatch);
            
            if (m_lasso != NULL) {
                m_lasso->render(renderContext, renderBatch);
            } else if (dragging()) {
                m_tool->renderHighlight(renderContext, renderBatch);
                m_tool->renderGuide(renderContext, renderBatch);
                renderMoveIndicator(inputState, renderContext, renderBatch);
            } else {
                const Model::Hit& hit = firstHit(inputState.pickResult());
                if (hit.isMatch()) {
                    const Vec3 position = hit.target<Vec3>();
                    m_tool->renderHighlight(renderContext, renderBatch, position);
                    if (m_tool->handleSelected(position)) {
                        renderMoveIndicator(inputState, renderContext, renderBatch);
                    } else {
                        if (hit.type() == VertexHandleManager::EdgeHandleHit)
                            m_tool->renderEdgeHighlight(renderContext, renderBatch, position);
                        else if (hit.type() == VertexHandleManager::FaceHandleHit)
                            m_tool->renderFaceHighlight(renderContext, renderBatch, position);
                    }
                    if (inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                        m_tool->renderGuide(renderContext, renderBatch, position);
                }
            }
        }

        bool VertexToolAdapter::doCancel() {
            return m_tool->cancel();
        }

        const Model::Hit& VertexToolAdapter::firstHit(const Model::PickResult& pickResult) const {
            static const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            return pickResult.query().type(any).occluded().first();
        }
        
        Model::Hit::List VertexToolAdapter::firstHits(const Model::PickResult& pickResult) const {
            Model::Hit::List result;
            Model::BrushSet brushes;
            
            static const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            const Model::Hit& first = pickResult.query().type(any).occluded().first();
            if (first.isMatch()) {
                const Vec3 firstHitPosition = first.target<Vec3>();

                const Model::Hit::List matches = pickResult.query().type(any).all();
                Model::Hit::List::const_iterator hIt, hEnd;
                for (hIt = matches.begin(), hEnd = matches.end(); hIt != hEnd; ++hIt) {
                    const Model::Hit& hit = *hIt;
                    const Vec3 hitPosition = hit.target<Vec3>();
                    
                    if (hitPosition.distanceTo(firstHitPosition) < MaxVertexDistance) {
                        const bool newBrush = m_tool->handleBrushes(hitPosition, brushes);
                        if (newBrush)
                            result.push_back(hit);
                    }
                }
            }
            
            return result;
        }

        VertexToolAdapter2D::VertexToolAdapter2D(VertexTool* tool) :
        VertexToolAdapter(tool, new MoveToolHelper2D(this, this)) {}
        
        void VertexToolAdapter2D::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }

        VertexToolAdapter3D::VertexToolAdapter3D(VertexTool* tool, MovementRestriction& movementRestriction) :
        VertexToolAdapter(tool, new MoveToolHelper3D(this, this, movementRestriction)) {}
        
        void VertexToolAdapter3D::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
        }
    }
}
