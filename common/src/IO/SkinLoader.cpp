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

#include "SkinLoader.h"

#include "IO/FileSystem.h"
#include "IO/MaterialUtils.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ReadWalTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "Result.h"
#include "assets/Material.h"
#include "assets/Palette.h"

#include "kdl/string_format.h"

namespace tb::IO
{

assets::Material loadSkin(
  const std::filesystem::path& path, const FileSystem& fs, Logger& logger)
{
  return loadSkin(path, fs, std::nullopt, logger);
}

assets::Material loadSkin(
  const std::filesystem::path& path,
  const FileSystem& fs,
  const std::optional<assets::Palette>& palette,
  Logger& logger)
{
  return fs.openFile(path)
         | kdl::and_then([&](auto file) -> Result<assets::Material, ReadMaterialError> {
             const auto extension = kdl::str_to_lower(path.extension().string());
             auto reader = file->reader().buffer();
             return (extension == ".wal" ? readWalTexture(reader, palette)
                                         : readFreeImageTexture(reader))
                    | kdl::transform([&](auto texture) {
                        auto textureResource = createTextureResource(std::move(texture));
                        return assets::Material{
                          path.stem().string(), std::move(textureResource)};
                      })
                    | kdl::or_else([&](auto e) {
                        return Result<assets::Material, ReadMaterialError>{
                          ReadMaterialError{path.stem().string(), std::move(e.msg)}};
                      });
           })
         | kdl::transform_error([&](auto e) -> assets::Material {
             logger.error() << "Could not load skin '" << path << "': " << e.msg;
             return loadDefaultMaterial(fs, path.stem().string(), logger);
           })
         | kdl::value();
}

} // namespace tb::IO
