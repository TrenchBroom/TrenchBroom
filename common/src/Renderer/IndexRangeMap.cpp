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

#include "IndexRangeMap.h"

#include "Renderer/PrimType.h"
#include "Renderer/VertexArray.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom
{
namespace Renderer
{
IndexRangeMap::IndicesAndCounts::IndicesAndCounts()
  : indices()
  , counts()
{
}

IndexRangeMap::IndicesAndCounts::IndicesAndCounts(const size_t index, const size_t count)
  : indices(1, static_cast<GLint>(index))
  , counts(1, static_cast<GLsizei>(count))
{
}

bool IndexRangeMap::IndicesAndCounts::empty() const
{
  return indices.empty();
}

size_t IndexRangeMap::IndicesAndCounts::size() const
{
  return indices.size();
}

void IndexRangeMap::IndicesAndCounts::reserve(const size_t capacity)
{
  indices.reserve(capacity);
  counts.reserve(capacity);
}

void IndexRangeMap::IndicesAndCounts::add(
  const PrimType primType,
  const size_t index,
  const size_t count,
  [[maybe_unused]] const bool dynamicGrowth)
{
  switch (primType)
  {
  case PrimType::Points:
  case PrimType::Lines:
  case PrimType::Triangles:
  case PrimType::Quads: {
    if (size() == 1)
    {
      const auto myIndex = indices.front();
      auto& myCount = counts.front();

      if (index == static_cast<size_t>(myIndex) + static_cast<size_t>(myCount))
      {
        myCount += static_cast<GLsizei>(count);
        break;
      }
    }
    switchFallthrough();
  }
  case PrimType::LineStrip:
  case PrimType::LineLoop:
  case PrimType::TriangleFan:
  case PrimType::TriangleStrip:
  case PrimType::QuadStrip:
  case PrimType::Polygon:
    assert(dynamicGrowth || indices.capacity() > indices.size());
    indices.push_back(static_cast<GLint>(index));
    counts.push_back(static_cast<GLsizei>(count));
    break;
  }
}

void IndexRangeMap::IndicesAndCounts::add(
  const IndicesAndCounts& other, [[maybe_unused]] const bool dynamicGrowth)
{
  assert(dynamicGrowth || indices.capacity() >= indices.size() + other.indices.size());
  indices = kdl::vec_concat(std::move(indices), other.indices);
  counts = kdl::vec_concat(std::move(counts), other.counts);
}

void IndexRangeMap::Size::inc(const PrimType primType, const size_t count)
{
  m_sizes[primType] += count;
}

void IndexRangeMap::Size::inc(const IndexRangeMap::Size& other)
{
  for (const auto& primType : PrimTypeValues)
  {
    inc(primType, other.m_sizes[primType]);
  }
}

void IndexRangeMap::Size::initialize(PrimTypeToIndexData& data) const
{
  for (const auto& primType : PrimTypeValues)
  {
    data[primType].reserve(m_sizes[primType]);
  }
}

IndexRangeMap::IndexRangeMap()
  : m_data(new PrimTypeToIndexData())
  , m_dynamicGrowth(true)
{
}

IndexRangeMap::IndexRangeMap(const Size& size)
  : m_data(new PrimTypeToIndexData())
  , m_dynamicGrowth(false)
{
  size.initialize(*m_data);
}

IndexRangeMap::IndexRangeMap(
  const PrimType primType, const size_t index, const size_t count)
  : m_data(new PrimTypeToIndexData())
  , m_dynamicGrowth(false)
{
  m_data->get(primType) = IndicesAndCounts(index, count);
}

IndexRangeMap::Size IndexRangeMap::size() const
{
  Size result;
  for (const auto& primType : PrimTypeValues)
  {
    result.inc(primType, m_data->get(primType).size());
  }
  return result;
}

void IndexRangeMap::add(const PrimType primType, const size_t index, const size_t count)
{
  auto& indicesAndCounts = m_data->get(primType);
  indicesAndCounts.add(primType, index, count, m_dynamicGrowth);
}

void IndexRangeMap::add(const IndexRangeMap& other)
{
  for (const auto& primType : PrimTypeValues)
  {
    const auto& indicesToAdd = other.m_data->get(primType);

    auto& indicesAndCounts = m_data->get(primType);
    indicesAndCounts.add(indicesToAdd, m_dynamicGrowth);
  }
}

void IndexRangeMap::render(VertexArray& vertexArray) const
{
  for (const auto& primType : PrimTypeValues)
  {
    const auto& indicesAndCounts = m_data->get(primType);
    if (!indicesAndCounts.empty())
    {
      const auto primCount = static_cast<GLsizei>(indicesAndCounts.size());
      vertexArray.render(
        primType, indicesAndCounts.indices, indicesAndCounts.counts, primCount);
    }
  }
}

void IndexRangeMap::forEachPrimitive(
  std::function<void(PrimType, size_t, size_t)> func) const
{
  for (const auto& primType : PrimTypeValues)
  {
    const auto& indicesAndCounts = m_data->get(primType);
    if (!indicesAndCounts.empty())
    {
      const auto primCount = indicesAndCounts.size();
      for (std::size_t i = 0; i < primCount; ++i)
      {
        func(
          primType,
          static_cast<std::size_t>(indicesAndCounts.indices[i]),
          static_cast<std::size_t>(indicesAndCounts.counts[i]));
      }
    }
  }
}
} // namespace Renderer
} // namespace TrenchBroom
