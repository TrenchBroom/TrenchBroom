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

#include "SetAny.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/HitAdapter.h"
#include "Model/NodeVisitor.h"
#include "Model/Object.h"
#include "Renderer/RenderContext.h"
#include "View/InputState.h"
#include "View/Grid.h"
#include "View/Lasso.h"
#include "View/MapDocument.h"
#include "View/MoveBrushEdgesCommand.h"
#include "View/MoveBrushFacesCommand.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/Selection.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/SplitBrushEdgesCommand.h"
#include "View/SplitBrushFacesCommand.h"
#include "View/VertexCommand.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        VertexTool::VertexTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_handleManager(m_document),
        m_mode(Mode_Move),
        m_changeCount(0),
        m_ignoreChangeNotifications(false),
        m_dragging(false) {}

        void VertexTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            m_handleManager.pick(pickRay, camera, pickResult, m_mode == Mode_Split);
        }
        
        bool VertexTool::deselectAll() {
            if (m_handleManager.selectedVertexHandles().empty() &&
                m_handleManager.selectedEdgeHandles().empty() &&
                m_handleManager.selectedFaceHandles().empty())
                return false;
            
            m_handleManager.deselectAllHandles();
            m_mode = Mode_Move;
            refreshViews();
            return true;
        }
        
        bool VertexTool::mergeVertices(const Model::Hit& hit) {
            if (m_handleManager.selectedVertexCount() != 1)
                return false;
            if (hit.type() != VertexHandleManager::VertexHandleHit)
                return false;
            const Vec3 targetPosition = hit.target<Vec3>();
            const Vec3 originalPosition = m_handleManager.selectedVertexHandlePositions().front();
            const Vec3 delta = targetPosition - originalPosition;
            moveVerticesAndRebuildBrushGeometry(delta);
            return true;
        }
        
        bool VertexTool::handleDoubleClicked(const Model::Hit& hit) {
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
            refreshViews();
            return true;
        }
        
        bool VertexTool::select(const Model::Hit::List& hits, const bool addToSelection) {
            assert(!hits.empty());
            const Model::Hit& hit = hits.front();
            if (hit.type() == VertexHandleManager::VertexHandleHit)
                selectVertex(hits, addToSelection);
            else if (hit.type() == VertexHandleManager::EdgeHandleHit)
                selectEdge(hits, addToSelection);
            else
                selectFace(hits, addToSelection);
            refreshViews();
            return true;
        }
        
        void VertexTool::select(const Lasso& lasso, const bool modifySelection) {
            if (m_handleManager.selectedEdgeCount() > 0) {
                const Vec3::List contained = lasso.containedPoints(m_handleManager.edgeHandlePositions());
                if (!modifySelection) m_handleManager.deselectAllEdgeHandles();
                m_handleManager.toggleEdgeHandles(contained);
            } else if (m_handleManager.selectedFaceCount() > 0) {
                const Vec3::List contained = lasso.containedPoints(m_handleManager.faceHandlePositions());
                if (!modifySelection) m_handleManager.deselectAllFaceHandles();
                m_handleManager.toggleFaceHandles(contained);
            } else {
                const Vec3::List contained = lasso.containedPoints(m_handleManager.vertexHandlePositions());
                if (!modifySelection) m_handleManager.deselectAllVertexHandles();
                m_handleManager.toggleVertexHandles(contained);
            }
            refreshViews();
        }

        bool VertexTool::beginMove(const Model::Hit& hit) {
            assert(hit.isMatch());
            
            const Vec3 handlePosition = hit.target<Vec3>();
            if (!m_handleManager.isHandleSelected(handlePosition)) {
                m_handleManager.deselectAllHandles();
                if (hit.type() == VertexHandleManager::VertexHandleHit)
                    m_handleManager.selectVertexHandle(handlePosition);
                else if (hit.type() == VertexHandleManager::EdgeHandleHit)
                    m_handleManager.selectEdgeHandle(handlePosition);
                else
                    m_handleManager.selectFaceHandle(handlePosition);
                refreshViews();
            }
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction(actionName());
            
            m_dragHandlePosition = handlePosition;
            m_dragging = true;
            return true;
        }

        Vec3 VertexTool::snapMoveDelta(const Vec3& delta, const Model::Hit& hit, const bool relative) {
            if (m_mode == Mode_Snap) {
                if (hit.isMatch() && !m_handleManager.isVertexHandleSelected(hit.target<Vec3>()))
                    return hit.target<Vec3>() - m_dragHandlePosition;
                return Vec3::Null;
            }
            
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            if (relative)
                return grid.snap(delta);
            return grid.snap(m_dragHandlePosition + delta) - m_dragHandlePosition;
        }

        MoveResult VertexTool::move(const Vec3& delta) {
            return moveVertices(delta);
        }

        void VertexTool::endMove() {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
            rebuildBrushGeometry();
            m_mode = Mode_Move;
            m_dragging = false;
        }
        
        void VertexTool::cancelMove() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_mode = Mode_Move;
            m_dragging = false;
        }

        void VertexTool::renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_handleManager.render(renderContext, renderBatch, m_mode == Mode_Split);
        }
        
        void VertexTool::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexTool::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            m_handleManager.renderHighlight(renderContext, renderBatch, position);
        }

        void VertexTool::renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            m_handleManager.renderEdgeHighlight(renderContext, renderBatch, handlePosition);
        }
        
        void VertexTool::renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            m_handleManager.renderFaceHighlight(renderContext, renderBatch, handlePosition);
        }

        void VertexTool::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderGuide(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexTool::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            m_handleManager.renderGuide(renderContext, renderBatch, position);
        }

        bool VertexTool::cancel() {
            if (m_handleManager.hasSelectedHandles()) {
                m_handleManager.deselectAllHandles();
                return true;
            }
            return false;
        }

        bool VertexTool::handleBrushes(const Vec3& position, Model::BrushSet& brushes) const {
            bool newBrush = true;
            const Model::BrushSet& handleBrushes = m_handleManager.brushes(position);
            Model::BrushSet::const_iterator bIt, bEnd;
            for (bIt = handleBrushes.begin(), bEnd = handleBrushes.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = *bIt;
                newBrush &= brushes.insert(brush).second;
            }
            return newBrush;
        }

        bool VertexTool::handleSelected(const Vec3& position) const {
            return m_handleManager.isEdgeHandleSelected(position);
        }

        bool VertexTool::hasSelectedHandles() const {
            return (m_handleManager.selectedVertexCount() > 0 ||
                    m_handleManager.selectedEdgeCount() > 0 ||
                    m_handleManager.selectedFaceCount() > 0);
        }

        void VertexTool::moveVerticesAndRebuildBrushGeometry(const Vec3& delta) {
            if (hasSelectedHandles()) {
                moveVertices(delta);
                rebuildBrushGeometry();
                m_mode = Mode_Move;
            }
        }

        bool VertexTool::canSnapVertices() const {
            if (m_handleManager.selectedEdgeCount() > 0 ||
                m_handleManager.selectedFaceCount() > 0)
                return false;
            MapDocumentSPtr document = lock(m_document);
            return m_handleManager.selectedVertexCount() > 0 || document->selectedNodes().hasOnlyBrushes();
        }
        
        void VertexTool::snapVertices(const size_t snapTo) {
            assert(canSnapVertices());
            MapDocumentSPtr document = lock(m_document);
            document->snapVertices(m_handleManager.selectedVertexHandles(), snapTo);
        }

        void VertexTool::selectVertex(const Model::Hit::List& hits, const bool addToSelection) {
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
                if (!addToSelection)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectVertexHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectVertexHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::selectEdge(const Model::Hit::List& hits, const bool addToSelection) {
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
                if (!addToSelection)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectEdgeHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectEdgeHandle(position);
                    }
                }
            }
        }
        
        void VertexTool::selectFace(const Model::Hit::List& hits, const bool addToSelection) {
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
                if (!addToSelection)
                    m_handleManager.deselectAllHandles();
                for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                    const Model::Hit& hit = *it;
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectFaceHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (it = hits.begin(), end = hits.end(); it != end; ++it) {
                        const Model::Hit& hit = *it;
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectFaceHandle(position);
                    }
                }
            }
        }
        
        String VertexTool::actionName() const {
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
            MapDocumentSPtr document = lock(m_document);
            const MapDocument::MoveVerticesResult result = document->moveVertices(m_handleManager.selectedVertexHandles(), delta);
            if (result.success) {
                if (!result.hasRemainingVertices)
                    return MoveResult_Conclude;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doMoveEdges(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->moveEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doMoveFaces(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->moveFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doSplitEdges(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->splitEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }
        
        MoveResult VertexTool::doSplitFaces(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->splitFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MoveResult_Continue;
            }
            return MoveResult_Deny;
        }

        void VertexTool::rebuildBrushGeometry() {
            MapDocumentSPtr document = lock(m_document);

            const SetBool ignoreChangeNotifications(m_ignoreChangeNotifications);
            
            const Vec3::List selectedVertexHandles = m_handleManager.selectedVertexHandlePositions();
            const Vec3::List selectedEdgeHandles   = m_handleManager.selectedEdgeHandlePositions();
            const Vec3::List selectedFaceHandles   = m_handleManager.selectedFaceHandlePositions();
            
            const Model::BrushSet brushes = m_handleManager.selectedBrushes();
            
            m_handleManager.removeBrushes(brushes.begin(), brushes.end());
            document->rebuildBrushGeometry(Model::BrushList(brushes.begin(), brushes.end()));
            m_handleManager.addBrushes(brushes.begin(), brushes.end());

            m_handleManager.reselectVertexHandles(brushes, selectedVertexHandles, 0.01);
            m_handleManager.reselectEdgeHandles(brushes, selectedEdgeHandles, 0.01);
            m_handleManager.reselectFaceHandles(brushes, selectedFaceHandles, 0.01);
        }

        bool VertexTool::doActivate() {
            MapDocumentSPtr document = lock(m_document);
            m_mode = Mode_Move;
            m_handleManager.clear();
            
            const Model::BrushList& selectedBrushes = document->selectedNodes().brushes();
            m_handleManager.addBrushes(selectedBrushes.begin(), selectedBrushes.end());
            m_changeCount = 0;
            
            bindObservers();
            return true;
        }
        
        bool VertexTool::doDeactivate() {
            unbindObservers();
            m_handleManager.clear();
            
            /*
             if (m_changeCount > 0) {
             RebuildBrushGeometryCommand* command = RebuildBrushGeometryCommand::rebuildGeometry(document, document.editStateManager().selectedBrushes(), m_changeCount);
             submitCommand(command);
             }
             */
            return true;
        }
        
        void VertexTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &VertexTool::selectionDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &VertexTool::nodesWillChange);
            document->nodesDidChangeNotifier.addObserver(this, &VertexTool::nodesDidChange);
            document->commandDoNotifier.addObserver(this, &VertexTool::commandDo);
            document->commandDoneNotifier.addObserver(this, &VertexTool::commandDone);
            document->commandDoFailedNotifier.addObserver(this, &VertexTool::commandDoFailed);
            document->commandUndoNotifier.addObserver(this, &VertexTool::commandUndo);
            document->commandUndoneNotifier.addObserver(this, &VertexTool::commandUndone);
            document->commandUndoFailedNotifier.addObserver(this, &VertexTool::commandUndoFailed);
        }
        
        void VertexTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &VertexTool::selectionDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &VertexTool::nodesWillChange);
                document->nodesDidChangeNotifier.removeObserver(this, &VertexTool::nodesDidChange);
                document->commandDoNotifier.removeObserver(this, &VertexTool::commandDo);
                document->commandDoneNotifier.removeObserver(this, &VertexTool::commandDone);
                document->commandDoFailedNotifier.removeObserver(this, &VertexTool::commandDoFailed);
                document->commandUndoNotifier.removeObserver(this, &VertexTool::commandUndo);
                document->commandUndoneNotifier.removeObserver(this, &VertexTool::commandUndone);
                document->commandUndoFailedNotifier.removeObserver(this, &VertexTool::commandUndoFailed);
            }
        }
        
        void VertexTool::commandDo(Command::Ptr command) {
            commandDoOrUndo(command);
        }
        
        void VertexTool::commandDone(Command::Ptr command) {
            commandDoneOrUndoFailed(command);
        }
        
        void VertexTool::commandDoFailed(Command::Ptr command) {
            commandDoFailedOrUndone(command);
        }
        
        void VertexTool::commandUndo(UndoableCommand::Ptr command) {
            commandDoOrUndo(command);
        }
        
        void VertexTool::commandUndone(UndoableCommand::Ptr command) {
            commandDoFailedOrUndone(command);
        }
        
        void VertexTool::commandUndoFailed(UndoableCommand::Ptr command) {
            commandDoneOrUndoFailed(command);
        }

        void VertexTool::commandDoOrUndo(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->removeBrushes(m_handleManager);
                m_ignoreChangeNotifications = true;
            }
        }
        
        void VertexTool::commandDoneOrUndoFailed(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addBrushes(m_handleManager);
                vertexCommand->selectNewHandlePositions(m_handleManager);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        void VertexTool::commandDoFailedOrUndone(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addBrushes(m_handleManager);
                vertexCommand->selectOldHandlePositions(m_handleManager);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        bool VertexTool::isVertexCommand(const Command::Ptr command) const {
            return (command->type() == SnapBrushVerticesCommand::Type ||
                    command->type() == MoveBrushVerticesCommand::Type ||
                    command->type() == MoveBrushEdgesCommand::Type ||
                    command->type() == MoveBrushFacesCommand::Type ||
                    command->type() == SplitBrushEdgesCommand::Type ||
                    command->type() == SplitBrushFacesCommand::Type);
        }

        class AddToHandleManager : public Model::NodeVisitor {
        private:
            VertexHandleManager& m_handleManager;
        public:
            AddToHandleManager(VertexHandleManager& handleManager) :
            m_handleManager(handleManager) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_handleManager.addBrush(brush); }
        };
        
        class RemoveFromHandleManager : public Model::NodeVisitor {
        private:
            VertexHandleManager& m_handleManager;
        public:
            RemoveFromHandleManager(VertexHandleManager& handleManager) :
            m_handleManager(handleManager) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_handleManager.removeBrush(brush); }
        };

        void VertexTool::selectionDidChange(const Selection& selection) {
            const Model::NodeList& selectedNodes = selection.selectedNodes();
            AddToHandleManager addVisitor(m_handleManager);
            Model::Node::accept(selectedNodes.begin(), selectedNodes.end(), addVisitor);
            
            const Model::NodeList& deselectedNodes = selection.deselectedNodes();
            RemoveFromHandleManager removeVisitor(m_handleManager);
            Model::Node::accept(deselectedNodes.begin(), deselectedNodes.end(), removeVisitor);
        }

        void VertexTool::nodesWillChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                RemoveFromHandleManager removeVisitor(m_handleManager);
                Model::Node::accept(nodes.begin(), nodes.end(), removeVisitor);
            }
        }
        
        void VertexTool::nodesDidChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                AddToHandleManager addVisitor(m_handleManager);
                Model::Node::accept(nodes.begin(), nodes.end(), addVisitor);
            }
        }

        
        String VertexTool::doGetIconName() const {
            return "VertexTool.png";
        }
    }
}
