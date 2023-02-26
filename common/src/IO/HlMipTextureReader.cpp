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

#include "HlMipTextureReader.h"

#include "Assets/Palette.h"
#include "IO/Reader.h"

#include <vector>

namespace TrenchBroom::IO
{

HlMipTextureReader::HlMipTextureReader(
  const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger)
  : MipTextureReader{nameStrategy, fs, logger}
{
}

Assets::Palette HlMipTextureReader::doGetPalette(
  Reader& reader, const size_t offset[], const size_t width, const size_t height) const
{
  // each texture has two bytes after the last mip and before the palette data starts
  const auto start = offset[0] + (width * height * 85 >> 6) + 2;
  reader.seekFromBegin(start);

  // each texture has two bytes of padding at the end
  auto data = std::vector<unsigned char>(reader.size() - start - 2);
  reader.read(data.data(), data.size());

  return Assets::Palette{data};
}

} // namespace TrenchBroom::IO
