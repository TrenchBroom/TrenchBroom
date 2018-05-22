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
#include "Assets/AssetTypes.h"
#include "Model/BrushFace.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/IndexArrayMap.h"
#include "Renderer/TexturedIndexArrayMap.h"

#include <map>
#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderBatch;
        class RenderContext;
        class TexturedIndexArrayMap;
        class Vbo;
        class VertexArrayInterface;
        class IndexHolder;

        using IndexArrayPtr = std::shared_ptr<IndexHolder>;
        using VertexArrayPtr = std::shared_ptr<VertexArrayInterface>;

        class FaceRenderer : public IndexedRenderable {
        private:
            struct RenderFunc;

            VertexArrayPtr m_vertexArray;
            IndexArrayPtr m_indexArray;
            TexturedIndexArrayMap m_indexRanges;
            Color m_faceColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            float m_alpha;
        public:
            FaceRenderer();
            FaceRenderer(VertexArrayPtr vertexArray, IndexArrayPtr indexArray, const TexturedIndexArrayMap& indexArrayMap, const Color& faceColor);
            
            FaceRenderer(const FaceRenderer& other);
            FaceRenderer& operator=(FaceRenderer other);
            friend void swap(FaceRenderer& left, FaceRenderer& right);

            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& color);
            void setAlpha(float alpha);
            
            void render(RenderBatch& renderBatch);
        private:
            void prepareVerticesAndIndices(Vbo& vertexVbo, Vbo& indexVbo) override;
            void doRender(RenderContext& context) override;
        };

        void swap(FaceRenderer& left, FaceRenderer& right);
    }
}

#endif /* defined(TrenchBroom_FaceRenderer) */
