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
#include "gl/ShaderManager.h"
#include "gl/ShaderProgram.h"

#include "vm/mat.h"
#include "vm/vec.h"

#include <cstring>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{
namespace
{

/**
 * Installs a GL_CURRENT_PROGRAM slot that always reports the given program as
 * active. ShaderProgram::set and ::activate both assert this via checkActive.
 */
void installActiveProgram(MockGl& gl, const GLuint programId)
{
  gl.onGetIntegerv = [programId](const GLenum pname, GLint* params) {
    if (pname == GL_CURRENT_PROGRAM)
    {
      *params = GLint(programId);
    }
  };
}

} // namespace

TEST_CASE("ShaderProgram")
{
  auto gl = MockGl{};

  SECTION("attach")
  {
    auto program = ShaderProgram{"test", 1u};
    auto shader = Shader{"test.vert", GL_VERTEX_SHADER, 2u};

    auto capturedProgramId = GLuint{0};
    auto capturedShaderId = GLuint{0};
    gl.onAttachShader = [&](const GLuint programId, const GLuint shaderId) {
      capturedProgramId = programId;
      capturedShaderId = shaderId;
    };

    program.attach(gl, shader);
    CHECK(capturedProgramId == 1u);
    CHECK(capturedShaderId == 2u);
  }

  SECTION("link")
  {
    auto program = ShaderProgram{"test", 1u};
    gl.onLinkProgram = [](GLuint) {};

    SECTION("success")
    {
      gl.onGetProgramiv = [](GLuint, const GLenum pname, GLint* params) {
        *params = (pname == GL_LINK_STATUS) ? 1 : 0;
      };

      CHECK(program.link(gl).is_success());
    }

    SECTION("failure returns an error built from the info log")
    {
      gl.onGetProgramiv = [](GLuint, const GLenum pname, GLint* params) {
        *params = (pname == GL_INFO_LOG_LENGTH) ? 5 : 0;
      };
      gl.onGetProgramInfoLog = [](GLuint, GLsizei, GLsizei*, GLchar* infoLog) {
        std::strcpy(infoLog, "oops");
      };

      CHECK(program.link(gl).is_error());
    }
  }

  SECTION("activate and deactivate")
  {
    auto program = ShaderProgram{"test", 1u};
    auto shaderManager = ShaderManager{[](const auto& path) { return path; }};

    installActiveProgram(gl, 1u);

    auto usedPrograms = std::vector<GLuint>{};
    gl.onUseProgram = [&](const GLuint programId) { usedPrograms.push_back(programId); };

    program.activate(gl, shaderManager);
    CHECK(usedPrograms == std::vector<GLuint>{1u});
    CHECK(shaderManager.currentProgram() == &program);

    program.deactivate(gl, shaderManager);
    CHECK(usedPrograms == std::vector<GLuint>{1u, 0u});
    CHECK(shaderManager.currentProgram() == nullptr);
  }

  SECTION("set")
  {
    auto program = ShaderProgram{"test", 1u};
    installActiveProgram(gl, 1u);

    auto locationQueries = 0;
    gl.onGetUniformLocation = [&](GLuint, const GLchar*) {
      ++locationQueries;
      return GLint{5};
    };

    SECTION("bool")
    {
      auto captured = std::pair<GLint, GLint>{-1, -1};
      gl.onUniform1i = [&](const GLint loc, const GLint v) { captured = {loc, v}; };

      program.set(gl, "flag", true);
      CHECK(captured == std::pair<GLint, GLint>{5, 1});
    }

    SECTION("int")
    {
      auto captured = std::pair<GLint, GLint>{-1, -1};
      gl.onUniform1i = [&](const GLint loc, const GLint v) { captured = {loc, v}; };

      program.set(gl, "count", 42);
      CHECK(captured == std::pair<GLint, GLint>{5, 42});
    }

    SECTION("size_t")
    {
      auto captured = std::pair<GLint, GLint>{-1, -1};
      gl.onUniform1i = [&](const GLint loc, const GLint v) { captured = {loc, v}; };

      program.set(gl, "count", size_t(42));
      CHECK(captured == std::pair<GLint, GLint>{5, 42});
    }

    SECTION("float")
    {
      auto captured = std::pair<GLint, GLfloat>{-1, -1.0f};
      gl.onUniform1f = [&](const GLint loc, const GLfloat v) { captured = {loc, v}; };

      program.set(gl, "factor", 1.5f);
      CHECK(captured == std::pair<GLint, GLfloat>{5, 1.5f});
    }

    SECTION("vec2f")
    {
      auto captured = vm::vec2f{};
      auto capturedLoc = GLint{-1};
      gl.onUniform2f = [&](const GLint loc, const GLfloat x, const GLfloat y) {
        capturedLoc = loc;
        captured = vm::vec2f{x, y};
      };

      program.set(gl, "v", vm::vec2f{1.0f, 2.0f});
      CHECK(capturedLoc == 5);
      CHECK(captured == vm::vec2f{1.0f, 2.0f});
    }

    SECTION("vec3f")
    {
      auto captured = vm::vec3f{};
      auto capturedLoc = GLint{-1};
      gl.onUniform3f =
        [&](const GLint loc, const GLfloat x, const GLfloat y, const GLfloat z) {
          capturedLoc = loc;
          captured = vm::vec3f{x, y, z};
        };

      program.set(gl, "v", vm::vec3f{1.0f, 2.0f, 3.0f});
      CHECK(capturedLoc == 5);
      CHECK(captured == vm::vec3f{1.0f, 2.0f, 3.0f});
    }

    SECTION("vec4f")
    {
      auto captured = vm::vec4f{};
      auto capturedLoc = GLint{-1};
      gl.onUniform4f = [&](
                         const GLint loc,
                         const GLfloat x,
                         const GLfloat y,
                         const GLfloat z,
                         const GLfloat w) {
        capturedLoc = loc;
        captured = vm::vec4f{x, y, z, w};
      };

      program.set(gl, "v", vm::vec4f{1.0f, 2.0f, 3.0f, 4.0f});
      CHECK(capturedLoc == 5);
      CHECK(captured == vm::vec4f{1.0f, 2.0f, 3.0f, 4.0f});
    }

    SECTION("mat2x2f")
    {
      auto capturedLoc = GLint{-1};
      gl.onUniformMatrix2fv = [&](const GLint loc, GLsizei, GLboolean, const GLfloat*) {
        capturedLoc = loc;
      };

      program.set(gl, "m", vm::mat2x2f{});
      CHECK(capturedLoc == 5);
    }

    SECTION("mat3x3f")
    {
      auto capturedLoc = GLint{-1};
      gl.onUniformMatrix3fv = [&](const GLint loc, GLsizei, GLboolean, const GLfloat*) {
        capturedLoc = loc;
      };

      program.set(gl, "m", vm::mat3x3f{});
      CHECK(capturedLoc == 5);
    }

    SECTION("mat4x4f")
    {
      auto capturedLoc = GLint{-1};
      gl.onUniformMatrix4fv = [&](const GLint loc, GLsizei, GLboolean, const GLfloat*) {
        capturedLoc = loc;
      };

      program.set(gl, "m", vm::mat4x4f{});
      CHECK(capturedLoc == 5);
    }

    SECTION("RgbF delegates to the vec3f overload")
    {
      auto capturedLoc = GLint{-1};
      gl.onUniform3f = [&](const GLint loc, GLfloat, GLfloat, GLfloat) {
        capturedLoc = loc;
      };

      program.set(gl, "c", RgbF{1.0f, 0.0f, 0.0f});
      CHECK(capturedLoc == 5);
    }

    SECTION("RgbaF delegates to the vec4f overload")
    {
      auto capturedLoc = GLint{-1};
      gl.onUniform4f = [&](const GLint loc, GLfloat, GLfloat, GLfloat, GLfloat) {
        capturedLoc = loc;
      };

      program.set(gl, "c", RgbaF{1.0f, 0.0f, 0.0f, 1.0f});
      CHECK(capturedLoc == 5);
    }

    SECTION("caches the uniform location across calls with the same name")
    {
      gl.onUniform1f = [](GLint, GLfloat) {};

      program.set(gl, "factor", 1.0f);
      program.set(gl, "factor", 2.0f);
      CHECK(locationQueries == 1);
    }
  }

  SECTION("findAttributeLocation caches the location across calls with the same name")
  {
    auto program = ShaderProgram{"test", 1u};

    auto locationQueries = 0;
    gl.onGetAttribLocation = [&](GLuint, const GLchar*) {
      ++locationQueries;
      return GLint{3};
    };

    CHECK(program.findAttributeLocation(gl, "position") == 3);
    CHECK(program.findAttributeLocation(gl, "position") == 3);
    CHECK(locationQueries == 1);
  }

  SECTION("destroy")
  {
    auto program = ShaderProgram{"test", 1u};

    auto deletedId = GLuint{0};
    gl.onDeleteProgram = [&](const GLuint id) { deletedId = id; };

    program.destroy(gl);
    CHECK(deletedId == 1u);

    // destroying an already-destroyed program is a no-op
    deletedId = 0;
    program.destroy(gl);
    CHECK(deletedId == 0u);
  }

  SECTION("move construction and move assignment transfer the program id")
  {
    auto original = ShaderProgram{"test", 1u};
    auto moved = ShaderProgram{std::move(original)};

    auto deletedId = GLuint{0};
    gl.onDeleteProgram = [&](const GLuint id) { deletedId = id; };

    // the moved-from program now holds id 0; destroying it is a no-op
    // NOLINTNEXTLINE(bugprone-use-after-move)
    original.destroy(gl);
    CHECK(deletedId == 0u);

    auto assigned = ShaderProgram{"other", 2u};
    assigned = std::move(moved);

    assigned.destroy(gl);
    CHECK(deletedId == 1u);
  }

  SECTION("createShaderProgram")
  {
    SECTION("success")
    {
      gl.onCreateProgram = []() { return GLuint{7}; };
      CHECK(createShaderProgram(gl, "test").is_success());
    }

    SECTION("failure when createProgram returns 0")
    {
      gl.onCreateProgram = []() { return GLuint{0}; };
      CHECK(createShaderProgram(gl, "test").is_error());
    }
  }
}

} // namespace tb::gl
