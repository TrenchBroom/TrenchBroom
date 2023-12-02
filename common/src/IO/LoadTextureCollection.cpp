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

#include "LoadTextureCollection.h"

#include "Assets/Palette.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Ensure.h"
#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/PathInfo.h"
#include "IO/PathMatcher.h"
#include "IO/ReadDdsTexture.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ReadM8Texture.h"
#include "IO/ReadMipTexture.h"
#include "IO/ReadQuake3ShaderTexture.h"
#include "IO/ReadWalTexture.h"
#include "IO/ResourceUtils.h"
#include "IO/TextureUtils.h"
#include "IO/TraversalMode.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/result_fold.h"
#include <kdl/overload.h>
#include <kdl/path_utils.h>
#include <kdl/result.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <ostream>
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

Result<Assets::Palette> loadPalette(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  if (textureConfig.palette.empty())
  {
    return Error{"Texture config is missing palette definition"};
  }

  return gameFS.openFile(textureConfig.palette).and_then([&](auto file) {
    return Assets::loadPalette(*file, textureConfig.palette);
  });
}

using ReadTextureFunc = std::function<Result<Assets::Texture, ReadTextureError>(
  const File&, const std::filesystem::path&)>;

Result<Assets::Texture, ReadTextureError> readTexture(
  const File& file,
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const size_t prefixLength,
  const std::optional<Assets::Palette>& palette)
{
  const auto extension = kdl::str_to_lower(path.extension().string());
  if (extension == ".d")
  {
    auto name = path.stem().string();
    if (!palette)
    {
      return ReadTextureError{std::move(name), "Could not load texture: missing palette"};
    }
    auto reader = file.reader().buffer();
    return readIdMipTexture(std::move(name), reader, *palette);
  }
  else if (extension == ".c")
  {
    auto name = path.stem().string();
    auto reader = file.reader().buffer();
    return readHlMipTexture(std::move(name), reader);
  }
  else if (extension == ".wal")
  {
    auto name = getTextureNameFromPathSuffix(path, prefixLength);
    auto reader = file.reader().buffer();
    return readWalTexture(std::move(name), reader, palette);
  }
  else if (extension == ".m8")
  {
    auto name = getTextureNameFromPathSuffix(path, prefixLength);
    auto reader = file.reader().buffer();
    return readM8Texture(std::move(name), reader);
  }
  else if (extension == ".dds")
  {
    auto name = getTextureNameFromPathSuffix(path, prefixLength);
    auto reader = file.reader().buffer();
    return readDdsTexture(std::move(name), reader);
  }
  else if (extension.empty())
  {
    auto name = getTextureNameFromPathSuffix(path, prefixLength);
    auto reader = file.reader().buffer();
    return readQuake3ShaderTexture(std::move(name), file, gameFS);
  }
  else if (isSupportedFreeImageExtension(extension))
  {
    auto name = getTextureNameFromPathSuffix(path, prefixLength);
    auto reader = file.reader().buffer();
    return readFreeImageTexture(std::move(name), reader);
  }

  auto name = getTextureNameFromPathSuffix(path, prefixLength);
  return ReadTextureError{
    std::move(name), "Unknown texture file extension: " + path.extension().string()};
}

Result<ReadTextureFunc> makeReadTextureFunc(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  return loadPalette(gameFS, textureConfig)
    .transform([](auto palette) { return std::optional{std::move(palette)}; })
    .transform_error([](auto) -> std::optional<Assets::Palette> { return std::nullopt; })
    .and_then([&](auto palette) -> Result<ReadTextureFunc> {
      return [&,
              palette = std::move(palette),
              prefixLength = kdl::path_length(textureConfig.root)](
               const File& file, const std::filesystem::path& path) {
        return readTexture(file, path, gameFS, prefixLength, palette);
      };
    });
}

} // namespace

Result<std::vector<std::filesystem::path>> findTextureCollections(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  return gameFS
    .find(
      textureConfig.root,
      TraversalMode::Recursive,
      makePathInfoPathMatcher({PathInfo::Directory}))
    .transform([&](auto paths) {
      paths.insert(paths.begin(), textureConfig.root);
      return paths;
    });
}

Result<Assets::TextureCollection> loadTextureCollection(
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const Model::TextureConfig& textureConfig,
  Logger& logger)
{
  if (gameFS.pathInfo(path) != PathInfo::Directory)
  {
    return Error{
      "Could not load texture collection '" + path.string() + "': not a directory"};
  }

  const auto pathMatcher = !textureConfig.extensions.empty()
                             ? makeExtensionPathMatcher(textureConfig.extensions)
                             : matchAnyPath;

  return makeReadTextureFunc(gameFS, textureConfig)
    .join(
      gameFS.find(path, TraversalMode::Flat, pathMatcher)
        .transform([&](auto texturePaths) {
          return kdl::vec_filter(std::move(texturePaths), [&](const auto& texturePath) {
            return !shouldExclude(texturePath.stem().string(), textureConfig.excludes);
          });
        }))
    .and_then([&](const auto& readTexture, auto texturePaths) {
      return kdl::fold_results(
               kdl::vec_transform(
                 texturePaths,
                 [&](const auto& texturePath) {
                   return gameFS.openFile(texturePath)
                     .and_then([&](auto file) { return readTexture(*file, texturePath); })
                     .or_else(makeReadTextureErrorHandler(gameFS, logger))
                     .transform([&](auto texture) {
                       gameFS.makeAbsolute(texturePath)
                         .transform([&](auto absPath) {
                           texture.setAbsolutePath(std::move(absPath));
                         })
                         .or_else([](auto) { return kdl::void_success; });
                       texture.setRelativePath(texturePath);
                       return texture;
                     });
                 }))
        .transform([&](auto textures) {
          return Assets::TextureCollection{path, std::move(textures)};
        });
    });
}

} // namespace TrenchBroom::IO
