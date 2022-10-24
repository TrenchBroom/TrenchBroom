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

#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/VertexListBuilder.h"

#include <vector>

namespace TrenchBroom
{
namespace Assets
{
class Texture;
}

namespace Renderer
{
/**
 * Builds an index range map and a corresponding vertex array by recording textured
 * rendering primitives. The recorded data can be used to create an vertex array that can
 * be uploaded to video card memory, and to render the recorded primitives using the index
 * ranges stored in the constructed index range map.
 */
template <typename VertexSpec>
class TexturedIndexRangeMapBuilder
{
public:
  using Vertex = typename VertexSpec::Vertex;
  using VertexList = std::vector<Vertex>;
  using Texture = Assets::Texture;

private:
  using IndexData = typename VertexListBuilder<VertexSpec>::Range;

private:
  VertexListBuilder<VertexSpec> m_vertexListBuilder;
  TexturedIndexRangeMap m_indexRange;

public:
  /**
   * Creates a new index range map builder that initializes its data structures to the
   * given sizes.
   *
   * @param vertexCount the total number of vertices to expect
   * @param indexRangeSize the size of the index range map to expect
   */
  TexturedIndexRangeMapBuilder(
    const size_t vertexCount, const TexturedIndexRangeMap::Size& indexRangeSize)
    : m_vertexListBuilder(vertexCount)
    , m_indexRange(indexRangeSize)
  {
  }

  /**
   * Returns the recorded vertices.
   *
   * @return the recorded vertices
   */
  const VertexList& vertices() const { return m_vertexListBuilder.vertices(); }

  /**
   * Returns the recorded vertices.
   *
   * @return the recorded vertices
   */
  VertexList& vertices() { return m_vertexListBuilder.vertices(); }

  /**
   * Returns the recorded index ranges.
   *
   * @return the recorded index ranges
   */
  const TexturedIndexRangeMap& indices() const { return m_indexRange; }

  /**
   * Returns the recorded index ranges.
   *
   * @return the recorded index ranges
   */
  TexturedIndexRangeMap& indices() { return m_indexRange; }

  /**
   * Adds a textured point primitive at the given position.
   *
   * @param texture the texture to use
   * @param v the position of the point to add
   */
  void addPoint(const Texture* texture, const Vertex& v)
  {
    add(texture, Renderer::PrimType::Points, m_vertexListBuilder.addPoint(v));
  }

  /**
   * Adds multiple textured point primitives at the given positions.
   *
   * @param texture the texture to use
   * @param vertices the positions of the points to add
   */
  void addPoints(const Texture* texture, const VertexList& vertices)
  {
    add(texture, Renderer::PrimType::Points, m_vertexListBuilder.addPoints(vertices));
  }

  /**
   * Adds a textured line with the given end points.
   *
   * @param texture the texture to use
   * @param v1 the position of the first end point
   * @param v2 the position of the second end point
   */
  void addLine(const Texture* texture, const Vertex& v1, const Vertex& v2)
  {
    add(texture, Renderer::PrimType::Lines, m_vertexListBuilder.addLine(v1, v2));
  }

  /**
   * Adds multiple textured lines with the given endpoints. Each line to be added consists
   * of two consecutive of the given list, so for each line, two elements of the list are
   * used.
   *
   * @param texture the texture to use
   * @param vertices the end points of the lines to add
   */
  void addLines(const Texture* texture, const VertexList& vertices)
  {
    add(texture, Renderer::PrimType::Lines, m_vertexListBuilder.addLines(vertices));
  }

  /**
   * Adds a textured line strip with the given points.
   *
   * @param texture the texture to use
   * @param vertices the end points of the lines to add
   */
  void addLineStrip(const Texture* texture, const VertexList& vertices)
  {
    add(
      texture, Renderer::PrimType::LineStrip, m_vertexListBuilder.addLineStrip(vertices));
  }

  /**
   * Adds a textured line loop with the given points.
   *
   * @param texture the texture to use
   * @param vertices the end points of the lines to add
   */
  void addLineLoop(const Texture* texture, const VertexList& vertices)
  {
    add(texture, Renderer::PrimType::LineLoop, m_vertexListBuilder.addLineLoop(vertices));
  }

  /**
   * Adds a textured triangle with the given corners.
   *
   * @param texture the texture to use
   * @param v1 the position of the first corner
   * @param v2 the position of the second corner
   * @param v3 the position of the third corner
   */
  void addTriangle(
    const Texture* texture, const Vertex& v1, const Vertex& v2, const Vertex& v3)
  {
    add(
      texture,
      Renderer::PrimType::Triangles,
      m_vertexListBuilder.addTriangle(v1, v2, v3));
  }

  /**
   * Adds multiple textured triangles using the corner positions in the given list. For
   * each triangle, three positions are used.
   *
   * @param texture the texture to use
   * @param vertices the corner positions
   */
  void addTriangles(const Texture* texture, const VertexList& vertices)
  {
    add(
      texture, Renderer::PrimType::Triangles, m_vertexListBuilder.addTriangles(vertices));
  }

  /**
   * Adds a textured triangle fan using the positions of the vertices in the given list.
   *
   * @param texture the texture to use
   * @param vertices the vertex positions
   */
  void addTriangleFan(const Texture* texture, const VertexList& vertices)
  {
    add(
      texture,
      Renderer::PrimType::TriangleFan,
      m_vertexListBuilder.addTriangleFan(vertices));
  }

  /**
   * Adds a textured triangle strip using the positions of the vertices in the given list.
   *
   * @param texture the texture to use
   * @param vertices the vertex positions
   */
  void addTriangleStrip(const Texture* texture, const VertexList& vertices)
  {
    add(
      texture,
      Renderer::PrimType::TriangleStrip,
      m_vertexListBuilder.addTriangleStrip(vertices));
  }

  /**
   * Adds a textured quad with the given corners.
   *
   * @param texture the texture to use
   * @param v1 the position of the first corner
   * @param v2 the position of the second corner
   * @param v3 the position of the third corner
   * @param v4 the position of the fourth corner
   */
  void addQuad(
    const Texture* texture,
    const Vertex& v1,
    const Vertex& v2,
    const Vertex& v3,
    const Vertex& v4)
  {
    add(texture, Renderer::PrimType::Quads, m_vertexListBuilder.addQuad(v1, v2, v3, v4));
  }

  /**
   * Adds multiple textured quads using the corner positions in the given list. For each
   * quad, four positions are used.
   *
   * @param texture the texture to use
   * @param vertices the corner positions
   */
  void addQuads(const Texture* texture, const VertexList& vertices)
  {
    add(texture, Renderer::PrimType::Quads, m_vertexListBuilder.addQuads(vertices));
  }

  /**
   * Adds a textured quad strip using the positions of the vertices in the given list.
   *
   * @param texture the texture to use
   * @param vertices the vertex positions
   */
  void addQuadStrip(const Texture* texture, const VertexList& vertices)
  {
    add(
      texture, Renderer::PrimType::QuadStrip, m_vertexListBuilder.addQuadStrip(vertices));
  }

  /**
   * Adds a textured polygon with the given corners.
   *
   * @param texture the texture to use
   * @param vertices the croner positions
   */
  void addPolygon(const Texture* texture, const VertexList& vertices)
  {
    add(texture, Renderer::PrimType::Polygon, m_vertexListBuilder.addPolygon(vertices));
  }

private:
  void add(const Texture* texture, const PrimType primType, const IndexData& data)
  {
    m_indexRange.add(texture, primType, data.index, data.count);
  }
};
} // namespace Renderer
} // namespace TrenchBroom
