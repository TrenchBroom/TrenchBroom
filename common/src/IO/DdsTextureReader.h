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

#pragma once

#include "Color.h"
#include "IO/TextureReader.h"
#include "Assets/TextureBuffer.h"
#include "Renderer/GL.h"

#include <cstdint>
#include <memory>

namespace TrenchBroom
{
class Logger;

namespace Assets
{
class TextureBuffer;
}

namespace IO
{
class File;
class FileSystem;
class Reader;

class DdsTextureReader : public TextureReader
{
public:
  static Color getAverageColor(const Assets::TextureBuffer& buffer, GLenum format);

  explicit DdsTextureReader(
    const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger);

private:
  Assets::Texture doReadTexture(std::shared_ptr<File> file) const override;
  static void readDdsMips(
    const size_t width,
    const size_t height,
    Reader& reader,
    Assets::TextureBufferList& buffers);
};
} // namespace IO
} // namespace TrenchBroom
