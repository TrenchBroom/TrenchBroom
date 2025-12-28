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

#include "mdl/LoadSkin.h"

#include "Logger.h"
#include "fs/FileSystem.h"
#include "mdl/LoadTexture.h"
#include "mdl/MaterialUtils.h"
#include "mdl/Palette.h"

namespace tb::mdl
{

gl::Material loadSkin(
  const std::filesystem::path& path, const fs::FileSystem& fs, Logger& logger)
{
  return loadSkin(path, fs, std::nullopt, logger);
}

gl::Material loadSkin(
  const std::filesystem::path& path,
  const fs::FileSystem& fs,
  const std::optional<Palette>& palette,
  Logger& logger)
{
  auto name = path.stem().string();

  return loadTexture(path, name, fs, palette) | kdl::transform([&](auto texture) {
           auto textureResource = createTextureResource(std::move(texture));
           return gl::Material{std::move(name), std::move(textureResource)};
         })
         | kdl::transform_error([&](auto e) {
             logger.error() << "Could not load skin '" << path << "': " << e.msg;
             return loadDefaultMaterial(fs, name, logger);
           })
         | kdl::value();
}

} // namespace tb::mdl
