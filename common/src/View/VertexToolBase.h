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

#ifndef VertexToolBase_h
#define VertexToolBase_h

#include "Disjunction.h"
#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "Model/World.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "View/Lasso.h"
#include "View/MapDocument.h"
#include "View/MoveBrushVerticesCommand.h"
#include "View/MoveBrushEdgesCommand.h"
#include "View/MoveBrushFacesCommand.h"
#include "View/RemoveBrushVerticesCommand.h"
#include "View/RemoveBrushEdgesCommand.h"
#include "View/RemoveBrushFacesCommand.h"
#include "View/Selection.h"
#include "View/Tool.h"
#include "View/VertexCommand.h"
#include "View/VertexHandleManager.h"
#include "View/ViewTypes.h"
#include "AddBrushVerticesCommand.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <algorithm>
#include <cassert>
#include <numeric>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class Grid;
        class Lasso;
        
        template <typename H>
        class VertexToolBase : public Tool {
        public:
            typedef enum {
                MR_Continue,
                MR_Deny,
                MR_Cancel
            } MoveResult;
        protected:
            MapDocumentWPtr m_document;
        private:
            size_t m_changeCount;
        protected:
            Disjunction m_ignoreChangeNotifications;
            
            H m_dragHandlePosition;
            bool m_dragging;
        protected:
            VertexToolBase(MapDocumentWPtr document) :
            Tool(false),
            m_document(document),
            m_changeCount(0),
            m_dragging(false) {}
        public:
            virtual ~VertexToolBase() override {}
        public:
            const Grid& grid() const {
                return lock(m_document)->grid();
            }

            const Model::BrushList& selectedBrushes() const {
                MapDocumentSPtr document = lock(m_document);
                return document->selectedNodes().brushes();
            }
        public:
            template <typename M, typename I>
            std::map<typename M::Handle, Model::BrushSet> buildBrushMap(const M& manager, I cur, I end) const {
                typedef typename M::Handle H2;
                std::map<H2, Model::BrushSet> result;
                while (cur != end) {
                    const H2& handle = *cur++;
                    result[handle] = findIncidentBrushes(manager, handle);
                }
                return result;
            }

            template <typename M, typename H2>
            Model::BrushSet findIncidentBrushes(const M& manager, const H2& handle) const {
                const Model::BrushList& brushes = selectedBrushes();
                return manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes));
            }

            template <typename M, typename I>
            Model::BrushSet findIncidentBrushes(const M& manager, I cur, I end) const {
                const Model::BrushList& brushes = selectedBrushes();
                Model::BrushSet result;
                auto out = std::inserter(result, std::end(result));
                
                std::for_each(cur, end, [&manager, &brushes, &out](const auto& handle) {
                    manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes), out);
                });

                return result;
            }

            virtual void pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const = 0;
        public: // Handle selection
            bool select(const Model::Hit::List& hits, const bool addToSelection) {
                assert(!hits.empty());
                const Model::Hit& firstHit = hits.front();
                if (firstHit.type() == handleManager().hitType()) {
                    if (!addToSelection)
                        handleManager().deselectAll();
                    
                    // Count the number of hit handles which are selected already.
                    const size_t selected = std::accumulate(std::begin(hits), std::end(hits), 0u, [this](const size_t cur, const Model::Hit& hit) {
                        const H& handle = hit.target<H>();
                        const bool curSelected = handleManager().selected(handle);
                        return cur + (curSelected ? 1 : 0);
                    });
                    
                    if (selected < hits.size()) {
                        for (const auto& hit : hits)
                            handleManager().select(hit.target<H>());
                    } else if (addToSelection) {
                        // The user meant to deselect a selected handle.
                        for (const auto& hit : hits)
                            handleManager().deselect(hit.target<H>());
                    }
                }
                refreshViews();
                return true;
            }
            
            void select(const Lasso& lasso, const bool modifySelection) {
                typedef std::vector<H> HandleList;
                
                const HandleList allHandles = handleManager().allHandles();
                HandleList selectedHandles;
                
                lasso.selected(std::begin(allHandles), std::end(allHandles), std::back_inserter(selectedHandles));
                if (!modifySelection)
                    handleManager().deselectAll();
                handleManager().toggle(std::begin(selectedHandles), std::end(selectedHandles));
            }

            bool selected(const Model::Hit& hit) const {
                const H& handle = hit.target<H>();
                return handleManager().selected(handle);
            }

            virtual bool deselectAll() {
                if (handleManager().anySelected()) {
                    handleManager().deselectAll();
                    refreshViews();
                    return true;
                }
                return false;
            }
        public:
            typedef VertexHandleManagerBaseT<H> HandleManager;
            virtual HandleManager& handleManager() = 0;
            virtual const HandleManager& handleManager() const = 0;
        public: // performing moves
            virtual bool startMove(const Model::Hit::List& hits) {
                assert(!hits.empty());

                // Delesect all handles if any of the hit handles is not already selected.
                if (std::any_of(std::begin(hits), std::end(hits), [&](const auto& hit) {
                    const H& handle = this->getHandlePosition(hit);
                    return !this->handleManager().selected(handle);
                })) {
                    handleManager().deselectAll();
                }

                // Now select all of the hit handles.
                for (const auto& hit : hits) {
                    const H& handle = getHandlePosition(hit);
                    if (hit.hasType(handleManager().hitType())) {
                        handleManager().select(handle);
                    }
                }
                refreshViews();

                MapDocumentSPtr document = lock(m_document);
                document->beginTransaction(actionName());

                m_dragHandlePosition = getHandlePosition(hits.front());
                m_dragging = true;
                m_ignoreChangeNotifications.pushLiteral();
                return true;
            }
            
            virtual MoveResult move(const vm::vec3& delta) = 0;
            
            virtual void endMove() {
                MapDocumentSPtr document = lock(m_document);
                document->commitTransaction();
                m_dragging = false;
                m_ignoreChangeNotifications.popLiteral();
            }
            
            virtual void cancelMove() {
                MapDocumentSPtr document = lock(m_document);
                document->cancelTransaction();
                m_dragging = false;
                m_ignoreChangeNotifications.popLiteral();
            }

        public: // csg convex merge
            bool canDoCsgConvexMerge() {
                return handleManager().selectedHandleCount() > 1;
            }

            void csgConvexMerge() {
                std::vector<vm::vec3> vertices;
                const auto handles = handleManager().selectedHandles();
                H::getVertices(std::begin(handles), std::end(handles), std::back_inserter(vertices));

                const Polyhedron3 polyhedron(vertices);
                if (!polyhedron.polyhedron() || !polyhedron.closed()) {
                    return;
                }

                MapDocumentSPtr document = lock(m_document);
                const Model::BrushBuilder builder(document->world(), document->worldBounds());
                auto* brush = builder.createBrush(polyhedron, document->currentTextureName());
                brush->cloneFaceAttributesFrom(document->selectedNodes().brushes());

                const Transaction transaction(document, "CSG Convex Merge");
                deselectAll();
                document->addNode(brush, document->currentParent());
            }

            virtual const H& getHandlePosition(const Model::Hit& hit) const {
                assert(hit.isMatch());
                assert(hit.hasType(handleManager().hitType()));
                return hit.target<H>();
            }
            
            virtual String actionName() const = 0;
        public:
            void moveSelection(const vm::vec3& delta) {
                const Disjunction::TemporarilySetLiteral ignoreChangeNotifications(m_ignoreChangeNotifications);

                Transaction transaction(m_document, actionName());
                move(delta);
            }
            
            bool canRemoveSelection() const {
                return handleManager().selectedHandleCount() > 0;
            }
        public: // rendering
            void renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
                Renderer::RenderService renderService(renderContext, renderBatch);
                if (!handleManager().allSelected()) {
                    renderHandles(handleManager().unselectedHandles(), renderService, pref(Preferences::HandleColor));
                }
                if (handleManager().anySelected()) {
                    renderHandles(handleManager().selectedHandles(), renderService, pref(Preferences::SelectedHandleColor));
                }
            }
            
            void renderDragHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
                renderHandle(renderContext, renderBatch, m_dragHandlePosition, pref(Preferences::SelectedHandleColor));
            }
            
            template <typename HH>
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HH& handle) const {
                renderHandle(renderContext, renderBatch, handle, pref(Preferences::HandleColor));
            }
            
            void renderDragHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
                renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
            }
            
            void renderDragGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const {
                renderGuide(renderContext, renderBatch, m_dragHandlePosition);
            }

            template <typename HH>
            void renderHandles(const std::vector<HH>& handles, Renderer::RenderService& renderService, const Color& color) const {
                renderService.setForegroundColor(color);
                renderService.renderHandles(VectorUtils::cast<typename HH::float_type>(handles));
            }
            
            template <typename HH>
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HH& handle, const Color& color) const {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(color);
                renderService.renderHandle(typename HH::float_type(handle));
            }
            
            template <typename HH>
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HH& handle) const {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderHandleHighlight(typename HH::float_type(handle));
            }
            
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& handle) const {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
                renderService.renderHandleHighlight(vm::vec3f(handle));
                
                renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
                renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
                renderService.renderString(StringUtils::toString(handle), vm::vec3f(handle));
            }

            template <typename HH>
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const HH& position) const {}
            
            virtual void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& position) const {}
        protected: // Tool interface
            virtual bool doActivate() override {
                m_changeCount = 0;
                bindObservers();
                
                const Model::BrushList& brushes = selectedBrushes();
                handleManager().clear();
                handleManager().addHandles(std::begin(brushes), std::end(brushes));

                return true;
            }
            
            virtual bool doDeactivate() override {
                unbindObservers();
                handleManager().clear();
                return true;
            }
        private: // Observers and state management
            void bindObservers() {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.addObserver(this,  &VertexToolBase::selectionDidChange);
                document->nodesWillChangeNotifier.addObserver(this,  &VertexToolBase::nodesWillChange);
                document->nodesDidChangeNotifier.addObserver(this,  &VertexToolBase::nodesDidChange);
                document->commandDoNotifier.addObserver(this,  &VertexToolBase::commandDo);
                document->commandDoneNotifier.addObserver(this,  &VertexToolBase::commandDone);
                document->commandDoFailedNotifier.addObserver(this,  &VertexToolBase::commandDoFailed);
                document->commandUndoNotifier.addObserver(this,  &VertexToolBase::commandUndo);
                document->commandUndoneNotifier.addObserver(this,  &VertexToolBase::commandUndone);
                document->commandUndoFailedNotifier.addObserver(this,  &VertexToolBase::commandUndoFailed);
            }
            
            void unbindObservers() {
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    document->selectionDidChangeNotifier.removeObserver(this,  &VertexToolBase::selectionDidChange);
                    document->nodesWillChangeNotifier.removeObserver(this,  &VertexToolBase::nodesWillChange);
                    document->nodesDidChangeNotifier.removeObserver(this,  &VertexToolBase::nodesDidChange);
                    document->commandDoNotifier.removeObserver(this,  &VertexToolBase::commandDo);
                    document->commandDoneNotifier.removeObserver(this,  &VertexToolBase::commandDone);
                    document->commandDoFailedNotifier.removeObserver(this,  &VertexToolBase::commandDoFailed);
                    document->commandUndoNotifier.removeObserver(this,  &VertexToolBase::commandUndo);
                    document->commandUndoneNotifier.removeObserver(this,  &VertexToolBase::commandUndone);
                    document->commandUndoFailedNotifier.removeObserver(this,  &VertexToolBase::commandUndoFailed);
                }
            }
            
            void commandDo(Command::Ptr command) {
                commandDoOrUndo(command);
            }
            
            void commandDone(Command::Ptr command) {
                commandDoneOrUndoFailed(command);
            }
            
            void commandDoFailed(Command::Ptr command) {
                commandDoFailedOrUndone(command);
            }
            
            void commandUndo(UndoableCommand::Ptr command) {
                commandDoOrUndo(command);
            }
            
            void commandUndone(UndoableCommand::Ptr command) {
                commandDoFailedOrUndone(command);
            }
            
            void commandUndoFailed(UndoableCommand::Ptr command) {
                commandDoneOrUndoFailed(command);
            }
            
            void commandDoOrUndo(Command::Ptr command) {
                if (isVertexCommand(command)) {
                    auto* vertexCommand = static_cast<VertexCommand*>(command.get());
                    deselectHandles();
                    removeHandles(vertexCommand);
                    m_ignoreChangeNotifications.pushLiteral();
                }
            }
            
            void commandDoneOrUndoFailed(Command::Ptr command) {
                if (isVertexCommand(command)) {
                    auto* vertexCommand = static_cast<VertexCommand*>(command.get());
                    addHandles(vertexCommand);
                    selectNewHandlePositions(vertexCommand);
                    m_ignoreChangeNotifications.popLiteral();
                }
            }
            
            void commandDoFailedOrUndone(Command::Ptr command) {
                if (isVertexCommand(command)) {
                    auto* vertexCommand = static_cast<VertexCommand*>(command.get());
                    addHandles(vertexCommand);
                    selectOldHandlePositions(vertexCommand);
                    m_ignoreChangeNotifications.popLiteral();
                }
            }
            
            bool isVertexCommand(const Command::Ptr command) const {
                return command->isType(
                        AddBrushVerticesCommand::Type,
                        RemoveBrushVerticesCommand::Type,
                        RemoveBrushEdgesCommand::Type,
                        RemoveBrushFacesCommand::Type,
                        MoveBrushVerticesCommand::Type,
                        MoveBrushEdgesCommand::Type,
                        MoveBrushFacesCommand::Type
                );
            }
            
            void selectionDidChange(const Selection& selection) {
                addHandles(selection.selectedNodes());
                removeHandles(selection.deselectedNodes());
            }
            
            void nodesWillChange(const Model::NodeList& nodes) {
                if (!m_ignoreChangeNotifications) {
                    removeHandles(nodes);
                }
            }
            
            void nodesDidChange(const Model::NodeList& nodes) {
                if (!m_ignoreChangeNotifications) {
                    addHandles(nodes);
                }
            }
        protected:
            virtual void addHandles(VertexCommand* command) {
                command->addHandles(handleManager());
            }
            
            virtual void removeHandles(VertexCommand* command) {
                command->removeHandles(handleManager());
            }

            virtual void deselectHandles() {
                handleManager().deselectAll();
            }

            virtual void selectNewHandlePositions(VertexCommand* command) {
                command->selectNewHandlePositions(handleManager());
            }
            
            virtual void selectOldHandlePositions(VertexCommand* command) {
                command->selectOldHandlePositions(handleManager());
            }
            
            template <typename HT>
            class AddHandles : public Model::NodeVisitor {
            private:
                VertexHandleManagerBaseT<HT>& m_handles;
            public:
                AddHandles(VertexHandleManagerBaseT<HT>& handles) :
                m_handles(handles) {}
            private:
                void doVisit(Model::World* world) override  {}
                void doVisit(Model::Layer* layer) override   {}
                void doVisit(Model::Group* group) override   {}
                void doVisit(Model::Entity* entity) override {}
                void doVisit(Model::Brush* brush) override   {
                    m_handles.addHandles(brush);
                }
            };
            
            template <typename HT>
            class RemoveHandles : public Model::NodeVisitor {
            private:
                VertexHandleManagerBaseT<HT>& m_handles;
            public:
                RemoveHandles(VertexHandleManagerBaseT<HT>& handles) :
                m_handles(handles) {}
            private:
                void doVisit(Model::World* world) override   {}
                void doVisit(Model::Layer* layer) override   {}
                void doVisit(Model::Group* group) override   {}
                void doVisit(Model::Entity* entity) override {}
                void doVisit(Model::Brush* brush) override   {
                    m_handles.removeHandles(brush);
                }
            };
            
            virtual void addHandles(const Model::NodeList& nodes) {
                AddHandles<H> addVisitor(handleManager());
                Model::Node::accept(std::begin(nodes), std::end(nodes), addVisitor);
            }
            
            virtual void removeHandles(const Model::NodeList& nodes) {
                RemoveHandles<H> removeVisitor(handleManager());
                Model::Node::accept(std::begin(nodes), std::end(nodes), removeVisitor);
            }
        };
    }
}

#endif /* VertexToolBase_h */
