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

#pragma once

#include "Logger.h"
#include "Result.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"

#include "kd/reflection_decl.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace tb
{
class Logger;

namespace fs
{
class FileSystem;
} // namespace fs

namespace mdl
{
class Material;
class Texture;

enum class TextureMask;

static const auto DefaultTexturePath = std::filesystem::path{"textures/__TB_empty.png"};

std::string getMaterialNameFromPathSuffix(
  const std::filesystem::path& path, size_t prefixLength);

Result<std::filesystem::path> findMaterialFile(
  const fs::FileSystem& fs,
  const std::filesystem::path& materialPath,
  const std::vector<std::filesystem::path>& extensions);

bool checkTextureDimensions(size_t width, size_t height);

size_t mipSize(size_t width, size_t height, size_t mipLevel);

struct ReadMaterialError
{
  std::string materialName;
  std::string msg;

  kdl_reflect_decl(ReadMaterialError, materialName, msg);
};

/**
 * Loads a default texture from the given file system. If the default texture cannot be
 * found or opened, an empty texture is returned.
 *
 * @param fs the file system used to locate the texture file
 * @param name the name of the texture to be returned
 * @return the default texture
 */
Texture loadDefaultTexture(const fs::FileSystem& fs, Logger& logger);

/**
 * Loads a default material from the given file system. If the default material cannot be
 * found or opened, an empty material is returned.
 *
 * @param fs the file system used to locate the material file
 * @param name the name of the material to be returned
 * @return the default material
 */
Material loadDefaultMaterial(const fs::FileSystem& fs, std::string name, Logger& logger);

inline auto makeReadTextureErrorHandler(const fs::FileSystem& fs, Logger& logger)
{
  return [&](Error e) {
    logger.error() << "Could not open texture file: " << e.msg;
    return Result<Texture>{loadDefaultTexture(fs, logger)};
  };
}

inline auto makeReadMaterialErrorHandler(const fs::FileSystem& fs, Logger& logger)
{
  return kdl::overload(
    [&](Error e) {
      logger.error() << "Could not open material file: " << e.msg;
      return Result<Material>{loadDefaultMaterial(fs, "", logger)};
    },
    [&](ReadMaterialError e) {
      logger.error() << "Could not read material '" << e.materialName << "': " << e.msg;
      return Result<Material>{loadDefaultMaterial(fs, e.materialName, logger)};
    });
}

TextureMask getTextureMaskFromName(std::string_view name);

} // namespace mdl
} // namespace tb
