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

#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
template <typename VertexSpec>
class VertexListBuilder
{
public:
  // FIXME: move out or make private
  struct Range
  {
    size_t index;
    size_t count;

    Range(const size_t i_index, const size_t i_count)
      : index(i_index)
      , count(i_count)
    {
    }
  };

private:
  using Vertex = typename VertexSpec::Vertex;
  using VertexList = std::vector<Vertex>;

private:
  VertexList m_vertices;
  bool m_dynamicGrowth;

public:
  explicit VertexListBuilder(const size_t capacity)
    : m_vertices()
    , m_dynamicGrowth(false)
  {
    m_vertices.reserve(capacity);
  }

  VertexListBuilder()
    : m_dynamicGrowth(true)
  {
  }

  size_t vertexCount() const { return m_vertices.size(); }

  const VertexList& vertices() const { return m_vertices; }

  VertexList& vertices() { return m_vertices; }

  Range addPoint(const Vertex& v1)
  {
    assert(checkCapacity(1));

    const size_t index = currentIndex();
    m_vertices.push_back(v1);

    return Range(index, 1);
  }

  Range addPoints(const VertexList& vertices) { return addVertices(vertices); }

  Range addLine(const Vertex& v1, const Vertex& v2)
  {
    assert(checkCapacity(2));

    const size_t index = currentIndex();
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);

    return Range(index, 2);
  }

  Range addLines(const VertexList& vertices)
  {
    assert(vertices.size() % 2 == 0);
    return addVertices(vertices);
  }

  Range addLineStrip(const VertexList& vertices)
  {
    assert(vertices.size() >= 2);
    return addVertices(vertices);
  }

  Range addLineLoop(const VertexList& vertices)
  {
    assert(vertices.size() >= 3);
    return addVertices(vertices);
  }

  Range addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
  {
    assert(checkCapacity(3));

    const size_t index = currentIndex();
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    return Range(index, 3);
  }

  Range addTriangles(const VertexList& vertices)
  {
    assert(vertices.size() % 3 == 0);
    return addVertices(vertices);
  }

  Range addTriangleFan(const VertexList& vertices)
  {
    assert(vertices.size() >= 3);
    return addVertices(vertices);
  }

  Range addTriangleStrip(const VertexList& vertices)
  {
    assert(vertices.size() >= 3);
    return addVertices(vertices);
  }

  Range addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4)
  {
    assert(checkCapacity(4));

    const size_t index = currentIndex();
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);
    m_vertices.push_back(v4);

    return Range(index, 4);
  }

  Range addQuads(const VertexList& vertices)
  {
    assert(vertices.size() % 4 == 0);
    return addVertices(vertices);
  }

  Range addQuadStrip(const VertexList& vertices)
  {
    assert(vertices.size() >= 4);
    assert(vertices.size() % 2 == 0);
    return addVertices(vertices);
  }

  Range addPolygon(const VertexList& vertices)
  {
    assert(vertices.size() >= 3);
    return addVertices(vertices);
  }

private:
  Range addVertices(const VertexList& vertices)
  {
    assert(checkCapacity(vertices.size()));

    const size_t index = currentIndex();
    const size_t count = vertices.size();
    m_vertices = kdl::vec_concat(std::move(m_vertices), vertices);

    return Range(index, count);
  }

  bool checkCapacity(const size_t toAdd) const
  {
    return m_dynamicGrowth || m_vertices.capacity() - m_vertices.size() >= toAdd;
  }

  size_t currentIndex() const { return vertexCount(); }
};
} // namespace Renderer
} // namespace TrenchBroom
