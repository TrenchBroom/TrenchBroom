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

#include "Macros.h"

#include <kdl/reflection_decl.h>
#include <kdl/result_forward.h>

#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class Texture;
}

namespace TrenchBroom::IO
{
class File;
class FileSystem;

std::string getTextureNameFromPathSuffix(
  const std::filesystem::path& path, size_t prefixLength);

bool checkTextureDimensions(size_t width, size_t height);

size_t mipSize(size_t width, size_t height, size_t mipLevel);

struct ReadTextureError
{
  std::string textureName;
  std::string msg;

  kdl_reflect_decl(ReadTextureError, textureName, msg);
};

std::function<kdl::result<Assets::Texture>(ReadTextureError)> makeReadTextureErrorHandler(
  const FileSystem& fs, Logger& logger);

} // namespace TrenchBroom::IO
