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

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Ensure.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "IO/PathInfo.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/ReadQuake3ShaderTexture.h"
#include "IO/ReadWalTexture.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"

#include <kdl/result.h>
#include <kdl/string_format.h>

#include <string>

namespace TrenchBroom
{
namespace IO
{

Assets::Texture loadSkin(const Path& path, const FileSystem& fs, Logger& logger)
{
  return loadSkin(path, fs, std::nullopt, logger);
}

Assets::Texture loadSkin(
  const Path& path,
  const FileSystem& fs,
  const std::optional<Assets::Palette>& palette,
  Logger& logger)
{
  try
  {
    const auto file = fs.openFile(path);
    const auto extension = kdl::str_to_lower(path.extension().asString());

    auto reader = file->reader().buffer();
    return (extension == ".wal" ? readWalTexture(path.stem().asString(), reader, palette)
                                : readFreeImageTexture(path.stem().asString(), reader))
      .or_else(makeReadTextureErrorHandler(fs, logger))
      .value();
  }
  catch (Exception& e)
  {
    logger.error() << "Could not load skin '" << path << "': " << e.what();
    return loadDefaultTexture(fs, path.stem().asString(), logger);
  }
}

Assets::Texture loadShader(const Path& path, const FileSystem& fs, Logger& logger)
{
  auto actualPath = !path.empty() && fs.pathInfo(path.deleteExtension()) == PathInfo::File
                      ? path.deleteExtension()
                      : path;
  const auto name = path.asGenericString();

  if (!path.empty())
  {
    logger.debug() << "Loading shader '" << path << "'";
    try
    {
      const auto file = fs.openFile(actualPath);
      return readQuake3ShaderTexture(name, *file, fs)
        .if_error([](const auto& e) { throw AssetException{e.msg.c_str()}; })
        .value();
    }
    catch (const Exception& e)
    {
      logger.error() << "Could not load shader '" << path << "': " << e.what();
      // fall through to return the default texture
    }
  }
  else
  {
    logger.warn() << "Could not load shader: Path is empty";
  }

  return loadDefaultTexture(fs, name, logger);
}
} // namespace IO
} // namespace TrenchBroom
