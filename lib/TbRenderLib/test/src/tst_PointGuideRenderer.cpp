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

#include "base/Color.h"
#include "gl/MockGl.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "gl/VertexType.h"
#include "render/PointGuideRenderer.h"

#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
using Vertex = gl::VertexTypes::P3C4::Vertex;
} // namespace

TEST_CASE("PointGuideRenderer")
{
  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);
  auto vboManager = gl::VboManager{};

  auto uploadCount = 0;
  auto lastUploadSize = GLsizeiptr{0};
  gl.onBufferSubData = [&](GLenum, GLintptr, const GLsizeiptr size, const void*) {
    ++uploadCount;
    lastUploadSize = size;
  };

  {
    // renderer must be destroyed (queuing its Vbo for destruction) before
    // destroyPendingVbos is called, and before vboManager itself is destroyed
    auto renderer = PointGuideRenderer{};
    renderer.setColor(Color{RgbaF{1.0f, 1.0f, 1.0f, 1.0f}});

    SECTION("setPosition")
    {
      SECTION("builds 6 axis-aligned spikes (24 vertices) from the position")
      {
        renderer.setPosition(vm::vec3d{1, 2, 3});
        renderer.prepare(gl, vboManager);

        CHECK(uploadCount == 1);
        CHECK(lastUploadSize == static_cast<GLsizeiptr>(24 * sizeof(Vertex)));
      }

      SECTION("with the same position again does not re-upload")
      {
        renderer.setPosition(vm::vec3d{1, 2, 3});
        renderer.prepare(gl, vboManager);
        REQUIRE(uploadCount == 1);

        renderer.setPosition(vm::vec3d{1, 2, 3});
        renderer.prepare(gl, vboManager);
        CHECK(uploadCount == 1); // still 1: nothing changed, so nothing to re-upload
      }

      SECTION("with a different position re-uploads")
      {
        renderer.setPosition(vm::vec3d{1, 2, 3});
        renderer.prepare(gl, vboManager);
        REQUIRE(uploadCount == 1);

        renderer.setPosition(vm::vec3d{4, 5, 6});
        renderer.prepare(gl, vboManager);
        CHECK(uploadCount == 2);
      }
    }
  }

  vboManager.destroyPendingVbos(gl);
}

} // namespace tb::render
