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

#include "VertexToolOld.h"

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
#include "View/RemoveBrushEdgesCommand.h"
#include "View/RemoveBrushFacesCommand.h"
#include "View/RemoveBrushVerticesCommand.h"
#include "View/Selection.h"
#include "View/SnapBrushVerticesCommand.h"
#include "View/SplitBrushEdgesCommand.h"
#include "View/SplitBrushFacesCommand.h"
#include "View/VertexCommand.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        VertexToolOld::VertexToolOld(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_handleManager(m_document),
        m_mode(Mode_Move),
        m_changeCount(0),
        m_ignoreChangeNotifications(false),
        m_dragging(false) {}

        const Grid& VertexToolOld::grid() const {
            return lock(m_document)->grid();
        }

        void VertexToolOld::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            m_handleManager.pick(pickRay, camera, pickResult, m_mode == Mode_Split);
        }
        
        bool VertexToolOld::deselectAll() {
            if (m_handleManager.selectedVertexHandles().empty() &&
                m_handleManager.selectedEdgeHandles().empty() &&
                m_handleManager.selectedFaceHandles().empty())
                return false;
            
            m_handleManager.deselectAllHandles();
            m_mode = Mode_Move;
            refreshViews();
            return true;
        }
        
        bool VertexToolOld::mergeVertices(const Model::Hit& hit) {
            if (m_handleManager.selectedVertexCount() != 1)
                return false;
            if (hit.type() != VertexHandleManagerOld::VertexHandleHit)
                return false;
            const Vec3 targetPosition = hit.target<Vec3>();
            const Vec3 originalPosition = m_handleManager.selectedVertexHandlePositions().front();
            const Vec3 delta = targetPosition - originalPosition;
            moveVerticesAndRebuildBrushGeometry(delta);
            return true;
        }
        
        bool VertexToolOld::handleDoubleClicked(const Model::Hit& hit) {
            if (hit.type() == VertexHandleManagerOld::EdgeHandleHit) {
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
        
        bool VertexToolOld::select(const Model::Hit::List& hits, const bool addToSelection) {
            assert(!hits.empty());
            const Model::Hit& hit = hits.front();
            if (hit.type() == VertexHandleManagerOld::VertexHandleHit)
                selectVertex(hits, addToSelection);
            else if (hit.type() == VertexHandleManagerOld::EdgeHandleHit)
                selectEdge(hits, addToSelection);
            else
                selectFace(hits, addToSelection);
            refreshViews();
            return true;
        }
        
        void VertexToolOld::select(const Lasso& lasso, const bool modifySelection) {
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

        bool VertexToolOld::canRemoveSelection() const {
            return m_handleManager.hasSelectedHandles();
        }
        
        void VertexToolOld::removeSelection() {
            assert(canRemoveSelection());
            
            if (m_handleManager.selectedVertexCount() > 0) {
                lock(m_document)->removeVertices(m_handleManager.selectedVertexHandles());
            } else if (m_handleManager.selectedEdgeCount() > 0) {
                lock(m_document)->removeEdges(m_handleManager.selectedEdgeHandles());
            } else if (m_handleManager.selectedFaceCount() > 0) {
                lock(m_document)->removeFaces(m_handleManager.selectedFaceHandles());
            }
        }

        bool VertexToolOld::beginMove(const Model::Hit& hit) {
            assert(hit.isMatch());
            
            const Vec3 handlePosition = hit.target<Vec3>();
            if (!m_handleManager.isHandleSelected(handlePosition)) {
                m_handleManager.deselectAllHandles();
                if (hit.type() == VertexHandleManagerOld::VertexHandleHit)
                    m_handleManager.selectVertexHandle(handlePosition);
                else if (hit.type() == VertexHandleManagerOld::EdgeHandleHit)
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

        VertexToolOld::MoveResult VertexToolOld::move(const Vec3& delta) {
            return moveVertices(delta);
        }

        void VertexToolOld::endMove() {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
            rebuildBrushGeometry();
            m_mode = Mode_Move;
            m_dragging = false;
        }
        
        void VertexToolOld::cancelMove() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_mode = Mode_Move;
            m_dragging = false;
        }

        void VertexToolOld::renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_handleManager.render(renderContext, renderBatch, m_mode == Mode_Split);
        }
        
        void VertexToolOld::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexToolOld::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            m_handleManager.renderHighlight(renderContext, renderBatch, position);
        }

        void VertexToolOld::renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            m_handleManager.renderEdgeHighlight(renderContext, renderBatch, handlePosition);
        }
        
        void VertexToolOld::renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition) {
            m_handleManager.renderFaceHighlight(renderContext, renderBatch, handlePosition);
        }

        void VertexToolOld::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderGuide(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexToolOld::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) {
            m_handleManager.renderGuide(renderContext, renderBatch, position);
        }

        bool VertexToolOld::cancel() {
            if (m_handleManager.hasSelectedHandles()) {
                m_handleManager.deselectAllHandles();
                return true;
            }
            return false;
        }

        // Indicates that none of the brushes incident to the given handle are in the given set.
        bool VertexToolOld::handleBrushes(const Vec3& position, Model::BrushSet& brushes) const {
            bool newBrush = true;
            const Model::BrushSet& handleBrushes = m_handleManager.brushes(position);
            for (Model::Brush* brush : handleBrushes)
                newBrush &= brushes.insert(brush).second;
            return newBrush;
        }

        bool VertexToolOld::handleSelected(const Vec3& position) const {
            return m_handleManager.isEdgeHandleSelected(position);
        }

        bool VertexToolOld::hasSelectedHandles() const {
            return (m_handleManager.selectedVertexCount() > 0 ||
                    m_handleManager.selectedEdgeCount() > 0 ||
                    m_handleManager.selectedFaceCount() > 0);
        }

        void VertexToolOld::moveVerticesAndRebuildBrushGeometry(const Vec3& delta) {
            if (hasSelectedHandles()) {
                moveVertices(delta);
                rebuildBrushGeometry();
                m_mode = Mode_Move;
            }
        }

        void VertexToolOld::selectVertex(const Model::Hit::List& hits, const bool addToSelection) {
            m_handleManager.deselectAllEdgeHandles();
            m_handleManager.deselectAllFaceHandles();

            if (!addToSelection)
                m_handleManager.deselectAllHandles();
            
            size_t selected = 0;
            for (const Model::Hit& hit : hits) {
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isVertexHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                for (const Model::Hit& hit : hits) {
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectVertexHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (const Model::Hit& hit : hits) {
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectVertexHandle(position);
                    }
                }
            }
        }
        
        void VertexToolOld::selectEdge(const Model::Hit::List& hits, const bool addToSelection) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllFaceHandles();
            
            size_t selected = 0;
            for (const Model::Hit& hit : hits) {
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isEdgeHandleSelected(position))
                    ++selected;
            }
            
            if (selected < hits.size()) {
                if (!addToSelection)
                    m_handleManager.deselectAllHandles();
                for (const Model::Hit& hit : hits) {
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectEdgeHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (const Model::Hit& hit : hits) {
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectEdgeHandle(position);
                    }
                }
            }
        }
        
        void VertexToolOld::selectFace(const Model::Hit::List& hits, const bool addToSelection) {
            m_handleManager.deselectAllVertexHandles();
            m_handleManager.deselectAllEdgeHandles();
            
            size_t selected = 0;
            for (const Model::Hit& hit : hits) {
                const Vec3 position = hit.target<Vec3>();
                if (m_handleManager.isFaceHandleSelected(position))
                    selected++;
            }
            
            if (selected < hits.size()) {
                if (!addToSelection)
                    m_handleManager.deselectAllHandles();
                for (const Model::Hit& hit : hits) {
                    const Vec3 position = hit.target<Vec3>();
                    m_handleManager.selectFaceHandle(position);
                }
            } else {
                if (addToSelection) {
                    for (const Model::Hit& hit : hits) {
                        const Vec3 position = hit.target<Vec3>();
                        m_handleManager.deselectFaceHandle(position);
                    }
                }
            }
        }
        
        String VertexToolOld::actionName() const {
            if (m_mode == Mode_Move) {
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
        
        VertexToolOld::MoveResult VertexToolOld::moveVertices(const Vec3& delta) {
            if (m_mode == Mode_Move) {
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
            return MR_Continue;
        }
        
        VertexToolOld::MoveResult VertexToolOld::doMoveVertices(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            const MapDocument::MoveVerticesResult result = document->moveVertices(m_handleManager.selectedVertexHandles(), delta);
            if (result.success) {
                if (!result.hasRemainingVertices)
                    return MR_Cancel;
                m_dragHandlePosition += delta;
                return MR_Continue;
            }
            return MR_Deny;
        }
        
        VertexToolOld::MoveResult VertexToolOld::doMoveEdges(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->moveEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_dragHandlePosition += delta;
                return MR_Continue;
            }
            return MR_Deny;
        }
        
        VertexToolOld::MoveResult VertexToolOld::doMoveFaces(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->moveFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_dragHandlePosition += delta;
                return MR_Continue;
            }
            return MR_Deny;
        }
        
        VertexToolOld::MoveResult VertexToolOld::doSplitEdges(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->splitEdges(m_handleManager.selectedEdgeHandles(), delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MR_Continue;
            }
            return MR_Deny;
        }
        
        VertexToolOld::MoveResult VertexToolOld::doSplitFaces(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            if (document->splitFaces(m_handleManager.selectedFaceHandles(), delta)) {
                m_mode = Mode_Move;
                m_dragHandlePosition += delta;
                return MR_Continue;
            }
            return MR_Deny;
        }

        void VertexToolOld::rebuildBrushGeometry() {
            MapDocumentSPtr document = lock(m_document);

            const SetBool ignoreChangeNotifications(m_ignoreChangeNotifications);
            
            const Vec3::List selectedVertexHandles = m_handleManager.selectedVertexHandlePositions();
            const Vec3::List selectedEdgeHandles   = m_handleManager.selectedEdgeHandlePositions();
            const Vec3::List selectedFaceHandles   = m_handleManager.selectedFaceHandlePositions();
            
            const Model::BrushSet brushes = m_handleManager.selectedBrushes();
            
            m_handleManager.removeBrushes(std::begin(brushes), std::end(brushes));
            document->rebuildBrushGeometry(Model::BrushList(std::begin(brushes), std::end(brushes)));
            m_handleManager.addBrushes(std::begin(brushes), std::end(brushes));

            m_handleManager.reselectVertexHandles(brushes, selectedVertexHandles, 0.01);
            m_handleManager.reselectEdgeHandles(brushes, selectedEdgeHandles, 0.01);
            m_handleManager.reselectFaceHandles(brushes, selectedFaceHandles, 0.01);
        }

        bool VertexToolOld::doActivate() {
            MapDocumentSPtr document = lock(m_document);
            m_mode = Mode_Move;
            m_handleManager.clear();
            
            const Model::BrushList& selectedBrushes = document->selectedNodes().brushes();
            m_handleManager.addBrushes(std::begin(selectedBrushes), std::end(selectedBrushes));
            m_changeCount = 0;
            
            bindObservers();
            return true;
        }
        
        bool VertexToolOld::doDeactivate() {
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
        
        void VertexToolOld::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &VertexToolOld::selectionDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &VertexToolOld::nodesWillChange);
            document->nodesDidChangeNotifier.addObserver(this, &VertexToolOld::nodesDidChange);
            document->commandDoNotifier.addObserver(this, &VertexToolOld::commandDo);
            document->commandDoneNotifier.addObserver(this, &VertexToolOld::commandDone);
            document->commandDoFailedNotifier.addObserver(this, &VertexToolOld::commandDoFailed);
            document->commandUndoNotifier.addObserver(this, &VertexToolOld::commandUndo);
            document->commandUndoneNotifier.addObserver(this, &VertexToolOld::commandUndone);
            document->commandUndoFailedNotifier.addObserver(this, &VertexToolOld::commandUndoFailed);
        }
        
        void VertexToolOld::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &VertexToolOld::selectionDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &VertexToolOld::nodesWillChange);
                document->nodesDidChangeNotifier.removeObserver(this, &VertexToolOld::nodesDidChange);
                document->commandDoNotifier.removeObserver(this, &VertexToolOld::commandDo);
                document->commandDoneNotifier.removeObserver(this, &VertexToolOld::commandDone);
                document->commandDoFailedNotifier.removeObserver(this, &VertexToolOld::commandDoFailed);
                document->commandUndoNotifier.removeObserver(this, &VertexToolOld::commandUndo);
                document->commandUndoneNotifier.removeObserver(this, &VertexToolOld::commandUndone);
                document->commandUndoFailedNotifier.removeObserver(this, &VertexToolOld::commandUndoFailed);
            }
        }
        
        void VertexToolOld::commandDo(Command::Ptr command) {
            commandDoOrUndo(command);
        }
        
        void VertexToolOld::commandDone(Command::Ptr command) {
            commandDoneOrUndoFailed(command);
        }
        
        void VertexToolOld::commandDoFailed(Command::Ptr command) {
            commandDoFailedOrUndone(command);
        }
        
        void VertexToolOld::commandUndo(UndoableCommand::Ptr command) {
            commandDoOrUndo(command);
        }
        
        void VertexToolOld::commandUndone(UndoableCommand::Ptr command) {
            commandDoFailedOrUndone(command);
        }
        
        void VertexToolOld::commandUndoFailed(UndoableCommand::Ptr command) {
            commandDoneOrUndoFailed(command);
        }

        void VertexToolOld::commandDoOrUndo(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->removeBrushes(m_handleManager);
                m_ignoreChangeNotifications = true;
            }
        }
        
        void VertexToolOld::commandDoneOrUndoFailed(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addBrushes(m_handleManager);
                vertexCommand->selectNewHandlePositions(m_handleManager);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        void VertexToolOld::commandDoFailedOrUndone(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addBrushes(m_handleManager);
                vertexCommand->selectOldHandlePositions(m_handleManager);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        bool VertexToolOld::isVertexCommand(const Command::Ptr command) const {
            return (command->type() == MoveBrushVerticesCommand::Type ||
                    command->type() == MoveBrushEdgesCommand::Type ||
                    command->type() == MoveBrushFacesCommand::Type ||
                    command->type() == SplitBrushEdgesCommand::Type ||
                    command->type() == SplitBrushFacesCommand::Type ||
                    command->type() == RemoveBrushVerticesCommand::Type ||
                    command->type() == RemoveBrushEdgesCommand::Type ||
                    command->type() == RemoveBrushFacesCommand::Type);
        }

        class AddToHandleManager : public Model::NodeVisitor {
        private:
            VertexHandleManagerOld& m_handleManager;
        public:
            AddToHandleManager(VertexHandleManagerOld& handleManager) :
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
            VertexHandleManagerOld& m_handleManager;
        public:
            RemoveFromHandleManager(VertexHandleManagerOld& handleManager) :
            m_handleManager(handleManager) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_handleManager.removeBrush(brush); }
        };

        void VertexToolOld::selectionDidChange(const Selection& selection) {
            const Model::NodeList& selectedNodes = selection.selectedNodes();
            AddToHandleManager addVisitor(m_handleManager);
            Model::Node::accept(std::begin(selectedNodes), std::end(selectedNodes), addVisitor);
            
            const Model::NodeList& deselectedNodes = selection.deselectedNodes();
            RemoveFromHandleManager removeVisitor(m_handleManager);
            Model::Node::accept(std::begin(deselectedNodes), std::end(deselectedNodes), removeVisitor);
        }

        void VertexToolOld::nodesWillChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                RemoveFromHandleManager removeVisitor(m_handleManager);
                Model::Node::accept(std::begin(nodes), std::end(nodes), removeVisitor);
            }
        }
        
        void VertexToolOld::nodesDidChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                AddToHandleManager addVisitor(m_handleManager);
                Model::Node::accept(std::begin(nodes), std::end(nodes), addVisitor);
            }
        }
    }
}
