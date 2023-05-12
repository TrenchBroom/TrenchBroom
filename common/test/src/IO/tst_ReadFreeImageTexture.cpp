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

#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/ReadFreeImageTexture.h"
#include "TestLogger.h"
#include "TestUtils.h"

#include <kdl/result.h>

#include <filesystem>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

static auto loadTexture(const std::string& name)
{
  auto diskFS =
    DiskFileSystem{std::filesystem::current_path() / "fixture/test/IO/Image/"};

  const auto file = diskFS.openFile(name);
  auto reader = file->reader().buffer();
  return readFreeImageTexture(name, reader);
}

static void assertTexture(
  const std::string& name, const size_t width, const size_t height)
{
  loadTexture(name)
    .transform([&](const auto& texture) {
      CHECK(texture.name() == name);
      CHECK(texture.width() == width);
      CHECK(texture.height() == height);
      CHECK((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
      CHECK(texture.type() == Assets::TextureType::Opaque);
    })
    .transform_error([](const auto&) { FAIL(); });
}

TEST_CASE("ReadFreeImageTextureTest.testLoadPngs")
{
  assertTexture("5x5.png", 5, 5);
  assertTexture("707x710.png", 707, 710);
}

TEST_CASE("ReadFreeImageTextureTest.testLoadCorruptPng")
{
  CHECK(loadTexture("corruptPngTest.png").is_error());
}

TEST_CASE("ReadFreeImageTextureTest.testLoad16BitPng")
{
  // we don't support this format currently
  CHECK(loadTexture("16bitGrayscale.png").is_error());
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2474
static void testImageContents(const Assets::Texture& texture, const ColorMatch match)
{
  const std::size_t w = 64u;
  const std::size_t h = 64u;

  CHECK(texture.width() == w);
  CHECK(texture.height() == h);
  CHECK(texture.buffersIfUnprepared().size() == 1u);
  CHECK((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
  CHECK(texture.type() == Assets::TextureType::Opaque);

  for (std::size_t y = 0; y < h; ++y)
  {
    for (std::size_t x = 0; x < w; ++x)
    {
      if (x == 0 && y == 0)
      {
        // top left pixel is red
        checkColor(texture, x, y, 255, 0, 0, 255, match);
      }
      else if (x == (w - 1) && y == (h - 1))
      {
        // bottom right pixel is green
        checkColor(texture, x, y, 0, 255, 0, 255, match);
      }
      else
      {
        // others are 161, 161, 161
        checkColor(texture, x, y, 161, 161, 161, 255, match);
      }
    }
  }
}

TEST_CASE("ReadFreeImageTextureTest.testPNGContents")
{
  testImageContents(loadTexture("pngContentsTest.png").value(), ColorMatch::Exact);
}

TEST_CASE("ReadFreeImageTextureTest.testJPGContents")
{
  testImageContents(loadTexture("jpgContentsTest.jpg").value(), ColorMatch::Approximate);
}

TEST_CASE("ReadFreeImageTextureTest.alphaMaskTest")
{
  const auto texture = loadTexture("alphaMaskTest.png").value();
  const std::size_t w = 25u;
  const std::size_t h = 10u;

  CHECK(texture.width() == w);
  CHECK(texture.height() == h);
  CHECK(texture.buffersIfUnprepared().size() == 1u);
  CHECK((GL_BGRA == texture.format() || GL_RGBA == texture.format()));
  CHECK(texture.type() == Assets::TextureType::Masked);

  auto& mip0Data = texture.buffersIfUnprepared().at(0);
  CHECK(mip0Data.size() == w * h * 4);

  for (std::size_t y = 0; y < h; ++y)
  {
    for (std::size_t x = 0; x < w; ++x)
    {
      if (x == 0 && y == 0)
      {
        // top left pixel is green opaque
        CHECK(getComponentOfPixel(texture, x, y, Component::R) == 0 /* R */);
        CHECK(getComponentOfPixel(texture, x, y, Component::G) == 255 /* G */);
        CHECK(getComponentOfPixel(texture, x, y, Component::B) == 0 /* B */);
        CHECK(getComponentOfPixel(texture, x, y, Component::A) == 255 /* A */);
      }
      else
      {
        // others are fully transparent (RGB values are unknown)
        CHECK(getComponentOfPixel(texture, x, y, Component::A) == 0 /* A */);
      }
    }
  }
}

} // namespace TrenchBroom::IO
