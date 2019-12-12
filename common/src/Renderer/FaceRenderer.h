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

#ifndef TrenchBroom_FaceRenderer
#define TrenchBroom_FaceRenderer

#include "Color.h"
#include "Assets/Asset_Forward.h"
#include "Renderer/Renderable.h"

#include <memory>
#include <unordered_map>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class BrushIndexArray;
        class BrushVertexArray;
        class RenderBatch;
        class RenderContext;
        class VboManager;

        using BrushVertexArrayPtr = std::shared_ptr<BrushVertexArray>;
        using TextureToBrushIndicesMap = std::unordered_map<const Assets::Texture*, std::shared_ptr<BrushIndexArray>>;
        using TextureToBrushIndicesMapPtr = std::shared_ptr<const TextureToBrushIndicesMap>;

        class FaceRenderer : public IndexedRenderable {
        private:
            struct RenderFunc;

            BrushVertexArrayPtr m_vertexArray;
            TextureToBrushIndicesMapPtr m_indexArrayMap;
            Color m_faceColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            float m_alpha;
        public:
            FaceRenderer();
            FaceRenderer(BrushVertexArrayPtr vertexArray, TextureToBrushIndicesMapPtr indexArrayMap, const Color& faceColor);

            FaceRenderer(const FaceRenderer& other);
            FaceRenderer& operator=(FaceRenderer other);
            friend void swap(FaceRenderer& left, FaceRenderer& right);

            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& color);
            void setAlpha(float alpha);

            void render(RenderBatch& renderBatch);
        private:
            void prepareVerticesAndIndices(VboManager& vboManager) override;
            void doRender(RenderContext& context) override;
        };

        void swap(FaceRenderer& left, FaceRenderer& right);
    }
}

#endif /* defined(TrenchBroom_FaceRenderer) */
