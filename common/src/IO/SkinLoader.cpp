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

#include "SkinLoader.h"

#include "Assets/Material.h"
#include "Assets/Palette.h"
#include "Assets/TextureResource.h"
#include "Ensure.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/MaterialUtils.h"
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ReadWalTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"

#include <string>

namespace TrenchBroom
{
namespace IO
{

Assets::Material loadSkin(
  const std::filesystem::path& path, const FileSystem& fs, Logger& logger)
{
  return loadSkin(path, fs, std::nullopt, logger);
}

Assets::Material loadSkin(
  const std::filesystem::path& path,
  const FileSystem& fs,
  const std::optional<Assets::Palette>& palette,
  Logger& logger)
{
  return fs.openFile(path)
         | kdl::and_then([&](auto file) -> Result<Assets::Material, ReadMaterialError> {
             const auto extension = kdl::str_to_lower(path.extension().string());
             auto reader = file->reader().buffer();
             return (extension == ".wal" ? readWalTexture(reader, palette)
                                         : readFreeImageTexture(reader))
                    | kdl::transform([&](auto texture) {
                        auto textureResource = createTextureResource(std::move(texture));
                        return Assets::Material{
                          path.stem().string(), std::move(textureResource)};
                      })
                    | kdl::or_else([&](auto e) {
                        return Result<Assets::Material, ReadMaterialError>{
                          ReadMaterialError{path.stem().string(), std::move(e.msg)}};
                      });
           })
         | kdl::transform_error([&](auto e) -> Assets::Material {
             logger.error() << "Could not load skin '" << path << "': " << e.msg;
             return loadDefaultMaterial(fs, path.stem().string(), logger);
           })
         | kdl::value();
}

} // namespace IO
} // namespace TrenchBroom
