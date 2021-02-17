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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityLinkRenderer.h"
#include "Renderer/GroupLinkRenderer.h"
#include "Renderer/ObjectRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "View/Selection.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/vector_set.h>

#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class MapRenderer::SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            SelectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}

            RenderSettings markFaces(const Model::BrushNode* brushNode) const override {
                if (!(visible(brushNode) && editable(brushNode))) {
                    return renderNothing();
                }

                const bool brushSelected = selected(brushNode);
                const Model::Brush& brush = brushNode->brush();
                for (const Model::BrushFace& face : brush.faces()) {
                    face.setMarked(brushSelected || selected(brushNode, face));
                }
                return std::make_tuple(FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderIfEitherFaceMarked);
            }
        };

        class MapRenderer::LockedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            LockedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}

            RenderSettings markFaces(const Model::BrushNode* brushNode) const override {
                if (!visible(brushNode)) {
                    return renderNothing();
                }

                const Model::Brush& brush = brushNode->brush();
                for (const Model::BrushFace& face : brush.faces()) {
                    face.setMarked(true);
                }

                return std::make_tuple(FaceRenderPolicy::RenderMarked,
                                       EdgeRenderPolicy::RenderAll);
            }
        };

        class MapRenderer::UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter {
        public:
            UnselectedBrushRendererFilter(const Model::EditorContext& context) :
            DefaultFilter(context) {}

            RenderSettings markFaces(const Model::BrushNode* brushNode) const override {
                const bool brushVisible = visible(brushNode);
                const bool brushEditable = editable(brushNode);

                const bool renderFaces = (brushVisible && brushEditable);
                      bool renderEdges = (brushVisible && !selected(brushNode));

                if (!renderFaces && !renderEdges) {
                    return renderNothing();
                }

                const Model::Brush& brush = brushNode->brush();
                
                bool anyFaceVisible = false;
                for (const Model::BrushFace& face : brush.faces()) {
                    const bool faceVisible = !selected(brushNode, face) && visible(brushNode, face);
                    face.setMarked(faceVisible);
                    anyFaceVisible |= faceVisible;
                }

                if (!anyFaceVisible) {
                    return renderNothing();
                }

                // Render all edges if only one face is visible.
                renderEdges |= anyFaceVisible;

                return std::make_tuple(renderFaces ? FaceRenderPolicy::RenderMarked : FaceRenderPolicy::RenderNone,
                                       renderEdges ? EdgeRenderPolicy::RenderAll : EdgeRenderPolicy::RenderNone);
            }
        };

        MapRenderer::MapRenderer(std::weak_ptr<View::MapDocument> document) :
        m_document(document),
        m_defaultRenderer(createDefaultRenderer(m_document)),
        m_selectionRenderer(createSelectionRenderer(m_document)),
        m_lockedRenderer(createLockRenderer(m_document)),
        m_entityLinkRenderer(std::make_unique<EntityLinkRenderer>(m_document)),
        m_groupLinkRenderer(std::make_unique<GroupLinkRenderer>(m_document)) {
            bindObservers();
            setupRenderers();
        }

        MapRenderer::~MapRenderer() {
            unbindObservers();
            clear();
        }

        std::unique_ptr<ObjectRenderer> MapRenderer::createDefaultRenderer(std::weak_ptr<View::MapDocument> document) {
            return std::make_unique<ObjectRenderer>(
                *kdl::mem_lock(document),
                kdl::mem_lock(document)->entityModelManager(),
                kdl::mem_lock(document)->editorContext(),
                UnselectedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
        }

        std::unique_ptr<ObjectRenderer> MapRenderer::createSelectionRenderer(std::weak_ptr<View::MapDocument> document) {
            return std::make_unique<ObjectRenderer>(
                *kdl::mem_lock(document),
                kdl::mem_lock(document)->entityModelManager(),
                kdl::mem_lock(document)->editorContext(),
                SelectedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
        }

        std::unique_ptr<ObjectRenderer> MapRenderer::createLockRenderer(std::weak_ptr<View::MapDocument> document) {
            return std::make_unique<ObjectRenderer>(
                *kdl::mem_lock(document),
                kdl::mem_lock(document)->entityModelManager(),
                kdl::mem_lock(document)->editorContext(),
                LockedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
        }

        void MapRenderer::clear() {
            m_defaultRenderer->clear();
            m_selectionRenderer->clear();
            m_lockedRenderer->clear();
            m_entityLinkRenderer->invalidate();
            m_groupLinkRenderer->invalidate();
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
            setupSelectionRenderer(*m_selectionRenderer);
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
            renderGroupLinks(renderContext, renderBatch);
        }

        void MapRenderer::commitPendingChanges() {
            auto document = kdl::mem_lock(m_document);
            document->commitPendingAssets();
        }

        class SetupGL : public Renderable {
        private:
            void doRender(RenderContext&) override {
                glAssert(glFrontFace(GL_CW))
                glAssert(glEnable(GL_CULL_FACE))
                glAssert(glEnable(GL_DEPTH_TEST))
                glAssert(glDepthFunc(GL_LEQUAL))
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

        void MapRenderer::renderGroupLinks(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_groupLinkRenderer->render(renderContext, renderBatch);
        }

        void MapRenderer::setupRenderers() {
            setupDefaultRenderer(*m_defaultRenderer);
            setupSelectionRenderer(*m_selectionRenderer);
            setupLockedRenderer(*m_lockedRenderer);
        }

        void MapRenderer::setupDefaultRenderer(ObjectRenderer& renderer) {
            renderer.setEntityOverlayTextColor(pref(Preferences::InfoOverlayTextColor));
            renderer.setGroupOverlayTextColor(pref(Preferences::GroupInfoOverlayTextColor));
            renderer.setOverlayBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
            renderer.setTint(false);
            renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

            renderer.setGroupBoundsColor(pref(Preferences::DefaultGroupColor));
            renderer.setEntityBoundsColor(pref(Preferences::UndefinedEntityColor));

            renderer.setBrushFaceColor(pref(Preferences::FaceColor));
            renderer.setBrushEdgeColor(pref(Preferences::EdgeColor));
        }

        void MapRenderer::setupSelectionRenderer(ObjectRenderer& renderer) {
            renderer.setEntityOverlayTextColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderer.setGroupOverlayTextColor(pref(Preferences::SelectedInfoOverlayTextColor));
            renderer.setOverlayBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
            renderer.setShowBrushEdges(true);
            renderer.setShowOccludedObjects(true);
            renderer.setOccludedEdgeColor(Color(pref(Preferences::SelectedEdgeColor), pref(Preferences::OccludedSelectedEdgeAlpha)));
            renderer.setTint(true);
            renderer.setTintColor(pref(Preferences::SelectedFaceColor));

            renderer.setOverrideGroupColors(true);
            renderer.setGroupBoundsColor(pref(Preferences::SelectedEdgeColor));

            renderer.setOverrideEntityBoundsColor(true);
            renderer.setEntityBoundsColor(pref(Preferences::SelectedEdgeColor));
            renderer.setShowEntityAngles(true);
            renderer.setEntityAngleColor(pref(Preferences::AngleIndicatorColor));

            renderer.setBrushFaceColor(pref(Preferences::FaceColor));
            renderer.setBrushEdgeColor(pref(Preferences::SelectedEdgeColor));
        }

        void MapRenderer::setupLockedRenderer(ObjectRenderer& renderer) {
            renderer.setEntityOverlayTextColor(pref(Preferences::LockedInfoOverlayTextColor));
            renderer.setGroupOverlayTextColor(pref(Preferences::LockedInfoOverlayTextColor));
            renderer.setOverlayBackgroundColor(pref(Preferences::LockedInfoOverlayBackgroundColor));
            renderer.setShowOccludedObjects(false);
            renderer.setTint(true);
            renderer.setTintColor(pref(Preferences::LockedFaceColor));
            renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

            renderer.setOverrideGroupColors(true);
            renderer.setGroupBoundsColor(pref(Preferences::LockedEdgeColor));

            renderer.setOverrideEntityBoundsColor(true);
            renderer.setEntityBoundsColor(pref(Preferences::LockedEdgeColor));
            renderer.setShowEntityAngles(false);

            renderer.setBrushFaceColor(pref(Preferences::FaceColor));
            renderer.setBrushEdgeColor(pref(Preferences::LockedEdgeColor));
        }

        void MapRenderer::updateRenderers(const Renderer renderers) {
            const auto renderDefault   = (renderers & Renderer_Default) != 0;
            const auto renderSelection = (renderers & Renderer_Selection) != 0;
            const auto renderLocked    = (renderers & Renderer_Locked) != 0;

            struct RenderableNodes {
                std::vector<Model::GroupNode*> groups;
                std::vector<Model::EntityNode*> entities;
                std::vector<Model::BrushNode*> brushes;
            };

            RenderableNodes defaultNodes;
            RenderableNodes selectedNodes;
            RenderableNodes lockedNodes;

            const auto selected = [](const auto* node) {
                return node->selected() || node->descendantSelected() || node->parentSelected();
            };

            auto document = kdl::mem_lock(m_document);
            document->world()->accept(kdl::overload(
                [](auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::GroupNode* group) {
                    if (group->locked()) {
                        if (renderLocked) lockedNodes.groups.push_back(group);
                    } else if (selected(group) || group->opened()) {
                        if (renderSelection) selectedNodes.groups.push_back(group);
                    } else {
                        if (renderDefault) defaultNodes.groups.push_back(group);
                    }
                    group->visitChildren(thisLambda);
                },
                [&](auto&& thisLambda, Model::EntityNode* entity) {
                    if (entity->locked()) {
                        if (renderLocked) lockedNodes.entities.push_back(entity);
                    } else if (selected(entity)) {
                        if (renderSelection) selectedNodes.entities.push_back(entity);
                    } else {
                        if (renderDefault) defaultNodes.entities.push_back(entity);
                    }
                    entity->visitChildren(thisLambda);
                },
                [&](Model::BrushNode* brush) {
                    if (brush->locked()) {
                        if (renderLocked) lockedNodes.brushes.push_back(brush);
                    } else if (selected(brush) || brush->hasSelectedFaces()) {
                        if (renderSelection) selectedNodes.brushes.push_back(brush);
                    }
                    if (!brush->selected() && !brush->parentSelected() && !brush->locked()) {
                        if (renderDefault) defaultNodes.brushes.push_back(brush);
                    }
                }
            ));

            if (renderDefault) {
                m_defaultRenderer->setObjects(defaultNodes.groups,
                                              defaultNodes.entities,
                                              defaultNodes.brushes);
            }
            if (renderSelection) {
                m_selectionRenderer->setObjects(selectedNodes.groups,
                                                selectedNodes.entities,
                                                selectedNodes.brushes);
            }
            if (renderLocked) {
                m_lockedRenderer->setObjects(lockedNodes.groups,
                                             lockedNodes.entities,
                                             lockedNodes.brushes);
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

        void MapRenderer::invalidateBrushesInRenderers(Renderer renderers, const std::vector<Model::BrushNode*>& brushes) {
            if ((renderers & Renderer_Default) != 0) {
                m_defaultRenderer->invalidateBrushes(brushes);
            }
            if ((renderers & Renderer_Selection) != 0) {
                m_selectionRenderer->invalidateBrushes(brushes);
            }
            if ((renderers& Renderer_Locked) != 0) {
                m_lockedRenderer->invalidateBrushes(brushes);
            }
        }

        void MapRenderer::invalidateEntityLinkRenderer() {
            m_entityLinkRenderer->invalidate();
        }

        void MapRenderer::invalidateGroupLinkRenderer() {
            m_groupLinkRenderer->invalidate();
        }

        void MapRenderer::reloadEntityModels() {
            m_defaultRenderer->reloadModels();
            m_selectionRenderer->reloadModels();
            m_lockedRenderer->reloadModels();
        }

        void MapRenderer::bindObservers() {
            assert(!kdl::mem_expired(m_document));
            auto document = kdl::mem_lock(m_document);
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
            document->textureCollectionsWillChangeNotifier.addObserver(this, &MapRenderer::textureCollectionsWillChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapRenderer::entityDefinitionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapRenderer::modsDidChange);
            document->editorContextDidChangeNotifier.addObserver(this, &MapRenderer::editorContextDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapRenderer::preferenceDidChange);
        }

        void MapRenderer::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
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
                document->textureCollectionsWillChangeNotifier.removeObserver(this, &MapRenderer::textureCollectionsWillChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapRenderer::entityDefinitionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapRenderer::modsDidChange);
                document->editorContextDidChangeNotifier.removeObserver(this, &MapRenderer::editorContextDidChange);
            }

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapRenderer::preferenceDidChange);
        }

        void MapRenderer::documentWasCleared(View::MapDocument*) {
            clear();
        }

        void MapRenderer::documentWasNewedOrLoaded(View::MapDocument*) {
            clear();
            updateRenderers(Renderer_All);
        }

        void MapRenderer::nodesWereAdded(const std::vector<Model::Node*>&) {
            updateRenderers(Renderer_All);
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::nodesWereRemoved(const std::vector<Model::Node*>&) {
            updateRenderers(Renderer_All);
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::nodesDidChange(const std::vector<Model::Node*>&) {
            invalidateRenderers(Renderer_Selection);
            invalidateEntityLinkRenderer();
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::nodeVisibilityDidChange(const std::vector<Model::Node*>&) {
            invalidateRenderers(Renderer_All);
        }

        void MapRenderer::nodeLockingDidChange(const std::vector<Model::Node*>&) {
            updateRenderers(Renderer_Default_Locked);
        }

        void MapRenderer::groupWasOpened(Model::GroupNode*) {
            updateRenderers(Renderer_Default_Selection);
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::groupWasClosed(Model::GroupNode*) {
            updateRenderers(Renderer_Default_Selection);
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&) {
            invalidateRenderers(Renderer_Selection);
        }

        void MapRenderer::selectionDidChange(const View::Selection& selection) {
            updateRenderers(Renderer_All); // need to update locked objects also because a selected object may have been reparented into a locked layer before deselection

            // selecting faces needs to invalidate the brushes
            if (!selection.selectedBrushFaces().empty()
                || !selection.deselectedBrushFaces().empty()) {

                
                const auto toBrush = [](const auto& handle) { return handle.node(); };
                auto brushes = kdl::vec_concat(kdl::vec_transform(selection.selectedBrushFaces(), toBrush), kdl::vec_transform(selection.deselectedBrushFaces(), toBrush));
                brushes = kdl::vec_sort_and_remove_duplicates(std::move(brushes));
                invalidateBrushesInRenderers(Renderer_All, brushes);
            }

            invalidateGroupLinkRenderer();
        }

        void MapRenderer::textureCollectionsWillChange() {
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
            invalidateGroupLinkRenderer();
        }

        void MapRenderer::preferenceDidChange(const IO::Path& path) {
            setupRenderers();

            auto document = kdl::mem_lock(m_document);
            if (document->isGamePathPreference(path)) {
                reloadEntityModels();
                invalidateRenderers(Renderer_All);
                invalidateEntityLinkRenderer();
                invalidateGroupLinkRenderer();
            }

            if (path.hasPrefix(IO::Path("Map view"), true)) {
                invalidateRenderers(Renderer_All);
                invalidateEntityLinkRenderer();
                invalidateGroupLinkRenderer();
            }
        }
    }
}
