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
#include "IO/FileSystemUtils.h"
#include "IO/Path.h"
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
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/result.h"
#include "kdl/string_compare.h"
#include "kdl/vector_utils.h"
#include <kdl/overload.h>
#include <kdl/reflection_impl.h>
#include <kdl/result.h>

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
    if (!textureConfig.palette.isEmpty())
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

kdl::result<ReadTextureFunc, LoadTextureCollectionError> makeReadTextureFunc(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  if (textureConfig.format.format == "idmip")
  {
    return loadPalette(gameFS, textureConfig)
      .and_then(
        [](auto palette) -> kdl::result<ReadTextureFunc, LoadTextureCollectionError> {
          return [palette = std::move(palette)](
                   const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
            auto name = file.path().basename();
            auto reader = file.reader().buffer();
            return readIdMipTexture(std::move(name), reader, palette);
          };
        })
      .or_else(
        [](auto error) -> kdl::result<ReadTextureFunc, LoadTextureCollectionError> {
          return LoadTextureCollectionError{
            "Could not load texture collection: " + error.msg};
        });
  }
  if (textureConfig.format.format == "hlmip")
  {
    return [&](const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
      auto reader = file.reader().buffer();
      return readHlMipTexture(file.path().basename(), reader);
    };
  }
  if (textureConfig.format.format == "wal")
  {
    return loadPalette(gameFS, textureConfig)
      .and_then([](auto palette) -> kdl::result<std::optional<Assets::Palette>> {
        return std::optional{std::move(palette)};
      })
      .or_else(
        [](auto) -> kdl::result<std::optional<Assets::Palette>> { return std::nullopt; })
      .and_then(
        [&](auto palette) -> kdl::result<ReadTextureFunc, LoadTextureCollectionError> {
          return
            [&, pathPrefix = textureConfig.root.length(), palette = std::move(palette)](
              const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
              auto name = getTextureNameFromPathSuffix(file.path(), pathPrefix);
              auto reader = file.reader().buffer();
              return readWalTexture(std::move(name), reader, palette);
            };
        });
  }
  if (textureConfig.format.format == "image")
  {
    return [&, pathPrefix = textureConfig.root.length()](
             const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
      auto name = getTextureNameFromPathSuffix(file.path(), textureConfig.root.length());
      auto reader = file.reader().buffer();
      return readFreeImageTexture(std::move(name), reader);
    };
  }
  if (textureConfig.format.format == "q3shader")
  {
    return [&, pathPrefix = textureConfig.root.length()](
             const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
      auto name = getTextureNameFromPathSuffix(file.path(), textureConfig.root.length());
      auto reader = file.reader().buffer();
      return readQuake3ShaderTexture(std::move(name), file, gameFS);
    };
  }
  if (textureConfig.format.format == "m8")
  {
    return [&, pathPrefix = textureConfig.root.length()](
             const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
      auto name = getTextureNameFromPathSuffix(file.path(), textureConfig.root.length());
      auto reader = file.reader().buffer();
      return readM8Texture(std::move(name), reader);
    };
  }
  if (textureConfig.format.format == "dds")
  {
    return [&, pathPrefix = textureConfig.root.length()](
             const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
      auto name = getTextureNameFromPathSuffix(file.path(), textureConfig.root.length());
      auto reader = file.reader().buffer();
      return readDdsTexture(std::move(name), reader);
    };
  }

  return [&](const File& file) -> kdl::result<Assets::Texture, ReadTextureError> {
    auto name = getTextureNameFromPathSuffix(file.path(), textureConfig.root.length());
    return ReadTextureError{
      std::move(name), "Unknown texture format: " + textureConfig.format.format};
  };
}

} // namespace

kdl_reflect_impl(LoadTextureCollectionError);

std::vector<Path> findTextureCollections(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig)
{
  auto paths = gameFS.findRecursively(
    textureConfig.root, makePathInfoPathMatcher({PathInfo::Directory}));
  paths.insert(paths.begin(), textureConfig.root);
  return paths;
}

kdl::result<Assets::TextureCollection, LoadTextureCollectionError> loadTextureCollection(
  const Path& path,
  const FileSystem& gameFS,
  const Model::TextureConfig& textureConfig,
  Logger& logger)
{
  if (gameFS.pathInfo(path) != PathInfo::Directory)
  {
    return LoadTextureCollectionError{
      "Could not load texture collection '" + path.asString() + "': not a directory"};
  }

  return makeReadTextureFunc(gameFS, textureConfig)
    .and_then(
      [&](const auto& readTexture)
        -> kdl::result<Assets::TextureCollection, LoadTextureCollectionError> {
        const auto texturePaths =
          gameFS.find(path, makeExtensionPathMatcher(textureConfig.format.extensions));
        auto textures = std::vector<Assets::Texture>();
        textures.reserve(texturePaths.size());

        for (const auto& texturePath : texturePaths)
        {
          try
          {
            auto file = gameFS.openFile(texturePath);

            // Store the absolute path to the original file (may be used by .obj export)
            const auto name = file->path().lastComponent().deleteExtension().asString();
            if (shouldExclude(name, textureConfig.excludes))
            {
              continue;
            }

            readTexture(*file)
              .or_else(makeReadTextureErrorHandler(gameFS, logger))
              .transform([&](auto texture) {
                texture.setAbsolutePath(safeMakeAbsolute(texturePath, [&](const auto& p) {
                                          return gameFS.makeAbsolute(p);
                                        }).value_or(Path{}));
                texture.setRelativePath(texturePath);
                textures.push_back(std::move(texture));
              });
          }
          catch (const std::exception& e)
          {
            return LoadTextureCollectionError{
              "Could not load texture collection '" + path.asString() + "': " + e.what()};
          }
        }

        return Assets::TextureCollection{path, std::move(textures)};
      });
}

} // namespace TrenchBroom::IO
