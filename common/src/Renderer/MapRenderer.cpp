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

#include "MapRenderer.h"

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/ObjectRenderer.h"
#include "Renderer/RenderUtils.h"
#include "View/Selection.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        class SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            SelectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !locked(brush) && (selected(brush) || hasSelectedFaces(brush)) && visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !locked(face) && (selected(face) || selected(face->brush())) && visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return selected(edge);
            }
        };

        class UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            UnselectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !locked(brush) && !selected(brush) && visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !locked(face) && !selected(face) && visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return !selected(edge);
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_selectionRenderer(lock(m_document)->entityModelManager(), lock(m_document)->editorContext(), SelectedBrushRendererFilter(lock(m_document)->editorContext())) {
            bindObservers();
        }

        MapRenderer::~MapRenderer() {
            unbindObservers();
            clear();
        }

        void MapRenderer::clear() {
            MapUtils::clearAndDelete(m_layerRenderers);
        }

        void MapRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            commitPendingChanges();
            setupGL(renderBatch);
            renderLayers(renderContext, renderBatch);
            renderSelection(renderContext, renderBatch);
        }
        
        void MapRenderer::commitPendingChanges() {
            View::MapDocumentSPtr document = lock(m_document);
            document->commitPendingAssets();
        }

        class SetupGL : public Renderable {
        private:
            void doPrepare(Vbo& vbo) {}
            void doRender(RenderContext& renderContext) {
                glFrontFace(GL_CW);
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glResetEdgeOffset();
            }
        };
        
        void MapRenderer::setupGL(RenderBatch& renderBatch) {
            renderBatch.addOneShot(new SetupGL());
        }
        
        void MapRenderer::renderLayers(RenderContext& renderContext, RenderBatch& renderBatch) {
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                // const Model::Layer* layer = it->first;
                ObjectRenderer* renderer = it->second;
                setupLayerRenderer(renderer);
                renderer->render(renderContext, renderBatch);
            }
        }
        
        void MapRenderer::setupLayerRenderer(ObjectRenderer* renderer) {
            PreferenceManager& prefs = PreferenceManager::instance();

            renderer->setOverlayTextColor(prefs.get(Preferences::InfoOverlayTextColor));
            renderer->setOverlayBackgroundColor(prefs.get(Preferences::InfoOverlayBackgroundColor));
            renderer->setTint(false);
            renderer->setTransparencyAlpha(prefs.get(Preferences::TransparentFaceAlpha));

            renderer->setEntityBoundsColor(prefs.get(Preferences::UndefinedEntityColor));

            renderer->setBrushFaceColor(prefs.get(Preferences::FaceColor));
            renderer->setBrushEdgeColor(prefs.get(Preferences::EdgeColor));
        }

        void MapRenderer::renderSelection(RenderContext& renderContext, RenderBatch& renderBatch) {
            setupSelectionRenderer(&m_selectionRenderer);
            m_selectionRenderer.render(renderContext, renderBatch);
        }
        
        void MapRenderer::setupSelectionRenderer(ObjectRenderer* renderer) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            renderer->setOverlayTextColor(prefs.get(Preferences::SelectedInfoOverlayTextColor));
            renderer->setOverlayBackgroundColor(prefs.get(Preferences::SelectedInfoOverlayBackgroundColor));
            renderer->setShowOccludedObjects(true);
            renderer->setOccludedEdgeColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
            renderer->setTint(true);
            renderer->setTintColor(prefs.get(Preferences::SelectedFaceColor));
            renderer->setTransparencyAlpha(prefs.get(Preferences::TransparentFaceAlpha));

            renderer->setOverrideEntityBoundsColor(true);
            renderer->setEntityBoundsColor(prefs.get(Preferences::SelectedEdgeColor));
            renderer->setShowEntityAngles(true);
            renderer->setEntityAngleColor(prefs.get(Preferences::AngleIndicatorColor));

            renderer->setBrushFaceColor(prefs.get(Preferences::FaceColor));
            renderer->setBrushEdgeColor(prefs.get(Preferences::SelectedEdgeColor));
        }

        void MapRenderer::bindObservers() {
            assert(!expired(m_document));
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->selectionDidChangeNotifier.addObserver(this, &MapRenderer::selectionDidChange);
        }
        
        void MapRenderer::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MapRenderer::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->selectionDidChangeNotifier.removeObserver(this, &MapRenderer::selectionDidChange);
            }
        }

        void MapRenderer::documentWasCleared(View::MapDocument* document) {
            m_layerRenderers.clear();
        }
        
        class MapRenderer::AddLayer : public Model::NodeVisitor {
        private:
            Assets::EntityModelManager& m_modelManager;
            const Model::EditorContext& m_editorContext;
            
            RendererMap& m_layerRenderers;
        public:
            AddLayer(Assets::EntityModelManager& modelManager, const Model::EditorContext& editorContext, RendererMap& layerRenderers) :
            m_modelManager(modelManager),
            m_editorContext(editorContext),
            m_layerRenderers(layerRenderers) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {
                ObjectRenderer* renderer = new ObjectRenderer(m_modelManager, m_editorContext, UnselectedBrushRendererFilter(m_editorContext));
                MapUtils::insertOrFail(m_layerRenderers, layer, renderer);
                renderer->addObjects(layer->children());
                stopRecursion(); // don't visit my children
            }
            void doVisit(Model::Group* group)   { assert(false); }
            void doVisit(Model::Entity* entity) { assert(false); }
            void doVisit(Model::Brush* brush)   { assert(false); }
        };
        
        void MapRenderer::documentWasNewedOrLoaded(View::MapDocument* document) {
            Model::World* world = document->world();
            Assets::EntityModelManager& modelManager = document->entityModelManager();
            const Model::EditorContext& editorContext = document->editorContext();
            AddLayer visitor(modelManager, editorContext, m_layerRenderers);
            world->acceptAndRecurse(visitor);
        }

        class MapRenderer::HandleSelectedNode : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer& m_selectionRenderer;
        public:
            HandleSelectedNode(RendererMap& layerRenderers, ObjectRenderer& selectionRenderer) :
            m_layerRenderers(layerRenderers),
            m_selectionRenderer(selectionRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            
            void doVisit(Model::Group* group) {
                Model::Layer* layer = group->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (group->selected() || group->descendantSelected()) {
                    layerRenderer->removeObject(group);
                    m_selectionRenderer.addObject(group);
                } else {
                    m_selectionRenderer.removeObject(group);
                    layerRenderer->addObject(group);
                }
            }
            
            void doVisit(Model::Entity* entity) {
                Model::Layer* layer = entity->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (entity->selected() || entity->descendantSelected()) {
                    layerRenderer->removeObject(entity);
                    m_selectionRenderer.addObject(entity);
                } else {
                    m_selectionRenderer.removeObject(entity);
                    layerRenderer->addObject(entity);
                }
            }
            
            void doVisit(Model::Brush* brush) {
                Model::Layer* layer = brush->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (brush->selected())
                    layerRenderer->removeObject(brush);
                else
                    layerRenderer->addObject(brush);
                if (brush->selected() || brush->descendantSelected())
                    m_selectionRenderer.addObject(brush);
                else
                    m_selectionRenderer.removeObject(brush);
            }
        };
        
        class MapRenderer::UpdateNode : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer& m_selectionRenderer;
        public:
            UpdateNode(RendererMap& layerRenderers, ObjectRenderer& selectionRenderer) :
            m_layerRenderers(layerRenderers),
            m_selectionRenderer(selectionRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   { handleNode(group, group->layer()); }
            void doVisit(Model::Entity* entity) { handleNode(entity, entity->layer()); }
            void doVisit(Model::Brush* brush)   { handleNode(brush, brush->layer()); }
            
            void handleNode(Model::Node* node, Model::Layer* layer) {
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (node->selected() || node->descendantSelected())
                    m_selectionRenderer.updateObject(node);
                if (!node->selected())
                    layerRenderer->updateObject(node);
            }
        };
        
        void MapRenderer::selectionDidChange(const View::Selection& selection) {
            HandleSelectedNode handleSelectedNode(m_layerRenderers, m_selectionRenderer);
            Model::Node::accept(selection.partiallySelectedNodes().begin(), selection.partiallySelectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.partiallyDeselectedNodes().begin(), selection.partiallyDeselectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.selectedNodes().begin(), selection.selectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.deselectedNodes().begin(), selection.deselectedNodes().end(), handleSelectedNode);

            UpdateNode updateNode(m_layerRenderers, m_selectionRenderer);
            
            const Model::BrushSet parentsOfSelectedFaces = collectBrushes(selection.selectedBrushFaces());
            Model::Node::accept(parentsOfSelectedFaces.begin(), parentsOfSelectedFaces.end(), updateNode);

            const Model::BrushSet parentsOfDeselectedFaces = collectBrushes(selection.deselectedBrushFaces());
            Model::Node::accept(parentsOfDeselectedFaces.begin(), parentsOfDeselectedFaces.end(), updateNode);
        }

        Model::BrushSet MapRenderer::collectBrushes(const Model::BrushFaceList& faces) {
            Model::BrushSet result;
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->brush();
                result.insert(brush);
            }
            return result;
        }
    }
}
