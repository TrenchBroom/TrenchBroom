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

#include "VertexTool.h"

#include "SetBool.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/HitAdapter.h"
#include "Model/Picker.h"
#include "Model/Object.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexTool::MaxVertexDistance = 0.25;
        const FloatType VertexTool::MaxVertexError = 0.01;
        
        VertexTool::VertexTool(MapDocumentWPtr document, MovementRestriction& movementRestriction, const Renderer::FontDescriptor& fontDescriptor) :
        MoveTool(document, movementRestriction),
        m_handleManager(document, fontDescriptor),
        m_mode(Mode_Move),
        m_changeCount(0),
        m_ignoreObjectChangeNotifications(false) {}

        bool VertexTool::hasSelectedHandles() const {
            return (m_handleManager.selectedVertexCount() > 0 ||
                    m_handleManager.selectedEdgeCount() > 0 ||
                    m_handleManager.selectedFaceCount() > 0);
        }

        void VertexTool::moveVerticesAndRebuildBrushGeometry(const Vec3& delta) {
            moveVertices(delta);
            m_mode = Mode_Move;
        }

        bool VertexTool::canSnapVertices() const {
            return m_mode == Mode_Move && document()->canSnapVertices(m_handleManager);
        }
        
        void VertexTool::snapVertices(const size_t snapTo) {
            assert(canSnapVertices());
            document()->snapVertices(m_handleManager, snapTo);
        }

        MoveResult VertexTool::moveVertices(const Vec3& delta) {
            if (m_mode == Mode_Move || m_mode == Mode_Snap) {
                assert((m_handleManager.selectedVertexCount() > 0) ^
                       (m_handleManager.selectedEdgeCount() > 0) ^
                       (m_handleManager.selectedFaceCount() > 0));
                
                if (m_handleManager.selectedVertexCount() > 0)
                    return doMoveVertices(delta);
                else if (m_handleManager.selectedEdgeCount() > 0)
                    return doMoveEdges(delta);
                else if (m_handleManager.selectedFaceCount() > 0)
                    return doMoveFaces(delta);
            } else {
                assert((m_handleManager.selectedVertexCount() == 0) &&
                       ((m_handleManager.selectedEdgeCount() == 1) ^
                        (m_handleManager.selectedFaceCount() == 1)));
                
                if (m_handleManager.selectedEdgeCount() > 0) {
                    return doSplitEdges(delta);
                } else if (m_handleManager.selectedFaceCount() > 0) {
                    return doSplitFaces(delta);
                }
            }
            return MoveResult_Continue;
        }
        
        MoveResult VertexTool::doMoveVertices(const Vec3& delta) {
            const MapDocument::MoveVerticesResult result = document()->moveVertices(m_handleManager, delta);
            if (result.success) {
                if (!result.hasRemainingVertices)
                    return MoveResult_Conclude;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doMoveEdges(const Vec3& delta) {
            if (document()->moveEdges(m_handleManager, delta)) {
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doMoveFaces(const Vec3& delta) {
            if (document()->moveFaces(m_handleManager, delta)) {
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doSplitEdges(const Vec3& delta) {
            if (document()->splitEdges(m_handleManager, delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doSplitFaces(const Vec3& delta) {
            if (document()->splitFaces(m_handleManager, delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }

        bool VertexTool::doHandleMove(const InputState& inputState) const {
            if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                  (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift))))
                return false;
            
            const Hit& hit = firstHit(inputState.hits());
            return hit.isMatch();
        }
        
        Vec3 VertexTool::doGetMoveOrigin(const InputState& inputState) const {
            const Hit& hit = firstHit(inputState.hits());
            assert(hit.isMatch());
            return hit.hitPoint();
        }
        
        String VertexTool::doGetActionName(const InputState& inputState) const {
            if (m_mode == Mode_Move || m_mode == Mode_Snap) {
                assert((m_handleManager.selectedVertexHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedEdgeHandles().empty() ? 0 : 1) +
                       (m_handleManager.selectedFaceHandles().empty() ? 0 : 1) == 1);
                
                if (!m_handleManager.selectedVertexHandles().empty())
                    return m_handleManager.selectedVertexHandles().size() == 1 ? "Move Vertex" : "Move Vertices";
                if (!m_handleManager.selectedEdgeHandles().empty())
                    return m_handleManager.selectedEdgeHandles().size() == 1 ? "Move Edge" : "Move Edges";
                return m_handleManager.selectedFaceHandles().size() == 1 ? "Move Face" : "Move Faces";
            }
            
            assert(m_handleManager.selectedVertexHandles().size() == 0 &&
                   ((m_handleManager.selectedEdgeHandles().size() == 1) ^
                    (m_handleManager.selectedFaceHandles().size() == 1))
                   );
            
            if (!m_handleManager.selectedEdgeHandles().empty())
                return "Split Edge";
            return "Split Face";
        }
        
        bool VertexTool::doStartMove(const InputState& inputState) {
            const Hit& hit = firstHit(inputState.hits());
            assert(hit.isMatch());
            m_dragHandlePosition = hit.target<Vec3>();
            controller()->beginUndoableGroup();
            return true;
        }
        
        Vec3 VertexTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            if (m_mode == Mode_Snap) {
                const Hit& hit = firstHit(inputState.hits());
                if (hit.isMatch() && !m_handleManager.isVertexHandleSelected(hit.target<Vec3>()))
                    return hit.target<Vec3>() - m_dragHandlePosition;
                return Vec3::Null;
            }

            const Grid& grid = document()->grid();
            if (inputState.modifierKeysDown(ModifierKeys::MKShift))
                return grid.snap(delta);
            
            return grid.snap(m_dragHandlePosition + delta) - m_dragHandlePosition;
        }
        
        MoveResult VertexTool::doMove(const Vec3& delta) {
            return moveVertices(delta);;
        }
        
        void VertexTool::doEndMove(const InputState& inputState) {
            controller()->closeGroup();
            m_mode = Mode_Move;
        }
        
        bool VertexTool::initiallyActive() const {
            return false;
        }
        
        bool VertexTool::doActivate(const InputState& inputState) {
            m_mode = Mode_Move;
            m_handleManager.clear();
            m_handleManager.addBrushes(document()->selectedBrushes());
            m_changeCount = 0;
            
            bindObservers();
            return true;
        }
        
        bool VertexTool::doDeactivate(const InputState& inputState) {
            unbindObservers();
            m_handleManager.clear();
            
            /*
            if (m_changeCount > 0) {
                RebuildBrushGeometryCommand* command = RebuildBrushGeometryCommand::rebuildGeometry(document(), document().editStateManager().selectedBrushes(), m_changeCount);
                submitCommand(command);
             }
             */
            return true;
        }
        
        bool VertexTool::doCancel(const InputState& inputState) {
            if (m_handleManager.hasSelectedHandles()) {
                m_handleManager.deselectAllHandles();
                return true;
            }
            
            return false;
        }

        void VertexTool::doPick(const InputState& inputState, Hits& hits) {
            m_handleManager.pick(inputState.pickRay(), hits, m_mode == Mode_Split);
        }
        
        bool VertexTool::doMouseDown(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;

            const Hits::List hits = firstHits(inputState.hits());
            if (hits.empty())
                return false;
            
            const Hit& hit = hits.front();
            if (hit.type() == VertexHandleManager::VertexHandleHit)
                vertexHandleClicked(inputState, hits);
            else if (hit.type() == VertexHandleManager::EdgeHandleHit)
                edgeHandleClicked(inputState, hits);
            else
                faceHandleClicked(inputState, hits);
            return true;
        }
        
        bool VertexTool::doMouseUp(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;
            
            const Hits::List hits = firstHits(inputState.hits());
            if (!hits.empty())
                return true;
            
            if (m_handleManager.selectedVertexHandles().empty() &&
                m_handleManager.selectedEdgeHandles().empty() &&
                m_handleManager.selectedFaceHandles().empty())
                return false;
            
            m_handleManager.deselectAllHandles();
            m_mode = Mode_Move;
            return true;
        }

        bool VertexTool::doMouseDoubleClick(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;

            const Hits::List hits = firstHits(inputState.hits());
            if (hits.empty())
                return false;
            
            const Hit& hit = hits.front();
            if (hit.type() == VertexHandleManager::VertexHandleHit) {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectVertexHandle(hit.target<Vec3>());
                m_mode = Mode_Snap;
            } else if (hit.type() == VertexHandleManager::EdgeHandleHit) {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectEdgeHandle(hit.target<Vec3>());
                m_mode = Mode_Split;
            } else {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectFaceHandle(hit.target<Vec3>());
                m_mode = Mode_Split;
            }
            
            return true;
        }

        bool VertexTool::dismissClick(const InputState& inputState) const {
            return !(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                     (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift) ||
                      inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd)));
        }
        
        void VertexTool::vertexHandleClicked(const InputState& inputState, const Hits::List& hits) {
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift) &&
                m_handleManager.selectedVertexCount() == 1) {
                const Hit& hit = hits.front();
                if (hit.type() == VertexHandleManager::VertexHandleHit) {
                    const Vec3 targetPosition = hit.target<Vec3>();
                    const Vec3 originalPosition = m_handleManager.selectedVertexHandlePositions().front();
                    const Vec3 delta = targetPosition - originalPosition;
                    moveVerticesAndRebuildBrushGeometry(delta);
                }
                return;
            }
            
            m_handleManager.deselectAllEdgeHandles();
            m_handleManager.deselectAllFaceHandles();
            
            size_t selected = 0;
            Hits::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isVertexHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectVertexHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectVertexHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::edgeHandleClicked(const InputState& inputState, const Hits::List& hits) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllFaceHandles();
            
            size_t selected = 0;
            Hits::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isEdgeHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectEdgeHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectEdgeHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::faceHandleClicked(const InputState& inputState, const Hits::List& hits) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllEdgeHandles();
            
            size_t selected = 0;
            Hits::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isFaceHandleSelected(position))
                    selected++;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectFaceHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectFaceHandle(position);
                    }
                }
            }
        }

        void VertexTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.setForceHideSelectionGuide();
        }

        void VertexTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            m_handleManager.render(renderContext, m_mode == Mode_Split);

            if (dragging()) {
                m_handleManager.renderHighlight(renderContext, m_dragHandlePosition);
                renderMoveIndicator(inputState, renderContext);
            } else {
                const Hit& hit = firstHit(inputState.hits());
                if (hit.isMatch()) {
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.renderHighlight(renderContext, position);
                    if (m_handleManager.isHandleSelected(position))
                        renderMoveIndicator(inputState, renderContext);
                }
            }
        }

        void VertexTool::bindObservers() {
            document()->selectionDidChangeNotifier.addObserver(this, &VertexTool::selectionDidChange);
            document()->objectsWillChangeNotifier.addObserver(this, &VertexTool::objectsWillChange);
            document()->objectsDidChangeNotifier.addObserver(this, &VertexTool::objectsDidChange);
            controller()->commandDoNotifier.addObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandDoneNotifier.addObserver(this, &VertexTool::commandDoneOrUndoFailed);
            controller()->commandDoFailedNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoNotifier.addObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandUndoneNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoFailedNotifier.addObserver(this, &VertexTool::commandDoneOrUndoFailed);
        }
        
        void VertexTool::unbindObservers() {
            if (!expired(document())) {
                document()->selectionDidChangeNotifier.removeObserver(this, &VertexTool::selectionDidChange);
                document()->objectsWillChangeNotifier.removeObserver(this, &VertexTool::objectsWillChange);
                document()->objectsDidChangeNotifier.removeObserver(this, &VertexTool::objectsDidChange);
            }
            if (!expired(controller())) {
                controller()->commandDoNotifier.removeObserver(this, &VertexTool::commandDoOrUndo);
                controller()->commandDoneNotifier.removeObserver(this, &VertexTool::commandDoneOrUndoFailed);
                controller()->commandDoFailedNotifier.removeObserver(this, &VertexTool::commandDoFailedOrUndone);
                controller()->commandUndoNotifier.removeObserver(this, &VertexTool::commandDoOrUndo);
                controller()->commandUndoneNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
                controller()->commandUndoFailedNotifier.removeObserver(this, &VertexTool::commandDoneOrUndoFailed);
            }
        }
        
        class AddToHandleManager : public Model::ObjectVisitor {
        private:
            VertexHandleManager& m_handleManager;
        public:
            AddToHandleManager(VertexHandleManager& handleManager) :
            m_handleManager(handleManager) {}
            
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush) {
                m_handleManager.addBrush(brush);
            }
        };
        
        class RemoveFromHandleManager : public Model::ObjectVisitor {
        private:
            VertexHandleManager& m_handleManager;
        public:
            RemoveFromHandleManager(VertexHandleManager& handleManager) :
            m_handleManager(handleManager) {}
            
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush) {
                m_handleManager.removeBrush(brush);
            }
        };

        void VertexTool::selectionDidChange(const Model::SelectionResult& selection) {
            const Model::ObjectSet& selectedObjects = selection.selectedObjects();
            AddToHandleManager addVisitor(m_handleManager);
            Model::Object::accept(selectedObjects.begin(), selectedObjects.end(), addVisitor);
            
            const Model::ObjectSet& deselectedObjects = selection.deselectedObjects();
            RemoveFromHandleManager removeVisitor(m_handleManager);
            Model::Object::accept(deselectedObjects.begin(), deselectedObjects.end(), removeVisitor);
        }

        void VertexTool::objectsWillChange(const Model::ObjectList& objects) {
            if (!m_ignoreObjectChangeNotifications) {
                RemoveFromHandleManager removeVisitor(m_handleManager);
                Model::Object::accept(objects.begin(), objects.end(), removeVisitor);
            }
        }
        
        void VertexTool::objectsDidChange(const Model::ObjectList& objects) {
            if (!m_ignoreObjectChangeNotifications) {
                AddToHandleManager addVisitor(m_handleManager);
                Model::Object::accept(objects.begin(), objects.end(), addVisitor);
            }
        }

        const Hit& VertexTool::firstHit(const Hits& hits) const {
            static const Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            return hits.findFirst(any, true);
        }

        Hits::List VertexTool::firstHits(const Hits& hits) const {
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
                        bool newBrush = true;
                        const Model::BrushList& handleBrushes = m_handleManager.brushes(hitPosition);
                        Model::BrushList::const_iterator bIt, bEnd;
                        for (bIt = handleBrushes.begin(), bEnd = handleBrushes.end(); bIt != bEnd; ++bIt) {
                            Model::Brush* brush = *bIt;
                            newBrush &= brushes.insert(brush).second;
                        }
                        
                        if (newBrush)
                            result.push_back(hit);
                    }
                }
            }
            
            return result;
        }
    }
}
