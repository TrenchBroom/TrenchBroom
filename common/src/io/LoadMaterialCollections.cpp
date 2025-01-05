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
#include "io/FileSystem.h"
#include "io/LoadShaders.h"
#include "io/MaterialUtils.h"
#include "io/PathInfo.h"
#include "io/PathMatcher.h"
#include "io/ReadDdsTexture.h"
#include "io/ReadFreeImageTexture.h"
#include "io/ReadM8Texture.h"
#include "io/ReadMipTexture.h"
#include "io/ReadWalTexture.h"
#include "io/ResourceUtils.h"
#include "io/TraversalMode.h"
#include "mdl/GameConfig.h"
#include "mdl/MaterialCollection.h"
#include "mdl/Palette.h"
#include "mdl/Quake3Shader.h"
#include "mdl/Texture.h"
#include "mdl/TextureResource.h"

#include "kdl/functional.h"
#include "kdl/grouped_range.h"
#include "kdl/map_utils.h"
#include "kdl/path_hash.h"
#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <string>

namespace tb::io
{

namespace
{

std::optional<Result<mdl::Palette>> loadPalette(
  const FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  if (materialConfig.palette.empty())
  {
    return std::nullopt;
  }

  return fs.openFile(materialConfig.palette) | kdl::and_then([&](auto file) {
           return mdl::loadPalette(*file, materialConfig.palette);
         });
}

bool shouldExclude(
  const std::string& materialName, const std::vector<std::string>& patterns)
{
  return std::any_of(patterns.begin(), patterns.end(), [&](const auto& pattern) {
    return kdl::ci::str_matches_glob(materialName, pattern);
  });
}

Result<std::vector<std::filesystem::path>> findTexturePaths(
  const FileSystem& fs, const mdl::MaterialConfig& materialConfig)
{
  return fs.find(
           materialConfig.root,
           TraversalMode::Recursive,
           makeExtensionPathMatcher(materialConfig.extensions))
         | kdl::transform([&](auto paths) {
             return kdl::vec_filter(std::move(paths), [&](const auto& path) {
               return !shouldExclude(path.stem().string(), materialConfig.excludes);
             });
           });
}

Result<std::vector<std::filesystem::path>> findAllMaterialPaths(
  const FileSystem& fs,
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
             return kdl::vec_sort(kdl::map_values(pathStemToPath));
           });
}

Result<std::filesystem::path> findShaderTexture(
  const std::filesystem::path& texturePath,
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig)
{
  if (texturePath.empty())
  {
    return Error{"Empty texture path"};
  }

  if (
    kdl::vec_contains(
      materialConfig.extensions, kdl::str_to_lower(texturePath.extension().string()))
    && fs.pathInfo(texturePath) == PathInfo::File)
  {
    return texturePath;
  }

  const auto directoryPath = texturePath.parent_path();
  const auto basename = texturePath.stem().string();
  return fs.find(
           texturePath.parent_path(),
           TraversalMode::Flat,
           kdl::lift_and(
             makeFilenamePathMatcher(basename + ".*"),
             makeExtensionPathMatcher(materialConfig.extensions)))
         | kdl::and_then([&](auto candidates) -> Result<std::filesystem::path> {
             if (!candidates.empty())
             {
               return candidates.front();
             }
             return Error{fmt::format("File not found: {}", texturePath.string())};
           });
}

