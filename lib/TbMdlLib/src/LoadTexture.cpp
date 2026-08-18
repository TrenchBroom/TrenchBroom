/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/LoadTexture.h"

#include "fs/FileSystem.h"
#include "gl/Texture.h"
#include "mdl/LoadDdsTexture.h"
#include "mdl/LoadImageTexture.h"
#include "mdl/LoadM32Texture.h"
#include "mdl/LoadM8Texture.h"
#include "mdl/LoadMipTexture.h"
#include "mdl/LoadShaders.h"
#include "mdl/LoadWalTexture.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Palette.h"

#include "kd/path_utils.h"
#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <string>

namespace tb::mdl
{

Result<gl::Texture> loadTexture(
  const std::filesystem::path& path,
  const std::string& name,
  const fs::FileSystem& fs,
  const std::optional<Palette>& palette)
{
  if (isIdMipTexture(path))
  {
    if (!palette)
    {
      return Error{"Palette is required for mip textures"};
    }

    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             const auto isMasked = isMaskedTextureName(name);
             return loadIdMipTexture(reader, *palette, isMasked);
           });
  }
  else if (isHlMipTexture(path))
  {
    const auto isMasked = isMaskedTextureName(name);
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadHlMipTexture(reader, isMasked);
           });
  }
  else if (isWalTexture(path))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadWalTexture(reader, palette);
           });
  }
  else if (isM8Texture(path))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadM8Texture(reader);
           });
  }
  else if (isM32Texture(path))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadM32Texture(reader);
           });
  }
  else if (isDdsTexture(path))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadDdsTexture(reader);
           });
  }
  else if (isImageTexture(path))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadImageTexture(reader);
           });
  }

  return Error{fmt::format(
    "Unknown texture file extension: {}", kdl::path_to_lower(path.extension()))};
}

} // namespace tb::mdl
