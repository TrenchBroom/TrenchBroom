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
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/PathInfo.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "IO/ResourceUtils.h"
#include "IO/WalTextureReader.h"
#include "Logger.h"

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
  const auto nameStrategy = TextureReader::StaticNameStrategy{path.basename()};

  try
  {
    const auto file = fs.openFile(path);
    const auto extension = kdl::str_to_lower(path.extension());

    if (extension == "wal")
    {
      auto reader = WalTextureReader{nameStrategy, fs, palette, logger};
      return reader.readTexture(file);
    }
    else
    {
      auto reader = FreeImageTextureReader{nameStrategy, fs, logger};
      return reader.readTexture(file);
    }
  }
  catch (Exception& e)
  {
    logger.error() << "Could not load skin '" << path << "': " << e.what();
    return loadDefaultTexture(fs, nameStrategy.textureName("", path), logger);
  }
}

Assets::Texture loadShader(const Path& path, const FileSystem& fs, Logger& logger)
{
  const auto nameStrategy = TextureReader::PathSuffixNameStrategy{0u};

  if (!path.isEmpty())
  {
    logger.debug() << "Loading shader '" << path << "'";
    try
    {
      const auto file = fs.pathInfo(path.deleteExtension()) == PathInfo::File
                          ? fs.openFile(path.deleteExtension())
                          : fs.openFile(path);

      auto reader = Quake3ShaderTextureReader{nameStrategy, fs, logger};
      return reader.readTexture(file);
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

  const auto name = nameStrategy.textureName("", path);
  return loadDefaultTexture(fs, name, logger);
}
} // namespace IO
} // namespace TrenchBroom
