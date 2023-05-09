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

#include "Assets/TextureCollection.h"

#include <filesystem>
#include <map>
#include <string>
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
struct TextureConfig;
}

namespace Assets
{
class Texture;
class TextureCollection;

class TextureManager
{
private:
  Logger& m_logger;

  std::vector<TextureCollection> m_collections;

  std::vector<size_t> m_toPrepare;
  std::vector<TextureCollection> m_toRemove;

  std::map<std::string, Texture*> m_texturesByName;
  std::vector<const Texture*> m_textures;

  int m_minFilter;
  int m_magFilter;
  bool m_resetTextureMode{false};

public:
  TextureManager(int magFilter, int minFilter, Logger& logger);
  ~TextureManager();

  void reload(const IO::FileSystem& fs, const Model::TextureConfig& textureConfig);

  // for testing
  void setTextureCollections(std::vector<TextureCollection> collections);

private:
  void setTextureCollections(
    const std::vector<std::filesystem::path>& paths,
    const IO::FileSystem& fs,
    const Model::TextureConfig& textureConfig);

  void addTextureCollection(Assets::TextureCollection collection);

public:
  void clear();

  void setTextureMode(int minFilter, int magFilter);
  void commitChanges();

  const Texture* texture(const std::string& name) const;
  Texture* texture(const std::string& name);

  const std::vector<const Texture*>& textures() const;
  const std::vector<TextureCollection>& collections() const;

private:
  void resetTextureMode();
  void prepare();

  void updateTextures();
};
} // namespace Assets
} // namespace TrenchBroom
