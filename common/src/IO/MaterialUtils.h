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

#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Error.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Macros.h"
#include "Result.h"

#include "kdl/reflection_decl.h"
#include "kdl/result.h"

#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class Material;
enum class TextureMask;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::IO
{
class File;
class FileSystem;

std::string getMaterialNameFromPathSuffix(
  const std::filesystem::path& path, size_t prefixLength);

bool checkTextureDimensions(size_t width, size_t height);

size_t mipSize(size_t width, size_t height, size_t mipLevel);

struct ReadMaterialError
{
  std::string materialName;
  std::string msg;

  kdl_reflect_decl(ReadMaterialError, materialName, msg);
};

inline auto makeReadTextureErrorHandler(const FileSystem& fs, Logger& logger)
{
  return [&](Error e) {
    logger.error() << "Could not open texture file: " << e.msg;
    return Result<Assets::Texture>{loadDefaultTexture(fs, logger)};
  };
}

inline auto makeReadMaterialErrorHandler(const FileSystem& fs, Logger& logger)
{
  return kdl::overload(
    [&](Error e) {
      logger.error() << "Could not open material file: " << e.msg;
      return Result<Assets::Material>{loadDefaultMaterial(fs, "", logger)};
    },
    [&](ReadMaterialError e) {
      logger.error() << "Could not read material '" << e.materialName << "': " << e.msg;
      return Result<Assets::Material>{loadDefaultMaterial(fs, e.materialName, logger)};
    });
}

Assets::TextureMask getTextureMaskFromName(std::string_view name);

} // namespace TrenchBroom::IO
