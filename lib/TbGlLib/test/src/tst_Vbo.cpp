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

#include "gl/MockGl.h"
#include "gl/TestUtils.h"
#include "gl/Vbo.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("Vbo")
{
  auto gl = MockGl{};
  installVboSupport(gl);

  SECTION("constructor generates and binds a buffer of the given capacity")
  {
    auto boundType = GLenum{0};
    auto boundId = GLuint{0};
    gl.onBindBuffer = [&](const GLenum type, const GLuint id) {
      boundType = type;
      boundId = id;
    };

    auto vbo = Vbo{gl, GL_ARRAY_BUFFER, 128u, GL_STATIC_DRAW};

    CHECK(vbo.capacity() == 128u);
    CHECK(vbo.offset() == 0u);
    CHECK(boundType == GL_ARRAY_BUFFER);
    CHECK(boundId != 0u);

    vbo.free(gl);
  }

  SECTION("bind and unbind")
  {
    auto vbo = Vbo{gl, GL_ARRAY_BUFFER, 128u, GL_STATIC_DRAW};

    auto calls = std::vector<GLuint>{};
    gl.onBindBuffer = [&](GLenum, const GLuint id) { calls.push_back(id); };

    vbo.bind(gl);
    REQUIRE(calls.size() == 1u);
    CHECK(calls[0] != 0u);

    vbo.unbind(gl);
    REQUIRE(calls.size() == 2u);
    CHECK(calls[1] == 0u);

    vbo.free(gl);
  }

  SECTION("writeBuffer writes at the given address and returns the number of bytes")
  {
    auto vbo = Vbo{gl, GL_ARRAY_BUFFER, 128u, GL_STATIC_DRAW};

    auto capturedOffset = GLintptr{-1};
    auto capturedSize = GLsizeiptr{-1};
    gl.onBufferSubData =
      [&](GLenum, const GLintptr offset, const GLsizeiptr size, const void*) {
        capturedOffset = offset;
        capturedSize = size;
      };

    const auto data = std::vector<int>{1, 2, 3, 4};
    const auto written = vbo.writeBuffer(gl, 16u, data);

    CHECK(written == data.size() * sizeof(int));
    CHECK(capturedOffset == 16);
    CHECK(capturedSize == GLsizeiptr(data.size() * sizeof(int)));

    vbo.free(gl);
  }

  SECTION("free deletes the buffer, allowing the Vbo to be destroyed afterwards")
  {
    auto vbo = Vbo{gl, GL_ARRAY_BUFFER, 128u, GL_STATIC_DRAW};

    auto deletedId = GLuint{0};
    gl.onDeleteBuffers = [&](const GLsizei n, const GLuint* buffers) {
      REQUIRE(n == 1);
      deletedId = buffers[0];
    };

    vbo.free(gl);
    CHECK(deletedId != 0u);

    // the destructor requires free() to have been called first; reaching the end of
    // this scope without a contract violation is the actual assertion here
  }
}

} // namespace tb::gl
