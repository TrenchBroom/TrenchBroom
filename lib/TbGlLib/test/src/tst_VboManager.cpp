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
#include "gl/VboManager.h"

#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{

TEST_CASE("VboManager")
{
  auto gl = MockGl{};
  installVboSupport(gl);

  auto vboManager = VboManager{};

  SECTION("initial state")
  {
    CHECK(vboManager.peakVboCount() == 0u);
    CHECK(vboManager.currentVboCount() == 0u);
    CHECK(vboManager.currentVboSize() == 0u);
  }

  SECTION("allocateVbo tracks the current and peak count and size")
  {
    auto vbo1 = vboManager.allocateVbo(gl, VboType::ArrayBuffer, 100u);
    CHECK(vbo1->capacity() == 100u);
    CHECK(vbo1->offset() == 0u);
    CHECK(vboManager.currentVboCount() == 1u);
    CHECK(vboManager.currentVboSize() == 100u);
    CHECK(vboManager.peakVboCount() == 1u);

    auto vbo2 =
      vboManager.allocateVbo(gl, VboType::ElementArrayBuffer, 50u, VboUsage::DynamicDraw);
    CHECK(vbo2->capacity() == 50u);
    CHECK(vboManager.currentVboCount() == 2u);
    CHECK(vboManager.currentVboSize() == 150u);
    CHECK(vboManager.peakVboCount() == 2u);

    vboManager.destroyVbo(std::move(vbo1));
    vboManager.destroyVbo(std::move(vbo2));
    vboManager.destroyPendingVbos(gl);
  }

  SECTION("destroyVbo decrements the current count and size, but not the peak")
  {
    auto vbo1 = vboManager.allocateVbo(gl, VboType::ArrayBuffer, 100u);
    auto vbo2 = vboManager.allocateVbo(gl, VboType::ArrayBuffer, 50u);
    REQUIRE(vboManager.peakVboCount() == 2u);

    vboManager.destroyVbo(std::move(vbo1));
    CHECK(vboManager.currentVboCount() == 1u);
    CHECK(vboManager.currentVboSize() == 50u);
    CHECK(vboManager.peakVboCount() == 2u);

    vboManager.destroyVbo(std::move(vbo2));
    CHECK(vboManager.currentVboCount() == 0u);
    CHECK(vboManager.currentVboSize() == 0u);
    CHECK(vboManager.peakVboCount() == 2u);

    vboManager.destroyPendingVbos(gl);
  }

  SECTION("destroyPendingVbos frees every Vbo passed to destroyVbo since the last call")
  {
    auto vbo1 = vboManager.allocateVbo(gl, VboType::ArrayBuffer, 100u);
    auto vbo2 = vboManager.allocateVbo(gl, VboType::ArrayBuffer, 50u);

    vboManager.destroyVbo(std::move(vbo1));
    vboManager.destroyVbo(std::move(vbo2));

    auto deletedIds = std::vector<GLuint>{};
    gl.onDeleteBuffers = [&](const GLsizei n, const GLuint* buffers) {
      REQUIRE(n == 1);
      deletedIds.push_back(buffers[0]);
    };

    vboManager.destroyPendingVbos(gl);
    CHECK(deletedIds.size() == 2u);

    // a second call has nothing left to free
    vboManager.destroyPendingVbos(gl);
    CHECK(deletedIds.size() == 2u);
  }
}

} // namespace tb::gl
