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

#include "gl/ActiveShader.h"
#include "gl/MockGl.h"
#include "gl/ShaderConfig.h"
#include "gl/ShaderManager.h"
#include "gl/TestUtils.h"

#include "kd/filesystem_utils.h"

#include <fstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("ActiveShader")
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
  const auto config = ShaderConfig{"test", {"test.vert"}, {}};
  REQUIRE(manager.loadProgram(gl, config).is_success());

  // installShaderCompileSupport hands out program id 1; report it as the current
  // program once activated, since ShaderProgram::activate asserts this
  gl.onGetIntegerv = [](const GLenum pname, GLint* params) {
    if (pname == GL_CURRENT_PROGRAM)
    {
      *params = GLint{1};
    }
  };

  auto usedPrograms = std::vector<GLuint>{};
  gl.onUseProgram = [&](const GLuint programId) { usedPrograms.push_back(programId); };

  SECTION("construction activates the program")
  {
    auto activeShader = ActiveShader{gl, manager, config};

    CHECK(usedPrograms == std::vector<GLuint>{1u});
    CHECK(manager.currentProgram() == &activeShader.program());
    CHECK(&activeShader.program() == &manager.program(config));
  }

  SECTION("destruction deactivates the program")
  {
    {
      auto activeShader = ActiveShader{gl, manager, config};
      CHECK(manager.currentProgram() != nullptr);
    }

    CHECK(usedPrograms == std::vector<GLuint>{1u, 0u});
    CHECK(manager.currentProgram() == nullptr);
  }
}

} // namespace tb::gl
