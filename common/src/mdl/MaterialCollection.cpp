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

#include "MaterialCollection.h"

#include "kd/const_overload.h"
#include "kd/reflection_impl.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tb::mdl
{

kdl_reflect_impl(MaterialCollection);

MaterialCollection::MaterialCollection() = default;

MaterialCollection::MaterialCollection(std::vector<Material> materials)
  : m_materials{std::move(materials)}
{
}

MaterialCollection::MaterialCollection(std::filesystem::path path)
  : m_path{std::move(path)}
{
}

MaterialCollection::MaterialCollection(
  std::filesystem::path path, std::vector<Material> materials)
  : m_path{std::move(path)}
  , m_materials{std::move(materials)}
{
}

const std::filesystem::path& MaterialCollection::path() const
{
  return m_path;
}

size_t MaterialCollection::materialCount() const
{
  return m_materials.size();
}

const std::vector<Material>& MaterialCollection::materials() const
{
  return m_materials;
}

std::vector<Material>& MaterialCollection::materials()
{
  return m_materials;
}

const Material* MaterialCollection::materialByIndex(const size_t index) const
{
  return index < m_materials.size() ? &m_materials[index] : nullptr;
}

Material* MaterialCollection::materialByIndex(const size_t index)
{
  return KDL_CONST_OVERLOAD(materialByIndex(index));
}

const Material* MaterialCollection::materialByName(const std::string& name) const
{
  const auto it = std::ranges::find_if(
    m_materials, [&](const auto& material) { return material.name() == name; });
  return it != m_materials.end() ? &*it : nullptr;
}

Material* MaterialCollection::materialByName(const std::string& name)
{
  return KDL_CONST_OVERLOAD(materialByName(name));
}

} // namespace tb::mdl
