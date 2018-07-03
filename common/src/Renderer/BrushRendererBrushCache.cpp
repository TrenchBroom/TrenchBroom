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

#include "BrushRendererBrushCache.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRendererBrushCache::BrushRendererBrushCache()
                : m_rendererCacheValid(false) {}

        void BrushRendererBrushCache::invalidateVertexCache() {
            m_rendererCacheValid = false;
            m_cachedVertices.clear();
            m_cachedEdges.clear();
            m_cachedFacesSortedByTexture.clear();
        }

        void BrushRendererBrushCache::validateVertexCache(const Model::Brush* brush) {
            if (m_rendererCacheValid)
                return;

            // build vertex cache and face cache

            m_cachedVertices.clear();
            m_cachedVertices.reserve(brush->vertexCount());

            m_cachedFacesSortedByTexture.clear();
            m_cachedFacesSortedByTexture.reserve(brush->faceCount());

            for (Model::BrushFace* face : brush->faces()) {
                const size_t startIndex = m_cachedVertices.size();

                const Model::BrushHalfEdge* first = face->geometry()->boundary().front();
                const Model::BrushHalfEdge* current = first;
                do {
                    Model::BrushVertex* vertex = current->origin();

                    // Set the vertex payload to the index, relative to the brush's first vertex being 0.
                    // This is used below when building the edge cache.
                    // NOTE: we'll overwrite the payload as we visit the same vertex several times while visiting
                    // different faces, this is fine.
                    const size_t currentIndex = m_cachedVertices.size();
                    vertex->setPayload(static_cast<GLuint>(currentIndex));

                    const Vec3 &position = vertex->position();
                    m_cachedVertices.push_back(BrushRendererBrushCache::Vertex(position, face->boundary().normal, face->textureCoords(position)));

                    // The boundary is in CCW order, but the renderer expects CW order:
                    current = current->previous();
                } while (current != first);

                // face cache
                CachedFace cachedFace;
                cachedFace.texture = face->texture();
                cachedFace.face = face;
                cachedFace.vertexCount = face->vertexCount();
                cachedFace.indexOfFirstVertexRelativeToBrush = startIndex;
                m_cachedFacesSortedByTexture.push_back(cachedFace);
            }

            // Sort by texture so BrushRenderer can efficiently step through the BrushFaces
            // grouped by texture (via `BrushRendererBrushCache::cachedFacesSortedByTexture()`), without needing to build an std::map

            std::sort(m_cachedFacesSortedByTexture.begin(),
                      m_cachedFacesSortedByTexture.end(),
                      [](const CachedFace& a, const CachedFace& b){ return a.texture < b.texture; });

            // Build edge index cache

            m_cachedEdges.clear();
            m_cachedEdges.reserve(brush->edgeCount());

            for (const Model::BrushEdge* currentEdge : brush->edges()) {
                CachedEdge edge;
                edge.face1 = currentEdge->firstFace()->payload();
                edge.face2 = currentEdge->secondFace()->payload();
                edge.vertexIndex1RelativeToBrush = currentEdge->firstVertex()->payload();
                edge.vertexIndex2RelativeToBrush = currentEdge->secondVertex()->payload();
                m_cachedEdges.push_back(edge);
            }

            m_rendererCacheValid = true;
        }

        const std::vector<BrushRendererBrushCache::Vertex>& BrushRendererBrushCache::cachedVertices() const {
            assert(m_rendererCacheValid);
            return m_cachedVertices;
        }

        const std::vector<BrushRendererBrushCache::CachedFace>& BrushRendererBrushCache::cachedFacesSortedByTexture() const {
            assert(m_rendererCacheValid);
            return m_cachedFacesSortedByTexture;
        }

        const std::vector<BrushRendererBrushCache::CachedEdge>& BrushRendererBrushCache::cachedEdges() const {
            assert(m_rendererCacheValid);
            return m_cachedEdges;
        }
    }
}
