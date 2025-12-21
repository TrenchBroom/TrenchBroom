/*
 Copyright (C) 2010 Kristian Duske

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

#include "LoadMaterialCollections.h"

#include "Logger.h"
#include "fs/FileSystem.h"
#include "fs/PathInfo.h"
#include "fs/PathMatcher.h"
#include "fs/TraversalMode.h"
#include "io/LoadFreeImageTexture.h"
#include "io/LoadShaders.h"
#include "io/LoadTexture.h"
#include "io/MaterialUtils.h"
#include "mdl/GameConfig.h"
#include "mdl/MaterialCollection.h"
#include "mdl/Palette.h"
#include "mdl/Quake3Shader.h"
#include "mdl/Texture.h"
#include "mdl/TextureResource.h"

#include "kd/contracts.h"
#include "kd/functional.h"
#include "kd/path_hash.h"
#include "kd/path_utils.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/chunk_by_view.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/string_compare.h"
#include "kd/string_format.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <ranges>
#include <string>

namespace tb::io
{

namespace
{

Result<std::optional<mdl::Palette>> loadPalette(
  const fs::FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  if (materialConfig.palette.empty())
  {
    return Result<std::optional<mdl::Palette>>{std::nullopt};
  }

  return fs.openFile(materialConfig.palette) | kdl::and_then([&](auto file) {
           return mdl::loadPalette(*file, materialConfig.palette);
         })
         | kdl::transform([](auto palette) { return std::optional{std::move(palette)}; });
}

bool shouldExclude(
  const std::string& materialName, const std::vector<std::string>& patterns)
{
  return std::ranges::any_of(patterns, [&](const auto& pattern) {
    return kdl::ci::str_matches_glob(materialName, pattern);
  });
}

Result<std::vector<std::filesystem::path>> findTexturePaths(
  const fs::FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  return fs.find(
           materialConfig.root,
           fs::TraversalMode::Recursive,
           fs::makeExtensionPathMatcher(materialConfig.extensions))
         | kdl::transform([&](auto paths) {
             return paths | std::views::filter([&](const auto& path) {
                      return !shouldExclude(
                        path.stem().string(), materialConfig.excludes);
                    })
                    | kdl::views::as_rvalue | kdl::ranges::to<std::vector>();
           });
}

Result<std::vector<std::filesystem::path>> findAllMaterialPaths(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::vector<mdl::Quake3Shader>& shaders)
{
  return findTexturePaths(fs, materialConfig)
         | kdl::transform([&](const auto& texturePaths) {
             auto pathStemToPath = std::unordered_map<
               std::filesystem::path,
               std::filesystem::path,
               kdl::path_hash>{};
             for (const auto& texturePath : texturePaths)
             {
               pathStemToPath[kdl::path_remove_extension(texturePath)] = texturePath;
             }
             for (const auto& shader : shaders)
             {
               pathStemToPath[shader.shaderPath] = shader.shaderPath;
             }
             return kdl::vec_sort(
               pathStemToPath | std::views::values | kdl::ranges::to<std::vector>());
           });
}

Result<std::filesystem::path> findShaderTexture(
  const std::filesystem::path& texturePath,
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig)
{
  if (texturePath.empty())
  {
    return Error{"Empty texture path"};
  }

  if (
    kdl::vec_contains(
      materialConfig.extensions, kdl::str_to_lower(texturePath.extension().string()))
    && fs.pathInfo(texturePath) == fs::PathInfo::File)
  {
    return texturePath;
  }

  const auto directoryPath = texturePath.parent_path();
  const auto basename = texturePath.stem().string();
  return fs.find(
           texturePath.parent_path(),
           fs::TraversalMode::Flat,
           kdl::logical_and(
             fs::makeFilenamePathMatcher(basename + ".*"),
             fs::makeExtensionPathMatcher(materialConfig.extensions)))
         | kdl::and_then([&](auto candidates) -> Result<std::filesystem::path> {
             if (!candidates.empty())
             {
               return candidates.front();
             }
             return Error{fmt::format("File not found: {}", texturePath)};
           });
}

Result<std::filesystem::path> findShaderTexture(
  const std::vector<mdl::Quake3ShaderStage>& stages,
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig)
{
  auto path = stages | kdl::first([&](const auto& stage) {
                return findShaderTexture(stage.map, fs, materialConfig);
              });
  if (path)
  {
    return std::move(*path);
  }
  return Error{"Could not find texture file"};
}

Result<std::filesystem::path> findShaderTexture(
  const mdl::Quake3Shader& shader,
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig)
{
  return findShaderTexture(shader.editorImage, fs, materialConfig)
         | kdl::or_else(
           [&](auto) { return findShaderTexture(shader.shaderPath, fs, materialConfig); })
         | kdl::or_else(
           [&](auto) { return findShaderTexture(shader.lightImage, fs, materialConfig); })
         | kdl::or_else(
           [&](auto) { return findShaderTexture(shader.stages, fs, materialConfig); })
         | kdl::transform_error([&](auto) { return DefaultTexturePath; });
}

Result<mdl::Material> loadShaderMaterial(
  const mdl::Quake3Shader& shader,
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource)
{
  return findShaderTexture(shader, fs, materialConfig) | kdl::transform([&](auto path_) {
           return [&, path = std::move(path_)]() {
             return fs.openFile(path) | kdl::and_then([&](auto file) {
                      auto reader = file->reader().buffer();
                      return loadFreeImageTexture(reader).transform([](auto texture) {
                        texture.setMask(mdl::TextureMask::Off);
                        return texture;
                      });
                    });
           };
         })
         | kdl::transform([&](auto textureLoader) {
             const auto prefixLength = kdl::path_length(materialConfig.root);
             auto shaderName =
               getMaterialNameFromPathSuffix(shader.shaderPath, prefixLength);

             auto textureResource = createResource(std::move(textureLoader));
             auto material =
               mdl::Material{std::move(shaderName), std::move(textureResource)};
             material.setSurfaceParms(shader.surfaceParms);

             // Note that Quake 3 has a different understanding of front and back, so we
             // need to invert them.
             switch (shader.culling)
             {
             case mdl::Quake3Shader::Culling::Front:
               material.setCulling(mdl::MaterialCulling::Back);
               break;
             case mdl::Quake3Shader::Culling::Back:
               material.setCulling(mdl::MaterialCulling::Front);
               break;
             case mdl::Quake3Shader::Culling::None:
               material.setCulling(mdl::MaterialCulling::None);
               break;
             }

             if (!shader.stages.empty())
             {
               const auto& stage = shader.stages.front();
               if (stage.blendFunc.enable())
               {
                 material.setBlendFunc(
                   glGetEnum(stage.blendFunc.srcFactor),
                   glGetEnum(stage.blendFunc.destFactor));
               }
               else
               {
                 material.disableBlend();
               }
             }

             return material;
           });
}

Result<mdl::Texture> findAndLoadTexture(
  const std::filesystem::path& path,
  const std::string& name,
  const std::vector<std::filesystem::path>& extensions,
  const fs::FileSystem& fs,
  const std::optional<mdl::Palette>& palette)
{
  return findMaterialFile(fs, path, extensions)
    .and_then([&](const auto& actualPath) -> Result<mdl::Texture> {
      return loadTexture(actualPath, name, fs, palette);
    });
}

mdl::ResourceLoader<mdl::Texture> makeTextureResourceLoader(
  const std::filesystem::path& path,
  const std::string& name,
  const std::vector<std::filesystem::path>& extensions,
  const fs::FileSystem& fs,
  const std::optional<mdl::Palette>& palette)
{
  return [&, path, name, palette]() -> Result<mdl::Texture> {
    return findAndLoadTexture(path, name, extensions, fs, palette)
           | kdl::or_else([&](auto e) -> Result<mdl::Texture> {
               return Error{fmt::format("Could not load texture '{}': {}", path, e.msg)};
             });
  };
}

Result<mdl::Material> loadTextureMaterial(
  const std::filesystem::path& texturePath,
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource,
  const std::optional<mdl::Palette>& palette)
{
  const auto prefixLength = kdl::path_length(materialConfig.root);
  const auto pathMatcher = !materialConfig.extensions.empty()
                             ? fs::makeExtensionPathMatcher(materialConfig.extensions)
                             : fs::matchAnyPath;
  auto name = getMaterialNameFromPathSuffix(texturePath, prefixLength);

  auto textureLoader =
    makeTextureResourceLoader(texturePath, name, materialConfig.extensions, fs, palette);
  auto textureResource = createResource(std::move(textureLoader));
  return mdl::Material{std::move(name), std::move(textureResource)};
}

std::string materialCollectionName(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath)
{
  const auto prefixLength = kdl::path_length(materialConfig.root);
  if (const auto collectionPath =
        kdl::path_clip(materialPath, prefixLength).parent_path();
      !collectionPath.empty())
  {
    // If the file was loaded from a subdirectory of the root, use the parent path.
    return materialPath.parent_path().generic_string();
  }

  if (const auto* metadata =
        fs.metadata(materialPath, fs::FileSystemMetadataKeys::ImageFilePath);
      metadata && std::holds_alternative<std::filesystem::path>(*metadata))
  {
    if (const auto imageFileName = std::get<std::filesystem::path>(*metadata).filename();
        kdl::path_has_extension(kdl::path_to_lower(imageFileName), ".wad"))
    {
      // If the texture was loaded from a WAD file, use the WAD file name as the
      // collection name.
      return imageFileName.filename().generic_string();
    }
  }

  // Otherwise, just use the root directory name.
  return materialConfig.root.generic_string();
}

std::vector<mdl::MaterialCollection> groupMaterialsIntoCollections(
  std::vector<mdl::Material> materials)
{
  materials = kdl::vec_sort(std::move(materials), [&](const auto& lhs, const auto& rhs) {
    return lhs.collectionName() < rhs.collectionName()   ? true
           : lhs.collectionName() > rhs.collectionName() ? false
                                                         : lhs.name() < rhs.name();
  });

  return materials | kdl::views::chunk_by([&](const auto& lhs, const auto& rhs) {
           return lhs.collectionName() == rhs.collectionName();
         })
         | std::views::transform([&](auto groupedMaterials) {
             contract_assert(!groupedMaterials.empty());

             auto materialCollectionName = groupedMaterials.front().collectionName();

             auto materialsForCollection = std::vector<mdl::Material>(
               std::move_iterator{groupedMaterials.begin()},
               std::move_iterator{groupedMaterials.end()});

             materialsForCollection = kdl::vec_sort(
               std::move(materialsForCollection), [&](const auto& lhs, const auto& rhs) {
                 return lhs.relativePath() < rhs.relativePath();
               });

             return mdl::MaterialCollection{
               std::move(materialCollectionName), std::move(materialsForCollection)};
           })
         | kdl::ranges::to<std::vector>();
}

} // namespace


Result<mdl::Material> loadMaterial(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath,
  const mdl::CreateTextureResource& createResource,
  const std::vector<mdl::Quake3Shader>& shaders,
  const std::optional<mdl::Palette>& palette)
{
  const auto materialPathStem = kdl::path_remove_extension(materialPath);
  const auto iShader = std::ranges::find_if(
    shaders, [&](const auto& shader) { return shader.shaderPath == materialPathStem; });

  return (iShader != shaders.end()
            ? loadShaderMaterial(*iShader, fs, materialConfig, createResource)
            : loadTextureMaterial(
                materialPath, fs, materialConfig, createResource, palette))
         | kdl::transform([&](auto material) {
             fs.makeAbsolute(materialPath)
               | kdl::transform([&](auto absPath) { material.setAbsolutePath(absPath); })
               | kdl::or_else([](auto) { return kdl::void_success; });
             material.setRelativePath(materialPath);
             material.setCollectionName(
               materialCollectionName(fs, materialConfig, materialPath));
             return material;
           });
}

Result<std::vector<mdl::MaterialCollection>> loadMaterialCollections(
  const fs::FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  return loadShaders(fs, materialConfig, taskManager, logger)
         | kdl::transform([&](auto shaders) {
             return shaders | std::views::filter([&](const auto& shader) {
                      return kdl::path_has_prefix(shader.shaderPath, materialConfig.root);
                    })
                    | kdl::views::as_rvalue | kdl::ranges::to<std::vector>();
           })
         | kdl::join(loadPalette(fs, materialConfig))
         | kdl::and_then([&](auto shaders, auto palette) {
             return findAllMaterialPaths(fs, materialConfig, shaders)
                    | kdl::and_then([&](const auto& materialPaths) {
                        return materialPaths
                               | std::views::transform([&](const auto& materialPath) {
                                   return loadMaterial(
                                     fs,
                                     materialConfig,
                                     materialPath,
                                     createResource,
                                     shaders,
                                     palette);
                                 })
                               | kdl::fold;
                      });
           })
         | kdl::transform([&](auto materials) {
             return groupMaterialsIntoCollections(std::move(materials));
           });
}

} // namespace tb::io
