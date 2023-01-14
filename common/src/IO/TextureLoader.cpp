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

#include "TextureLoader.h"

#include "Assets/Palette.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Ensure.h"
#include "IO/DdsTextureReader.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/HlMipTextureReader.h"
#include "IO/IdMipTextureReader.h"
#include "IO/M8TextureReader.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "IO/TextureCollectionLoader.h"
#include "IO/WalTextureReader.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include <kdl/overload.h>

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
TextureLoader::TextureLoader(
  const FileSystem& gameFS,
  const std::vector<IO::Path>& fileSearchPaths,
  const Model::TextureConfig& textureConfig,
  Logger& logger)
  : m_textureExtensions(getTextureExtensions(textureConfig))
  , m_textureReader(createTextureReader(gameFS, textureConfig, logger))
  , m_textureCollectionLoader(
      createTextureCollectionLoader(gameFS, fileSearchPaths, textureConfig, logger))
{
  ensure(m_textureReader != nullptr, "textureReader is null");
  ensure(m_textureCollectionLoader != nullptr, "textureCollectionLoader is null");
}

TextureLoader::~TextureLoader() = default;

std::vector<std::string> TextureLoader::getTextureExtensions(
  const Model::TextureConfig& textureConfig)
{
  return textureConfig.format.extensions;
}

std::unique_ptr<TextureReader> TextureLoader::createTextureReader(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger)
{
  const auto prefixLength = getRootDirectory(textureConfig.package).length();
  const TextureReader::PathSuffixNameStrategy nameStrategy(prefixLength);

  if (textureConfig.format.format == "idmip")
  {
    return std::make_unique<IdMipTextureReader>(
      nameStrategy, gameFS, logger, loadPalette(gameFS, textureConfig, logger));
  }
  else if (textureConfig.format.format == "hlmip")
  {
    return std::make_unique<HlMipTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "wal")
  {
    return std::make_unique<WalTextureReader>(
      nameStrategy, gameFS, logger, loadPalette(gameFS, textureConfig, logger));
  }
  else if (textureConfig.format.format == "image")
  {
    return std::make_unique<FreeImageTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "q3shader")
  {
    return std::make_unique<Quake3ShaderTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "m8")
  {
    return std::make_unique<M8TextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "dds")
  {
    return std::make_unique<DdsTextureReader>(nameStrategy, gameFS, logger);
  }
  else
  {
    throw GameException("Unknown texture format '" + textureConfig.format.format + "'");
  }
}

Assets::Palette TextureLoader::loadPalette(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger)
{
  if (textureConfig.palette.isEmpty())
  {
    return Assets::Palette();
  }

  try
  {
    const auto& path = textureConfig.palette;
    logger.info() << "Loading palette file " << path;
    return Assets::Palette::loadFile(gameFS, path);
  }
  catch (const Exception& e)
  {
    logger.error() << e.what();
    return Assets::Palette();
  }
}

std::unique_ptr<TextureCollectionLoader> TextureLoader::createTextureCollectionLoader(
  const FileSystem& gameFS,
  const std::vector<IO::Path>& fileSearchPaths,
  const Model::TextureConfig& textureConfig,
  Logger& logger)
{
  using Model::GameConfig;
  return std::visit(
    kdl::overload(
      [&](const Model::TextureFilePackageConfig&)
        -> std::unique_ptr<TextureCollectionLoader> {
        return std::make_unique<FileTextureCollectionLoader>(
          logger, fileSearchPaths, textureConfig.excludes);
      },
      [&](const Model::TextureDirectoryPackageConfig&)
        -> std::unique_ptr<TextureCollectionLoader> {
        return std::make_unique<DirectoryTextureCollectionLoader>(
          logger, gameFS, textureConfig.excludes);
      }),
    textureConfig.package);
}

Assets::TextureCollection TextureLoader::loadTextureCollection(const Path& path)
{
  return m_textureCollectionLoader->loadTextureCollection(
    path, m_textureExtensions, *m_textureReader);
}

void TextureLoader::loadTextures(
  const std::vector<Path>& paths, Assets::TextureManager& textureManager)
{
  textureManager.setTextureCollections(paths, *this);
}
} // namespace IO
} // namespace TrenchBroom
