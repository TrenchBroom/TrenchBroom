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

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace Assets
{
class Palette;
class TextureCollection;
class TextureManager;
} // namespace Assets

namespace Model
{
struct TextureConfig;
}

namespace IO
{
class FileSystem;
class TextureReader;

class TextureLoader
{
private:
  const FileSystem& m_gameFS;
  Path m_textureRoot;
  std::vector<std::string> m_textureExtensions;
  std::vector<std::string> m_textureExclusionPatterns;
  std::unique_ptr<TextureReader> m_textureReader;
  Logger& m_logger;

public:
  TextureLoader(
    const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);
  ~TextureLoader();

private:
  static std::unique_ptr<TextureReader> createTextureReader(
    const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);
  static Assets::Palette loadPalette(
    const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger);

public:
  std::vector<Path> findTextureCollections() const;
  Assets::TextureCollection loadTextureCollection(const Path& path) const;

  deleteCopyAndMove(TextureLoader);
};
} // namespace IO
} // namespace TrenchBroom
