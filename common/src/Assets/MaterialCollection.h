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

#include "Assets/Material.h"

#include "kdl/reflection_decl.h"

#include <filesystem>
#include <string>
#include <vector>

namespace TrenchBroom::Assets
{

class MaterialCollection
{
private:
  std::filesystem::path m_path;
  std::vector<Material> m_materials;

  friend class Material;

  kdl_reflect_decl(MaterialCollection, m_path, m_materials);

public:
  MaterialCollection();
  explicit MaterialCollection(std::vector<Material> materials);
  explicit MaterialCollection(std::filesystem::path path);
  MaterialCollection(std::filesystem::path path, std::vector<Material> materials);

  MaterialCollection(const MaterialCollection&) = delete;
  MaterialCollection& operator=(const MaterialCollection&) = delete;

  MaterialCollection(MaterialCollection&& other) = default;
  MaterialCollection& operator=(MaterialCollection&& other) = default;

  const std::filesystem::path& path() const;
  size_t materialCount() const;

  const std::vector<Material>& materials() const;
  std::vector<Material>& materials();

  const Material* materialByIndex(size_t index) const;
  Material* materialByIndex(size_t index);

  const Material* materialByName(const std::string& name) const;
  Material* materialByName(const std::string& name);

  void setFilterMode(int minFilter, int magFilter);
};

} // namespace TrenchBroom::Assets
