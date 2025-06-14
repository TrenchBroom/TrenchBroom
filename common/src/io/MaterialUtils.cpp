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

#include "MaterialUtils.h"

#include "io/FileSystem.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"
#include "mdl/TextureBuffer.h"

#include "kdl/functional.h"
#include "kdl/path_utils.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_compare.h"

namespace tb::io
{

std::string getMaterialNameFromPathSuffix(
  const std::filesystem::path& path, size_t prefixLength)
{
  return prefixLength < kdl::path_length(path)
           ? kdl::path_remove_extension(kdl::path_clip(path, prefixLength))
               .generic_string()
           : "";
}

Result<std::filesystem::path> findMaterialFile(
  const FileSystem& fs,
  const std::filesystem::path& materialPath,
  const std::vector<std::filesystem::path>& extensions)
{
  if (fs.pathInfo(materialPath) == PathInfo::File)
  {
    return materialPath;
  }

  if (fs.pathInfo(materialPath.parent_path()) != PathInfo::Directory)
  {
    return materialPath;
  }

  const auto matcher = kdl::logical_and(
    makeFilenamePathMatcher(
      kdl::path_remove_extension(materialPath.filename()).string() + ".*"),
    makeExtensionPathMatcher(extensions));

  return fs.find(materialPath.parent_path(), TraversalMode::Flat, matcher)
         | kdl::transform([&](const auto& candidates) {
             return !candidates.empty() ? candidates.front() : materialPath;
           });
}

bool checkTextureDimensions(size_t width, size_t height)
{
  return width > 0 && height > 0 && width <= 8192 && height <= 8192;
}

size_t mipSize(const size_t width, const size_t height, const size_t mipLevel)
{
  const auto size = mdl::sizeAtMipLevel(width, height, mipLevel);
  return size.x() * size.y();
}

kdl_reflect_impl(ReadMaterialError);

mdl::TextureMask getTextureMaskFromName(std::string_view name)
{
  return kdl::cs::str_is_prefix(name, "{") ? mdl::TextureMask::On : mdl::TextureMask::Off;
}

} // namespace tb::io
