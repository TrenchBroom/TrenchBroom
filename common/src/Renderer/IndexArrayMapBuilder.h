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

#include "Renderer/GL.h"
#include "Renderer/IndexArrayMap.h"

#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
/**
 * Builds an index array map by recording rendering primitives. The recorded data can be
 * used to create an index array that can be uploaded to video card memory, and to render
 * the recorded primitives with the indices from that array using the constructed index
 * array map.
 */
class IndexArrayMapBuilder
{
public:
  using Index = GLuint;
  using IndexList = std::vector<Index>;

private:
  IndexList m_indices;
  IndexArrayMap m_ranges;

public:
  /**
   * Creates a new builder with the internal index array map initialized to the given
   * size.
   *
   * @param size the size to initialize to
   */
  explicit IndexArrayMapBuilder(const IndexArrayMap::Size& size);

  /**
   * Returns the recorded indices.
   *
   * @return the recorded indices
   */
  const IndexList& indices() const;

  /**
   * Returns the recorded indices.
   *
   * @return the recorded indices
   */
  IndexList& indices();

  /**
   * Returns the recorded index ranges for the primitives that were added.
   *
   * @return the recorded index ranges
   */
  const IndexArrayMap& ranges() const;

  /**
   * Adds a point, represented by a vertex in a vertex array at the given index.
   *
   * @param i the index to record
   */
  void addPoint(Index i);

  /**
   * Adds multiple points, represented by the vertices in a vertex array at the given
   * indices.
   *
   * @param indices the indices to record
   */
  void addPoints(const IndexList& indices);

  /**
   * Adds a line, represented by the vertices in a vertex array at the given two indices.
   *
   * @param i1 the index of the start vertex to record
   * @param i2 the index of the end vertex to record
   */
  void addLine(Index i1, Index i2);

  /**
   * Adds multiple lines, each represented by two vertices in a vertex array. The given
   * index array contains pairs of indices, where each pair consists of the index of the
   * first and the index of the second vertex.
   *
   * @param indices a list of indices containing the pairs of vertex indices to record
   */
  void addLines(const IndexList& indices);

  /**
   * Adds a triangle, represented by the vertices in a vertex array at the given indices.
   *
   * @param i1 the index of the first vertex to record
   * @param i2 the index of the second vertex to record
   * @param i3 the index of the third vertex to record
   */
  void addTriangle(Index i1, Index i2, Index i3);

  /**
   * Adds multiple triangles, each represented by three vertices in a vertex array. The
   * given index array contains triples of indices, where each triple consists of the
   * indices of the three vertices making up the triangle to add.
   *
   * @param indices a list of indices containing the triples of vertex indices to record
   */
  void addTriangles(const IndexList& indices);

  /**
   * Adds a quad, represented by the vertices in a vertex array at the given indices.
   *
   * @param i1 the index of the first vertex to record
   * @param i2 the index of the second vertex to record
   * @param i3 the index of the third vertex to record
   * @param i4 the index of the fourth vertex to record
   */
  void addQuad(Index, Index i1, Index i2, Index i3, Index i4);

  /**
   * Adds multiple quads, each represented by four vertices in a vertex array. The given
   * index array contains four-tuples of indices, where each tuple consists of the indices
   * of the four vertices making up the quad to add.
   *
   * @param indices a list of indices containing the four-tuples of vertex indices to
   * record
   */
  void addQuads(const IndexList& indices);

  /**
   * Adds multiple quads by adding a range of indices specified by the given base index
   * and length. Specifically, a call to this method records a sequence of indices which
   * is computed using the given base index and vertex count as follows.
   *
   * index 1 = baseIndex + 0;
   * index 2 = baseIndex + 1;
   * ...
   * index n = baseIndex + vertexCount - 1;
   *
   * @param baseIndex the base index at which to start the range
   * @param vertexCount the number of vertices contained in the range
   */
  void addQuads(Index baseIndex, size_t vertexCount);

  /**
   * Adds a polygon with the given indices. Note that the polygon is translated to a set
   * of triangles and no actual polygon is recorded at all.
   *
   * @param indices the indices of the vertices making up the polygon to add
   */
  void addPolygon(const IndexList& indices);

  /**
   * Adds a polygon with indices computed from the given range. The polygons vertices are
   * expected to be stored sequentially in a vertex array, starting at the given base
   * index. The given vertex count indicates the number of vertices to add. Note that the
   * polygon is translated to a set of triangles.
   *
   * @param baseIndex the index of the first vertex of the polygon
   * @param vertexCount the number of vertices of the polygon
   */
  void addPolygon(Index baseIndex, size_t vertexCount);

private:
  void add(PrimType primType, const IndexList& indices);
};
} // namespace Renderer
} // namespace TrenchBroom
