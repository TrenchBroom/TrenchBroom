/*
 Copyright (C) 2023 iOrange
 Copyright (C) 2023 Kristian Duske

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

#include "TestLogger.h"
#include "TestUtils.h"

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/DdsTextureReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"

#include <memory>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
static Assets::Texture loadTexture(const std::string& name)
{
  const auto ddsPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Dds/");
  DiskFileSystem diskFS(ddsPath);

  TextureReader::TextureNameStrategy nameStrategy;
  NullLogger logger;
  DdsTextureReader textureLoader(nameStrategy, diskFS, logger);

  return textureLoader.readTexture(diskFS.openFile(Path(name)));
}

static void assertTexture(
  const std::string& name, const size_t width, const size_t height, const GLenum format)
{
  const auto texture = loadTexture(name);

  CHECK(texture.name() == name);
  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
  CHECK(texture.format() == format);
  CHECK(texture.type() == Assets::TextureType::Opaque);
}

TEST_CASE("DdsTextureReaderTest.testLoadDds")
{
  assertTexture("dds_rgb.dds", 128, 128, GL_BGR);
  assertTexture("dds_rgba.dds", 128, 128, GL_BGRA);
  assertTexture("dds_bc1.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
  assertTexture("dds_bc2.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
  assertTexture("dds_bc3.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}
} // namespace IO
} // namespace TrenchBroom
