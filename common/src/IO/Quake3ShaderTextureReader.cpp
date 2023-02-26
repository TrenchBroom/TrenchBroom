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

#include "Quake3ShaderTextureReader.h"

#include "Assets/Quake3Shader.h"
#include "Assets/Texture.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/PathInfo.h"
#include "IO/ResourceUtils.h"
#include "Renderer/GL.h"

#include <kdl/functional.h>

#include <string>
#include <vector>

namespace TrenchBroom::IO
{

Quake3ShaderTextureReader::Quake3ShaderTextureReader(
  GetTextureName getTextureName, const FileSystem& fs, Logger& logger)
  : TextureReader{std::move(getTextureName), fs, logger}
{
}

Assets::Texture Quake3ShaderTextureReader::doReadTexture(std::shared_ptr<File> file) const
{
  const auto* shaderFile = dynamic_cast<ObjectFile<Assets::Quake3Shader>*>(file.get());
  if (!shaderFile)
  {
    throw AssetException{"File is not a shader"};
  }

  const auto& shader = shaderFile->object();
  const auto texturePath = findTexturePath(shader);
  if (texturePath.isEmpty())
  {
    throw AssetException{
      "Could not find texture path for shader '" + shader.shaderPath.asString() + "'"};
  }

  auto texture = loadTextureImage(shader.shaderPath, texturePath);
  texture.setSurfaceParms(shader.surfaceParms);
  texture.setOpaque();

  // Note that Quake 3 has a different understanding of front and back, so we need to
  // invert them.
  switch (shader.culling)
  {
  case Assets::Quake3Shader::Culling::Front:
    texture.setCulling(Assets::TextureCulling::Back);
    break;
  case Assets::Quake3Shader::Culling::Back:
    texture.setCulling(Assets::TextureCulling::Front);
    break;
  case Assets::Quake3Shader::Culling::None:
    texture.setCulling(Assets::TextureCulling::None);
    break;
  }

  if (!shader.stages.empty())
  {
    const auto& stage = shader.stages.front();
    if (stage.blendFunc.enable())
    {
      texture.setBlendFunc(
        glGetEnum(stage.blendFunc.srcFactor), glGetEnum(stage.blendFunc.destFactor));
    }
    else
    {
      texture.disableBlend();
    }
  }

  return texture;
}

Assets::Texture Quake3ShaderTextureReader::loadTextureImage(
  const Path& shaderPath, const Path& imagePath) const
{
  if (m_fs.pathInfo(imagePath) != PathInfo::File)
  {
    throw AssetException{"Image file '" + imagePath.asString() + "' does not exist"};
  }

  auto name = textureName(shaderPath);
  auto imageReader =
    FreeImageTextureReader{makeGetTextureNameFromString(std::move(name)), m_fs, m_logger};
  return imageReader.readTexture(m_fs.openFile(imagePath));
}

Path Quake3ShaderTextureReader::findTexturePath(const Assets::Quake3Shader& shader) const
{
  if (const auto path = findTexture(shader.editorImage); !path.isEmpty())
  {
    return path;
  }
  if (const auto path = findTexture(shader.shaderPath); !path.isEmpty())
  {
    return path;
  }
  if (const auto path = findTexture(shader.lightImage); !path.isEmpty())
  {
    return path;
  }
  for (const auto& stage : shader.stages)
  {
    if (const auto path = findTexture(stage.map); !path.isEmpty())
    {
      return path;
    }
  }

  return Path{};
}

Path Quake3ShaderTextureReader::findTexture(const Path& texturePath) const
{
  if (
    !texturePath.isEmpty()
    && (texturePath.extension().empty() || m_fs.pathInfo(texturePath) == PathInfo::File))
  {
    const auto directoryPath = texturePath.deleteLastComponent();
    const auto basename = texturePath.basename();
    const auto candidates = m_fs.find(
      texturePath.deleteLastComponent(),
      kdl::lift_and(
        makeFilenamePathMatcher(basename + ".*"),
        makeExtensionPathMatcher({"tga", "png", "jpg", "jpeg"})));
    return !candidates.empty() ? candidates.front() : Path{};
  }
  // texture path is empty OR (the extension is not empty AND the file exists)
  return texturePath;
}

} // namespace TrenchBroom::IO
