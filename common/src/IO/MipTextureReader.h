/*
 Copyright (C) 2010-2016 Kristian Duske

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

#pragma once

#include "IO/TextureReader.h"

#include <memory>
#include <string>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class Palette;
}

namespace TrenchBroom::IO
{
class BufferedReader;
class File;
class FileSystem;
class Reader;

class MipTextureReader : public TextureReader
{
protected:
  MipTextureReader(
    const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger);

public:
  ~MipTextureReader() override;

public:
  static size_t mipFileSize(size_t width, size_t height, size_t mipLevels);
  /**
   * Reads the texture name or returns an empty string in case of error.
   * Doesn't modify the provided reader.
   */
  static std::string getTextureName(const BufferedReader& reader);

protected:
  Assets::Texture doReadTexture(std::shared_ptr<File> file) const override;
  virtual Assets::Palette doGetPalette(
    Reader& reader, const size_t offset[], size_t width, size_t height) const = 0;
};

} // namespace TrenchBroom::IO
