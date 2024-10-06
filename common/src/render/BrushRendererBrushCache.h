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

#include "render/GLVertexType.h"

#include <vector>

namespace tb::mdl
{
class BrushNode;
class BrushFace;
class Material;
} // namespace tb::mdl

namespace tb::render
{

class BrushRendererBrushCache
{
public:
  using VertexSpec = render::GLVertexTypes::P3NT2;
  using Vertex = VertexSpec::Vertex;

  struct CachedFace
  {
    const mdl::Material* material;
    const mdl::BrushFace* face;
    size_t vertexCount;
    size_t indexOfFirstVertexRelativeToBrush;

    CachedFace(const mdl::BrushFace* i_face, size_t i_indexOfFirstVertexRelativeToBrush);
  };

  struct CachedEdge
  {
    const mdl::BrushFace* face1;
    const mdl::BrushFace* face2;
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
  void validateVertexCache(const mdl::BrushNode& brushNode);

  /**
   * Returns all vertices for all faces of the brush.
   */
  const std::vector<Vertex>& cachedVertices() const;
  const std::vector<CachedFace>& cachedFacesSortedByMaterial() const;
  const std::vector<CachedEdge>& cachedEdges() const;
};

} // namespace tb::render
