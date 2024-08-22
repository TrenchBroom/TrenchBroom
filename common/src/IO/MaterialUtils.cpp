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

#include "MaterialUtils.h"

#include "Assets/TextureBuffer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"

#include "kdl/path_utils.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"

namespace TrenchBroom::IO
{

std::string getMaterialNameFromPathSuffix(
  const std::filesystem::path& path, size_t prefixLength)
{
  return prefixLength < kdl::path_length(path)
           ? kdl::path_remove_extension(kdl::path_clip(path, prefixLength))
               .generic_string()
           : "";
}

bool checkTextureDimensions(size_t width, size_t height)
{
  return width > 0 && height > 0 && width <= 8192 && height <= 8192;
}

size_t mipSize(const size_t width, const size_t height, const size_t mipLevel)
{
  const auto size = Assets::sizeAtMipLevel(width, height, mipLevel);
  return size.x() * size.y();
}

kdl_reflect_impl(ReadMaterialError);

Assets::TextureMask getTextureMaskFromName(std::string_view name)
{
  return kdl::cs::str_is_prefix(name, "{") ? Assets::TextureMask::On
                                           : Assets::TextureMask::Off;
}

} // namespace TrenchBroom::IO
