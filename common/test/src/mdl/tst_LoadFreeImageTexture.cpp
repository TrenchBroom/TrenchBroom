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

#include "TestUtils.h"
#include "fs/DiskFileSystem.h"
#include "gl/Texture.h"
#include "mdl/LoadFreeImageTexture.h"

#include "kd/result.h"

#include <filesystem>
#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

auto loadTexture(const std::string& name)
{
  auto diskFS = fs::DiskFileSystem{
    std::filesystem::current_path() / "fixture" / "test" / "io" / "Image"};

  return diskFS.openFile(name) | kdl::and_then([](const auto& file) {
           auto reader = file->reader().buffer();
           return loadFreeImageTexture(reader);
         });
}

void assertTexture(const std::string& name, const size_t width, const size_t height)
{
  loadTexture(name) | kdl::transform([&](const auto& texture) {
    CHECK(texture.width() == width);
    CHECK(texture.height() == height);
    CHECK((texture.format() == GL_BGRA || texture.format() == GL_RGBA));
    CHECK(texture.mask() == gl::TextureMask::Off);
  }) | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2474
void testImageContents(Result<gl::Texture> result, const ColorMatch match)
{
  result | kdl::transform([&](const auto& texture) {
    const std::size_t w = 64u;
    const std::size_t h = 64u;

    CHECK(texture.width() == w);
    CHECK(texture.height() == h);
    CHECK(texture.buffersIfLoaded().size() == 1u);
    CHECK((texture.format() == GL_BGRA || texture.format() == GL_RGBA));
    CHECK(texture.mask() == gl::TextureMask::Off);

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
  }) | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
}

} // namespace

TEST_CASE("loadFreeImageTexture")
{
  SECTION("loading PNGs")
  {
    assertTexture("5x5.png", 5, 5);
    assertTexture("707x710.png", 707, 710);
    testImageContents(loadTexture("pngContentsTest.png"), ColorMatch::Exact);
    CHECK(loadTexture("corruptPngTest.png").is_error());

    // we don't support this format currently
    CHECK(loadTexture("16bitGrayscale.png").is_error());
  }

  SECTION("loading JPGs")
  {
    testImageContents(loadTexture("jpgContentsTest.jpg"), ColorMatch::Approximate);
  }

  SECTION("alpha mask")
  {
    const auto texture = loadTexture("alphaMaskTest.png") | kdl::value();
    const std::size_t w = 25u;
    const std::size_t h = 10u;

    CHECK(texture.width() == w);
    CHECK(texture.height() == h);
    CHECK(texture.buffersIfLoaded().size() == 1u);
    CHECK((texture.format() == GL_BGRA || texture.format() == GL_RGBA));
    CHECK(texture.mask() == gl::TextureMask::On);

    auto& mip0Data = texture.buffersIfLoaded().at(0);
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
}

TEST_CASE("isSupportedFreeImageExtension")
{
  CHECK(isSupportedFreeImageExtension(".jpg"));
  CHECK(isSupportedFreeImageExtension(".jpeg"));
  CHECK(isSupportedFreeImageExtension(".JPG"));
  CHECK_FALSE(isSupportedFreeImageExtension("jpg"));
}

} // namespace tb::mdl
