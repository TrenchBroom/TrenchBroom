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

#include "mdl/MaterialUtils.h"

#include "Logger.h"
#include "fs/FileSystem.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "gl/Material.h"
#include "gl/Texture.h"
#include "gl/TextureBuffer.h"
#include "gl/TextureResource.h"
#include "mdl/LoadFreeImageTexture.h"

#include "kd/functional.h"
#include "kd/path_utils.h"
#include "kd/reflection_impl.h"
#include "kd/set_temp.h"
#include "kd/string_compare.h"

namespace tb::mdl
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
  const fs::FileSystem& fs,
  const std::filesystem::path& materialPath,
  const std::vector<std::filesystem::path>& extensions)
{
  if (fs.pathInfo(materialPath) == fs::PathInfo::File)
  {
    return materialPath;
  }

  if (fs.pathInfo(materialPath.parent_path()) != fs::PathInfo::Directory)
  {
    return materialPath;
  }

  const auto matcher = kdl::logical_and(
    fs::makeFilenamePathMatcher(
      kdl::path_remove_extension(materialPath.filename()).string() + ".*"),
    fs::makeExtensionPathMatcher(extensions));

  return fs.find(materialPath.parent_path(), fs::TraversalMode::Flat, matcher)
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
  const auto size = gl::sizeAtMipLevel(width, height, mipLevel);
  return size.x() * size.y();
}

kdl_reflect_impl(ReadMaterialError);

gl::TextureMask getTextureMaskFromName(std::string_view name)
{
  return kdl::cs::str_is_prefix(name, "{") ? gl::TextureMask::On : gl::TextureMask::Off;
}

gl::Texture loadDefaultTexture(const fs::FileSystem& fs, Logger& logger)
{
  // recursion guard
  static auto executing = false;
  if (!executing)
  {
    const auto set_executing = kdl::set_temp{executing};

    return fs.openFile(DefaultTexturePath) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return loadFreeImageTexture(reader);
           })
           | kdl::transform_error([&](auto e) {
               logger.error() << "Could not load default texture: " << e.msg;
               return gl::Texture{32, 32};
             })
           | kdl::value();
  }
  else
  {
    logger.error() << "Could not load default texture";
  }

  return gl::Texture{32, 32};
}

gl::Material loadDefaultMaterial(
  const fs::FileSystem& fs, std::string name, Logger& logger)
{
  auto textureResource = createTextureResource(loadDefaultTexture(fs, logger));
  return gl::Material{std::move(name), std::move(textureResource)};
}

} // namespace tb::mdl
