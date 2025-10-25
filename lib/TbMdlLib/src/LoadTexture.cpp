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
#include "mdl/LoadFreeImageTexture.h"
#include "mdl/LoadM8Texture.h"
#include "mdl/LoadMipTexture.h"
#include "mdl/LoadShaders.h"
#include "mdl/LoadWalTexture.h"
#include "mdl/LoadSwlTexture.h"
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
  const auto extension = kdl::path_to_lower(path.extension());
  if (extension == ".d")
  {
    if (!palette)
    {
      return Error{"Palette is required for mip textures"};
    }

    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             const auto mask = getTextureMaskFromName(name);
             return loadIdMipTexture(reader, *palette, mask);
           });
  }
  else if (extension == ".c")
  {
    const auto mask = getTextureMaskFromName(name);
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadHlMipTexture(reader, mask);
           });
  }
  else if (extension == ".wal")
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadWalTexture(reader, palette);
           });
  }
  else if (extension == ".swl")
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadSwlTexture(reader);
           });
  }
  else if (extension == ".m8")
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadM8Texture(reader);
           });
  }
  else if (extension == ".dds")
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadDdsTexture(reader);
           });
  }
  else if (isSupportedFreeImageExtension(extension))
  {
    return fs.openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadFreeImageTexture(reader);
           });
  }

  return Error{fmt::format("Unknown texture file extension: {}", extension)};
}

} // namespace tb::mdl
