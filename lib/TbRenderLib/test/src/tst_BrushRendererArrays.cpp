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
#include "render/BrushRendererArrays.h"

#include "vm/vec.h"

#include <cstdint>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("DirtyRangeTracker")
{
  SECTION("constructor")
  {
    SECTION("with a capacity produces a clean tracker with that capacity")
    {
      const auto tracker = DirtyRangeTracker{100};
      CHECK(tracker.capacity() == 100u);
      CHECK(tracker.clean());
    }

    SECTION("default produces a clean, zero-capacity tracker")
    {
      const auto tracker = DirtyRangeTracker{};
      CHECK(tracker.capacity() == 0u);
      CHECK(tracker.clean());
    }
  }

  SECTION("markDirty")
  {
    SECTION("makes the tracker unclean")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(10, 5);
      CHECK(!tracker.clean());
    }

    SECTION("records exactly the given range on a clean tracker")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(10, 5);
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 10u);
      CHECK(tracker.m_dirtyRange->size == 5u);
    }

    SECTION("grows the dirty range to cover a later, disjoint range")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(10, 5); // [10, 15)
      tracker.markDirty(50, 5); // [50, 55)
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 10u);
      CHECK(tracker.m_dirtyRange->size == 45u); // union spans [10, 55)
    }

    SECTION("grows the dirty range to cover an earlier, disjoint range")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(50, 5); // [50, 55)
      tracker.markDirty(10, 5); // [10, 15)
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 10u);
      CHECK(tracker.m_dirtyRange->size == 45u); // union spans [10, 55)
    }

    SECTION("with a zero-length range on a clean tracker stays clean")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(10, 0);
      CHECK(tracker.clean());
    }

    SECTION("with a zero-length range does not widen an existing dirty range")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.markDirty(10, 5); // [10, 15)
      tracker.markDirty(50, 0); // touches nothing
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 10u);
      CHECK(tracker.m_dirtyRange->size == 5u);

      tracker.markDirty(0, 0); // touches nothing, even though 0 < the current start
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 10u);
      CHECK(tracker.m_dirtyRange->size == 5u);
    }

    SECTION("out of bounds throws")
    {
      auto tracker = DirtyRangeTracker{100};
      CHECK_THROWS_AS(tracker.markDirty(95, 10), std::invalid_argument);
    }
  }

  SECTION("expand")
  {
    SECTION("marks the newly added range as dirty")
    {
      auto tracker = DirtyRangeTracker{100};
      tracker.expand(150);
      CHECK(tracker.capacity() == 150u);
      REQUIRE(tracker.m_dirtyRange.has_value());
      CHECK(tracker.m_dirtyRange->pos == 100u);
      CHECK(tracker.m_dirtyRange->size == 50u);
    }

    SECTION("to a capacity that is not greater throws")
    {
      auto tracker = DirtyRangeTracker{100};
      CHECK_THROWS_AS(tracker.expand(100), std::invalid_argument);
      CHECK_THROWS_AS(tracker.expand(50), std::invalid_argument);
    }
  }
}

TEST_CASE("BrushIndexArray")
{
  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);
  auto vboManager = gl::VboManager{};

  SECTION("constructor")
  {
    auto array = BrushIndexArray{};
    CHECK(!array.hasValidIndices());
    CHECK(array.prepared());
  }

  SECTION("getPointerToInsertElementsAt")
  {
    SECTION("writes indices and marks them valid")
    {
      auto array = BrushIndexArray{};
      const auto [block, dest] = array.getPointerToInsertElementsAt(3);
      REQUIRE(block != nullptr);
      dest[0] = 7;
      dest[1] = 8;
      dest[2] = 9;

      CHECK(array.hasValidIndices());
      CHECK(!array.prepared());
    }

    SECTION("grows the underlying storage on demand")
    {
      auto array = BrushIndexArray{};

      // exhaust the initial (zero) capacity, forcing an internal expand()
      const auto [block1, dest1] = array.getPointerToInsertElementsAt(4);
      REQUIRE(block1 != nullptr);
      for (auto i = 0u; i < 4; ++i)
      {
        dest1[i] = i;
      }

      // request more than remains, forcing a second expand()
      const auto [block2, dest2] = array.getPointerToInsertElementsAt(100);
      REQUIRE(block2 != nullptr);
      for (auto i = 0u; i < 100; ++i)
      {
        dest2[i] = i;
      }

      CHECK(array.hasValidIndices());
    }
  }

  SECTION("zeroElementsWithKey frees the allocation and zeroes the indices")
  {
    auto array = BrushIndexArray{};
    const auto [block, dest] = array.getPointerToInsertElementsAt(3);
    dest[0] = 7;
    dest[1] = 8;
    dest[2] = 9;

    array.zeroElementsWithKey(block);
    CHECK(!array.hasValidIndices());
  }

  SECTION("prepare uploads the array")
  {
    auto array = BrushIndexArray{};
    const auto [block, dest] = array.getPointerToInsertElementsAt(3);
    dest[0] = 7;
    dest[1] = 8;
    dest[2] = 9;

    array.prepare(gl, vboManager);
    CHECK(array.prepared());
  }

  SECTION("setup, render and cleanup issue a draw call over the indices")
  {
    auto array = BrushIndexArray{};
    const auto [block, dest] = array.getPointerToInsertElementsAt(3);
    dest[0] = 7;
    dest[1] = 8;
    dest[2] = 9;

    array.prepare(gl, vboManager);
    gl.onBindBuffer = [](GLenum, GLuint) {};

    auto capturedMode = GLenum{0};
    auto capturedCount = GLsizei{-1};
    gl.onDrawElements = [&](const GLenum mode, const GLsizei count, GLenum, const void*) {
      capturedMode = mode;
      capturedCount = count;
    };

    array.setup(gl);
    array.render(gl, gl::PrimType::Triangles);
    array.cleanup(gl);

    CHECK(capturedMode == GL_TRIANGLES);
    CHECK(capturedCount == 3);
  }

  SECTION("setup, render and cleanup issue a draw call over just the given sub-range")
  {
    auto array = BrushIndexArray{};
    const auto [block, dest] = array.getPointerToInsertElementsAt(6);
    for (auto i = 0u; i < 6; ++i)
    {
      dest[i] = i;
    }

    array.prepare(gl, vboManager);
    gl.onBindBuffer = [](GLenum, GLuint) {};

    auto capturedMode = GLenum{0};
    auto capturedCount = GLsizei{-1};
    auto capturedIndices = static_cast<const void*>(nullptr);
    gl.onDrawElements =
      [&](const GLenum mode, const GLsizei count, GLenum, const void* indices) {
        capturedMode = mode;
        capturedCount = count;
        capturedIndices = indices;
      };

    array.setup(gl);

    // baseline: offset 0 gives the buffer's own base pointer
    array.render(gl, gl::PrimType::Triangles, 0, 6);
    const auto* baseIndices = capturedIndices;

    array.render(gl, gl::PrimType::Triangles, 3, 3);
    array.cleanup(gl);

    CHECK(capturedMode == GL_TRIANGLES);
    CHECK(capturedCount == 3);
    // the sub-range starts 3 GLuint's past the buffer's base pointer -- computed via
    // uintptr_t rather than pointer arithmetic since baseIndices is null in this test
    // (it stands for the VBO's own base, per glDrawElements' offset-as-pointer idiom),
    // and offsetting a null pointer is undefined behavior
    const auto expectedIndices = reinterpret_cast<const void*>(
      reinterpret_cast<std::uintptr_t>(baseIndices) + 3 * sizeof(GLuint));
    CHECK(capturedIndices == expectedIndices);
  }

  vboManager.destroyPendingVbos(gl);
}

