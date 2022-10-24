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

#include "IndexArrayMap.h"

#include "Renderer/IndexArray.h"

namespace TrenchBroom
{
namespace Renderer
{
IndexArrayMap::IndexArrayRange::IndexArrayRange(
  const size_t i_offset, const size_t i_capacity)
  : offset{i_offset}
  , capacity{i_capacity}
  , count{0u}
{
}

size_t IndexArrayMap::IndexArrayRange::add(const size_t i_count)
{
  assert(capacity - count >= i_count);
  const auto result = offset + count;
  count += i_count;
  return result;
}

IndexArrayMap::Size::Size()
  : m_indexCount{0u}
{
}

void IndexArrayMap::Size::inc(const PrimType primType, const size_t count)
{
  m_sizes[primType] +=
    count; // unknown map values are value constructed, which initializes to 0 for size_t
  m_indexCount += count;
}

void IndexArrayMap::Size::inc(const IndexArrayMap::Size& other)
{
  for (const auto& [primType, size] : other.m_sizes)
  {
    inc(primType, size);
  }
}

size_t IndexArrayMap::Size::indexCount() const
{
  return m_indexCount;
}

void IndexArrayMap::Size::initialize(
  PrimTypeToRangeMap& data, const size_t baseOffset) const
{
  auto offset = baseOffset;
  for (const auto& [primType, size] : m_sizes)
  {
    data.emplace(primType, IndexArrayRange{offset, size});
    offset += size;
  }
}

IndexArrayMap::IndexArrayMap(const Size& size)
  : IndexArrayMap{size, 0u}
{
}

IndexArrayMap::IndexArrayMap(const Size& size, const size_t baseOffset)
{
  size.initialize(m_ranges, baseOffset);
}

IndexArrayMap::Size IndexArrayMap::size() const
{
  Size result;
  for (const auto& [primType, range] : m_ranges)
  {
    result.inc(primType, range.capacity);
  }
  return result;
}

size_t IndexArrayMap::add(const PrimType primType, const size_t count)
{
  auto it = m_ranges.find(primType);
  assert(it != std::end(m_ranges));
  return it->second.add(count);
}

void IndexArrayMap::render(IndexArray& indexArray) const
{
  for (const auto& [primType, range] : m_ranges)
  {
    indexArray.render(primType, range.offset, range.count);
  }
}
} // namespace Renderer
} // namespace TrenchBroom
