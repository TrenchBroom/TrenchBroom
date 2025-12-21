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

#include "ResourceUtils.h"

#include "Logger.h"
#include "fs/FileSystem.h"
#include "io/LoadFreeImageTexture.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"
#include "mdl/TextureResource.h"

#include "kd/result.h"
#include "kd/set_temp.h"

#include <string>

namespace tb::io
{

mdl::Texture loadDefaultTexture(const fs::FileSystem& fs, Logger& logger)
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
               return mdl::Texture{32, 32};
             })
           | kdl::value();
  }
  else
  {
    logger.error() << "Could not load default texture";
  }

  return mdl::Texture{32, 32};
}

mdl::Material loadDefaultMaterial(
  const fs::FileSystem& fs, std::string name, Logger& logger)
{
  auto textureResource = createTextureResource(loadDefaultTexture(fs, logger));
  return mdl::Material{std::move(name), std::move(textureResource)};
}

} // namespace tb::io
