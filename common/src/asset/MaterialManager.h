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

#include "asset/MaterialCollection.h"
#include "asset/TextureResource.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace tb
{
class Logger;

namespace io
{
class FileSystem;
} // namespace io

namespace mdl
{
struct MaterialConfig;
}

namespace asset
{
class Material;
class MaterialCollection;
class ResourceId;

class MaterialManager
{
private:
  Logger& m_logger;

  std::vector<MaterialCollection> m_collections;

  std::unordered_map<std::string, Material*> m_materialsByName;
  std::vector<const Material*> m_materials;

public:
  explicit MaterialManager(Logger& logger);
  ~MaterialManager();

  void reload(
    const io::FileSystem& fs,
    const mdl::MaterialConfig& materialConfig,
    const asset::CreateTextureResource& createResource);

  // for testing
  void setMaterialCollections(std::vector<MaterialCollection> collections);

private:
  void addMaterialCollection(asset::MaterialCollection collection);

public:
  void clear();

  const Material* material(const std::string& name) const;
  Material* material(const std::string& name);

  const std::vector<const Material*> findMaterialsByTextureResourceId(
    const std::vector<ResourceId>& textureResourceIds) const;

  const std::vector<const Material*>& materials() const;
  const std::vector<MaterialCollection>& collections() const;

private:
  void updateMaterials();
};
} // namespace asset
} // namespace tb
