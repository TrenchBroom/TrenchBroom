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

#include "Assets/MaterialCollection.h"
#include "Assets/TextureResource.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace IO
{
class FileSystem;
} // namespace IO

namespace Model
{
struct MaterialConfig;
}

namespace Assets
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

  int m_minFilter;
  int m_magFilter;
  bool m_resetFilterMode{false};

public:
  MaterialManager(int magFilter, int minFilter, Logger& logger);
  ~MaterialManager();

  void reload(
    const IO::FileSystem& fs,
    const Model::MaterialConfig& materialConfig,
    const Assets::CreateTextureResource& createResource);

  // for testing
  void setMaterialCollections(std::vector<MaterialCollection> collections);

private:
  void addMaterialCollection(Assets::MaterialCollection collection);

public:
  void clear();

  void setFilterMode(int minFilter, int magFilter);

  const Material* material(const std::string& name) const;
  Material* material(const std::string& name);

  const std::vector<const Material*> findMaterialsByTextureResourceId(
    const std::vector<ResourceId>& textureResourceIds) const;

  const std::vector<const Material*>& materials() const;
  const std::vector<MaterialCollection>& collections() const;

private:
  void resetFilterMode();

  void updateMaterials();
};
} // namespace Assets
} // namespace TrenchBroom
