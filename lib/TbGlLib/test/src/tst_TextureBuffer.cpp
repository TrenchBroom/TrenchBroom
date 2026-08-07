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

#include "gl/TextureBuffer.h"

#include "vm/vec.h"

#include <sstream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("TextureBuffer")
{
  SECTION("default constructor")
  {
    const auto buffer = TextureBuffer{};
    CHECK(buffer.size() == 0u);
  }

  SECTION("constructor with size")
  {
    auto buffer = TextureBuffer{16u};
    CHECK(buffer.size() == 16u);
    REQUIRE(buffer.data() != nullptr);

    // the buffer is writable
    buffer.data()[0] = 42;
    CHECK(buffer.data()[0] == 42);
    CHECK(std::as_const(buffer).data()[0] == 42);
  }
}

TEST_CASE("TextureBuffer utilities")
{
  SECTION("sizeAtMipLevel")
  {
    CHECK(sizeAtMipLevel(100, 50, 0) == vm::vec2s{100, 50});
    CHECK(sizeAtMipLevel(100, 50, 1) == vm::vec2s{50, 25});

    // each dimension is clamped to a minimum of 1, even once the other dimension has
    // further levels to go
    CHECK(sizeAtMipLevel(8, 2, 2) == vm::vec2s{2, 1});
    CHECK(sizeAtMipLevel(8, 2, 3) == vm::vec2s{1, 1});
  }

  SECTION("isCompressedFormat")
  {
    CHECK(isCompressedFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT));
    CHECK(isCompressedFormat(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT));
    CHECK(isCompressedFormat(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT));
    CHECK(!isCompressedFormat(GL_RGBA));
    CHECK(!isCompressedFormat(GL_RGB));
  }

  SECTION("blockSizeForFormat")
  {
    CHECK(blockSizeForFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) == 8u);
    CHECK(blockSizeForFormat(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) == 16u);
    CHECK(blockSizeForFormat(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) == 16u);
  }

  SECTION("bytesPerPixelForFormat")
  {
    CHECK(bytesPerPixelForFormat(GL_RGB) == 3u);
    CHECK(bytesPerPixelForFormat(GL_BGR) == 3u);
    CHECK(bytesPerPixelForFormat(GL_RGBA) == 4u);
    CHECK(bytesPerPixelForFormat(GL_BGRA) == 4u);
  }

  SECTION("setMipBufferSize")
  {
    SECTION("uncompressed format")
    {
      auto buffers = TextureBufferList{};
      setMipBufferSize(buffers, 3, 8, 8, GL_RGBA);

      REQUIRE(buffers.size() == 3u);
      CHECK(buffers[0].size() == 4u * 8u * 8u);
      CHECK(buffers[1].size() == 4u * 4u * 4u);
      CHECK(buffers[2].size() == 4u * 2u * 2u);
    }

    SECTION("compressed format")
    {
      auto buffers = TextureBufferList{};
      setMipBufferSize(buffers, 1, 8, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);

      REQUIRE(buffers.size() == 1u);
      // block size 8, with a 8x8 mip covering 2x2 4x4-pixel blocks
      CHECK(buffers[0].size() == 8u * 2u * 2u);
    }
  }
}

} // namespace tb::gl
