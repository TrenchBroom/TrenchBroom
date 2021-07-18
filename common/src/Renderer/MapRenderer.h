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

#pragma once

#include "Macros.h"
#include "NotifierConnection.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    class Color;

    namespace IO {
        class Path;
    }

    namespace View {
        // FIXME: Renderer should not depend on View
        class MapDocument;
        class Selection;
    }

    namespace Model {
        class BrushNode;
        class BrushFaceHandle;
        class GroupNode;
        class LayerNode;
        class Node;
    }

    namespace Renderer {
        class EntityLinkRenderer;
        class GroupLinkRenderer;
        class ObjectRenderer;
        class RenderBatch;
        class RenderContext;

        class MapRenderer {
        private:
            class SelectedBrushRendererFilter;
            class LockedBrushRendererFilter;
            class UnselectedBrushRendererFilter;

            using RendererMap = std::map<Model::LayerNode*, ObjectRenderer*>;

            std::weak_ptr<View::MapDocument> m_document;

            std::unique_ptr<ObjectRenderer> m_defaultRenderer;
            std::unique_ptr<ObjectRenderer> m_selectionRenderer;
            std::unique_ptr<ObjectRenderer> m_lockedRenderer;
            std::unique_ptr<EntityLinkRenderer> m_entityLinkRenderer;
            std::unique_ptr<GroupLinkRenderer> m_groupLinkRenderer;

            typedef enum {
                Renderer_Default            = 1,
                Renderer_Selection          = 2,
                Renderer_Locked             = 4,
                Renderer_Default_Selection  = Renderer_Default | Renderer_Selection,
                Renderer_Default_Locked     = Renderer_Default | Renderer_Locked,
                Renderer_All                = Renderer_Default | Renderer_Selection | Renderer_Locked
            } Renderer;

            std::unordered_map<Model::Node*, Renderer> m_trackedNodes;

            NotifierConnection m_notifierConnection;
        public:
            explicit MapRenderer(std::weak_ptr<View::MapDocument> document);
            ~MapRenderer();

            deleteCopyAndMove(MapRenderer)
        private:
            static std::unique_ptr<ObjectRenderer> createDefaultRenderer(std::weak_ptr<View::MapDocument> document);
            static std::unique_ptr<ObjectRenderer> createSelectionRenderer(std::weak_ptr<View::MapDocument> document);
            static std::unique_ptr<ObjectRenderer> createLockRenderer(std::weak_ptr<View::MapDocument> document);
            void clear();
        public: // color config
            void overrideSelectionColors(const Color& color, float mix);
            void restoreSelectionColors();
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void commitPendingChanges();
            void setupGL(RenderBatch& renderBatch);
            void renderDefaultOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderDefaultTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSelectionOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSelectionTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderLockedOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderLockedTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderGroupLinks(RenderContext& renderContext, RenderBatch& renderBatch);

            void setupRenderers();
            void setupDefaultRenderer(ObjectRenderer& renderer);
            void setupSelectionRenderer(ObjectRenderer& renderer);
            void setupLockedRenderer(ObjectRenderer& renderer);

            Renderer determineRenderers(Model::Node* node);
            /**
             * - Determine which renderers the given node should be in
             * - Remove from any renderers the node shouldn't be in
             * - Add to new renderers, if not already present
             * - Invalidate, for any renderers it was already present in
             */
            void updateAndInvalidateNode(Model::Node* node);
            void removeNode(Model::Node* node);
            void updateAllNodes();
            /**
             * Clears the set of nodes being tracked and repopulates it by traversing the node tree from the world.
             *
             * This moves nodes between default / selection / locked renderers as needed,
             * but doesn't otherwise invalidate them.
             * (in particular, brushes are not updated unless they move between renderers.)
             * If brushes are modified, you need to call invalidateRenderers() or invalidateObjectsInRenderers()
             */
            void updateRenderers(Renderer renderers);
            /**
             * Marks the nodes that are already tracked in the given renderers as invalid, i.e.
             * needing to be re-rendered.
             */
            void invalidateRenderers(Renderer renderers);
            void invalidateBrushesInRenderers(Renderer renderers, const std::vector<Model::BrushNode*>& brushes);
            void invalidateEntityLinkRenderer();
            void invalidateGroupLinkRenderer();
            void reloadEntityModels();
        private: // notification
            void connectObservers();

            void documentWasCleared(View::MapDocument* document);
            void documentWasNewedOrLoaded(View::MapDocument* document);

            void nodesWereAdded(const std::vector<Model::Node*>& nodes);
            void nodesWereRemoved(const std::vector<Model::Node*>& nodes);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);

            void nodeVisibilityDidChange(const std::vector<Model::Node*>& nodes);
            void nodeLockingDidChange(const std::vector<Model::Node*>& nodes);

            void groupWasOpened(Model::GroupNode* group);
            void groupWasClosed(Model::GroupNode* group);

            void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces);

            void selectionDidChange(const View::Selection& selection);

            void textureCollectionsWillChange();
            void entityDefinitionsDidChange();
            void modsDidChange();

            void editorContextDidChange();

            void preferenceDidChange(const IO::Path& path);
        };
    }
}