TEST_CASE("BrushVertexArray")
{
  using Vertex = gl::VertexTypes::P3NT2::Vertex;

  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);
  auto vboManager = gl::VboManager{};
  auto shaderProgram = gl::ShaderProgram{"test", 1u};

  gl.onVertexPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
  gl.onNormalPointer = [](GLenum, GLsizei, const GLvoid*) {};
  gl.onTexCoordPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
  gl.onEnableClientState = [](GLenum) {};
  gl.onDisableClientState = [](GLenum) {};
  gl.onClientActiveTexture = [](GLenum) {};

  SECTION("constructor")
  {
    auto array = BrushVertexArray{};
    CHECK(array.prepared());
  }

  SECTION("getPointerToInsertVerticesAt")
  {
    SECTION("writes vertices and makes the array unprepared")
    {
      auto array = BrushVertexArray{};
      const auto [block, dest] = array.getPointerToInsertVerticesAt(2);
      REQUIRE(block != nullptr);
      dest[0] = Vertex{vm::vec3f{0, 0, 0}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};
      dest[1] = Vertex{vm::vec3f{1, 0, 0}, vm::vec3f{0, 0, 1}, vm::vec2f{1, 0}};

      CHECK(!array.prepared());
    }

    SECTION("grows the underlying storage on demand")
    {
      auto array = BrushVertexArray{};

      const auto [block1, dest1] = array.getPointerToInsertVerticesAt(4);
      REQUIRE(block1 != nullptr);

      const auto [block2, dest2] = array.getPointerToInsertVerticesAt(100);
      REQUIRE(block2 != nullptr);
      for (auto i = 0u; i < 100; ++i)
      {
        dest2[i] = Vertex{vm::vec3f{0, 0, 0}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};
      }

      CHECK(!array.prepared());
    }
  }

  SECTION("deleteVerticesWithKey frees the allocation without zeroing the vertices")
  {
    auto array = BrushVertexArray{};
    const auto [block, dest] = array.getPointerToInsertVerticesAt(2);
    const auto freedPos = block->pos;
    dest[0] = Vertex{vm::vec3f{1, 2, 3}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};
    dest[1] = Vertex{vm::vec3f{4, 5, 6}, vm::vec3f{0, 0, 1}, vm::vec2f{1, 0}};

    array.deleteVerticesWithKey(block);

    // the freed space is not zeroed, only marked reusable: a same-size request
    // afterward reuses the same position
    const auto [block2, dest2] = array.getPointerToInsertVerticesAt(2);
    CHECK(block2->pos == freedPos);
    dest2[0] = Vertex{vm::vec3f{7, 8, 9}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};
  }

  SECTION("prepare uploads the array")
  {
    auto array = BrushVertexArray{};
    const auto [block, dest] = array.getPointerToInsertVerticesAt(1);
    dest[0] = Vertex{vm::vec3f{0, 0, 0}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};

    array.prepare(gl, vboManager);
    CHECK(array.prepared());
  }

  SECTION("setup binds the array for rendering, cleanup unbinds it")
  {
    auto array = BrushVertexArray{};
    const auto [block, dest] = array.getPointerToInsertVerticesAt(1);
    dest[0] = Vertex{vm::vec3f{0, 0, 0}, vm::vec3f{0, 0, 1}, vm::vec2f{0, 0}};

    array.prepare(gl, vboManager);
    CHECK(array.setup(gl, shaderProgram));
    array.cleanup(gl, shaderProgram);
  }

  vboManager.destroyPendingVbos(gl);
}

} // namespace tb::render
