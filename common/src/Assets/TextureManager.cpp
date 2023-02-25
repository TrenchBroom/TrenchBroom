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

#include "TextureManager.h"

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Exceptions.h"
#include "IO/TextureLoader.h"
#include "Logger.h"

#include <kdl/map_utils.h>
#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Assets
{

TextureManager::TextureManager(int magFilter, int minFilter, Logger& logger)
  : m_logger{logger}
  , m_minFilter{minFilter}
  , m_magFilter{magFilter}
{
}

TextureManager::~TextureManager() = default;

void TextureManager::reload(const IO::TextureLoader& loader)
{
  setTextureCollections(loader.findTextureCollections(), loader);
}

void TextureManager::setTextureCollections(std::vector<TextureCollection> collections)
{
  for (auto& collection : collections)
  {
    addTextureCollection(std::move(collection));
  }
  updateTextures();
}

void TextureManager::setTextureCollections(
  const std::vector<IO::Path>& paths, const IO::TextureLoader& loader)
{
  auto collections = std::move(m_collections);
  clear();

  for (const auto& path : paths)
  {
    const auto it =
      std::find_if(collections.begin(), collections.end(), [&](const auto& c) {
        return c.path() == path;
      });
    if (it == collections.end() || !it->loaded())
    {
      try
      {
        const auto startTime = std::chrono::high_resolution_clock::now();
        auto collection = loader.loadTextureCollection(path);
        const auto endTime = std::chrono::high_resolution_clock::now();

        m_logger.info() << "Loaded texture collection '" << path << "' in "
                        << std::chrono::duration_cast<std::chrono::milliseconds>(
                             endTime - startTime)
                             .count()
                        << "ms";
        addTextureCollection(std::move(collection));
      }
      catch (const Exception& e)
      {
        addTextureCollection(Assets::TextureCollection{path});
        if (it == collections.end())
        {
          m_logger.error() << "Could not load texture collection '" << path
                           << "': " << e.what();
        }
      }
    }
    else
    {
      addTextureCollection(std::move(*it));
    }

    if (it != collections.end())
    {
      collections.erase(it);
    }
  }

  updateTextures();
  m_toRemove = kdl::vec_concat(std::move(m_toRemove), std::move(collections));
}

void TextureManager::addTextureCollection(Assets::TextureCollection collection)
{
  const auto index = m_collections.size();
  m_collections.push_back(std::move(collection));

  if (m_collections[index].loaded() && !m_collections[index].prepared())
  {
    m_toPrepare.push_back(index);
  }

  m_logger.debug() << "Added texture collection " << m_collections[index].path();
}

void TextureManager::clear()
{
  m_collections.clear();

  m_toPrepare.clear();
  m_texturesByName.clear();
  m_textures.clear();

  // Remove logging because it might fail when the document is already destroyed.
}

void TextureManager::setTextureMode(const int minFilter, const int magFilter)
{
  m_minFilter = minFilter;
  m_magFilter = magFilter;
  m_resetTextureMode = true;
}

void TextureManager::commitChanges()
{
  resetTextureMode();
  prepare();
  m_toRemove.clear();
}

const Texture* TextureManager::texture(const std::string& name) const
{
  auto it = m_texturesByName.find(kdl::str_to_lower(name));
  return it != m_texturesByName.end() ? it->second : nullptr;
}

Texture* TextureManager::texture(const std::string& name)
{
  return const_cast<Texture*>(const_cast<const TextureManager*>(this)->texture(name));
}

const std::vector<const Texture*>& TextureManager::textures() const
{
  return m_textures;
}

const std::vector<TextureCollection>& TextureManager::collections() const
{
  return m_collections;
}

void TextureManager::resetTextureMode()
{
  if (m_resetTextureMode)
  {
    for (auto& collection : m_collections)
    {
      collection.setTextureMode(m_minFilter, m_magFilter);
    }
    m_resetTextureMode = false;
  }
}

void TextureManager::prepare()
{
  for (const auto index : m_toPrepare)
  {
    auto& collection = m_collections[index];
    collection.prepare(m_minFilter, m_magFilter);
  }
  m_toPrepare.clear();
}

void TextureManager::updateTextures()
{
  m_texturesByName.clear();
  m_textures.clear();

  for (auto& collection : m_collections)
  {
    for (auto& texture : collection.textures())
    {
      const auto key = kdl::str_to_lower(texture.name());
      texture.setOverridden(false);

      auto mIt = m_texturesByName.find(key);
      if (mIt != m_texturesByName.end())
      {
        mIt->second->setOverridden(true);
        mIt->second = &texture;
      }
      else
      {
        m_texturesByName.insert(std::make_pair(key, &texture));
      }
    }
  }

  m_textures = kdl::vec_transform(kdl::map_values(m_texturesByName), [](auto* t) {
    return const_cast<const Texture*>(t);
  });
}
} // namespace Assets
} // namespace TrenchBroom
