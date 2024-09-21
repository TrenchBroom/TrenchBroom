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

#include "MaterialIndexArrayMapBuilder.h"

#include "Renderer/PrimType.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom
{
namespace Renderer
{
MaterialIndexArrayMapBuilder::MaterialIndexArrayMapBuilder(
  const MaterialIndexArrayMap::Size& size)
  : m_ranges{size}
{
  m_indices.resize(size.indexCount());
}

MaterialIndexArrayMapBuilder::IndexList& MaterialIndexArrayMapBuilder::indices()
{
  return m_indices;
}

MaterialIndexArrayMap& MaterialIndexArrayMapBuilder::ranges()
{
  return m_ranges;
}

void MaterialIndexArrayMapBuilder::addPoint(const Material* material, const Index i)
{
  const size_t offset = m_ranges.add(material, PrimType::Points, 1);
  m_indices[offset] = i;
}

void MaterialIndexArrayMapBuilder::addPoints(
  const Material* material, const IndexList& indices)
{
  add(material, PrimType::Points, indices);
}

void MaterialIndexArrayMapBuilder::addLine(
  const Material* material, const Index i1, const Index i2)
{
  const size_t offset = m_ranges.add(material, PrimType::Lines, 2);
  m_indices[offset + 0] = i1;
  m_indices[offset + 1] = i2;
}

void MaterialIndexArrayMapBuilder::addLines(
  const Material* material, const IndexList& indices)
{
  assert(indices.size() % 2 == 0);
  add(material, PrimType::Lines, indices);
}

void MaterialIndexArrayMapBuilder::addTriangle(
  const Material* material, const Index i1, const Index i2, const Index i3)
{
  const size_t offset = m_ranges.add(material, PrimType::Triangles, 3);
  m_indices[offset + 0] = i1;
  m_indices[offset + 1] = i2;
  m_indices[offset + 2] = i3;
}

void MaterialIndexArrayMapBuilder::addTriangles(
  const Material* material, const IndexList& indices)
{
  assert(indices.size() % 3 == 0);
  add(material, PrimType::Triangles, indices);
}

void MaterialIndexArrayMapBuilder::addQuad(
  const Material* material,
  const Index,
  const Index i1,
  const Index i2,
  const Index i3,
  const Index i4)
{
  const size_t offset = m_ranges.add(material, PrimType::Quads, 4);
  m_indices[offset + 0] = i1;
  m_indices[offset + 1] = i2;
  m_indices[offset + 2] = i3;
  m_indices[offset + 3] = i4;
}

void MaterialIndexArrayMapBuilder::addQuads(
  const Material* material, const IndexList& indices)
{
  assert(indices.size() % 4 == 0);
  add(material, PrimType::Quads, indices);
}

void MaterialIndexArrayMapBuilder::addQuads(
  const Material* material, const Index baseIndex, const size_t vertexCount)
{
  assert(vertexCount % 4 == 0);
  IndexList indices(vertexCount);

  for (size_t i = 0; i < vertexCount; ++i)
  {
    indices[i] = baseIndex + static_cast<Index>(i);
  }

  add(material, PrimType::Quads, indices);
}

void MaterialIndexArrayMapBuilder::addPolygon(
  const Material* material, const IndexList& indices)
{
  const size_t count = indices.size();

  auto polyIndices = IndexList{};
  polyIndices.reserve(3 * (count - 2));

  for (size_t i = 0; i < count - 2; ++i)
  {
    polyIndices.push_back(indices[0]);
    polyIndices.push_back(indices[i + 1]);
    polyIndices.push_back(indices[i + 2]);
  }

  add(material, PrimType::Triangles, polyIndices);
}

void MaterialIndexArrayMapBuilder::addPolygon(
  const Material* material, const Index baseIndex, const size_t vertexCount)
{
  auto polyIndices = IndexList{};
  polyIndices.reserve(3 * (vertexCount - 2));

  for (size_t i = 0; i < vertexCount - 2; ++i)
  {
    polyIndices.push_back(baseIndex);
    polyIndices.push_back(baseIndex + static_cast<Index>(i + 1));
    polyIndices.push_back(baseIndex + static_cast<Index>(i + 2));
  }

  add(material, PrimType::Triangles, polyIndices);
}

void MaterialIndexArrayMapBuilder::add(
  const Material* material, const PrimType primType, const IndexList& indices)
{
  const size_t offset = m_ranges.add(material, primType, indices.size());
  auto dest = std::begin(m_indices);
  std::advance(dest, static_cast<IndexList::iterator::difference_type>(offset));
  std::copy(std::begin(indices), std::end(indices), dest);
}
} // namespace Renderer
} // namespace TrenchBroom
