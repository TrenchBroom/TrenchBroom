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

#include "MapRenderer.h"

#include "CollectionUtils.h"
#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/Tutorial.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/Camera.h"
#include "Renderer/EntityLinkRenderer.h"
#include "Renderer/ObjectRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/RenderUtils.h"
#include "View/Selection.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        class MapRenderer::SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            SelectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
        private:
            void doProvideFaces(const Model::Brush* brush, BrushRenderer::FaceAcceptor&  faceAcceptor) const override {
                const bool brushSelected = selected(brush);
                
                if (visible(brush) && editable(brush)) {
                    for (const Model::BrushFace* face : brush->faces()) {
                        if (brushSelected || selected(face))
                            faceAcceptor.accept(face);
                    }
                }
            }
            
            void doProvideEdges(const Model::Brush* brush, BrushRenderer::EdgeAcceptor&  edgeAcceptor) const override {
                const bool brushSelected = selected(brush);
                
                if (visible(brush) && editable(brush)) {
                    for (const Model::BrushEdge* edge : brush->edges()) {
                        const Model::BrushFace* first = edge->firstFace()->payload();
                        const Model::BrushFace* second = edge->secondFace()->payload();
                        assert(second->brush() == brush);
                        
                        if (brushSelected || selected(first) || selected(second))
                            edgeAcceptor.accept(edge);
                    }
                }
            }
            
            bool doIsTransparent(const Model::Brush* brush) const override {
                return false;
            }
        };
        
        class MapRenderer::LockedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            LockedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
        private:
            void doProvideFaces(const Model::Brush* brush, BrushRenderer::FaceAcceptor&  faceAcceptor) const override {
                const bool brushVisible = visible(brush);
                
                if (brushVisible) {
                    // collect all faces
                    for (const Model::BrushFace* face : brush->faces())
                         faceAcceptor.accept(face);
                }
            }
            
            void doProvideEdges(const Model::Brush* brush, BrushRenderer::EdgeAcceptor&  edgeAcceptor) const override {
                const bool brushVisible = visible(brush);
                
                if (brushVisible) {
                    // collect all edges
                    for (const Model::BrushEdge* edge : brush->edges())
                         edgeAcceptor.accept(edge);
                }
            }
            
            bool doIsTransparent(const Model::Brush* brush) const override {
                return brush->transparent();
            }
        };
        
        class MapRenderer::UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            UnselectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}
        private:
            void doProvideFaces(const Model::Brush* brush, BrushRenderer::FaceAcceptor&  faceAcceptor) const override {
                if (visible(brush) && editable(brush)) {
                    for (const Model::BrushFace* face : brush->faces()) {
                        if (!selected(face))
                            faceAcceptor.accept(face);
                    }
                }
            }
            
            void doProvideEdges(const Model::Brush* brush, BrushRenderer::EdgeAcceptor&  edgeAcceptor) const override {
                if (visible(brush) && !selected(brush)) {
                    for (const Model::BrushEdge* edge : brush->edges()) {
                        const Model::BrushFace* first = edge->firstFace()->payload();
                        const Model::BrushFace* second = edge->secondFace()->payload();
                        assert(second->brush() == brush);
                        
                        if (!selected(first) && !selected(second))
                            edgeAcceptor.accept(edge);
                    }
                }
            }
            
            bool doIsTransparent(const Model::Brush* brush) const override {
                return brush->transparent();
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_defaultRenderer(createDefaultRenderer(m_document)),
        m_selectionRenderer(createSelectionRenderer(m_document)),
        m_lockedRenderer(createLockRenderer(m_document)),
        m_entityLinkRenderer(new EntityLinkRenderer(m_document)) {
            bindObservers();
            setupRenderers();
        }
        
        MapRenderer::~MapRenderer() {
            unbindObservers();
            clear();
            delete m_entityLinkRenderer;
            delete m_lockedRenderer;
            delete m_selectionRenderer;
            delete m_defaultRenderer;
        }
        
        ObjectRenderer* MapRenderer::createDefaultRenderer(View::MapDocumentWPtr document) {
            return new ObjectRenderer(lock(document)->entityModelManager(),
                                      lock(document)->editorContext(),
                                      UnselectedBrushRendererFilter(lock(document)->editorContext()));
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
            m_defaultRenderer->clear();
            m_selectionRenderer->clear();
            m_lockedRenderer->clear();
            m_entityLinkRenderer->invalidate();
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
            renderDefaultOpaque(renderContext, renderBatch);
            renderLockedOpaque(renderContext, renderBatch);
            renderSelectionOpaque(renderContext, renderBatch);
            
            renderDefaultTransparent(renderContext, renderBatch);
            renderLockedTransparent(renderContext, renderBatch);
            renderSelectionTransparent(renderContext, renderBatch);
            
            renderEntityLinks(renderContext, renderBatch);
            renderTutorialMessages(renderContext, renderBatch);
        }
        
        void MapRenderer::commitPendingChanges() {
            View::MapDocumentSPtr document = lock(m_document);
            document->commitPendingAssets();
        }
        
        class SetupGL : public Renderable {
        private:
            void doRender(RenderContext& renderContext) {
                glAssert(glFrontFace(GL_CW));
                glAssert(glEnable(GL_CULL_FACE));
                glAssert(glEnable(GL_DEPTH_TEST));
                glAssert(glDepthFunc(GL_LEQUAL));
                glResetEdgeOffset();
            }
        };
        
        void MapRenderer::setupGL(RenderBatch& renderBatch) {
            renderBatch.addOneShot(new SetupGL());
        }
        
        void MapRenderer::renderDefaultOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_defaultRenderer->setShowOverlays(renderContext.render3D());
            m_defaultRenderer->renderOpaque(renderContext, renderBatch);
        }
        
        void MapRenderer::renderDefaultTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_defaultRenderer->setShowOverlays(renderContext.render3D());
            m_defaultRenderer->renderTransparent(renderContext, renderBatch);
        }
        
        void MapRenderer::renderSelectionOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!renderContext.hideSelection()) {
                m_selectionRenderer->renderOpaque(renderContext, renderBatch);
            }
        }
        
        void MapRenderer::renderSelectionTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!renderContext.hideSelection()) {
                m_selectionRenderer->renderTransparent(renderContext, renderBatch);
            }
        }
        
        void MapRenderer::renderLockedOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_lockedRenderer->setShowOverlays(renderContext.render3D());
            m_lockedRenderer->renderOpaque(renderContext, renderBatch);
        }
        
        void MapRenderer::renderLockedTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_lockedRenderer->setShowOverlays(renderContext.render3D());
            m_lockedRenderer->renderTransparent(renderContext, renderBatch);
        }
        
        void MapRenderer::renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_entityLinkRenderer->render(renderContext, renderBatch);
        }
        
        class MapRenderer::MatchTutorialEntities {
        private:
            const Assets::EntityDefinition* m_definition;
        public:
            MatchTutorialEntities(const Assets::EntityDefinition* definition) :
            m_definition(definition) {}
            
            bool operator()(const Model::Brush* brush) {
                return brush->entity() != NULL && brush->entity()->definition() == m_definition;
            }
            
            bool operator()(const Model::Node* node) { return false; }
        };
        
        class MapRenderer::FilterTutorialEntities : public Model::FilteringNodeCollectionStrategy<Model::UniqueNodeCollectionStrategy> {
        private:
            Model::Node* getNode(Model::Brush* brush) const { return brush->entity();  }
        };
        
        class MapRenderer::CollectTutorialEntitiesVisitor : public Model::CollectMatchingNodesVisitor<MatchTutorialEntities, FilterTutorialEntities> {
        public:
            CollectTutorialEntitiesVisitor(const Assets::EntityDefinition* definition) :
            CollectMatchingNodesVisitor(MatchTutorialEntities(definition)) {}
        };
        
        void MapRenderer::renderTutorialMessages(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (renderContext.render3D()) {
                View::MapDocumentSPtr document = lock(m_document);
                const Assets::EntityDefinition* definition = document->entityDefinitionManager().definition(Model::Tutorial::Classname);
                const Model::NodeList nodes = document->findNodesContaining(renderContext.camera().position());
                if (!nodes.empty()) {
                    RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::TutorialOverlayTextColor));
                    renderService.setBackgroundColor(pref(Preferences::TutorialOverlayBackgroundColor));
                    
                    CollectTutorialEntitiesVisitor collect(definition);
                    Model::Node::accept(std::begin(nodes), std::end(nodes), collect);

                    for (const Model::Node* node : collect.nodes()) {
                        const Model::Entity* entity = static_cast<const Model::Entity*>(node);
                        const Model::AttributeValue& message = entity->attribute(Model::Tutorial::Message);
                        if (!message.empty())
                            renderService.renderHeadsUp(message);
                    }
                }
            }
        }

        void MapRenderer::setupRenderers() {
            setupDefaultRenderer(m_defaultRenderer);
            setupSelectionRenderer(m_selectionRenderer);
            setupLockedRenderer(m_lockedRenderer);
            setupEntityLinkRenderer();
        }
        
        void MapRenderer::setupDefaultRenderer(ObjectRenderer* renderer) {
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
            renderer->setShowBrushEdges(true);
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
        
        class MapRenderer::CollectRenderableNodes : public Model::NodeVisitor {
        private:
            Renderer m_renderers;
            Model::NodeCollection m_defaultNodes;
            Model::NodeCollection m_selectedNodes;
            Model::NodeCollection m_lockedNodes;
        public:
            CollectRenderableNodes(const Renderer renderers) : m_renderers(renderers) {}
            
            const Model::NodeCollection& defaultNodes() const  { return m_defaultNodes;  }
            const Model::NodeCollection& selectedNodes() const { return m_selectedNodes; }
            const Model::NodeCollection& lockedNodes() const   { return m_lockedNodes;   }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            
            void doVisit(Model::Group* group)   {
                if (group->locked()) {
                    if (collectLocked()) m_lockedNodes.addNode(group);
                } else if (selected(group) || group->opened()) {
                    if (collectSelection()) m_selectedNodes.addNode(group);
                } else {
                    if (collectDefault()) m_defaultNodes.addNode(group);
                }
            }
            
            void doVisit(Model::Entity* entity) {
                if (entity->locked()) {
                    if (collectLocked()) m_lockedNodes.addNode(entity);
                } else if (selected(entity)) {
                    if (collectSelection()) m_selectedNodes.addNode(entity);
                } else {
                    if (collectDefault()) m_defaultNodes.addNode(entity);
                }
            }
            
            void doVisit(Model::Brush* brush)   {
                if (brush->locked()) {
                    if (collectLocked()) m_lockedNodes.addNode(brush);
                } else if (selected(brush)) {
                    if (collectSelection()) m_selectedNodes.addNode(brush);
                }
                if (!brush->selected() && !brush->parentSelected() && !brush->locked()) {
                    if (collectDefault()) m_defaultNodes.addNode(brush);
                }
            }
            
            bool collectLocked() const    { return (m_renderers & Renderer_Locked)    != 0; }
            bool collectSelection() const { return (m_renderers & Renderer_Selection) != 0; }
            bool collectDefault() const   { return (m_renderers & Renderer_Default)   != 0; }
            
            bool selected(const Model::Node* node) const {
                return node->selected() || node->descendantSelected() || node->parentSelected();
            }
        };
        
        void MapRenderer::updateRenderers(const Renderer renderers) {
            View::MapDocumentSPtr document = lock(m_document);
            Model::World* world = document->world();
            
            CollectRenderableNodes collect(renderers);
            world->acceptAndRecurse(collect);
            
            if ((renderers & Renderer_Default) != 0) {
                m_defaultRenderer->setObjects(collect.defaultNodes().groups(),
                                              collect.defaultNodes().entities(),
                                              collect.defaultNodes().brushes());
            }
            if ((renderers & Renderer_Selection) != 0) {
                m_selectionRenderer->setObjects(collect.selectedNodes().groups(),
                                                collect.selectedNodes().entities(),
                                                collect.selectedNodes().brushes());
            }
            if ((renderers& Renderer_Locked) != 0) {
                m_lockedRenderer->setObjects(collect.lockedNodes().groups(),
                                             collect.lockedNodes().entities(),
                                             collect.lockedNodes().brushes());
            }
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::invalidateRenderers(Renderer renderers) {
            if ((renderers & Renderer_Default) != 0)
                m_defaultRenderer->invalidate();
            if ((renderers & Renderer_Selection) != 0)
                m_selectionRenderer->invalidate();
            if ((renderers& Renderer_Locked) != 0)
                m_lockedRenderer->invalidate();
        }

        void MapRenderer::invalidateEntityLinkRenderer() {
            m_entityLinkRenderer->invalidate();
        }

        void MapRenderer::reloadEntityModels() {
            m_defaultRenderer->reloadModels();
            m_selectionRenderer->reloadModels();
            m_lockedRenderer->reloadModels();
        }

        void MapRenderer::bindObservers() {
            assert(!expired(m_document));
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->nodesWereAddedNotifier.addObserver(this, &MapRenderer::nodesWereAdded);
            document->nodesWereRemovedNotifier.addObserver(this, &MapRenderer::nodesWereRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &MapRenderer::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &MapRenderer::nodeVisibilityDidChange);
            document->nodeLockingDidChangeNotifier.addObserver(this, &MapRenderer::nodeLockingDidChange);
            document->groupWasOpenedNotifier.addObserver(this, &MapRenderer::groupWasOpened);
            document->groupWasClosedNotifier.addObserver(this, &MapRenderer::groupWasClosed);
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
                document->nodesWereRemovedNotifier.removeObserver(this, &MapRenderer::nodesWereRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &MapRenderer::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &MapRenderer::nodeVisibilityDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &MapRenderer::nodeLockingDidChange);
                document->groupWasOpenedNotifier.removeObserver(this, &MapRenderer::groupWasOpened);
                document->groupWasClosedNotifier.removeObserver(this, &MapRenderer::groupWasClosed);
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
        
        void MapRenderer::documentWasCleared(View::MapDocument* document) {
            clear();
        }
        
        void MapRenderer::documentWasNewedOrLoaded(View::MapDocument* document) {
            clear();
            updateRenderers(Renderer_All);
        }
        
        void MapRenderer::nodesWereAdded(const Model::NodeList& nodes) {
            updateRenderers(Renderer_Default);
        }
        
        void MapRenderer::nodesWereRemoved(const Model::NodeList& nodes) {
            updateRenderers(Renderer_Default);
        }
        
        void MapRenderer::nodesDidChange(const Model::NodeList& nodes) {
            invalidateRenderers(Renderer_Selection);
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::nodeVisibilityDidChange(const Model::NodeList& nodes) {
            updateRenderers(Renderer_All);
        }
        
        void MapRenderer::nodeLockingDidChange(const Model::NodeList& nodes) {
            updateRenderers(Renderer_Default_Locked);
        }
        
        void MapRenderer::groupWasOpened(Model::Group* group) {
            updateRenderers(Renderer_Default_Selection);
        }
        
        void MapRenderer::groupWasClosed(Model::Group* group) {
            updateRenderers(Renderer_Default_Selection);
        }

        void MapRenderer::brushFacesDidChange(const Model::BrushFaceList& faces) {
            invalidateRenderers(Renderer_Selection);
        }
        
        void MapRenderer::selectionDidChange(const View::Selection& selection) {
            updateRenderers(Renderer_All); // need to update locked objects also because a selected object may have been reparented into a locked layer before deselection
        }
        
        Model::BrushSet MapRenderer::collectBrushes(const Model::BrushFaceList& faces) {
            Model::BrushSet result;
            for (const Model::BrushFace* face : faces)
                result.insert(face->brush());
            return result;
        }
        
        void MapRenderer::textureCollectionsDidChange() {
            invalidateRenderers(Renderer_All);
        }
        
        void MapRenderer::entityDefinitionsDidChange() {
            reloadEntityModels();
            invalidateRenderers(Renderer_All);
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::modsDidChange() {
            reloadEntityModels();
            invalidateRenderers(Renderer_All);
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::editorContextDidChange() {
            invalidateRenderers(Renderer_All);
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::mapViewConfigDidChange() {
            invalidateRenderers(Renderer_All);
            invalidateEntityLinkRenderer();
        }
        
        void MapRenderer::preferenceDidChange(const IO::Path& path) {
            setupRenderers();
            
            View::MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                reloadEntityModels();
                invalidateRenderers(Renderer_All);
                invalidateEntityLinkRenderer();
            }
        }
    }
}
