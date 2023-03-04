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

#pragma once

#include "IO/TextureReader.h"

#include <memory>

namespace TrenchBroom::Assets
{
class Quake3Shader;
}

namespace TrenchBroom::IO
{
class File;
class FileSystem;
class Path;

/**
 * Loads a texture that represents a Quake 3 shader from the file system. Uses a given
 * file system to locate the actual editor image for the shader. The shader is expected to
 * be readily parsed and available as a virtual object file in the file system.
 */
class Quake3ShaderTextureReader : public TextureReader
{
public:
  /**
   * Creates a texture reader using the given name strategy and file system to locate the
   * texture image.
   *
   * @param getTextureName the strategy to determine the texture name
   * @param fs the file system to use when locating the texture image
   * @param logger the logger to use
   */
  Quake3ShaderTextureReader(
    GetTextureName getTextureName, const FileSystem& fs, Logger& logger);

private:
  Assets::Texture doReadTexture(std::shared_ptr<File> file) const override;
  kdl::result<Assets::Texture, ReadTextureError> loadTextureImage(
    const Path& shaderPath, const Path& imagePath) const;
  Path findTexturePath(const Assets::Quake3Shader& shader) const;
  Path findTexture(const Path& texturePath) const;
};

} // namespace TrenchBroom::IO
