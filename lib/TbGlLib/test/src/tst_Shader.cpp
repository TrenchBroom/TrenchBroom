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
#include "gl/Shader.h"

#include "kd/filesystem_utils.h"

#include <cstring>
#include <fstream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("Shader")
{
  auto gl = MockGl{};

  SECTION("attach")
  {
    auto shader = Shader{"test.vert", GL_VERTEX_SHADER, 1u};

    auto capturedProgramId = GLuint{0};
    auto capturedShaderId = GLuint{0};
    gl.onAttachShader = [&](const GLuint programId, const GLuint shaderId) {
      capturedProgramId = programId;
      capturedShaderId = shaderId;
    };

    shader.attach(gl, 7u);
    CHECK(capturedProgramId == 7u);
    CHECK(capturedShaderId == 1u);
  }

  SECTION("destroy")
  {
    auto shader = Shader{"test.vert", GL_VERTEX_SHADER, 1u};

    auto deletedId = GLuint{0};
    gl.onDeleteShader = [&](const GLuint id) { deletedId = id; };

    shader.destroy(gl);
    CHECK(deletedId == 1u);

    // destroying an already-destroyed shader is a no-op
    deletedId = 0;
    shader.destroy(gl);
    CHECK(deletedId == 0u);
  }

  SECTION("move construction and move assignment transfer the shader id")
  {
    auto original = Shader{"test.vert", GL_VERTEX_SHADER, 1u};
    auto moved = Shader{std::move(original)};

    auto deletedId = GLuint{0};
    gl.onDeleteShader = [&](const GLuint id) { deletedId = id; };

    // the moved-from shader now holds id 0; destroying it is a no-op
    // NOLINTNEXTLINE(bugprone-use-after-move)
    original.destroy(gl);
    CHECK(deletedId == 0u);

    auto assigned = Shader{"other.vert", GL_VERTEX_SHADER, 2u};
    assigned = std::move(moved);

    assigned.destroy(gl);
    CHECK(deletedId == 1u);
  }

  SECTION("loadShader")
  {
    auto tmpFile = kdl::tmp_file{};
    {
      auto ofs = std::ofstream{tmpFile.path()};
      ofs << "void main() {}\n";
    }

    SECTION("success")
    {
      gl.onCreateShader = [](GLenum) { return GLuint{9}; };
      gl.onShaderSource = [](GLuint, GLsizei, const GLchar* const*, const GLint*) {};
      gl.onCompileShader = [](GLuint) {};
      gl.onGetShaderiv = [](GLuint, const GLenum pname, GLint* params) {
        *params = (pname == GL_COMPILE_STATUS) ? 1 : 0;
      };

      CHECK(loadShader(gl, tmpFile.path(), GL_VERTEX_SHADER).is_success());
    }

    SECTION("compile failure returns an error built from the info log")
    {
      gl.onCreateShader = [](GLenum) { return GLuint{9}; };
      gl.onShaderSource = [](GLuint, GLsizei, const GLchar* const*, const GLint*) {};
      gl.onCompileShader = [](GLuint) {};
      gl.onGetShaderiv = [](GLuint, const GLenum pname, GLint* params) {
        *params = (pname == GL_INFO_LOG_LENGTH) ? 5 : 0;
      };
      gl.onGetShaderInfoLog = [](GLuint, GLsizei, GLsizei*, GLchar* infoLog) {
        std::strcpy(infoLog, "oops");
      };

      CHECK(loadShader(gl, tmpFile.path(), GL_VERTEX_SHADER).is_error());
    }

    SECTION("returns an error when createShader fails")
    {
      gl.onCreateShader = [](GLenum) { return GLuint{0}; };

      CHECK(loadShader(gl, tmpFile.path(), GL_VERTEX_SHADER).is_error());
    }
  }

  SECTION("loadShader returns an error when the file does not exist")
  {
    gl.onCreateShader = [](GLenum) { return GLuint{9}; };

    CHECK(
      loadShader(gl, "/nonexistent/path/to/shader.vert", GL_VERTEX_SHADER).is_error());
  }
}

} // namespace tb::gl
