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
#include "Controller/BrushVertexHandleCommand.h"
#include "Controller/MoveBrushEdgesCommand.h"
#include "Controller/MoveBrushFacesCommand.h"
#include "Controller/MoveBrushVerticesCommand.h"
#include "Controller/RebuildBrushGeometryCommand.h"
#include "Controller/SnapBrushVerticesCommand.h"
#include "Controller/SplitBrushEdgesCommand.h"
#include "Controller/SplitBrushFacesCommand.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Picker.h"
#include "Model/Object.h"
#include "Model/SelectionResult.h"
#include "Renderer/RenderContext.h"
#include "View/ControllerFacade.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const FloatType VertexTool::MaxVertexDistance = 0.25;
        
        VertexTool::VertexTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction, Renderer::TextureFont& font) :
        MoveTool(next, document, controller, movementRestriction),
        m_handleManager(font),
        m_mode(VMMove),
        m_changeCount(0),
        m_ignoreObjectChangeNotifications(false) {}

        bool VertexTool::hasSelectedHandles() const {
            return (m_handleManager.selectedVertexCount() > 0 ||
                    m_handleManager.selectedEdgeCount() > 0 ||
                    m_handleManager.selectedFaceCount() > 0);
        }

        void VertexTool::moveVerticesAndRebuildBrushGeometry(const Vec3& delta) {
            controller()->beginUndoableGroup();
            moveVertices(delta);
            controller()->rebuildBrushGeometry(document()->selectedBrushes());
            controller()->closeGroup();
            m_mode = VMMove;
        }

        bool VertexTool::canSnapVertices() const {
            return (m_mode == VMMove &&
                    (m_handleManager.selectedVertexCount() > 0 ||
                     m_handleManager.selectedEdgeCount() == 0 ||
                     m_handleManager.selectedFaceCount() == 0));
        }
        
        void VertexTool::snapVertices(const size_t snapTo) {
            assert(canSnapVertices());
            controller()->beginUndoableGroup();
            if (m_handleManager.selectedVertexCount() > 0)
                controller()->snapVertices(m_handleManager.selectedVertexHandles(), snapTo);
            else
                controller()->snapVertices(document()->selectedBrushes(), snapTo);
            controller()->rebuildBrushGeometry(document()->selectedBrushes());
            controller()->closeGroup();
        }

        MoveResult VertexTool::moveVertices(const Vec3& delta) {
            if (m_mode == VMMove || m_mode == VMSnap) {
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
            return Continue;
        }
        
        MoveResult VertexTool::doMoveVertices(const Vec3& delta) {
            using namespace Controller;
            const ControllerFacade::MoveVerticesResult result = controller()->moveVertices(m_handleManager.selectedVertexHandles(), delta);
            if (result.success) {
                if (!result.hasRemainingVertices)
                    return Conclude;
                m_dragHandlePosition += delta;
                return Continue;
            }
            return Deny;
        }
        
        MoveResult VertexTool::doMoveEdges(const Vec3& delta) {
            if (controller()->moveEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_dragHandlePosition += delta;
                return Continue;
            }
            return Deny;
        }
        
        MoveResult VertexTool::doMoveFaces(const Vec3& delta) {
            if (controller()->moveFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_dragHandlePosition += delta;
                return Continue;
            }
            return Deny;
        }
        
        MoveResult VertexTool::doSplitEdges(const Vec3& delta) {
            if (controller()->splitEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_mode = VMMove;
                m_dragHandlePosition += delta;
                return Continue;
            }
            return Deny;
        }
        
        MoveResult VertexTool::doSplitFaces(const Vec3& delta) {
            if (controller()->splitFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_mode = VMMove;
                m_dragHandlePosition += delta;
                return Continue;
            }
            return Deny;
        }

        bool VertexTool::doHandleMove(const InputState& inputState) const {
            if (!(inputState.mouseButtonsPressed(MouseButtons::MBLeft) &&
                  (inputState.modifierKeysPressed(ModifierKeys::MKNone) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
                   inputState.modifierKeysPressed(ModifierKeys::MKAlt | ModifierKeys::MKShift))))
                return false;
            
            const Model::Hit hit = firstHit(inputState.pickResult());
            return hit.isMatch();
        }
        
        Vec3 VertexTool::doGetMoveOrigin(const InputState& inputState) const {
            const Model::Hit hit = firstHit(inputState.pickResult());
            assert(hit.isMatch());
            return hit.hitPoint();
        }
        
        String VertexTool::doGetActionName(const InputState& inputState) const {
            if (m_mode == VMMove || m_mode == VMSnap) {
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
            const Model::Hit hit = firstHit(inputState.pickResult());
            assert(hit.isMatch());
            m_dragHandlePosition = hit.target<Vec3>();
            controller()->beginUndoableGroup();
            return true;
        }
        
        Vec3 VertexTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            if (m_mode == VMSnap) {
                const Model::Hit hit = firstHit(inputState.pickResult());
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
            controller()->rebuildBrushGeometry(document()->selectedBrushes());
            controller()->closeGroup();
            m_mode = VMMove;
        }
        
        bool VertexTool::initiallyActive() const {
            return false;
        }
        
        bool VertexTool::doActivate(const InputState& inputState) {
            m_mode = VMMove;
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
        
        void VertexTool::bindObservers() {
            document()->selectionDidChangeNotifier.addObserver(this, &VertexTool::selectionDidChange);
            document()->objectWillChangeNotifier.addObserver(this, &VertexTool::objectWillChange);
            document()->objectDidChangeNotifier.addObserver(this, &VertexTool::objectDidChange);
            controller()->commandDoNotifier.addObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandDoneNotifier.addObserver(this, &VertexTool::commandDoneOrUndoFailed);
            controller()->commandDoFailedNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoNotifier.addObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandUndoneNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoFailedNotifier.addObserver(this, &VertexTool::commandDoneOrUndoFailed);
        }
        
        void VertexTool::unbindObservers() {
            document()->selectionDidChangeNotifier.removeObserver(this, &VertexTool::selectionDidChange);
            document()->objectWillChangeNotifier.removeObserver(this, &VertexTool::objectWillChange);
            document()->objectDidChangeNotifier.removeObserver(this, &VertexTool::objectDidChange);
            controller()->commandDoNotifier.removeObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandDoneNotifier.removeObserver(this, &VertexTool::commandDoneOrUndoFailed);
            controller()->commandDoFailedNotifier.removeObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoNotifier.removeObserver(this, &VertexTool::commandDoOrUndo);
            controller()->commandUndoneNotifier.addObserver(this, &VertexTool::commandDoFailedOrUndone);
            controller()->commandUndoFailedNotifier.removeObserver(this, &VertexTool::commandDoneOrUndoFailed);
        }

        void VertexTool::doPick(const InputState& inputState, Model::PickResult& pickResult) {
            m_handleManager.pick(inputState.pickRay(), pickResult, m_mode == VMSplit);
        }
        
        bool VertexTool::doMouseDown(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;

            const Model::Hit::List hits = firstHits(inputState.pickResult());
            if (hits.empty())
                return false;
            
            const Model::Hit& firstHit = hits.front();
            if (firstHit.type() == VertexHandleManager::VertexHandleHit)
                vertexHandleClicked(inputState, hits);
            else if (firstHit.type() == VertexHandleManager::EdgeHandleHit)
                edgeHandleClicked(inputState, hits);
            else
                faceHandleClicked(inputState, hits);
            return true;
        }
        
        bool VertexTool::doMouseUp(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;
            
            const Model::Hit::List hits = firstHits(inputState.pickResult());
            if (!hits.empty())
                return true;
            
            if (m_handleManager.selectedVertexHandles().empty() &&
                m_handleManager.selectedEdgeHandles().empty() &&
                m_handleManager.selectedFaceHandles().empty())
                return false;
            
            m_handleManager.deselectAllHandles();
            m_mode = VMMove;
            return true;
        }

        bool VertexTool::doMouseDoubleClick(const InputState& inputState) {
            if (dismissClick(inputState))
                return false;

            const Model::Hit::List hits = firstHits(inputState.pickResult());
            if (hits.empty())
                return false;
            
            const Model::Hit& firstHit = hits.front();
            if (firstHit.type() == VertexHandleManager::VertexHandleHit) {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectVertexHandle(firstHit.target<Vec3>());
                m_mode = VMSnap;
            } else if (firstHit.type() == VertexHandleManager::EdgeHandleHit) {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectEdgeHandle(firstHit.target<Vec3>());
                m_mode = VMSplit;
            } else {
                m_handleManager.deselectAllHandles();
                m_handleManager.selectFaceHandle(firstHit.target<Vec3>());
                m_mode = VMSplit;
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
        
        void VertexTool::vertexHandleClicked(const InputState& inputState, const Model::Hit::List& hits) {
            m_handleManager.deselectAllEdgeHandles();
            m_handleManager.deselectAllFaceHandles();
            
            size_t selected = 0;
            Model::Hit::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Model::Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isVertexHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectVertexHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectVertexHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::edgeHandleClicked(const InputState& inputState, const Model::Hit::List& hits) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllFaceHandles();
            
            size_t selected = 0;
            Model::Hit::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Model::Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isEdgeHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectEdgeHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectEdgeHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::faceHandleClicked(const InputState& inputState, const Model::Hit::List& hits) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllEdgeHandles();
            
            size_t selected = 0;
            Model::Hit::List::const_iterator it, end;
            for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                const Model::Hit& hit = *it;
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isFaceHandleSelected(position))
                    selected++;
            }
            
            if (selected < hits.size()) {
                if (inputState.modifierKeys() != ModifierKeys::MKCtrlCmd)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectFaceHandle(position);
                }
            } else {
                if (inputState.modifierKeys() == ModifierKeys::MKCtrlCmd) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
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
            m_handleManager.render(renderContext, m_mode == VMSplit);

            if (dragging()) {
                m_handleManager.renderHighlight(renderContext, m_dragHandlePosition);
                renderMoveIndicator(inputState, renderContext);
            } else {
                const Model::Hit hit = firstHit(inputState.pickResult());
                if (hit.isMatch()) {
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.renderHighlight(renderContext, position);
                    if (m_handleManager.isHandleSelected(position))
                        renderMoveIndicator(inputState, renderContext);
                }
            }
        }

        void VertexTool::selectionDidChange(const Model::SelectionResult& selection) {
            Model::ObjectSet::const_iterator it, end;

            const Model::ObjectSet& selectedObjects = selection.selectedObjects();
            for (it = selectedObjects.begin(), end = selectedObjects.end(); it != end; ++it) {
                Model::Object* object = *it;
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.addBrush(brush);
                }
            }
            
            const Model::ObjectSet& deselectedObjects = selection.deselectedObjects();
            for (it = deselectedObjects.begin(), end = deselectedObjects.end(); it != end; ++it) {
                Model::Object* object = *it;
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.removeBrush(brush);
                }
            }
        }

        void VertexTool::objectWillChange(Model::Object* object) {
            if (!m_ignoreObjectChangeNotifications) {
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.removeBrush(brush);
                }
            }
        }

        void VertexTool::objectDidChange(Model::Object* object) {
            if (!m_ignoreObjectChangeNotifications) {
                if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    m_handleManager.addBrush(brush);
                }
            }
        }

        void VertexTool::commandDoOrUndo(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == MoveBrushVerticesCommand::Type ||
                command->type() == MoveBrushEdgesCommand::Type ||
                command->type() == MoveBrushFacesCommand::Type ||
                command->type() == SplitBrushEdgesCommand::Type ||
                command->type() == SplitBrushFacesCommand::Type ||
                command->type() == SnapBrushVerticesCommand::Type ||
                command->type() == RebuildBrushGeometryCommand::Type) {
                BrushVertexHandleCommand::Ptr handleCommand = Command::cast<BrushVertexHandleCommand>(command);
                handleCommand->removeBrushes(m_handleManager);
                m_ignoreObjectChangeNotifications = true;
            }
        }

        void VertexTool::commandDoneOrUndoFailed(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == MoveBrushVerticesCommand::Type ||
                command->type() == MoveBrushEdgesCommand::Type ||
                command->type() == MoveBrushFacesCommand::Type ||
                command->type() == SplitBrushEdgesCommand::Type ||
                command->type() == SplitBrushFacesCommand::Type ||
                command->type() == SnapBrushVerticesCommand::Type ||
                command->type() == RebuildBrushGeometryCommand::Type) {
                BrushVertexHandleCommand::Ptr handleCommand = Command::cast<BrushVertexHandleCommand>(command);
                handleCommand->addBrushes(m_handleManager);
                handleCommand->selectNewHandlePositions(m_handleManager);
                m_ignoreObjectChangeNotifications = false;
            }
        }
        
        void VertexTool::commandDoFailedOrUndone(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == MoveBrushVerticesCommand::Type ||
                command->type() == MoveBrushEdgesCommand::Type ||
                command->type() == MoveBrushFacesCommand::Type ||
                command->type() == SplitBrushEdgesCommand::Type ||
                command->type() == SplitBrushFacesCommand::Type ||
                command->type() == SnapBrushVerticesCommand::Type ||
                command->type() == RebuildBrushGeometryCommand::Type) {
                BrushVertexHandleCommand::Ptr handleCommand = Command::cast<BrushVertexHandleCommand>(command);
                handleCommand->addBrushes(m_handleManager);
                handleCommand->selectOldHandlePositions(m_handleManager);
                m_ignoreObjectChangeNotifications = false;
            }
        }

        Model::Hit VertexTool::firstHit(const Model::PickResult& pickResult) const {
            const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            return Model::firstHit(pickResult, any, true).hit;
        }

        Model::Hit::List VertexTool::firstHits(const Model::PickResult& pickResult) const {
            Model::Hit::List result;
            Model::BrushSet brushes;
            
            const Model::Hit::HitType any = VertexHandleManager::VertexHandleHit | VertexHandleManager::EdgeHandleHit | VertexHandleManager::FaceHandleHit;
            const Model::PickResult::FirstHit first = Model::firstHit(pickResult, any, true);
            if (first.matches) {
                const Vec3 firstHitPosition = first.hit.target<Vec3>();

                const Model::Hit::List hits = Model::hits(pickResult, any);
                Model::Hit::List::const_iterator hIt, hEnd;
                for (hIt = hits.begin(), hEnd = hits.end(); hIt != hEnd; ++hIt) {
                    const Model::Hit& hit = *hIt;
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
