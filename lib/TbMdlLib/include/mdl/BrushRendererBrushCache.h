/*
 Copyright (C) 2010 Kristian Duske

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

#include "gl/VertexType.h"

#include <vector>

namespace tb
{
namespace gl
{
class Material;
}

namespace mdl
{
class BrushNode;
class BrushFace;

class BrushRendererBrushCache
{
public:
  using VertexSpec = gl::VertexTypes::P3NT2;
  using Vertex = VertexSpec::Vertex;

  struct CachedFace
  {
    const gl::Material* material;
    const BrushFace* face;
    size_t vertexCount;
    size_t indexOfFirstVertexRelativeToBrush;

    CachedFace(const BrushFace* i_face, size_t i_indexOfFirstVertexRelativeToBrush);
  };

  struct CachedEdge
  {
    const BrushFace* face1;
    const BrushFace* face2;
    size_t vertexIndex1RelativeToBrush;
    size_t vertexIndex2RelativeToBrush;
  };

private:
  std::vector<Vertex> m_cachedVertices;
  std::vector<CachedEdge> m_cachedEdges;
  std::vector<CachedFace> m_cachedFacesSortedByMaterial;
  bool m_rendererCacheValid;

public:
  BrushRendererBrushCache();

  /**
   * Only exposed to be called by BrushFace
   */
  void invalidateVertexCache();
  /**
   * Call this before cachedVertices()/cachedFacesSortedByMaterial()/cachedEdges()
   *
   * NOTE: The reason for having this cache is we often need to re-upload the brush to
   * VBO's when the brush itself hasn't changed, but we're moving it between VBO's for
   * different rendering styles (default/selected/locked), or need to re-evaluate the
   * BrushRenderer::Filter to exclude certain faces/edges.
   */
  void validateVertexCache(const BrushNode& brushNode);

  /**
   * Returns all vertices for all faces of the brush.
   */
  const std::vector<Vertex>& cachedVertices() const;
  const std::vector<CachedFace>& cachedFacesSortedByMaterial() const;
  const std::vector<CachedEdge>& cachedEdges() const;
};

} // namespace mdl
} // namespace tb
