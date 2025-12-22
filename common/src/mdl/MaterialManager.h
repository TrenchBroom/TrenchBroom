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

#include "gl/ResourceId.h"
#include "mdl/MaterialCollection.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace tb
{
class Logger;

namespace gl
{
class Material;
}

namespace mdl
{
class MaterialCollection;

class MaterialManager
{
private:
  Logger& m_logger;

  std::vector<MaterialCollection> m_collections;

  std::unordered_map<std::string, gl::Material*> m_materialsByName;
  std::vector<const gl::Material*> m_materials;

public:
  explicit MaterialManager(Logger& logger);
  ~MaterialManager();

  void setMaterialCollections(std::vector<MaterialCollection> collections);

  void clear();

  const gl::Material* material(const std::string& name) const;
  gl::Material* material(const std::string& name);

  const std::vector<const gl::Material*> findMaterialsByTextureResourceId(
    const std::vector<gl::ResourceId>& textureResourceIds) const;

  const std::vector<const gl::Material*>& materials() const;
  const std::vector<MaterialCollection>& collections() const;

private:
  void addMaterialCollection(MaterialCollection collection);
  void updateMaterials();
};

} // namespace mdl
} // namespace tb
