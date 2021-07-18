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

#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/GroupRenderer.h"
#include "Renderer/PatchRenderer.h"

#include <vector>

namespace TrenchBroom {
    class Color;
    class Logger;

    namespace Assets {
        class EntityModelManager;
    }

    namespace Model {
        class BrushNode;
        class EditorContext;
        class EntityNode;
        class GroupNode;
        class Node;
        class PatchNode;
    }

    namespace Renderer {
        class FontManager;
        class RenderBatch;

        class ObjectRenderer {
        private:
            GroupRenderer m_groupRenderer;
            EntityRenderer m_entityRenderer;
            BrushRenderer m_brushRenderer;
            PatchRenderer m_patchRenderer;
        public:
            template <typename BrushFilterT>
            ObjectRenderer(Logger& logger, Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext, const BrushFilterT& brushFilter) :
            m_groupRenderer(editorContext),
            m_entityRenderer(logger, entityModelManager, editorContext),
            m_brushRenderer(brushFilter),
            m_patchRenderer{} {}
        public: // object management
            void setObjects(const std::vector<Model::GroupNode*>& groups, const std::vector<Model::EntityNode*>& entities, const std::vector<Model::BrushNode*>& brushes, const std::vector<Model::PatchNode*>& patches);
            void addNode(Model::Node* node);
            void removeNode(Model::Node* node);
            void invalidateNode(Model::Node* node);
            void invalidate();
            void invalidateBrushes(const std::vector<Model::BrushNode*>& brushes);
            void clear();
            void reloadModels();
        public: // configuration
            void setShowOverlays(bool showOverlays);
            void setEntityOverlayTextColor(const Color& overlayTextColor);
            void setGroupOverlayTextColor(const Color& overlayTextColor);
            void setOverlayBackgroundColor(const Color& overlayBackgroundColor);

            void setTint(bool tint);
            void setTintColor(const Color& tintColor);

            void setShowOccludedObjects(bool showOccludedObjects);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);

            void setTransparencyAlpha(float transparencyAlpha);

            void setShowEntityAngles(bool showAngles);
            void setEntityAngleColor(const Color& color);

            void setOverrideGroupColors(bool overrideGroupColors);
            void setGroupBoundsColor(const Color& color);

            void setOverrideEntityBoundsColor(bool overrideEntityBoundsColor);
            void setEntityBoundsColor(const Color& color);

            void setShowBrushEdges(bool showBrushEdges);
            void setBrushFaceColor(const Color& brushFaceColor);
            void setBrushEdgeColor(const Color& brushEdgeColor);

            void setShowHiddenObjects(bool showHiddenObjects);
        public: // rendering
            void renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            ObjectRenderer(const ObjectRenderer&);
            ObjectRenderer& operator=(const ObjectRenderer&);
        };
    }
}

