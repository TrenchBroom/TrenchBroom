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

#include "ReadQuake3ShaderTexture.h"

#include "Assets/Material.h"
#include "Assets/Quake3Shader.h"
#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/TraversalMode.h"

#include "kdl/functional.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace TrenchBroom::IO
{

namespace
{

std::optional<std::filesystem::path> findImage(
  const std::filesystem::path& texturePath, const FileSystem& fs)
{
  static const auto imageExtensions =
    std::vector<std::string>{".tga", ".png", ".jpg", ".jpeg"};

  if (!texturePath.empty())
  {
    if (
      kdl::vec_contains(
        imageExtensions, kdl::str_to_lower(texturePath.extension().string()))
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
               makeExtensionPathMatcher(imageExtensions)))
           | kdl::transform([](auto candidates) -> std::optional<std::filesystem::path> {
               if (!candidates.empty())
               {
                 return candidates.front();
               }
               return std::nullopt;
             })
           | kdl::value_or(std::nullopt);
  }

  // texture path is empty OR (the extension is not empty AND the file does not exist)
  return std::nullopt;
}

std::optional<std::filesystem::path> findImagePath(
  const Assets::Quake3Shader& shader, const FileSystem& fs)
{
  if (const auto path = findImage(shader.editorImage, fs))
  {
    return path;
  }
  if (const auto path = findImage(shader.shaderPath, fs))
  {
    return path;
  }
  if (const auto path = findImage(shader.lightImage, fs))
  {
    return path;
  }
  for (const auto& stage : shader.stages)
  {
    if (const auto path = findImage(stage.map, fs))
    {
      return path;
    }
  }

  return std::nullopt;
}

Result<Assets::Texture> loadTextureImage(
  const std::filesystem::path& imagePath, const FileSystem& fs)
{
  auto imageName = imagePath.filename();
  if (fs.pathInfo(imagePath) != PathInfo::File)
  {
    return Error{"Image file '" + imagePath.string() + "' does not exist"};
  }

  return fs.openFile(imagePath) | kdl::and_then([&](auto file) {
           auto reader = file->reader().buffer();
           return readFreeImageTexture(reader);
         })
         | kdl::or_else([&](auto e) { return Result<Assets::Texture>{Error{e.msg}}; });
}

} // namespace

Result<Assets::Material, ReadMaterialError> readQuake3ShaderTexture(
  std::string shaderName, const File& file, const FileSystem& fs)
{
  const auto* shaderFile = dynamic_cast<const ObjectFile<Assets::Quake3Shader>*>(&file);
  if (!shaderFile)
  {
    return ReadMaterialError{std::move(shaderName), "Shader not found: " + shaderName};
  }

  const auto& shader = shaderFile->object();
  const auto imagePath = findImagePath(shader, fs);
  if (!imagePath)
  {
    return ReadMaterialError{
      std::move(shaderName),
      "Could not find texture path for shader '" + shader.shaderPath.string() + "'"};
  }

  return loadTextureImage(*imagePath, fs) | kdl::and_then([&](auto texture) {
           texture.setMask(Assets::TextureMask::Off);

           auto material = Assets::Material{std::move(shaderName), std::move(texture)};
           material.setSurfaceParms(shader.surfaceParms);

           // Note that Quake 3 has a different understanding of front and back, so we
           // need to invert them.
           switch (shader.culling)
           {
           case Assets::Quake3Shader::Culling::Front:
             material.setCulling(Assets::MaterialCulling::Back);
             break;
           case Assets::Quake3Shader::Culling::Back:
             material.setCulling(Assets::MaterialCulling::Front);
             break;
           case Assets::Quake3Shader::Culling::None:
             material.setCulling(Assets::MaterialCulling::None);
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

           return Result<Assets::Material>{std::move(material)};
         })
         | kdl::or_else([&](auto e) {
             return Result<Assets::Material, ReadMaterialError>{
               ReadMaterialError{std::move(shaderName), e.msg}};
           });
}

} // namespace TrenchBroom::IO
