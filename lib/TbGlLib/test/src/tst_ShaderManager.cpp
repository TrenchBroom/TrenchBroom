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
#include "gl/ShaderConfig.h"
#include "gl/ShaderManager.h"
#include "gl/TestUtils.h"

#include "kd/filesystem_utils.h"

#include <fstream>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("ShaderManager")
{
  SECTION("currentProgram is initially null")
  {
    auto manager = ShaderManager{[](const auto& path) { return path; }};
    CHECK(manager.currentProgram() == nullptr);
  }

  SECTION("loadProgram")
  {
    auto tmpFile = kdl::tmp_file{};
    {
      auto ofs = std::ofstream{tmpFile.path()};
      ofs << "void main() {}\n";
    }

    auto gl = MockGl{};
    installShaderCompileSupport(gl);

    auto manager =
      ShaderManager{[&](const std::filesystem::path&) { return tmpFile.path(); }};
    const auto config = ShaderConfig{"test", {"test.vert"}, {"test.frag"}};

    SECTION("succeeds and registers the program under its name")
    {
      REQUIRE(manager.loadProgram(gl, config).is_success());

      // program() would fail its internal contract if the name were not found
      manager.program(config);
    }

    SECTION("loading a program under the same name twice fails")
    {
      REQUIRE(manager.loadProgram(gl, config).is_success());
      CHECK(manager.loadProgram(gl, config).is_error());
    }

    SECTION("reuses an already-loaded shader across programs")
    {
      auto shaderCreations = 0;
      gl.onCreateShader = [&](GLenum) {
        ++shaderCreations;
        return GLuint{2};
      };

      const auto otherConfig = ShaderConfig{"test2", {"test.vert"}, {}};
      REQUIRE(manager.loadProgram(gl, config).is_success());
      REQUIRE(manager.loadProgram(gl, otherConfig).is_success());

      // the first loadProgram compiles two distinct shaders, "test.vert" and
      // "test.frag"; the second loadProgram only references "test.vert", which is
      // already cached and is not compiled again
      CHECK(shaderCreations == 2);
    }

    SECTION("fails if a shader file cannot be found")
    {
      auto brokenManager = ShaderManager{
        [](const std::filesystem::path&) { return "/nonexistent/shader.vert"; }};

      CHECK(brokenManager.loadProgram(gl, config).is_error());
    }
  }

  SECTION("loadProgram fails if the program cannot be created")
  {
    auto gl = MockGl{};
    gl.onCreateProgram = []() { return GLuint{0}; };

    auto manager = ShaderManager{[](const auto& path) { return path; }};
    const auto config = ShaderConfig{"test", {}, {}};

    CHECK(manager.loadProgram(gl, config).is_error());
  }
}

} // namespace tb::gl
