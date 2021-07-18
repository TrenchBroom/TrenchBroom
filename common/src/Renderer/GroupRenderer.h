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

#include "AttrString.h"
#include "Color.h"
#include "Renderer/EdgeRenderer.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        class GroupNode;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;

        class GroupRenderer {
        private:
            class GroupNameAnchor;

            const Model::EditorContext& m_editorContext;
            std::vector<Model::GroupNode*> m_groups;

            DirectEdgeRenderer m_boundsRenderer;
            bool m_boundsValid;

            bool m_overrideColors;
            bool m_showOverlays;
            Color m_overlayTextColor;
            Color m_overlayBackgroundColor;
            bool m_showOccludedOverlays;
            Color m_boundsColor;
            bool m_showOccludedBounds;
            Color m_occludedBoundsColor;
        public:
            GroupRenderer(const Model::EditorContext& editorContext);

            void setGroups(const std::vector<Model::GroupNode*>& groups);
            void invalidate();
            void clear();

            void addGroup(Model::GroupNode* group);
            void removeGroup(Model::GroupNode* group);
            void invalidateGroup(Model::GroupNode* group);

            template <typename Iter>
            void addGroups(Iter cur, const Iter end) {
                while (cur != end) {
                    addGroup(*cur);
                    ++cur;
                }
            }
            template <typename Iter>
            void updateGroups(Iter cur, const Iter end) {
                while (cur != end) {
                    updateGroup(*cur);
                    ++cur;
                }
            }

            template <typename Iter>
            void removeGroups(Iter cur, const Iter end) {
                while (cur != end) {
                    removeGroup(*cur);
                    ++cur;
                }
            }

            void setOverrideColors(bool overrideColors);

            void setShowOverlays(bool showOverlays);
            void setOverlayTextColor(const Color& overlayTextColor);
            void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
            void setShowOccludedOverlays(bool showOccludedOverlays);

            void setBoundsColor(const Color& boundsColor);

            void setShowOccludedBounds(bool showOccludedBounds);
            void setOccludedBoundsColor(const Color& occludedBoundsColor);
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void renderBounds(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderNames(RenderContext& renderContext, RenderBatch& renderBatch);

            void invalidateBounds();
            void validateBounds();

            bool shouldRenderGroup(const Model::GroupNode* group) const;

            AttrString groupString(const Model::GroupNode* group) const;
            Color groupColor(const Model::GroupNode* group) const;
        };
    }
}

