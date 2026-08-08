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
#include "gl/ShaderProgram.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "render/Circle.h"

#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("Circle")
{
  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);
  auto vboManager = gl::VboManager{};
  auto shaderProgram = gl::ShaderProgram{"test", 1u};

  gl.onVertexPointer = [](GLint, GLenum, GLsizei, const GLvoid*) {};
  gl.onEnableClientState = [](GLenum) {};
  gl.onDisableClientState = [](GLenum) {};
  gl.onBindBuffer = [](GLenum, GLuint) {};

  SECTION("prepared and prepare")
  {
    auto circle = Circle{1.0f, 8, false};
    CHECK(!circle.prepared());

    circle.prepare(gl, vboManager);
    CHECK(circle.prepared());
  }

  SECTION("render")
  {
    SECTION("an unfilled circle renders a LineLoop with segments + 1 vertices")
    {
      auto circle = Circle{1.0f, 8, false};
      circle.prepare(gl, vboManager);

      auto capturedMode = GLenum{0};
      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](const GLenum mode, GLint, const GLsizei count) {
        capturedMode = mode;
        capturedCount = count;
      };

      circle.render(gl, shaderProgram);
      CHECK(capturedMode == GL_LINE_LOOP);
      CHECK(capturedCount == 9);
    }

    SECTION("a filled circle renders a TriangleFan with segments + 2 vertices")
    {
      auto circle = Circle{1.0f, 8, true};
      circle.prepare(gl, vboManager);

      auto capturedMode = GLenum{0};
      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](const GLenum mode, GLint, const GLsizei count) {
        capturedMode = mode;
        capturedCount = count;
      };

      circle.render(gl, shaderProgram);
      CHECK(capturedMode == GL_TRIANGLE_FAN);
      CHECK(capturedCount == 10);
    }

    SECTION(
      "the startAngle/angleLength constructor still counts the center for a "
      "filled circle")
    {
      auto circle = Circle{1.0f, 8, true, 0.0f, vm::Cf::pi()};
      circle.prepare(gl, vboManager);

      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](
                          GLenum, GLint, const GLsizei count) { capturedCount = count; };

      circle.render(gl, shaderProgram);
      CHECK(capturedCount == 10);
    }

    SECTION("the 3D axis constructor produces segments + 1 vertices when unfilled")
    {
      auto circle =
        Circle{1.0f, 8, false, vm::axis::z, vm::vec3f{1, 0, 0}, vm::vec3f{0, 1, 0}};
      circle.prepare(gl, vboManager);

      auto capturedCount = GLsizei{-1};
      gl.onDrawArrays = [&](
                          GLenum, GLint, const GLsizei count) { capturedCount = count; };

      circle.render(gl, shaderProgram);
      CHECK(capturedCount == 9);
    }
  }

  vboManager.destroyPendingVbos(gl);
}

} // namespace tb::render
