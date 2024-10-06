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

#include "MaterialManager.h"

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/Resource.h"
#include "IO/LoadMaterialCollections.h"
#include "Logger.h"

#include "kdl/map_utils.h"
#include "kdl/result.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace tb::Assets
{

MaterialManager::MaterialManager(Logger& logger)
  : m_logger{logger}
{
}

MaterialManager::~MaterialManager() = default;

void MaterialManager::reload(
  const IO::FileSystem& fs,
  const Model::MaterialConfig& materialConfig,
  const Assets::CreateTextureResource& createResource)
{
  clear();
  IO::loadMaterialCollections(fs, materialConfig, createResource, m_logger)
    | kdl::transform([&](auto materialCollections) {
        for (auto& collection : materialCollections)
        {
          addMaterialCollection(std::move(collection));
        }
        updateMaterials();
      })
    | kdl::transform_error([&](auto e) {
        m_logger.error() << "Could not reload material collections: " + e.msg;
      });
}

void MaterialManager::setMaterialCollections(std::vector<MaterialCollection> collections)
{
  for (auto& collection : collections)
  {
    addMaterialCollection(std::move(collection));
  }
  updateMaterials();
}

void MaterialManager::addMaterialCollection(Assets::MaterialCollection collection)
{
  const auto index = m_collections.size();
  m_collections.push_back(std::move(collection));

  m_logger.debug() << "Added material collection " << m_collections[index].path();
}

void MaterialManager::clear()
{
  m_collections.clear();
  m_materialsByName.clear();
  m_materials.clear();

  // Remove logging because it might fail when the document is already destroyed.
}

const Material* MaterialManager::material(const std::string& name) const
{
  auto it = m_materialsByName.find(kdl::str_to_lower(name));
  return it != m_materialsByName.end() ? it->second : nullptr;
}

Material* MaterialManager::material(const std::string& name)
{
  return const_cast<Material*>(const_cast<const MaterialManager*>(this)->material(name));
}

const std::vector<const Material*> MaterialManager::findMaterialsByTextureResourceId(
  const std::vector<ResourceId>& textureResourceIds) const
{
  const auto resourceIdSet =
    std::unordered_set<ResourceId>{textureResourceIds.begin(), textureResourceIds.end()};
  return kdl::vec_filter(m_materials, [&](const auto* material) {
    return resourceIdSet.count(material->textureResource().id()) > 0;
  });
}

const std::vector<const Material*>& MaterialManager::materials() const
{
  return m_materials;
}

const std::vector<MaterialCollection>& MaterialManager::collections() const
{
  return m_collections;
}

void MaterialManager::updateMaterials()
{
  m_materialsByName.clear();
  m_materials.clear();

  for (auto& collection : m_collections)
  {
    for (auto& material : collection.materials())
    {
      const auto key = kdl::str_to_lower(material.name());

      auto mIt = m_materialsByName.find(key);
      if (mIt != m_materialsByName.end())
      {
        mIt->second = &material;
      }
      else
      {
        m_materialsByName.insert(std::make_pair(key, &material));
      }
    }
  }

  m_materials = kdl::vec_transform(kdl::map_values(m_materialsByName), [](auto* t) {
    return const_cast<const Material*>(t);
  });
}
} // namespace tb::Assets
