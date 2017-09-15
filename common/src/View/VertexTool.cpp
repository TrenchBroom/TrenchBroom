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

#include "VertexTool.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "View/Lasso.h"
#include "View/MapDocument.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/RemoveBrushVerticesCommand.h"
#include "View/Selection.h"
#include "View/SplitBrushEdgesCommand.h"
#include "View/SplitBrushFacesCommand.h"
#include "View/VertexCommand.h"

#include <cassert>
#include <numeric>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType VertexTool::VertexHandleHit = VertexHandleManager::HandleHit;
        const Model::Hit::HitType VertexTool::EdgeHandleHit = EdgeHandleManager::HandleHit;
        const Model::Hit::HitType VertexTool::FaceHandleHit = FaceHandleManager::HandleHit;
        const Model::Hit::HitType VertexTool::SplitHandleHit = EdgeHandleHit | FaceHandleHit;
        const Model::Hit::HitType VertexTool::AnyHandleHit = VertexHandleHit | EdgeHandleHit | FaceHandleHit;

        VertexTool::VertexTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_mode(Mode_Move),
        m_changeCount(0),
        m_ignoreChangeNotifications(0),
        m_dragging(false),
        m_guideRenderer(document) {}

        const Grid& VertexTool::grid() const {
            return lock(m_document)->grid();
        }

        Model::BrushSet VertexTool::findIncidentBrushes(const Vec3& handle) const {
            return findIncidentBrushes(m_vertexHandles, handle);
        }

        
        Model::BrushSet VertexTool::findIncidentBrushes(const Edge3& handle) const {
            return findIncidentBrushes(m_edgeHandles, handle);
        }
        
        Model::BrushSet VertexTool::findIncidentBrushes(const Polygon3& handle) const {
            return findIncidentBrushes(m_faceHandles, handle);
        }

        void VertexTool::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            m_vertexHandles.pick(pickRay, camera, pickResult);
            m_edgeHandles.pick(pickRay, camera, pickResult);
            m_faceHandles.pick(pickRay, camera, pickResult);
        }

        bool VertexTool::select(const Model::Hit::List& hits, const bool addToSelection) {
            assert(!hits.empty());
            const Model::Hit& firstHit = hits.front();
            if (firstHit.type() == VertexHandleManager::HandleHit) {
                if (!addToSelection)
                    m_vertexHandles.deselectAll();
                
                // Count the number of hit handles which are selected already.
                const size_t selected = std::accumulate(std::begin(hits), std::end(hits), 0u, [this](const size_t cur, const Model::Hit& hit) {
                    const Vec3& handle = hit.target<Vec3>();
                    const bool curSelected = m_vertexHandles.selected(handle);
                    return cur + (curSelected ? 1 : 0);
                });

                if (selected < hits.size()) {
                    for (const auto& hit : hits)
                        m_vertexHandles.select(hit.target<Vec3>());
                } else if (addToSelection) {
                    // The user meant to deselect a selected handle.
                    for (const auto& hit : hits)
                        m_vertexHandles.deselect(hit.target<Vec3>());
                }
            }
            refreshViews();
            return true;
        }
        
        void VertexTool::select(const Lasso& lasso, const bool modifySelection) {
            const Vec3::List handles = lasso.containedPoints(m_vertexHandles.allHandles());
            if (!modifySelection)
                m_vertexHandles.deselectAll();
            m_vertexHandles.toggle(std::begin(handles), std::end(handles));
            refreshViews();
        }

        bool VertexTool::deselectAll() {
            if (m_vertexHandles.anySelected()) {
                m_vertexHandles.deselectAll();
                resetModeAfterDeselection();
                refreshViews();
                return true;
            }
            return false;
        }

        bool VertexTool::startMove(const Model::Hit& hit, const Ray3& pickRay) {
            assert(hit.isMatch());
            assert(hit.hasType(AnyHandleHit));
            
            const Vec3 handle = getHandlePosition(hit, pickRay);
            if (hit.hasType(VertexHandleHit)) {
                if (!m_vertexHandles.selected(handle)) {
                    m_vertexHandles.deselectAll();
                    m_vertexHandles.select(handle);
                    refreshViews();
                }
            } else {
                assert(hit.hasType(SplitHandleHit));
                m_vertexHandles.deselectAll();
                if (hit.hasType(EdgeHandleHit)) {
                    m_edgeHandles.select(hit.target<Edge3>());
                    m_mode = Mode_Split_Edge;
                } else {
                    m_faceHandles.select(hit.target<Polygon3>());
                    m_mode = Mode_Split_Face;
                }
            }
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction(actionName());

            m_dragHandlePosition = handle;
            m_dragging = true;
            return true;
        }
        
        VertexTool::MoveResult VertexTool::move(const Vec3& delta) {
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

        Vec3 VertexTool::getHandlePosition(const Model::Hit& hit, const Ray3& pickRay) {
            assert(hit.isMatch());
            assert(hit.hasType(AnyHandleHit));
            
            if (hit.hasType(VertexHandleHit)) {
                return hit.target<Vec3>();
            } else if (hit.hasType(EdgeHandleHit)) {
                const Edge3& edge = hit.target<Edge3>();
                const Ray3::LineDistance distance = pickRay.distanceToSegment(edge.start(), edge.end());
                assert(!distance.parallel);
                
                return edge.pointAtDistance(distance.lineDistance);
            } else {
                assert(hit.hasType(FaceHandleHit));
                return hit.hitPoint();
            }
        }
        
        String VertexTool::actionName() const {
            switch (m_mode) {
                case Mode_Move:
                    return StringUtils::safePlural(m_vertexHandles.selectedHandleCount(), "Move Vertex", "Move Vertices");
                case Mode_Split_Edge:
                    return "Split Edge";
                case Mode_Split_Face:
                    return "Split Face";
                switchDefault();
            }
        }

        VertexTool::MoveResult VertexTool::moveVertices(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);

            if (m_mode == Mode_Move) {
                const Vec3::List handles = m_vertexHandles.selectedHandles();
                const Model::VertexToBrushesMap brushMap = buildBrushMap(handles);
                
                const MapDocument::MoveVerticesResult result = document->moveVertices(brushMap, delta);
                if (result.success) {
                    if (!result.hasRemainingVertices)
                        return MR_Cancel;
                    m_dragHandlePosition += delta;
                    return MR_Continue;
                }
                return MR_Deny;
            } else {
                Model::BrushSet brushes;
                if (m_mode == Mode_Split_Edge) {
                    assert(m_edgeHandles.selectedHandleCount() == 1);
                    const Edge3 handle = m_edgeHandles.selectedHandles().front();
                    brushes = findIncidentBrushes(handle);
                } else {
                    assert(m_mode == Mode_Split_Face);
                    assert(m_faceHandles.selectedHandleCount() == 1);
                    const Polygon3 handle = m_faceHandles.selectedHandles().front();
                    brushes = findIncidentBrushes(handle);
                }
                
                const Model::VertexToBrushesMap vertices { std::make_pair(m_dragHandlePosition + delta, brushes) };
                if (document->addVertices(vertices)) {
                    m_mode = Mode_Move;
                    m_edgeHandles.deselectAll();
                    m_faceHandles.deselectAll();
                    m_dragHandlePosition += delta;
                    m_vertexHandles.select(m_dragHandlePosition);
                }
                
                return MR_Continue;
            }
        }

        Model::VertexToBrushesMap VertexTool::buildBrushMap(const Vec3::List& handles) const {
            Model::VertexToBrushesMap result;
            for (const auto& handle : handles) {
                result[handle] = findIncidentBrushes(handle);
            }
            return result;
        }

        void VertexTool::renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
            Renderer::RenderService renderService(renderContext, renderBatch);
            if (!m_vertexHandles.allSelected())
                renderHandles(m_vertexHandles.unselectedHandles(), renderService, pref(Preferences::HandleColor));
            if (m_vertexHandles.anySelected())
                renderHandles(m_vertexHandles.selectedHandles(), renderService, pref(Preferences::SelectedHandleColor));
        }

        void VertexTool::renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handle) const {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::HandleColor));
            renderService.renderPointHandle(handle);
        }

        void VertexTool::renderDragHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
            renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexTool::renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handle) const {
            Renderer::RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
            renderService.renderPointHandleHighlight(handle);
            
            renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderService.renderString(handle.asString(), handle);
        }
        
        void VertexTool::renderDragGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
            renderGuide(renderContext, renderBatch, m_dragHandlePosition);
        }
        
        void VertexTool::renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) const {
            m_guideRenderer.setPosition(position);
            m_guideRenderer.setColor(Color(pref(Preferences::HandleColor), 0.5f));
            renderBatch.add(&m_guideRenderer);
        }

        void VertexTool::renderHandles(const Vec3::List& handles, Renderer::RenderService& renderService, const Color& color) const {
            renderService.setForegroundColor(color);
            renderService.renderPointHandles(VectorUtils::cast<Vec3f>(handles));
        }
        
        void VertexTool::rebuildBrushGeometry() {
            
        }

        bool VertexTool::doActivate() {
            m_vertexHandles.clear();
            m_edgeHandles.clear();
            m_faceHandles.clear();
            
            m_mode = Mode_Move;
            m_changeCount = 0;
            
            const Model::BrushList& brushes = selectedBrushes();
            m_vertexHandles.addHandles(std::begin(brushes), std::end(brushes));
            m_edgeHandles.addHandles(std::begin(brushes), std::end(brushes));
            m_faceHandles.addHandles(std::begin(brushes), std::end(brushes));
            
            bindObservers();
            return true;
        }
        
        bool VertexTool::doDeactivate() {
            unbindObservers();
            m_vertexHandles.clear();
            m_edgeHandles.clear();
            m_faceHandles.clear();
            
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
                vertexCommand->removeHandles(m_vertexHandles);
                vertexCommand->removeHandles(m_edgeHandles);
                vertexCommand->removeHandles(m_faceHandles);
                m_ignoreChangeNotifications = true;
            }
        }
        
        void VertexTool::commandDoneOrUndoFailed(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addHandles(m_vertexHandles);
                vertexCommand->addHandles(m_edgeHandles);
                vertexCommand->addHandles(m_faceHandles);
                vertexCommand->selectNewHandlePositions(m_vertexHandles);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        void VertexTool::commandDoFailedOrUndone(Command::Ptr command) {
            if (isVertexCommand(command)) {
                VertexCommand* vertexCommand = static_cast<VertexCommand*>(command.get());
                vertexCommand->addHandles(m_vertexHandles);
                vertexCommand->addHandles(m_edgeHandles);
                vertexCommand->addHandles(m_faceHandles);
                vertexCommand->selectOldHandlePositions(m_vertexHandles);
                m_ignoreChangeNotifications = false;
                
                if (!m_dragging)
                    rebuildBrushGeometry();
            }
        }
        
        bool VertexTool::isVertexCommand(const Command::Ptr command) const {
            return (command->type() == MoveBrushVerticesCommand::Type ||
                    command->type() == SplitBrushEdgesCommand::Type ||
                    command->type() == SplitBrushFacesCommand::Type ||
                    command->type() == RemoveBrushVerticesCommand::Type);
        }
        
        class VertexTool::AddToHandleManager : public Model::NodeVisitor {
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
            void doVisit(Model::Brush* brush)   { m_handleManager.addHandles(brush); }
        };
        
        class VertexTool::RemoveFromHandleManager : public Model::NodeVisitor {
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
            void doVisit(Model::Brush* brush)   { m_handleManager.removeHandles(brush); }
        };

        void VertexTool::selectionDidChange(const Selection& selection) {
            const Model::NodeList& selectedNodes = selection.selectedNodes();
            AddToHandleManager addVisitor(m_vertexHandles);
            Model::Node::accept(std::begin(selectedNodes), std::end(selectedNodes), addVisitor);
            
            const Model::NodeList& deselectedNodes = selection.deselectedNodes();
            RemoveFromHandleManager removeVisitor(m_vertexHandles);
            Model::Node::accept(std::begin(deselectedNodes), std::end(deselectedNodes), removeVisitor);
        }
        
        void VertexTool::nodesWillChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                RemoveFromHandleManager removeVisitor(m_vertexHandles);
                Model::Node::accept(std::begin(nodes), std::end(nodes), removeVisitor);
            }
        }
        
        void VertexTool::nodesDidChange(const Model::NodeList& nodes) {
            if (!m_ignoreChangeNotifications) {
                AddToHandleManager addVisitor(m_vertexHandles);
                Model::Node::accept(std::begin(nodes), std::end(nodes), addVisitor);
            }
        }
        
        const Model::BrushList& VertexTool::selectedBrushes() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().brushes();
        }

        void VertexTool::resetModeAfterDeselection() {
            if (!m_vertexHandles.anySelected())
                m_mode = Mode_Move;
        }
    }
}
