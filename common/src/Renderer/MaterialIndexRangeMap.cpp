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

#include "MaterialIndexRangeMap.h"

#include "Renderer/RenderUtils.h"

#include <cassert>

namespace TrenchBroom
{
namespace Renderer
{
MaterialIndexRangeMap::Size::Size()
  : m_current(std::end(m_sizes))
{
}

void MaterialIndexRangeMap::Size::inc(
  const Material* material, const PrimType primType, const size_t vertexCount)
{
  auto& sizeForKey = findCurrent(material);
  sizeForKey.inc(primType, vertexCount);
}

void MaterialIndexRangeMap::Size::inc(const MaterialIndexRangeMap::Size& other)
{
  for (const auto& [material, indexRange] : other.m_sizes)
  {
    auto& sizeForKey = findCurrent(material);
    sizeForKey.inc(indexRange);
  }
}

IndexRangeMap::Size& MaterialIndexRangeMap::Size::findCurrent(const Material* material)
{
  if (!isCurrent(material))
  {
    const auto result = m_sizes.try_emplace(material);
    m_current = result.first;
  }
  return m_current->second;
}

bool MaterialIndexRangeMap::Size::isCurrent(const Material* material) const
{
  if (m_current == std::end(m_sizes))
  {
    return false;
  }

  const auto& cmp = m_sizes.key_comp();
  const auto* currentMaterial = m_current->first;
  return !cmp(material, currentMaterial) && !cmp(currentMaterial, material);
}

void MaterialIndexRangeMap::Size::initialize(MaterialToIndexRangeMap& data) const
{
  for (const auto& [material, size] : m_sizes)
  {
    data.insert(std::make_pair(material, IndexRangeMap(size)));
  }
}

MaterialIndexRangeMap::MaterialIndexRangeMap()
  : m_data(new MaterialToIndexRangeMap())
  , m_current(m_data->end())
{
}

MaterialIndexRangeMap::MaterialIndexRangeMap(const Size& size)
  : m_data(new MaterialToIndexRangeMap())
  , m_current(m_data->end())
{
  size.initialize(*m_data);
}

MaterialIndexRangeMap::MaterialIndexRangeMap(
  const Material* material, IndexRangeMap primitives)
  : m_data(new MaterialToIndexRangeMap())
  , m_current(m_data->end())
{
  add(material, std::move(primitives));
}

MaterialIndexRangeMap::MaterialIndexRangeMap(
  const Material* material,
  const PrimType primType,
  const size_t index,
  const size_t vertexCount)
  : m_data(new MaterialToIndexRangeMap())
  , m_current(m_data->end())
{
  m_data->insert(std::make_pair(material, IndexRangeMap(primType, index, vertexCount)));
}

void MaterialIndexRangeMap::add(
  const Material* material,
  const PrimType primType,
  const size_t index,
  const size_t vertexCount)
{
  auto& current = findCurrent(material);
  current.add(primType, index, vertexCount);
}

void MaterialIndexRangeMap::add(const Material* material, IndexRangeMap primitives)
{
  m_data->insert(std::make_pair(material, std::move(primitives)));
}

void MaterialIndexRangeMap::add(const MaterialIndexRangeMap& other)
{
  for (const auto& [material, indexRangeMap] : *other.m_data)
  {
    auto& current = findCurrent(material);
    current.add(indexRangeMap);
  }
}

void MaterialIndexRangeMap::render(VertexArray& vertexArray)
{
  DefaultMaterialRenderFunc func;
  render(vertexArray, func);
}

void MaterialIndexRangeMap::render(VertexArray& vertexArray, MaterialRenderFunc& func)
{
  for (const auto& [material, indexArray] : *m_data)
  {
    func.before(material);
    indexArray.render(vertexArray);
    func.after(material);
  }
}

void MaterialIndexRangeMap::forEachPrimitive(
  std::function<void(const Material*, PrimType, size_t, size_t)> func) const
{
  for (const auto& entry : *m_data)
  {
    const auto* material = entry.first;
    const auto& indexArray = entry.second;

    indexArray.forEachPrimitive(
      [&func, &material](
        const PrimType primType, const size_t index, const size_t count) {
        func(material, primType, index, count);
      });
  }
}

IndexRangeMap& MaterialIndexRangeMap::findCurrent(const Material* material)
{
  if (!isCurrent(material))
  {
    m_current = m_data->find(material);
  }
  assert(m_current != m_data->end());
  return m_current->second;
}

bool MaterialIndexRangeMap::isCurrent(const Material* material) const
{
  if (m_current == m_data->end())
  {
    return false;
  }

  const auto& cmp = m_data->key_comp();
  const auto* currentMaterial = m_current->first;
  return !cmp(material, currentMaterial) && !cmp(currentMaterial, material);
}
} // namespace Renderer
} // namespace TrenchBroom
