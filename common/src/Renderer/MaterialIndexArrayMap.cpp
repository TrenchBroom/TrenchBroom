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

#include "MaterialIndexArrayMap.h"

#include "Renderer/RenderUtils.h"

#include <cassert>

namespace TrenchBroom
{
namespace Renderer
{
MaterialIndexArrayMap::Size::Size()
  : m_indexCount{0u}
{
}

size_t MaterialIndexArrayMap::Size::indexCount() const
{
  return m_indexCount;
}

void MaterialIndexArrayMap::Size::inc(
  const Material* material, const PrimType primType, const size_t count)
{
  m_sizes[material].inc(primType, count);
  m_indexCount += count;
}

void MaterialIndexArrayMap::Size::inc(
  const MaterialIndexArrayMap::Material* material, const IndexArrayMap::Size& size)
{
  m_sizes[material].inc(size);
  m_indexCount += size.indexCount();
}

void MaterialIndexArrayMap::Size::initialize(MaterialToIndexArrayMap& ranges) const
{
  size_t baseOffset = 0;

  for (const auto& [material, size] : m_sizes)
  {
    ranges.emplace(material, IndexArrayMap{size, baseOffset});
    baseOffset += size.indexCount();
  }
}

MaterialIndexArrayMap::MaterialIndexArrayMap() = default;

MaterialIndexArrayMap::MaterialIndexArrayMap(const Size& size)
{
  size.initialize(m_ranges);
}

MaterialIndexArrayMap::Size MaterialIndexArrayMap::size() const
{
  auto result = Size{};
  for (const auto& [material, indexArray] : m_ranges)
  {
    result.inc(material, indexArray.size());
  }
  return result;
}

size_t MaterialIndexArrayMap::add(
  const Material* material, const PrimType primType, const size_t count)
{
  auto it = m_ranges.find(material);
  assert(it != std::end(m_ranges));
  return it->second.add(primType, count);
}

void MaterialIndexArrayMap::render(IndexArray& indexArray, MaterialRenderFunc& func)
{
  for (const auto& [material, indexRange] : m_ranges)
  {
    func.before(material);
    indexRange.render(indexArray);
    func.after(material);
  }
}
} // namespace Renderer
} // namespace TrenchBroom
