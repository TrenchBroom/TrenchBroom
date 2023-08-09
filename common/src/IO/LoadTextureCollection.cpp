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
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FileSystemError.h"
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

#include <kdl/overload.h>
#include <kdl/path_utils.h>
#include <kdl/reflection_impl.h>
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

kdl::result<Assets::Palette, Assets::LoadPaletteError> loadPalette(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  try
  {
    if (!textureConfig.palette.empty())
    {
      const auto& path = textureConfig.palette;
      auto file = gameFS.openFile(path);
      return Assets::loadPalette(*file);
    }
    return Assets::LoadPaletteError{"Texture config is missing palette definition"};
  }
  catch (const Exception& e)
  {
    return Assets::LoadPaletteError{std::string{"Could not load Palette: "} + e.what()};
  }
}

using ReadTextureFunc =
  std::function<kdl::result<Assets::Texture, ReadTextureError>(const File&)>;

kdl::result<Assets::Texture, ReadTextureError> readTexture(
  const File& file,
  const FileSystem& gameFS,
  const size_t prefixLength,
  const std::optional<Assets::Palette>& palette)
{
  static const auto imageFileExtensions =
    std::vector<std::string>{".jpg", ".jpeg", ".png", ".tga", ".bmp"};

  const auto extension = kdl::str_to_lower(file.path().extension().string());
  if (extension == ".d")
  {
    auto name = file.path().stem().string();
    if (!palette)
    {
      return ReadTextureError{std::move(name), "Could not load texture: missing palette"};
    }
    auto reader = file.reader().buffer();
    return readIdMipTexture(std::move(name), reader, *palette);
  }
  else if (extension == ".c")
  {
    auto name = file.path().stem().string();
    auto reader = file.reader().buffer();
    return readHlMipTexture(std::move(name), reader);
  }
  else if (extension == ".wal")
  {
    auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
    auto reader = file.reader().buffer();
    return readWalTexture(std::move(name), reader, palette);
  }
  else if (extension == ".m8")
  {
    auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
    auto reader = file.reader().buffer();
    return readM8Texture(std::move(name), reader);
  }
  else if (extension == ".dds")
  {
    auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
    auto reader = file.reader().buffer();
    return readDdsTexture(std::move(name), reader);
  }
  else if (extension.empty())
  {
    auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
    auto reader = file.reader().buffer();
    return readQuake3ShaderTexture(std::move(name), file, gameFS);
  }
  else if (kdl::vec_contains(imageFileExtensions, extension))
  {
    auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
    auto reader = file.reader().buffer();
    return readFreeImageTexture(std::move(name), reader);
  }

  auto name = getTextureNameFromPathSuffix(file.path(), prefixLength);
  return ReadTextureError{
    std::move(name),
    "Unknown texture file extension: " + file.path().extension().string()};
}

kdl::result<ReadTextureFunc, LoadTextureCollectionError> makeReadTextureFunc(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  return loadPalette(gameFS, textureConfig)
    .transform([](auto palette) { return std::optional{std::move(palette)}; })
    .transform_error([](auto) -> std::optional<Assets::Palette> { return std::nullopt; })
    .and_then(
      [&](auto palette) -> kdl::result<ReadTextureFunc, LoadTextureCollectionError> {
        return [&,
                palette = std::move(palette),
                prefixLength = kdl::path_length(textureConfig.root)](const File& file) {
          return readTexture(file, gameFS, prefixLength, palette);
        };
      });
}

} // namespace

kdl_reflect_impl(LoadTextureCollectionError);

std::vector<std::filesystem::path> findTextureCollections(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  auto paths = gameFS.find(
    textureConfig.root,
    TraversalMode::Recursive,
    makePathInfoPathMatcher({PathInfo::Directory}));
  paths.insert(paths.begin(), textureConfig.root);
  return paths;
}

kdl::result<Assets::TextureCollection, LoadTextureCollectionError> loadTextureCollection(
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const Model::TextureConfig& textureConfig,
  Logger& logger)
{
  if (gameFS.pathInfo(path) != PathInfo::Directory)
  {
    return LoadTextureCollectionError{
      "Could not load texture collection '" + path.string() + "': not a directory"};
  }

  return makeReadTextureFunc(gameFS, textureConfig)
    .and_then(
      [&](const auto& readTexture)
        -> kdl::result<Assets::TextureCollection, LoadTextureCollectionError> {
        const auto pathMatcher = !textureConfig.extensions.empty()
                                   ? makeExtensionPathMatcher(textureConfig.extensions)
                                   : matchAnyPath;
        const auto texturePaths = gameFS.find(path, TraversalMode::Flat, pathMatcher);
        auto textures = std::vector<Assets::Texture>{};
        textures.reserve(texturePaths.size());

        for (const auto& texturePath : texturePaths)
        {
          try
          {
            auto file = gameFS.openFile(texturePath);

            // Store the absolute path to the original file
            // (may be used by .obj export)
            const auto name = file->path().stem().string();
            if (shouldExclude(name, textureConfig.excludes))
            {
              continue;
            }

            readTexture(*file)
              .or_else(makeReadTextureErrorHandler(gameFS, logger))
              .transform([&](auto texture) {
                gameFS.makeAbsolute(texturePath)
                  .transform(
                    [&](auto absPath) { texture.setAbsolutePath(std::move(absPath)); })
                  .or_else([](auto) { return kdl::void_success; });
                texture.setRelativePath(texturePath);
                textures.push_back(std::move(texture));
              });
          }
          catch (const std::exception& e)
          {
            return LoadTextureCollectionError{
              "Could not load texture collection '" + path.string() + "': " + e.what()};
          }
        }

        return Assets::TextureCollection{path, std::move(textures)};
      });
}

} // namespace TrenchBroom::IO
