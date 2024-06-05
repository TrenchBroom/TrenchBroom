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

#include "LoadMaterialCollection.h"

#include "Assets/MaterialCollection.h"
#include "Assets/MaterialManager.h"
#include "Assets/Palette.h"
#include "Ensure.h"
#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/MaterialUtils.h"
#include "IO/PathInfo.h"
#include "IO/PathMatcher.h"
#include "IO/ReadDdsTexture.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ReadM8Texture.h"
#include "IO/ReadMipTexture.h"
#include "IO/ReadQuake3ShaderTexture.h"
#include "IO/ReadWalTexture.h"
#include "IO/ResourceUtils.h"
#include "IO/TraversalMode.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/overload.h"
#include "kdl/parallel.h"
#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <ostream>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

namespace
{
bool shouldExclude(
  const std::string& materialName, const std::vector<std::string>& patterns)
{
  return std::any_of(patterns.begin(), patterns.end(), [&](const auto& pattern) {
    return kdl::ci::str_matches_glob(materialName, pattern);
  });
}

Result<Assets::Palette> loadPalette(
  const FileSystem& gameFS, const Model::MaterialConfig& materialConfig)
{
  if (materialConfig.palette.empty())
  {
    return Error{"Material config is missing palette definition"};
  }

  return gameFS.openFile(materialConfig.palette).and_then([&](auto file) {
    return Assets::loadPalette(*file, materialConfig.palette);
  });
}

using ReadMaterialFunc = std::function<Result<Assets::Material, ReadMaterialError>(
  const File&, const std::filesystem::path&)>;

Result<Assets::Material, ReadMaterialError> readMaterial(
  const File& file,
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const size_t prefixLength,
  const std::optional<Assets::Palette>& palette)
{
  // TODO: extract this into a separate function and replace SkinLoader with it!

  auto name = getMaterialNameFromPathSuffix(path, prefixLength);
  const auto extension = kdl::str_to_lower(path.extension().string());
  if (extension == ".d")
  {
    if (!palette)
    {
      return ReadMaterialError{
        std::move(name), "Could not load texture: missing palette"};
    }
    auto reader = file.reader().buffer();
    const auto mask = getTextureMaskFromName(name);
    return readIdMipTexture(reader, *palette, mask)
      .transform([&](auto texture) {
        return Assets::Material{std::move(name), std::move(texture)};
      })
      .or_else([&](auto e) {
        return Result<Assets::Material, ReadMaterialError>{
          ReadMaterialError{std::move(name), std::move(e.msg)}};
      });
  }
  else if (extension == ".c")
  {
    auto reader = file.reader().buffer();
    const auto mask = getTextureMaskFromName(name);
    return readHlMipTexture(reader, mask) | kdl::transform([&](auto texture) {
             return Assets::Material{std::move(name), std::move(texture)};
           })
           | kdl::or_else([&](auto e) {
               return Result<Assets::Material, ReadMaterialError>{
                 ReadMaterialError{std::move(name), std::move(e.msg)}};
             });
  }
  else if (extension == ".wal")
  {
    auto reader = file.reader().buffer();
    return readWalTexture(reader, palette) | kdl::transform([&](auto texture) {
             return Assets::Material{std::move(name), std::move(texture)};
           })
           | kdl::or_else([&](auto e) {
               return Result<Assets::Material, ReadMaterialError>{
                 ReadMaterialError{std::move(name), std::move(e.msg)}};
             });
  }
  else if (extension == ".m8")
  {
    auto reader = file.reader().buffer();
    return readM8Texture(reader) | kdl::transform([&](auto texture) {
             return Assets::Material{std::move(name), std::move(texture)};
           })
           | kdl::or_else([&](auto e) {
               return Result<Assets::Material, ReadMaterialError>{
                 ReadMaterialError{std::move(name), std::move(e.msg)}};
             });
  }
  else if (extension == ".dds")
  {
    auto reader = file.reader().buffer();
    return readDdsTexture(std::move(name), reader);
  }
  else if (extension.empty())
  {
    auto reader = file.reader().buffer();
    return readQuake3ShaderTexture(std::move(name), file, gameFS);
  }
  else if (isSupportedFreeImageExtension(extension))
  {
    auto reader = file.reader().buffer();
    return readFreeImageTexture(std::move(name), reader);
  }

  return ReadMaterialError{
    std::move(name), "Unknown texture file extension: " + path.extension().string()};
}

Result<ReadMaterialFunc> makeReadMaterialFunc(
  const FileSystem& gameFS, const Model::MaterialConfig& materialConfig)
{
  return loadPalette(gameFS, materialConfig)
    .transform([](auto palette) { return std::optional{std::move(palette)}; })
    .transform_error([](auto) -> std::optional<Assets::Palette> { return std::nullopt; })
    .and_then([&](auto palette) -> Result<ReadMaterialFunc> {
      return [&,
              palette = std::move(palette),
              prefixLength = kdl::path_length(materialConfig.root)](
               const File& file, const std::filesystem::path& path) {
        return readMaterial(file, path, gameFS, prefixLength, palette);
      };
    });
}

} // namespace

Result<std::vector<std::filesystem::path>> findMaterialCollections(
  const FileSystem& gameFS, const Model::MaterialConfig& materialConfig)
{
  return gameFS
    .find(
      materialConfig.root,
      TraversalMode::Recursive,
      makePathInfoPathMatcher({PathInfo::Directory}))
    .transform([&](auto paths) {
      paths.insert(paths.begin(), materialConfig.root);
      return paths;
    });
}

Result<Assets::MaterialCollection> loadMaterialCollection(
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const Model::MaterialConfig& materialConfig,
  Logger& logger)
{
  if (gameFS.pathInfo(path) != PathInfo::Directory)
  {
    return Error{
      "Could not load material collection '" + path.string() + "': not a directory"};
  }

  const auto pathMatcher = !materialConfig.extensions.empty()
                             ? makeExtensionPathMatcher(materialConfig.extensions)
                             : matchAnyPath;

  return gameFS.find(path, TraversalMode::Flat, pathMatcher)
    .transform([&](auto materialPaths) {
      return kdl::vec_filter(std::move(materialPaths), [&](const auto& materialPath) {
        return !shouldExclude(materialPath.stem().string(), materialConfig.excludes);
      });
    })
    .join(makeReadMaterialFunc(gameFS, materialConfig))
    .and_then([&](auto materialPaths, const auto& readMaterial) {
      return kdl::fold_results(
               kdl::vec_parallel_transform(
                 std::move(materialPaths),
                 [&](const auto materialPath) {
                   return gameFS.openFile(materialPath)
                     .and_then([&](const auto& file) {
                       return readMaterial(*file, materialPath)
                         .transform([&](auto material) {
                           gameFS.makeAbsolute(materialPath)
                             .transform([&](auto absPath) {
                               material.setAbsolutePath(std::move(absPath));
                             })
                             .or_else([](auto) { return kdl::void_success; });
                           material.setRelativePath(materialPath);
                           return material;
                         });
                     })
                     .or_else(makeReadMaterialErrorHandler(gameFS, logger));
                 }))
        .transform([&](auto materials) {
          return Assets::MaterialCollection{path, std::move(materials)};
        });
    });
}

} // namespace TrenchBroom::IO
