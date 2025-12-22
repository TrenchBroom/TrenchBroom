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
#include "mdl/TextureResource.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;

namespace fs
{
class FileSystem;
} // namespace fs

namespace mdl
{
class Material;
class MaterialCollection;

struct MaterialConfig;

class MaterialManager
{
private:
  CreateTextureResource m_createResource;
  Logger& m_logger;

  std::vector<MaterialCollection> m_collections;

  std::unordered_map<std::string, Material*> m_materialsByName;
  std::vector<const Material*> m_materials;

public:
  MaterialManager(CreateTextureResource createResource, Logger& logger);
  ~MaterialManager();

  void reload(
    const fs::FileSystem& fs,
    const MaterialConfig& materialConfig,
    kdl::task_manager& taskManager);

  // for testing
  void setMaterialCollections(std::vector<MaterialCollection> collections);

private:
  void addMaterialCollection(MaterialCollection collection);

public:
  void clear();

  const Material* material(const std::string& name) const;
  Material* material(const std::string& name);

  const std::vector<const Material*> findMaterialsByTextureResourceId(
    const std::vector<gl::ResourceId>& textureResourceIds) const;

  const std::vector<const Material*>& materials() const;
  const std::vector<MaterialCollection>& collections() const;

private:
  void updateMaterials();
};
} // namespace mdl
} // namespace tb
