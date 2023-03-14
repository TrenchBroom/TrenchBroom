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
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FileSystemUtils.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/HlMipTextureReader.h"
#include "IO/IdMipTextureReader.h"
#include "IO/M8TextureReader.h"
#include "IO/Path.h"
#include "IO/PathInfo.h"
#include "IO/PathMatcher.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "IO/WalTextureReader.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/string_compare.h"
#include "kdl/vector_utils.h"
#include <kdl/overload.h>
#include <kdl/result.h>

#include <string>
#include <vector>

namespace TrenchBroom::IO
{

namespace
{
bool shouldExclude(
  const std::string& textureName, const std::vector<std::string>& patterns)
{
  return std::any_of(patterns.begin(), patterns.end(), [&](const auto& pattern) {
    return kdl::ci::str_matches_glob(textureName, pattern);
  });
}
} // namespace

TextureLoader::TextureLoader(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger)
  : m_gameFS{gameFS}
  , m_textureRoot{textureConfig.root}
  , m_textureExtensions{textureConfig.format.extensions}
  , m_textureExclusionPatterns{textureConfig.excludes}
  , m_textureReader{createTextureReader(m_gameFS, textureConfig, logger)}
  , m_logger{logger}
{
}

TextureLoader::~TextureLoader() = default;

std::unique_ptr<TextureReader> TextureLoader::createTextureReader(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger)
{
  if (textureConfig.format.format == "idmip")
  {
    const auto nameStrategy = TextureReader::TextureNameStrategy{};
    auto palette = loadPalette(gameFS, textureConfig, logger);
    return std::make_unique<IdMipTextureReader>(
      nameStrategy, gameFS, std::move(*palette), logger);
  }
  else if (textureConfig.format.format == "hlmip")
  {
    const auto nameStrategy = TextureReader::TextureNameStrategy{};
    return std::make_unique<HlMipTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "wal")
  {
    const auto prefixLength = textureConfig.root.length();
    const auto nameStrategy = TextureReader::PathSuffixNameStrategy{prefixLength};
    auto palette = loadPalette(gameFS, textureConfig, logger);
    return std::make_unique<WalTextureReader>(
      nameStrategy, gameFS, std::move(palette), logger);
  }
  else if (textureConfig.format.format == "image")
  {
    const auto prefixLength = textureConfig.root.length();
    const auto nameStrategy = TextureReader::PathSuffixNameStrategy{prefixLength};
    return std::make_unique<FreeImageTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "q3shader")
  {
    const auto prefixLength = textureConfig.root.length();
    const auto nameStrategy = TextureReader::PathSuffixNameStrategy{prefixLength};
    return std::make_unique<Quake3ShaderTextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "m8")
  {
    const auto prefixLength = textureConfig.root.length();
    const auto nameStrategy = TextureReader::PathSuffixNameStrategy{prefixLength};
    return std::make_unique<M8TextureReader>(nameStrategy, gameFS, logger);
  }
  else if (textureConfig.format.format == "dds")
  {
    const auto prefixLength = textureConfig.root.length();
    const auto nameStrategy = TextureReader::PathSuffixNameStrategy{prefixLength};
    return std::make_unique<DdsTextureReader>(nameStrategy, gameFS, logger);
  }
  else
  {
    throw GameException{"Unknown texture format '" + textureConfig.format.format + "'"};
  }
}

std::optional<Assets::Palette> TextureLoader::loadPalette(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig, Logger& logger)
{
  try
  {
    if (!textureConfig.palette.isEmpty())
    {
      const auto& path = textureConfig.palette;
      logger.info() << "Loading palette file " << path;
      auto file = gameFS.openFile(path);
      return Assets::loadPalette(*file)
        .transform([](auto palette) { return std::optional{std::move(palette)}; })
        .if_error([&](const auto error) {
          logger.error() << "Could not load palette file: " << error;
          return std::nullopt;
        })
        .value();
    }
  }
  catch (const Exception& e)
  {
    logger.error() << "Could not load palette file: " << e.what();
  }
  return std::nullopt;
}

std::vector<Path> TextureLoader::findTextureCollections() const
{
  auto paths = m_gameFS.findRecursively(
    m_textureRoot, makePathInfoPathMatcher({PathInfo::Directory}));
  paths.insert(paths.begin(), m_textureRoot);
  return paths;
}

Assets::TextureCollection TextureLoader::loadTextureCollection(const Path& path) const
{
  const auto texturePaths =
    m_gameFS.find(path, makeExtensionPathMatcher(m_textureExtensions));
  auto textures = std::vector<Assets::Texture>();
  textures.reserve(texturePaths.size());

  for (const auto& texturePath : texturePaths)
  {
    try
    {
      auto file = m_gameFS.openFile(texturePath);

      // Store the absolute path to the original file (may be used by .obj export)
      const auto name = file->path().lastComponent().deleteExtension().asString();
      if (shouldExclude(name, m_textureExclusionPatterns))
      {
        continue;
      }

      auto texture = m_textureReader->readTexture(file);
      texture.setAbsolutePath(safeMakeAbsolute(texturePath, [&](const auto& p) {
                                return m_gameFS.makeAbsolute(p);
                              }).value_or(Path{}));
      texture.setRelativePath(texturePath);
      textures.push_back(std::move(texture));
    }
    catch (const std::exception& e)
    {
      m_logger.warn() << e.what();
    }
  }

  return Assets::TextureCollection{path, std::move(textures)};
}

} // namespace TrenchBroom::IO
