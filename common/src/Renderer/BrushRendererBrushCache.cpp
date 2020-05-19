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

#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Polyhedron.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        BrushRendererBrushCache::CachedFace::CachedFace(const Model::BrushFace* i_face,
                                                        const size_t i_indexOfFirstVertexRelativeToBrush)
                : texture(i_face->texture()),
                  face(i_face),
                  vertexCount(i_face->vertexCount()),
                  indexOfFirstVertexRelativeToBrush(i_indexOfFirstVertexRelativeToBrush) {}

        BrushRendererBrushCache::CachedEdge::CachedEdge(const Model::BrushFace* i_face1,
                                                        const Model::BrushFace* i_face2,
                                                        const size_t i_vertexIndex1RelativeToBrush,
                                                        const size_t i_vertexIndex2RelativeToBrush)
                : face1(i_face1),
                  face2(i_face2),
                  vertexIndex1RelativeToBrush(i_vertexIndex1RelativeToBrush),
                  vertexIndex2RelativeToBrush(i_vertexIndex2RelativeToBrush) {}

        BrushRendererBrushCache::BrushRendererBrushCache()
                : m_rendererCacheValid(false) {}

        void BrushRendererBrushCache::invalidateVertexCache() {
            m_rendererCacheValid = false;
            m_cachedVertices.clear();
            m_cachedEdges.clear();
            m_cachedFacesSortedByTexture.clear();
        }

        void BrushRendererBrushCache::validateVertexCache(const Model::BrushNode* brushNode) {
            if (m_rendererCacheValid) {
                return;
            }

            // build vertex cache and face cache
            const Model::Brush& brush = brushNode->brush();

            m_cachedVertices.clear();
            m_cachedVertices.reserve(brush.vertexCount());

            m_cachedFacesSortedByTexture.clear();
            m_cachedFacesSortedByTexture.reserve(brush.faceCount());

            for (const Model::BrushFace* face : brush.faces()) {
                const auto indexOfFirstVertexRelativeToBrush = m_cachedVertices.size();

                // The boundary is in CCW order, but the renderer expects CW order:
                auto& boundary = face->geometry()->boundary();
                for (auto it = std::rbegin(boundary), end = std::rend(boundary); it != end; ++it) {
                    Model::BrushHalfEdge* current = *it;
                    Model::BrushVertex* vertex = current->origin();

                    // Set the vertex payload to the index, relative to the brush's first vertex being 0.
                    // This is used below when building the edge cache.
                    // NOTE: we'll overwrite the payload as we visit the same vertex several times while visiting
                    // different faces, this is fine.
                    const auto currentIndex = m_cachedVertices.size();
                    vertex->setPayload(static_cast<GLuint>(currentIndex));

                    const auto& position = vertex->position();
                    m_cachedVertices.emplace_back(vm::vec3f(position), vm::vec3f(face->boundary().normal), face->textureCoords(position));

                    current = current->previous();
                }

                // face cache
                m_cachedFacesSortedByTexture.emplace_back(face, indexOfFirstVertexRelativeToBrush);
            }

            // Sort by texture so BrushRenderer can efficiently step through the BrushFaces
            // grouped by texture (via `BrushRendererBrushCache::cachedFacesSortedByTexture()`), without needing to build an std::map

            std::sort(m_cachedFacesSortedByTexture.begin(),
                      m_cachedFacesSortedByTexture.end(),
                      [](const CachedFace& a, const CachedFace& b){ return a.texture < b.texture; });

            // Build edge index cache

            m_cachedEdges.clear();
            m_cachedEdges.reserve(brush.edgeCount());

            for (const Model::BrushEdge* currentEdge : brush.edges()) {
                const auto faceIndex1 = currentEdge->firstFace()->payload();
                const auto faceIndex2 = currentEdge->secondFace()->payload();
                assert(faceIndex1 && faceIndex2);
                
                const auto* face1 = brush.face(*faceIndex1);
                const auto* face2 = brush.face(*faceIndex2);
                
                const auto vertexIndex1RelativeToBrush = currentEdge->firstVertex()->payload();
                const auto vertexIndex2RelativeToBrush = currentEdge->secondVertex()->payload();

                m_cachedEdges.emplace_back(face1, face2, vertexIndex1RelativeToBrush, vertexIndex2RelativeToBrush);
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
