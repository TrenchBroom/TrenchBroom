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
#include "gl/PrimType.h"
#include "gl/ShaderProgram.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "gl/VertexArray.h"
#include "gl/VertexType.h"

#include "vm/vec.h"

#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("VertexArray")
{
  SECTION("a default-constructed array is empty")
  {
    const auto array = VertexArray{};
    CHECK(array.empty());
    CHECK(array.sizeInBytes() == 0u);
    CHECK(array.vertexCount() == 0u);
    CHECK(!array.prepared());
  }

  SECTION("copy construction copies the given vertices")
  {
    auto vertices = std::vector<VertexTypes::P3::Vertex>{
      VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{1, 0, 0}},
    };
    const auto array = VertexArray::copy(vertices);

    CHECK(!array.empty());
    CHECK(array.vertexCount() == 2u);
    CHECK(array.sizeInBytes() == 2u * sizeof(VertexTypes::P3::Vertex));

    // the source vector is left unchanged by copy()
    CHECK(vertices.size() == 2u);
  }

  SECTION("move construction moves the given vertices")
  {
    auto vertices = std::vector<VertexTypes::P3::Vertex>{
      VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
    };
    const auto array = VertexArray::move(std::move(vertices));

    CHECK(array.vertexCount() == 1u);
  }

  SECTION("ref construction references the given vertices")
  {
    const auto vertices = std::vector<VertexTypes::P3::Vertex>{
      VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{1, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{0, 1, 0}},
    };
    const auto array = VertexArray::ref(vertices);

    CHECK(array.vertexCount() == 3u);
    CHECK(array.sizeInBytes() == 3u * sizeof(VertexTypes::P3::Vertex));
  }

  SECTION("prepare")
  {
    auto gl = MockGl{};
    installVboSupport(gl);
    auto vboManager = VboManager{};

    SECTION("on an empty array is a no-op")
    {
      auto array = VertexArray{};

      array.prepare(gl, vboManager);
      CHECK(array.prepared());
      CHECK(vboManager.currentVboCount() == 0u);
    }

    SECTION("uploads a non-empty array's vertices exactly once")
    {
      auto vertices = std::vector<VertexTypes::P3::Vertex>{
        VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
        VertexTypes::P3::Vertex{vm::vec3f{1, 0, 0}},
      };
      auto array = VertexArray::copy(vertices);

      CHECK(!array.prepared());
      array.prepare(gl, vboManager);
      CHECK(array.prepared());
      CHECK(vboManager.currentVboCount() == 1u);

      // preparing an already-prepared array is a no-op
      array.prepare(gl, vboManager);
      CHECK(vboManager.currentVboCount() == 1u);

      // dropping the array queues its Vbo for destruction; free it before the
      // VboManager itself is destroyed, since the Vbo destructor requires that
      array = VertexArray{};
      vboManager.destroyPendingVbos(gl);
    }
  }

  SECTION("setup on an empty array returns false")
  {
    auto gl = MockGl{};
    auto shaderProgram = ShaderProgram{"test", 1u};
    auto array = VertexArray{};

    CHECK(!array.setup(gl, shaderProgram));
  }

  SECTION("setup, render and cleanup on a prepared, non-empty array")
  {
    auto gl = MockGl{};
    installVboSupport(gl);
    auto vboManager = VboManager{};
    auto shaderProgram = ShaderProgram{"test", 1u};

    auto vertices = std::vector<VertexTypes::P3::Vertex>{
      VertexTypes::P3::Vertex{vm::vec3f{0, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{1, 0, 0}},
      VertexTypes::P3::Vertex{vm::vec3f{0, 1, 0}},
    };
    auto array = VertexArray::copy(vertices);
    array.prepare(gl, vboManager);

    // every call the P3 attribute type and the render overloads below can make must
    // have a slot assigned before setup(), since an exception thrown partway
    // through would skip the Vbo cleanup at the end of this section and abort on
    // unwind instead (the Vbo destructor requires free() to have been called)
    gl.onVertexPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
    gl.onDrawArrays = [](GLenum, GLint, GLsizei) {};
    gl.onDrawElements = [](GLenum, GLsizei, GLenum, const void*) {};
    gl.onMultiDrawArrays = [](GLenum, const GLint*, const GLsizei*, GLsizei) {};

    auto boundBuffers = std::vector<GLuint>{};
    gl.onBindBuffer = [&](GLenum, const GLuint id) { boundBuffers.push_back(id); };

    auto vertexArrayEnabled = false;
    gl.onEnableClientState = [&](const GLenum cap) {
      if (cap == GL_VERTEX_ARRAY)
      {
        vertexArrayEnabled = true;
      }
    };

    REQUIRE(array.setup(gl, shaderProgram));
    CHECK(vertexArrayEnabled);
    REQUIRE(boundBuffers.size() == 1u);
    CHECK(boundBuffers[0] != 0u);

    SECTION("render(gl, primType) renders the whole array")
    {
      auto capturedFirst = GLint{-1};
      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](GLenum, const GLint first, const GLsizei count) {
        capturedFirst = first;
        capturedCount = count;
      };

      array.render(gl, PrimType::Triangles);
      CHECK(capturedFirst == 0);
      CHECK(capturedCount == 3);
    }

    SECTION("render(gl, primType, index, count) renders a sub-range")
    {
      auto capturedFirst = GLint{-1};
      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](GLenum, const GLint first, const GLsizei count) {
        capturedFirst = first;
        capturedCount = count;
      };

      array.render(gl, PrimType::Triangles, 1, 2);
      CHECK(capturedFirst == 1);
      CHECK(capturedCount == 2);
    }

    SECTION("render(gl, primType, indices, counts, primCount) issues a multi-draw call")
    {
      auto capturedPrimCount = GLsizei{-1};
      gl.onMultiDrawArrays =
        [&](GLenum, const GLint*, const GLsizei*, const GLsizei primCount) {
          capturedPrimCount = primCount;
        };

      const auto indices = Indices{0, 1};
      const auto counts = Counts{1, 2};
      array.render(gl, PrimType::Triangles, indices, counts, 2);
      CHECK(capturedPrimCount == 2);
    }

    SECTION("render(gl, primType, indices, count) issues an indexed draw call")
    {
      auto capturedCount = GLsizei{-1};
      auto capturedIndices = static_cast<const void*>(nullptr);
      gl.onDrawElements = [&](GLenum, const GLsizei count, GLenum, const void* indices) {
        capturedCount = count;
        capturedIndices = indices;
      };

      const auto indices = Indices{0, 2, 1};
      array.render(gl, PrimType::Triangles, indices, 3);
      CHECK(capturedCount == 3);
      CHECK(capturedIndices == indices.data());
    }

    auto vertexArrayDisabled = false;
    gl.onDisableClientState = [&](const GLenum cap) {
      if (cap == GL_VERTEX_ARRAY)
      {
        vertexArrayDisabled = true;
      }
    };

    array.cleanup(gl, shaderProgram);
    CHECK(vertexArrayDisabled);
    REQUIRE(boundBuffers.size() == 2u);
    CHECK(boundBuffers[1] == 0u);

    // dropping the array queues its Vbo for destruction; free it before the
    // VboManager itself is destroyed, since the Vbo destructor requires that
    array = VertexArray{};
    vboManager.destroyPendingVbos(gl);
  }
}

} // namespace tb::gl
