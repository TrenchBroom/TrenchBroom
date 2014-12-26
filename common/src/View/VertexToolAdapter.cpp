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

#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/VertexTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexToolAdapter::MaxVertexDistance = 0.25;

        VertexToolAdapter::VertexToolAdapter(VertexTool* tool, MoveToolHelper* helper) :
        MoveToolAdapter(helper),
        m_tool(tool) {
            assert(m_tool != NULL);
        }

        VertexToolAdapter::~VertexToolAdapter() {}

        bool VertexToolAdapter::doMouseDown(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;
            
            const Hits::List hits = firstHits(inputState.hits());
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
            
            const Hits::List hits = firstHits(inputState.hits());
            if (hits.empty())
                return false;
            
            const Hit& hit = hits.front();
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

        bool VertexToolAdapter::doHandleMove(const InputState& inputState) const {
            if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                  (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift))))
                return false;
            
            const Hit& hit = firstHit(inputState.hits());
            return hit.isMatch();
        }
        
        Vec3 VertexToolAdapter::doGetMoveOrigin(const InputState& inputState) const {
            const Hit& hit = firstHit(inputState.hits());
            assert(hit.isMatch());
            return hit.hitPoint();
        }
        
        bool VertexToolAdapter::doStartMove(const InputState& inputState) {
            const Hit& hit = firstHit(inputState.hits());
            assert(hit.isMatch());
            return m_tool->beginMove(hit);
        }
        
        Vec3 VertexToolAdapter::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            const Hit& hit = firstHit(inputState.hits());
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
            
            if (dragging()) {
                m_tool->renderHighlight(renderContext, renderBatch);
                renderMoveIndicator(inputState, renderContext, renderBatch);
            } else {
                const Hit& hit = firstHit(inputState.hits());
                if (hit.isMatch()) {
                    const Vec3 position = hit.target<Vec3>();
                    m_tool->renderHighlight(renderContext, renderBatch, position);
                    if (m_tool->handleSelected(position))
                        renderMoveIndicator(inputState, renderContext, renderBatch);
                }
            }
        }

        const Hit& VertexToolAdapter::firstHit(const Hits& hits) const {
            static const Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            return hits.findFirst(any, true);
        }
        
        Hits::List VertexToolAdapter::firstHits(const Hits& hits) const {
            Hits::List result;
            Model::BrushSet brushes;
            
            static const Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            const Hit& first = hits.findFirst(any, true);
            if (first.isMatch()) {
                const Vec3 firstHitPosition = first.target<Vec3>();
                
                const Hits::List matches = hits.filter(any);
                Hits::List::const_iterator hIt, hEnd;
                for (hIt = matches.begin(), hEnd = matches.end(); hIt != hEnd; ++hIt) {
                    const Hit& hit = *hIt;
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
        VertexToolAdapter(tool, new MoveToolHelper2D(tool)) {}
        
        void VertexToolAdapter2D::doPick(const InputState& inputState, Hits& hits) {
            m_tool->pick(inputState.pickRay(), hits);
        }

        VertexToolAdapter3D::VertexToolAdapter3D(VertexTool* tool, MovementRestriction& movementRestriction) :
        VertexToolAdapter(tool, new MoveToolHelper3D(tool, movementRestriction)) {}
        
        void VertexToolAdapter3D::doPick(const InputState& inputState, Hits& hits) {
            m_tool->pick(inputState.pickRay(), hits);
        }
    }
}
