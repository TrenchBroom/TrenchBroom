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

#ifndef TrenchBroom_BrushRendererBrushCache
#define TrenchBroom_BrushRendererBrushCache

#include "Renderer/GLVertexType.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushNode;
        class BrushFace;
    }

    namespace Renderer {
        class BrushRendererBrushCache {
        public:
            using VertexSpec = Renderer::GLVertexTypes::P3NT2;
            using Vertex = VertexSpec::Vertex;

            struct CachedFace {
                const Assets::Texture* texture;
                Model::BrushFace* face;
                size_t vertexCount;
                size_t indexOfFirstVertexRelativeToBrush;

                CachedFace(Model::BrushFace* i_face,
                           size_t i_indexOfFirstVertexRelativeToBrush);
            };

            struct CachedEdge {
                Model::BrushFace* face1;
                Model::BrushFace* face2;
                size_t vertexIndex1RelativeToBrush;
                size_t vertexIndex2RelativeToBrush;

                CachedEdge(Model::BrushFace* i_face1,
                           Model::BrushFace* i_face2,
                           size_t i_vertexIndex1RelativeToBrush,
                           size_t i_vertexIndex2RelativeToBrush);
            };

        private:
            std::vector<Vertex> m_cachedVertices;
            std::vector<CachedEdge> m_cachedEdges;
            std::vector<CachedFace> m_cachedFacesSortedByTexture;
            bool m_rendererCacheValid;

        public:
            BrushRendererBrushCache();

            /**
             * Only exposed to be called by BrushFace
             */
            void invalidateVertexCache();
            /**
             * Call this before cachedVertices()/cachedFacesSortedByTexture()/cachedEdges()
             *
             * NOTE: The reason for having this cache is we often need to re-upload the brush to VBO's when the brush
             * itself hasn't changed, but we're moving it between VBO's for different rendering styles
             * (default/selected/locked), or need to re-evaluate the BrushRenderer::Filter to exclude certain
             * faces/edges.
             */
            void validateVertexCache(const Model::BrushNode* brush);

            /**
             * Returns all vertices for all faces of the brush.
             */
            const std::vector<Vertex>& cachedVertices() const;
            const std::vector<CachedFace>& cachedFacesSortedByTexture() const;
            const std::vector<CachedEdge>& cachedEdges() const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushRendererBrushCache) */
