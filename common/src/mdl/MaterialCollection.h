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

#pragma once

#include "gl/Material.h"

#include "kd/reflection_decl.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::mdl
{

class MaterialCollection
{
private:
  std::filesystem::path m_path;
  std::vector<gl::Material> m_materials;

  kdl_reflect_decl(MaterialCollection, m_path, m_materials);

public:
  MaterialCollection();
  explicit MaterialCollection(std::vector<gl::Material> materials);
  explicit MaterialCollection(std::filesystem::path path);
  MaterialCollection(std::filesystem::path path, std::vector<gl::Material> materials);

  MaterialCollection(const MaterialCollection&) = delete;
  MaterialCollection& operator=(const MaterialCollection&) = delete;

  MaterialCollection(MaterialCollection&& other) = default;
  MaterialCollection& operator=(MaterialCollection&& other) = default;

  const std::filesystem::path& path() const;
  size_t materialCount() const;

  const std::vector<gl::Material>& materials() const;
  std::vector<gl::Material>& materials();

  const gl::Material* materialByIndex(size_t index) const;
  gl::Material* materialByIndex(size_t index);

  const gl::Material* materialByName(const std::string& name) const;
  gl::Material* materialByName(const std::string& name);
};

} // namespace tb::mdl
