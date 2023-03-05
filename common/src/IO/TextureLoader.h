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

#include "IO/Path.h"
#include "Macros.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class Palette;
class Texture;
class TextureCollection;
class TextureManager;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
struct TextureConfig;
}

namespace TrenchBroom::IO
{
class File;
class FileSystem;

class TextureLoader
{
private:
  const FileSystem& m_gameFS;
  Path m_textureRoot;
  std::vector<std::string> m_textureExtensions;
  std::vector<std::string> m_textureExclusionPatterns;
  std::function<Assets::Texture(const File&)> m_readTexture;
  Logger& m_logger;

public:
  TextureLoader(
    const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);
  ~TextureLoader();

public:
  std::vector<Path> findTextureCollections() const;
  Assets::TextureCollection loadTextureCollection(const Path& path) const;

  deleteCopyAndMove(TextureLoader);
};

} // namespace TrenchBroom::IO
