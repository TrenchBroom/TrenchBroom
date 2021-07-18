/*
 Copyright (C) 2021 Kristian Duske

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

#include "Color.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/TexturedIndexArrayRenderer.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PatchNode;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        class VboManager;

        class PatchRenderer : public IndexedRenderable {
        private:
            bool m_valid = true;
            std::vector<Model::PatchNode*> m_patchNodes;

            TexturedIndexArrayRenderer m_patchMeshRenderer;
            DirectEdgeRenderer m_edgeRenderer;

            Color m_defaultColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            float m_alpha;

            bool m_showEdges;
            Color m_edgeColor;
            bool m_showOccludedEdges;
            Color m_occludedEdgeColor;
        public:
            PatchRenderer();

            void setDefaultColor(const Color& faceColor);
            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& color);
            void setTransparencyAlpha(float alpha);

            /**
             * Specifies whether or not patch edges should be rendered.
             */
            void setShowEdges(bool showEdges);

            /**
             * The color to render patch edges with.
             */
            void setEdgeColor(const Color& edgeColor);

            /**
             * Specifies whether or not occluded edges should be visible.
             */
            void setShowOccludedEdges(bool showOccludedEdges);

            /**
             * The color to render occluded edges with.
             */
            void setOccludedEdgeColor(const Color& occludedEdgeColor);

            void setPatches(std::vector<Model::PatchNode*> patchNodes);
            void invalidate();
            void clear();

            void addPatch(Model::PatchNode* patchNode);
            void removePatch(Model::PatchNode* patchNode);
            void invalidatePatch(Model::PatchNode* patchNode);

            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void validate();
        private: // implement IndexedRenderable interface
            void prepareVerticesAndIndices(VboManager& vboManager) override;
            void doRender(RenderContext& renderContext) override;
        };
    }
}
