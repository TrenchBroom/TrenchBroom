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

#include "TexturedIndexArrayMap.h"

#include "Renderer/RenderUtils.h"

#include <cassert>

namespace TrenchBroom
{
namespace Renderer
{
TexturedIndexArrayMap::Size::Size()
  : m_indexCount{0u}
{
}

size_t TexturedIndexArrayMap::Size::indexCount() const
{
  return m_indexCount;
}

void TexturedIndexArrayMap::Size::inc(
  const Texture* texture, const PrimType primType, const size_t count)
{
  m_sizes[texture].inc(primType, count);
  m_indexCount += count;
}

void TexturedIndexArrayMap::Size::inc(
  const TexturedIndexArrayMap::Texture* texture, const IndexArrayMap::Size& size)
{
  m_sizes[texture].inc(size);
  m_indexCount += size.indexCount();
}

void TexturedIndexArrayMap::Size::initialize(TextureToIndexArrayMap& ranges) const
{
  size_t baseOffset = 0;

  for (const auto& [texture, size] : m_sizes)
  {
    ranges.emplace(texture, IndexArrayMap{size, baseOffset});
    baseOffset += size.indexCount();
  }
}

TexturedIndexArrayMap::TexturedIndexArrayMap() = default;

TexturedIndexArrayMap::TexturedIndexArrayMap(const Size& size)
{
  size.initialize(m_ranges);
}

TexturedIndexArrayMap::Size TexturedIndexArrayMap::size() const
{
  auto result = Size{};
  for (const auto& [texture, indexArray] : m_ranges)
  {
    result.inc(texture, indexArray.size());
  }
  return result;
}

size_t TexturedIndexArrayMap::add(
  const Texture* texture, const PrimType primType, const size_t count)
{
  auto it = m_ranges.find(texture);
  assert(it != std::end(m_ranges));
  return it->second.add(primType, count);
}

void TexturedIndexArrayMap::render(IndexArray& indexArray)
{
  auto func = DefaultTextureRenderFunc{};
  render(indexArray, func);
}

void TexturedIndexArrayMap::render(IndexArray& indexArray, TextureRenderFunc& func)
{
  for (const auto& [texture, indexRange] : m_ranges)
  {
    func.before(texture);
    indexRange.render(indexArray);
    func.after(texture);
  }
}
} // namespace Renderer
} // namespace TrenchBroom
