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

#include "TestUtils.h"
#include "io/DiskFileSystem.h"
#include "io/ReadDdsTexture.h"
#include "mdl/Palette.h"
#include "mdl/Texture.h"

#include "kdl/result.h"

#include <filesystem>
#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{
namespace
{

mdl::Texture loadTexture(const std::string& name)
{
  const auto ddsPath = std::filesystem::current_path() / "fixture/test/io/Dds/";
  auto diskFS = DiskFileSystem{ddsPath};

  const auto file = diskFS.openFile(name) | kdl::value();
  auto reader = file->reader().buffer();
  return readDdsTexture(reader) | kdl::value();
}

void assertTexture(
  const std::string& name, const size_t width, const size_t height, const GLenum format)
{
  const auto texture = loadTexture(name);

  CHECK(texture.width() == width);
  CHECK(texture.height() == height);
  CHECK(texture.format() == format);
  CHECK(texture.mask() == mdl::TextureMask::Off);
}

} // namespace

TEST_CASE("ReadDdsTextureTest.testLoadDds")
{
  assertTexture("dds_rgb.dds", 128, 128, GL_BGR);
  assertTexture("dds_rgba.dds", 128, 128, GL_BGRA);
  assertTexture("dds_bc1.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
  assertTexture("dds_bc2.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
  assertTexture("dds_bc3.dds", 128, 128, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}

} // namespace tb::io
