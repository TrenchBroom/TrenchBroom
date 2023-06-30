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

#include "IO/TextureUtils.h"

#include <kdl/result_forward.h>

#include <string>

namespace TrenchBroom::IO
{
class File;
class FileSystem;

/**
 * Loads a texture that represents a Quake 3 shader from the file system. Uses a given
 * file system to locate the actual editor image for the shader. The shader is expected to
 * be readily parsed and available as a virtual object file in the file system.
 */
kdl::result<Assets::Texture, ReadTextureError> readQuake3ShaderTexture(
  std::string shaderName, const File& file, const FileSystem& fs);

} // namespace TrenchBroom::IO