Result<std::filesystem::path> findShaderTexture(
  const std::vector<mdl::Quake3ShaderStage>& stages,
  const FileSystem& fs,
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
  const FileSystem& fs,
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
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource)
{
  return findShaderTexture(shader, fs, materialConfig) | kdl::transform([&](auto path_) {
           return [&, path = std::move(path_)]() {
             return fs.openFile(path) | kdl::and_then([&](auto file) {
                      auto reader = file->reader().buffer();
                      return readFreeImageTexture(reader).transform([](auto texture) {
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

Result<mdl::Texture> loadTexture(
  const std::filesystem::path& path,
  const std::string& name,
  const std::vector<std::string>& extensions,
  const FileSystem& fs,
  const std::optional<Result<mdl::Palette>>& paletteResult)
{
  return findMaterialFile(fs, path, extensions)
    .and_then([&](const auto& actualPath) -> Result<mdl::Texture> {
      const auto extension = kdl::str_to_lower(actualPath.extension().string());
      if (extension == ".d")
      {
        if (!paletteResult)
        {
          return Error{"Palette is required for mip textures"};
        }

        return fs.openFile(actualPath).join(*paletteResult)
               | kdl::and_then([&](auto file, const auto& palette) {
                   auto reader = file->reader().buffer();
                   const auto mask = getTextureMaskFromName(name);
                   return readIdMipTexture(reader, palette, mask);
                 });
      }
      else if (extension == ".c")
      {
        const auto mask = getTextureMaskFromName(name);
        return fs.openFile(actualPath) | kdl::and_then([&](auto file) {
                 auto reader = file->reader().buffer();
                 return readHlMipTexture(reader, mask);
               });
      }
      else if (extension == ".wal")
      {
        auto palette = std::optional<mdl::Palette>{};
        if (paletteResult)
        {
          if (paletteResult->is_error())
          {
            return Error{
              std::visit([](const auto& e) { return e.msg; }, paletteResult->error())};
          }
          palette = paletteResult->value();
        }

        return fs.openFile(actualPath) | kdl::and_then([&](auto file) {
                 auto reader = file->reader().buffer();
                 return readWalTexture(reader, palette);
               });
      }
      else if (extension == ".m8")
      {
        return fs.openFile(actualPath) | kdl::and_then([&](auto file) {
                 auto reader = file->reader().buffer();
                 return readM8Texture(reader);
               });
      }
      else if (extension == ".dds")
      {
        return fs.openFile(actualPath) | kdl::and_then([&](auto file) {
                 auto reader = file->reader().buffer();
                 return readDdsTexture(reader);
               });
      }
      else if (isSupportedFreeImageExtension(extension))
      {
        return fs.openFile(actualPath) | kdl::and_then([&](auto file) {
                 auto reader = file->reader().buffer();
                 return readFreeImageTexture(reader);
               });
      }

      return Error{"Unknown texture file extension: " + extension};
    });
}

mdl::ResourceLoader<mdl::Texture> makeTextureResourceLoader(
  const std::filesystem::path& path,
  const std::string& name,
  const std::vector<std::string>& extensions,
  const FileSystem& fs,
  const std::optional<Result<mdl::Palette>>& paletteResult)
{
  return [&, path, name, paletteResult]() -> Result<mdl::Texture> {
    return loadTexture(path, name, extensions, fs, paletteResult)
           | kdl::or_else([&](auto e) -> Result<mdl::Texture> {
               return Error{
                 fmt::format("Could not load texture '{}': {}", path.string(), e.msg)};
             });
  };
}

Result<mdl::Material> loadTextureMaterial(
  const std::filesystem::path& texturePath,
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource,
  const std::optional<Result<mdl::Palette>>& paletteResult)
{
  const auto prefixLength = kdl::path_length(materialConfig.root);
  const auto pathMatcher = !materialConfig.extensions.empty()
                             ? makeExtensionPathMatcher(materialConfig.extensions)
                             : matchAnyPath;
  auto name = getMaterialNameFromPathSuffix(texturePath, prefixLength);

  auto textureLoader = makeTextureResourceLoader(
    texturePath, name, materialConfig.extensions, fs, paletteResult);
  auto textureResource = createResource(std::move(textureLoader));
  return mdl::Material{std::move(name), std::move(textureResource)};
}

std::string materialCollectionName(
  const FileSystem& fs,
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
        fs.metadata(materialPath, io::FileSystemMetadataKeys::ImageFilePath);
      metadata && std::holds_alternative<std::filesystem::path>(*metadata))
  {
    if (const auto imageFileName = std::get<std::filesystem::path>(*metadata).filename();
        kdl::ci::str_is_equal(imageFileName.extension().string(), ".wad"))
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

  auto materialsByCollection =
    kdl::make_grouped_range(materials, [&](const auto& lhs, const auto& rhs) {
      return lhs.collectionName() == rhs.collectionName();
    });

  return kdl::vec_transform(materialsByCollection, [&](auto groupedMaterials) {
    assert(!groupedMaterials.empty());

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
  });
}

} // namespace


Result<mdl::Material> loadMaterial(
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const std::filesystem::path& materialPath,
  const mdl::CreateTextureResource& createResource,
  const std::vector<mdl::Quake3Shader>& shaders,
  const std::optional<Result<mdl::Palette>>& paletteResult)
{
  const auto materialPathStem = kdl::path_remove_extension(materialPath);
  const auto iShader =
    std::find_if(shaders.begin(), shaders.end(), [&](const auto& shader) {
      return shader.shaderPath == materialPathStem;
    });

  return (iShader != shaders.end()
            ? loadShaderMaterial(*iShader, fs, materialConfig, createResource)
            : loadTextureMaterial(
                materialPath, fs, materialConfig, createResource, paletteResult))
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
  const FileSystem& fs,
  const mdl::MaterialConfig& materialConfig,
  const mdl::CreateTextureResource& createResource,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  const auto paletteResult = loadPalette(fs, materialConfig);

  return loadShaders(fs, materialConfig, taskManager, logger)
         | kdl::transform([&](auto shaders) {
             return kdl::vec_filter(std::move(shaders), [&](const auto& shader) {
               return kdl::path_has_prefix(shader.shaderPath, materialConfig.root);
             });
           })
         | kdl::and_then([&](auto shaders) {
             return findAllMaterialPaths(fs, materialConfig, shaders)
                    | kdl::and_then([&](const auto& materialPaths) {
                        return kdl::vec_transform(
                                 materialPaths,
                                 [&](const auto& materialPath) {
                                   return loadMaterial(
                                     fs,
                                     materialConfig,
                                     materialPath,
                                     createResource,
                                     shaders,
                                     paletteResult);
                                 })
                               | kdl::fold;
                      });
           })
         | kdl::transform([&](auto materials) {
             return groupMaterialsIntoCollections(std::move(materials));
           });
}

} // namespace tb::io
