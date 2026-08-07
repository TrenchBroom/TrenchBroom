/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/CatchConfig.h"
#include "mdl/ImageLoader.h"

#include <array>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

// clang-format off
// A minimal uncompressed 24bpp BMP, 4x2 pixels, with a distinct color per pixel:
//   top row:    black, yellow, cyan,    magenta
//   bottom row: red,   green,  blue,    white
// BMP pixel rows are stored bottom-up, and each pixel as BGR.
constexpr auto Bmp4x2 = std::array<unsigned char, 78>{
  // BITMAPFILEHEADER
  'B', 'M',                   // bfType
  78, 0, 0, 0,                // bfSize
  0, 0,                       // bfReserved1
  0, 0,                       // bfReserved2
  54, 0, 0, 0,                // bfOffBits
  // BITMAPINFOHEADER
  40, 0, 0, 0,                // biSize
  4, 0, 0, 0,                 // biWidth
  2, 0, 0, 0,                 // biHeight
  1, 0,                       // biPlanes
  24, 0,                      // biBitCount
  0, 0, 0, 0,                 // biCompression
  24, 0, 0, 0,                // biSizeImage
  0, 0, 0, 0,                 // biXPelsPerMeter
  0, 0, 0, 0,                 // biYPelsPerMeter
  0, 0, 0, 0,                 // biClrUsed
  0, 0, 0, 0,                 // biClrImportant
  // pixel data, bottom row first, each pixel as BGR
  0, 0, 255,    0, 255, 0,    255, 0, 0,    255, 255, 255,
  0, 0, 0,      0, 255, 255,  255, 255, 0,  255, 0, 255,
};
// clang-format on

// clang-format off
// A minimal uncompressed 8bpp indexed BMP, 4x2 pixels, with a 4-color palette
// (red, green, blue, white). Pixel rows are stored bottom-up, one palette index
// per pixel:
//   top row:    white, blue,  green, red
//   bottom row: red,   green, blue,  white
constexpr auto IndexedBmp4x2 = std::array<unsigned char, 78>{
  // BITMAPFILEHEADER
  'B', 'M',                   // bfType
  78, 0, 0, 0,                // bfSize
  0, 0,                       // bfReserved1
  0, 0,                       // bfReserved2
  70, 0, 0, 0,                // bfOffBits
  // BITMAPINFOHEADER
  40, 0, 0, 0,                // biSize
  4, 0, 0, 0,                 // biWidth
  2, 0, 0, 0,                 // biHeight
  1, 0,                       // biPlanes
  8, 0,                       // biBitCount
  0, 0, 0, 0,                 // biCompression
  8, 0, 0, 0,                 // biSizeImage
  0, 0, 0, 0,                 // biXPelsPerMeter
  0, 0, 0, 0,                 // biYPelsPerMeter
  4, 0, 0, 0,                 // biClrUsed
  0, 0, 0, 0,                 // biClrImportant
  // color table, each entry as BGR0: red, green, blue, white
  0, 0, 255, 0,   0, 255, 0, 0,   255, 0, 0, 0,   255, 255, 255, 0,
  // pixel data, bottom row first, each pixel a palette index
  0, 1, 2, 3,
  3, 2, 1, 0,
};
// clang-format on

} // namespace

TEST_CASE("ImageLoader")
{
  SECTION("loadPixels")
  {
    const auto* begin = reinterpret_cast<const char*>(Bmp4x2.data());
    const auto* end = begin + Bmp4x2.size();
    auto loader = ImageLoader{ImageLoader::BMP, begin, end};

    REQUIRE(loader.width() == 4u);
    REQUIRE(loader.height() == 2u);
    REQUIRE(loader.hasPixels());
    REQUIRE_FALSE(loader.hasIndices());

    // clang-format off
    const auto expected = std::vector<unsigned char>{
      // row 0 (top of the image)
      0, 0, 0,      255, 255, 0,  0, 255, 255,  255, 0, 255,
      // row 1 (bottom of the image)
      255, 0, 0,    0, 255, 0,    0, 0, 255,     255, 255, 255,
    };
    // clang-format on

    CHECK(loader.loadPixels(ImageLoader::RGB) == expected);
  }

  SECTION("indexed images")
  {
    const auto* begin = reinterpret_cast<const char*>(IndexedBmp4x2.data());
    const auto* end = begin + IndexedBmp4x2.size();
    auto loader = ImageLoader{ImageLoader::BMP, begin, end};

    REQUIRE(loader.width() == 4u);
    REQUIRE(loader.height() == 2u);
    REQUIRE(loader.hasPalette());
    REQUIRE(loader.hasIndices());

    SECTION("loadPalette")
    {
      // an 8bpp bitmap always reports a 256-entry palette, regardless of biClrUsed;
      // entries beyond the 4 the fixture actually populated are filled by FreeImage
      // with an unspecified fallback (observed to be a grayscale ramp), so only the
      // entries the fixture controls are checked here
      const auto palette = loader.loadPalette();
      REQUIRE(palette.size() == 256u * 3u);

      const auto actualPrefix =
        std::vector<unsigned char>(palette.begin(), palette.begin() + 12);
      const auto expectedPrefix = std::vector<unsigned char>{
        255,
        0,
        0, // red
        0,
        255,
        0, // green
        0,
        0,
        255, // blue
        255,
        255,
        255, // white
      };

      CHECK(actualPrefix == expectedPrefix);
    }

    SECTION("loadIndices")
    {
      // clang-format off
      const auto expected = std::vector<unsigned char>{
        3, 2, 1, 0, // row 0 (top of the image)
        0, 1, 2, 3, // row 1 (bottom of the image)
      };
      // clang-format on

      CHECK(loader.loadIndices() == expected);
    }

    SECTION("loadPixels")
    {
      // clang-format off
      const auto expected = std::vector<unsigned char>{
        // row 0 (top of the image)
        255, 255, 255,   0, 0, 255,   0, 255, 0,   255, 0, 0,
        // row 1 (bottom of the image)
        255, 0, 0,       0, 255, 0,   0, 0, 255,   255, 255, 255,
      };
      // clang-format on

      CHECK(loader.loadPixels(ImageLoader::RGB) == expected);
    }
  }
}

} // namespace tb::mdl
