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

#include "TextureCollectionLoader.h"

#include "Assets/TextureCollection.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/TextureReader.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace IO {
TextureCollectionLoader::TextureCollectionLoader(
  Logger& logger, const std::vector<std::string>& exclusions)
  : m_logger(logger)
  , m_textureExclusions(exclusions) {}

TextureCollectionLoader::~TextureCollectionLoader() = default;

bool TextureCollectionLoader::shouldExclude(const std::string& textureName) {
  for (const auto& pattern : m_textureExclusions) {
    if (kdl::ci::str_matches_glob(textureName, pattern)) {
      return true;
    }
  }
  return false;
}

FileTextureCollectionLoader::FileTextureCollectionLoader(
  Logger& logger, const std::vector<IO::Path>& searchPaths,
  const std::vector<std::string>& exclusions)
  : TextureCollectionLoader(logger, exclusions)
  , m_searchPaths(searchPaths) {}

Assets::TextureCollection FileTextureCollectionLoader::loadTextureCollection(
  const Path& path, const std::vector<std::string>& textureExtensions,
  const TextureReader& textureReader) {
  const auto wadPath = Disk::resolvePath(m_searchPaths, path);
  WadFileSystem wadFS(wadPath, m_logger);

  const auto texturePaths = wadFS.findItems(Path(""), FileExtensionMatcher(textureExtensions));
  auto textures = std::vector<Assets::Texture>();
  textures.reserve(texturePaths.size());

  for (const auto& texturePath : texturePaths) {
    try {
      auto file = wadFS.openFile(texturePath);
      const auto name = file->path().lastComponent().deleteExtension().asString();
      if (shouldExclude(name)) {
        continue;
      }
      textures.push_back(textureReader.readTexture(file));
    } catch (const std::exception& e) { m_logger.warn() << e.what(); }
  }

  return Assets::TextureCollection(path, std::move(textures));
}

DirectoryTextureCollectionLoader::DirectoryTextureCollectionLoader(
  Logger& logger, const FileSystem& gameFS, const std::vector<std::string>& exclusions)
  : TextureCollectionLoader(logger, exclusions)
  , m_gameFS(gameFS) {}

Assets::TextureCollection DirectoryTextureCollectionLoader::loadTextureCollection(
  const Path& path, const std::vector<std::string>& textureExtensions,
  const TextureReader& textureReader) {
  const auto texturePaths = m_gameFS.findItems(path, FileExtensionMatcher(textureExtensions));
  auto textures = std::vector<Assets::Texture>();
  textures.reserve(texturePaths.size());

  for (const auto& texturePath : texturePaths) {
    try {
      auto file = m_gameFS.openFile(texturePath);

      // Store the absolute path to the original file (may be used by .obj export)
      IO::Path absolutePath;
      try {
        absolutePath = m_gameFS.makeAbsolute(texturePath);
      } catch (const FileSystemException& e) { m_logger.debug() << e.what(); }

      const auto name = file->path().lastComponent().deleteExtension().asString();
      if (shouldExclude(name)) {
        continue;
      }
      auto texture = textureReader.readTexture(file);
      texture.setAbsolutePath(absolutePath);
      texture.setRelativePath(texturePath);
      textures.push_back(std::move(texture));
    } catch (const std::exception& e) { m_logger.warn() << e.what(); }
  }

  return Assets::TextureCollection(path, std::move(textures));
}
} // namespace IO
} // namespace TrenchBroom
