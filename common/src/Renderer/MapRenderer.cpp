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
#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityLinkRenderer.h"
#include "Renderer/ObjectRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "View/Selection.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        class MapRenderer::SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            SelectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
            
            bool doShow(const Model::Brush* brush) const {
                return editable(brush) && (selected(brush) || hasSelectedFaces(brush)) && visible(brush);
            }
            
            bool doShow(const Model::BrushFace* face) const {
                return editable(face) && (selected(face) || selected(face->brush())) && visible(face);
            }
            
            bool doShow(const Model::BrushEdge* edge) const {
                return selected(edge);
            }
            
            bool doIsTransparent(const Model::Brush* brush) const {
                return false;
            }
        };
        
        class MapRenderer::LockedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            LockedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
            
            bool doShow(const Model::Brush* brush) const {
                return !editable(brush);
            }
            
            bool doShow(const Model::BrushFace* face) const {
                return true;
            }
            
            bool doShow(const Model::BrushEdge* edge) const {
                return true;
            }
            
            bool doIsTransparent(const Model::Brush* brush) const {
                return brush->transparent();
            }
        };

        class MapRenderer::UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            UnselectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
            
            bool doShow(const Model::Brush* brush) const {
                return editable(brush) && !selected(brush) && visible(brush);
            }
            
            bool doShow(const Model::BrushFace* face) const {
                return editable(face) && !selected(face) && visible(face);
            }
            
            bool doShow(const Model::BrushEdge* edge) const {
                return !selected(edge);
            }
            
            bool doIsTransparent(const Model::Brush* brush) const {
                return brush->transparent();
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_selectionRenderer(createSelectionRenderer(m_document)),
        m_lockedRenderer(createLockRenderer(m_document)),
        m_entityLinkRenderer(new EntityLinkRenderer(m_document)) {
            bindObservers();
            setupRenderers();
        }

        MapRenderer::~MapRenderer() {
            unbindObservers();
            clear();
            delete m_lockedRenderer;
            delete m_selectionRenderer;
            delete m_entityLinkRenderer;
        }

        ObjectRenderer* MapRenderer::createSelectionRenderer(View::MapDocumentWPtr document) {
            return new ObjectRenderer(lock(document)->entityModelManager(),
                                      lock(document)->editorContext(),
                                      SelectedBrushRendererFilter(lock(document)->editorContext()));
        }
        
        ObjectRenderer* MapRenderer::createLockRenderer(View::MapDocumentWPtr document) {
            return new ObjectRenderer(lock(document)->entityModelManager(),
                                      lock(document)->editorContext(),
                                      LockedBrushRendererFilter(lock(document)->editorContext()));
        }

        void MapRenderer::clear() {
            MapUtils::clearAndDelete(m_layerRenderers);
        }

        void MapRenderer::overrideSelectionColors(const Color& color, const float mix) {
            const Color edgeColor = pref(Preferences::SelectedEdgeColor).mixed(color, mix);
            const Color occludedEdgeColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);
            const Color tintColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);
            
            m_selectionRenderer->setEntityBoundsColor(edgeColor);
            m_selectionRenderer->setBrushEdgeColor(edgeColor);
            m_selectionRenderer->setOccludedEdgeColor(occludedEdgeColor);
            m_selectionRenderer->setTintColor(tintColor);
        }
        
        void MapRenderer::restoreSelectionColors() {
            setupSelectionRenderer(m_selectionRenderer);
        }

        void MapRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            commitPendingChanges();
            setupGL(renderBatch);
            renderLayers(renderContext, renderBatch);
            renderSelection(renderContext, renderBatch);
            renderLocked(renderContext, renderBatch);
            renderEntityLinks(renderContext, renderBatch);
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
            const Model::EditorContext& editorContext = lock(m_document)->editorContext();
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                const Model::Layer* layer = it->first;
                if (editorContext.visible(layer)) {
                    ObjectRenderer* renderer = it->second;
                    renderer->render(renderContext, renderBatch);
                }
            }
        }
        
        void MapRenderer::renderSelection(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!renderContext.hideSelection())
                m_selectionRenderer->render(renderContext, renderBatch);
        }
        
        void MapRenderer::renderLocked(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_lockedRenderer->render(renderContext, renderBatch);
        }

        void MapRenderer::renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_entityLinkRenderer->render(renderContext, renderBatch);
        }

        void MapRenderer::setupRenderers() {
            setupLayerRenderers();
            setupSelectionRenderer(m_selectionRenderer);
            setupLockedRenderer(m_lockedRenderer);
            setupEntityLinkRenderer();
        }

        void MapRenderer::setupLayerRenderers() {
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                ObjectRenderer* renderer = it->second;
                setupLayerRenderer(renderer);
            }
        }

        void MapRenderer::setupLayerRenderer(ObjectRenderer* renderer) {
            renderer->setOverlayTextColor(pref(Preferences::InfoOverlayTextColor));
            renderer->setOverlayBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
            renderer->setTint(false);
            renderer->setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

            renderer->setGroupBoundsColor(pref(Preferences::DefaultGroupColor));
            renderer->setEntityBoundsColor(pref(Preferences::UndefinedEntityColor));
            
            renderer->setBrushFaceColor(pref(Preferences::FaceColor));
            renderer->setBrushEdgeColor(pref(Preferences::EdgeColor));
        }
        
        void MapRenderer::setupSelectionRenderer(ObjectRenderer* renderer) {
            renderer->setOverlayTextColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderer->setOverlayBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderer->setShowOccludedObjects(true);
            renderer->setOccludedEdgeColor(pref(Preferences::OccludedSelectedEdgeColor));
            renderer->setTint(true);
            renderer->setTintColor(pref(Preferences::SelectedFaceColor));

            renderer->setOverrideGroupBoundsColor(true);
            renderer->setGroupBoundsColor(pref(Preferences::SelectedEdgeColor));
            
            renderer->setOverrideEntityBoundsColor(true);
            renderer->setEntityBoundsColor(pref(Preferences::SelectedEdgeColor));
            renderer->setShowEntityAngles(true);
            renderer->setEntityAngleColor(pref(Preferences::AngleIndicatorColor));

            renderer->setBrushFaceColor(pref(Preferences::FaceColor));
            renderer->setBrushEdgeColor(pref(Preferences::SelectedEdgeColor));
        }

        void MapRenderer::setupLockedRenderer(ObjectRenderer* renderer) {
            renderer->setOverlayTextColor(pref(Preferences::LockedInfoOverlayTextColor));
            renderer->setOverlayBackgroundColor(pref(Preferences::LockedInfoOverlayBackgroundColor));
            renderer->setShowOccludedObjects(false);
            renderer->setTint(true);
            renderer->setTintColor(pref(Preferences::LockedFaceColor));
            renderer->setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));
            
            renderer->setOverrideGroupBoundsColor(true);
            renderer->setGroupBoundsColor(pref(Preferences::LockedEdgeColor));
            
            renderer->setOverrideEntityBoundsColor(true);
            renderer->setEntityBoundsColor(pref(Preferences::LockedEdgeColor));
            renderer->setShowEntityAngles(false);
            
            renderer->setBrushFaceColor(pref(Preferences::FaceColor));
            renderer->setBrushEdgeColor(pref(Preferences::LockedEdgeColor));
        }

        void MapRenderer::setupEntityLinkRenderer() {
        }

        void MapRenderer::invalidateLayerRenderers() {
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                ObjectRenderer* renderer = it->second;
                renderer->invalidate();
            }
        }
        
        void MapRenderer::invalidateSelectionRenderer() {
            m_selectionRenderer->invalidate();
        }
        
        void MapRenderer::invalidateLockedRenderer() {
            m_lockedRenderer->invalidate();
        }
        
        void MapRenderer::invalidateEntityLinkRenderer() {
            m_entityLinkRenderer->invalidate();
        }

        void MapRenderer::bindObservers() {
            assert(!expired(m_document));
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->nodesWereAddedNotifier.addObserver(this, &MapRenderer::nodesWereAdded);
            document->nodesWillBeRemovedNotifier.addObserver(this, &MapRenderer::nodesWillBeRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &MapRenderer::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &MapRenderer::nodeVisibilityDidChange);
            document->nodeLockingDidChangeNotifier.addObserver(this, &MapRenderer::nodeLockingDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &MapRenderer::brushFacesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapRenderer::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &MapRenderer::textureCollectionsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapRenderer::entityDefinitionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapRenderer::modsDidChange);
            document->editorContextDidChangeNotifier.addObserver(this, &MapRenderer::editorContextDidChange);
            document->mapViewConfigDidChangeNotifier.addObserver(this, &MapRenderer::mapViewConfigDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapRenderer::preferenceDidChange);
        }
        
        void MapRenderer::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MapRenderer::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->nodesWereAddedNotifier.removeObserver(this, &MapRenderer::nodesWereAdded);
                document->nodesWillBeRemovedNotifier.removeObserver(this, &MapRenderer::nodesWillBeRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &MapRenderer::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &MapRenderer::nodeVisibilityDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &MapRenderer::nodeLockingDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &MapRenderer::brushFacesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapRenderer::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &MapRenderer::textureCollectionsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapRenderer::entityDefinitionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapRenderer::modsDidChange);
                document->editorContextDidChangeNotifier.removeObserver(this, &MapRenderer::editorContextDidChange);
                document->mapViewConfigDidChangeNotifier.removeObserver(this, &MapRenderer::mapViewConfigDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapRenderer::preferenceDidChange);
        }

        class MapRenderer::HandleSelectedNode : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer* m_selectionRenderer;
            ObjectRenderer* m_lockedRenderer;
        public:
            HandleSelectedNode(RendererMap& layerRenderers, ObjectRenderer* selectionRenderer, ObjectRenderer* lockedRenderer) :
            m_layerRenderers(layerRenderers),
            m_selectionRenderer(selectionRenderer),
            m_lockedRenderer(lockedRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            
            void doVisit(Model::Group* group) {
                Model::Layer* layer = group->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (group->selected() || group->descendantSelected() || group->parentSelected()) {
                    layerRenderer->removeObject(group);
                    m_selectionRenderer->addObject(group);
                } else {
                    m_selectionRenderer->removeObject(group);
                    if (group->locked())
                        m_lockedRenderer->addObject(group);
                    else
                        layerRenderer->addObject(group);
                }
            }
            
            void doVisit(Model::Entity* entity) {
                Model::Layer* layer = entity->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (entity->selected() || entity->descendantSelected() || entity->parentSelected()) {
                    layerRenderer->removeObject(entity);
                    m_selectionRenderer->addObject(entity);
                } else {
                    m_selectionRenderer->removeObject(entity);
                    if (entity->locked())
                        m_lockedRenderer->addObject(entity);
                    else
                        layerRenderer->addObject(entity);
                }
            }
            
            void doVisit(Model::Brush* brush) {
                Model::Layer* layer = brush->layer();
                
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                
                if (brush->selected() || brush->parentSelected())
                    layerRenderer->removeObject(brush);
                else if (brush->locked())
                    m_lockedRenderer->addObject(brush);
                else
                    layerRenderer->addObject(brush);
                
                if (brush->selected() || brush->descendantSelected() || brush->parentSelected())
                    m_selectionRenderer->addObject(brush);
                else
                    m_selectionRenderer->removeObject(brush);
            }
        };
        
        class MapRenderer::AddNode : public Model::NodeVisitor {
        private:
            Assets::EntityModelManager& m_modelManager;
            const Model::EditorContext& m_editorContext;

            RendererMap& m_layerRenderers;
        public:
            AddNode(Assets::EntityModelManager& modelManager, const Model::EditorContext& editorContext, RendererMap& layerRenderers) :
            m_modelManager(modelManager),
            m_editorContext(editorContext),
            m_layerRenderers(layerRenderers) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {
                ObjectRenderer* renderer = new ObjectRenderer(m_modelManager, m_editorContext, UnselectedBrushRendererFilter(m_editorContext));
                MapUtils::insertOrFail(m_layerRenderers, layer, renderer);
            }
            void doVisit(Model::Group* group)   { handleNode(group, group->layer()); }
            void doVisit(Model::Entity* entity) { handleNode(entity, entity->layer()); }
            void doVisit(Model::Brush* brush)   { handleNode(brush, brush->layer()); }
            
            void handleNode(Model::Node* node, Model::Layer* layer) {
                assert(layer != NULL);
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                layerRenderer->addObject(node);
            }
        };
        
        void MapRenderer::documentWasCleared(View::MapDocument* document) {
            m_layerRenderers.clear();
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::documentWasNewedOrLoaded(View::MapDocument* document) {
            Model::World* world = document->world();
            AddNode visitor(document->entityModelManager(), document->editorContext(), m_layerRenderers);
            world->acceptAndRecurse(visitor);
            setupLayerRenderers();
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::nodesWereAdded(const Model::NodeList& nodes) {
            View::MapDocumentSPtr document = lock(m_document);
            AddNode visitor(document->entityModelManager(), document->editorContext(), m_layerRenderers);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
            setupLayerRenderers();
            invalidateEntityLinkRenderer();
        }
        
        class MapRenderer::RemoveNode : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
        public:
            RemoveNode(RendererMap& layerRenderers) :
            m_layerRenderers(layerRenderers) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   { CHECK_BOOL(MapUtils::removeAndDelete(m_layerRenderers, layer)); }
            void doVisit(Model::Group* group)   { handleNode(group, group->layer()); }
            void doVisit(Model::Entity* entity) { handleNode(entity, entity->layer()); }
            void doVisit(Model::Brush* brush)   { handleNode(brush, brush->layer()); }
            
            void handleNode(Model::Node* node, Model::Layer* layer) {
                assert(layer != NULL);
                ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                assert(layerRenderer != NULL);
                layerRenderer->removeObject(node);
            }
        };

        void MapRenderer::nodesWillBeRemoved(const Model::NodeList& nodes) {
            RemoveNode visitor(m_layerRenderers);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
            setupLayerRenderers();
            invalidateEntityLinkRenderer();
        }

        class MapRenderer::UpdateNode : public Model::NodeVisitor {
        private:
            ObjectRenderer* m_selectionRenderer;
        public:
            UpdateNode(ObjectRenderer* selectionRenderer) :
            m_selectionRenderer(selectionRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   { handleNode(group); }
            void doVisit(Model::Entity* entity) { handleNode(entity); }
            void doVisit(Model::Brush* brush)   { handleNode(brush); }
            
            void handleNode(Model::Node* node) {
                // assert(node->selected() || node->descendantSelected());
                if (node->selected() || node->descendantSelected())
                    m_selectionRenderer->updateObject(node);
            }
        };
        
        void MapRenderer::nodesDidChange(const Model::NodeList& nodes) {
            MapRenderer::UpdateNode visitor(m_selectionRenderer);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
            invalidateEntityLinkRenderer();
        }

        class MapRenderer::UpdateVisibility : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer* m_lockedRenderer;
        public:
            UpdateVisibility(RendererMap& layerRenderers, ObjectRenderer* lockedRenderer) :
            m_layerRenderers(layerRenderers),
            m_lockedRenderer(lockedRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   { handleNode(group, group->layer()); }
            void doVisit(Model::Entity* entity) { handleNode(entity, entity->layer()); }
            void doVisit(Model::Brush* brush)   { handleNode(brush, brush->layer()); }
            
            void handleNode(Model::Node* node, Model::Layer* layer) {
                assert(layer != NULL);
                if (!node->selected() && !node->descendantSelected()) {
                    ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                    assert(layerRenderer != NULL);
                    if (node->hidden())
                        layerRenderer->removeObject(node);
                    else if (node->locked())
                        m_lockedRenderer->addObject(node);
                    else
                        layerRenderer->addObject(node);
                }
            }
        };

        void MapRenderer::nodeVisibilityDidChange(const Model::NodeList& nodes) {
            MapRenderer::UpdateVisibility visitor(m_layerRenderers, m_lockedRenderer);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
            invalidateEntityLinkRenderer();
        }
        
        class MapRenderer::UpdateLocking : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer* m_lockedRenderer;
        public:
            UpdateLocking(RendererMap& layerRenderers, ObjectRenderer* lockedRenderer) :
            m_layerRenderers(layerRenderers),
            m_lockedRenderer(lockedRenderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   { handleNode(group, group->layer()); }
            void doVisit(Model::Entity* entity) { handleNode(entity, entity->layer()); }
            void doVisit(Model::Brush* brush)   { handleNode(brush, brush->layer()); }
            
            void handleNode(Model::Node* node, Model::Layer* layer) {
                assert(layer != NULL);
                if (!node->selected() && !node->descendantSelected()) {
                    ObjectRenderer* layerRenderer = MapUtils::find(m_layerRenderers, layer, static_cast<ObjectRenderer*>(NULL));
                    assert(layerRenderer != NULL);
                    if (node->locked()) {
                        layerRenderer->removeObject(node);
                        m_lockedRenderer->addObject(node);
                    } else {
                        m_lockedRenderer->removeObject(node);
                        layerRenderer->addObject(node);
                    }
                }
            }
        };
        
        void MapRenderer::nodeLockingDidChange(const Model::NodeList& nodes) {
            MapRenderer::UpdateLocking visitor(m_layerRenderers, m_lockedRenderer);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
            invalidateEntityLinkRenderer();
        }

        void MapRenderer::brushFacesDidChange(const Model::BrushFaceList& faces) {
            m_selectionRenderer->updateBrushFaces(faces);
        }

        class MapRenderer::UpdateSelectedNode : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
            ObjectRenderer* m_selectionRenderer;
        public:
            UpdateSelectedNode(RendererMap& layerRenderers, ObjectRenderer* selectionRenderer) :
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
                
                if (node->selected() || node->descendantSelected() || node->parentSelected())
                    m_selectionRenderer->updateObject(node);
                else
                    layerRenderer->updateObject(node);
            }
        };
        
        void MapRenderer::selectionDidChange(const View::Selection& selection) {
            HandleSelectedNode handleSelectedNode(m_layerRenderers, m_selectionRenderer, m_lockedRenderer);
            Model::Node::accept(selection.partiallySelectedNodes().begin(), selection.partiallySelectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.partiallyDeselectedNodes().begin(), selection.partiallyDeselectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.recursivelySelectedNodes().begin(), selection.recursivelySelectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.recursivelyDeselectedNodes().begin(), selection.recursivelyDeselectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.selectedNodes().begin(), selection.selectedNodes().end(), handleSelectedNode);
            Model::Node::accept(selection.deselectedNodes().begin(), selection.deselectedNodes().end(), handleSelectedNode);

            UpdateSelectedNode updateNode(m_layerRenderers, m_selectionRenderer);
            
            const Model::BrushSet parentsOfSelectedFaces = collectBrushes(selection.selectedBrushFaces());
            Model::Node::accept(parentsOfSelectedFaces.begin(), parentsOfSelectedFaces.end(), updateNode);

            const Model::BrushSet parentsOfDeselectedFaces = collectBrushes(selection.deselectedBrushFaces());
            Model::Node::accept(parentsOfDeselectedFaces.begin(), parentsOfDeselectedFaces.end(), updateNode);

            invalidateEntityLinkRenderer();
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

        void MapRenderer::textureCollectionsDidChange() {
            invalidateLayerRenderers();
            invalidateSelectionRenderer();
        }
        
        void MapRenderer::entityDefinitionsDidChange() {
            invalidateLayerRenderers();
            invalidateSelectionRenderer();
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::modsDidChange() {
            invalidateLayerRenderers();
            invalidateSelectionRenderer();
            invalidateEntityLinkRenderer();
        }

        void MapRenderer::editorContextDidChange() {
            invalidateLayerRenderers();
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::mapViewConfigDidChange() {
            invalidateLayerRenderers();
            invalidateEntityLinkRenderer();
        }

        void MapRenderer::preferenceDidChange(const IO::Path& path) {
            setupRenderers();
        }
    }
}
