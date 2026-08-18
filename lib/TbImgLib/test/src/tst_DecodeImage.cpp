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

#include "TestEnvironment.h"
#include "img/DecodeImage.h"
#include "img/Image.h"

#include "kd/result.h"

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::img
{
namespace
{

std::vector<unsigned char> readFile(const std::filesystem::path& path)
{
  auto stream = std::ifstream{path, std::ios::binary | std::ios::ate};
  const auto size = static_cast<size_t>(stream.tellg());
  stream.seekg(0);

  auto result = std::vector<unsigned char>(size);
  stream.read(reinterpret_cast<char*>(result.data()), static_cast<std::streamsize>(size));
  return result;
}

Result<Image> decodeFixture(const std::string& name)
{
  const auto data = readFile(getFixtureRoot() / "test" / "img" / "DecodeImage" / name);
  return decodeImage(data.data(), data.size());
}

std::array<unsigned char, 4> pixelAt(const Image& image, const size_t x, const size_t y)
{
  const auto i = (y * image.width + x) * 4;
  return {image.pixels[i], image.pixels[i + 1], image.pixels[i + 2], image.pixels[i + 3]};
}

} // namespace

TEST_CASE("decodeImage")
{
  SECTION("loading PNGs")
  {
    const auto small = decodeFixture("5x5.png") | kdl::value();
    CHECK(small.width == 5u);
    CHECK(small.height == 5u);
    CHECK(small.pixels.size() == 5u * 5u * 4u);

    const auto large = decodeFixture("707x710.png") | kdl::value();
    CHECK(large.width == 707u);
    CHECK(large.height == 710u);
    CHECK(large.pixels.size() == 707u * 710u * 4u);

    CHECK(decodeFixture("corruptPngTest.png").is_error());

    // we don't support this format currently
    CHECK(decodeFixture("16bitGrayscale.png").is_error());
  }

  SECTION("loading JPGs")
  {
    const auto image = decodeFixture("jpgContentsTest.jpg") | kdl::value();
    CHECK(image.width == 64u);
    CHECK(image.height == 64u);
    CHECK(image.pixels.size() == 64u * 64u * 4u);
  }

  // https://github.com/TrenchBroom/TrenchBroom/issues/2474
  SECTION("pixel contents")
  {
    const auto image = decodeFixture("pngContentsTest.png") | kdl::value();

    // top left pixel is red
    CHECK(pixelAt(image, 0, 0) == std::array<unsigned char, 4>{255, 0, 0, 255});
    // bottom right pixel is green
    CHECK(
      pixelAt(image, image.width - 1, image.height - 1)
      == std::array<unsigned char, 4>{0, 255, 0, 255});
    // others are 161, 161, 161
    CHECK(pixelAt(image, 1, 1) == std::array<unsigned char, 4>{161, 161, 161, 255});

    CHECK(image.alphaDomain == ImageAlphaDomain::Opaque);
  }

  SECTION("transparency")
  {
    const auto image = decodeFixture("alphaMaskTest.png") | kdl::value();
    CHECK(image.width == 25u);
    CHECK(image.height == 10u);
    CHECK(image.alphaDomain == ImageAlphaDomain::Binary);

    // top left pixel is green, opaque
    CHECK(pixelAt(image, 0, 0) == std::array<unsigned char, 4>{0, 255, 0, 255});
    // other pixels are fully transparent (RGB values are unknown)
    CHECK(pixelAt(image, 1, 0)[3] == 0);
  }

  SECTION("intermediate alpha")
  {
    const auto binary = decodeFixture("alphaMaskTest.png") | kdl::value();
    CHECK(binary.alphaDomain == ImageAlphaDomain::Binary);

    const auto opaque = decodeFixture("pngContentsTest.png") | kdl::value();
    CHECK(opaque.alphaDomain == ImageAlphaDomain::Opaque);

    const auto graduated = decodeFixture("gradientAlphaTest.png") | kdl::value();
    CHECK(graduated.alphaDomain == ImageAlphaDomain::Graduated);
  }
}

} // namespace tb::img
