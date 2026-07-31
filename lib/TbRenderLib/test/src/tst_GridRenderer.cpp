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
#include "gl/OrthographicCamera.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "gl/VertexType.h"
#include "render/GridRenderer.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <cstring>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
using Vertex = gl::VertexTypes::P3::Vertex;

std::vector<std::byte> capturePreparedVertices(
  const gl::OrthographicCamera& camera, const vm::bbox3d& worldBounds)
{
  auto gl = gl::MockGl{};
  gl::installVboSupport(gl);

  auto captured = std::vector<std::byte>{};
  gl.onBufferSubData = [&](GLenum, GLintptr, const GLsizeiptr size, const void* data) {
    const auto* bytes = static_cast<const std::byte*>(data);
    captured.assign(bytes, bytes + size);
  };

  auto vboManager = gl::VboManager{};
  {
    // GridRenderer must be destroyed (queuing its Vbo for destruction) before
    // destroyPendingVbos is called, and before vboManager itself is destroyed
    auto gridRenderer = GridRenderer{camera, worldBounds};
    gridRenderer.prepare(gl, vboManager);
  }
  vboManager.destroyPendingVbos(gl);

  return captured;
}

std::vector<std::byte> toBytes(const std::vector<Vertex>& vertices)
{
  const auto* bytes = reinterpret_cast<const std::byte*>(vertices.data());
  return {bytes, bytes + vertices.size() * sizeof(Vertex)};
}
} // namespace

TEST_CASE("GridRenderer")
{
  const auto worldBounds = vm::bbox3d{{-100, -200, -300}, {100, 200, 300}};

  SECTION("constructor")
  {
    SECTION("looking down the x axis renders a quad at the world bounds' min x")
    {
      const auto camera = gl::OrthographicCamera{
        1.0f,
        100.0f,
        gl::Camera::Viewport{0, 0, 40, 20},
        vm::vec3f{5, 6, 7},
        vm::vec3f{1, 0, 0},
        vm::vec3f{0, 0, 1}};

      const auto expected = std::vector<Vertex>{
        Vertex{vm::vec3f{float(worldBounds.min.x()), 6.0f - 20.0f, 7.0f - 10.0f}},
        Vertex{vm::vec3f{float(worldBounds.min.x()), 6.0f - 20.0f, 7.0f + 10.0f}},
        Vertex{vm::vec3f{float(worldBounds.min.x()), 6.0f + 20.0f, 7.0f + 10.0f}},
        Vertex{vm::vec3f{float(worldBounds.min.x()), 6.0f + 20.0f, 7.0f - 10.0f}},
      };

      const auto actual = capturePreparedVertices(camera, worldBounds);
      CHECK(actual == toBytes(expected));
    }

    SECTION("looking down the y axis renders a quad at the world bounds' max y")
    {
      const auto camera = gl::OrthographicCamera{
        1.0f,
        100.0f,
        gl::Camera::Viewport{0, 0, 40, 20},
        vm::vec3f{5, 6, 7},
        vm::vec3f{0, 1, 0},
        vm::vec3f{0, 0, 1}};

      const auto expected = std::vector<Vertex>{
        Vertex{vm::vec3f{5.0f - 20.0f, float(worldBounds.max.y()), 7.0f - 10.0f}},
        Vertex{vm::vec3f{5.0f - 20.0f, float(worldBounds.max.y()), 7.0f + 10.0f}},
        Vertex{vm::vec3f{5.0f + 20.0f, float(worldBounds.max.y()), 7.0f + 10.0f}},
        Vertex{vm::vec3f{5.0f + 20.0f, float(worldBounds.max.y()), 7.0f - 10.0f}},
      };

      const auto actual = capturePreparedVertices(camera, worldBounds);
      CHECK(actual == toBytes(expected));
    }

    SECTION("looking down the z axis renders a quad at the world bounds' min z")
    {
      const auto camera = gl::OrthographicCamera{
        1.0f,
        100.0f,
        gl::Camera::Viewport{0, 0, 40, 20},
        vm::vec3f{5, 6, 7},
        vm::vec3f{0, 0, 1},
        vm::vec3f{0, 1, 0}};

      const auto expected = std::vector<Vertex>{
        Vertex{vm::vec3f{5.0f - 20.0f, 6.0f - 10.0f, float(worldBounds.min.z())}},
        Vertex{vm::vec3f{5.0f - 20.0f, 6.0f + 10.0f, float(worldBounds.min.z())}},
        Vertex{vm::vec3f{5.0f + 20.0f, 6.0f + 10.0f, float(worldBounds.min.z())}},
        Vertex{vm::vec3f{5.0f + 20.0f, 6.0f - 10.0f, float(worldBounds.min.z())}},
      };

      const auto actual = capturePreparedVertices(camera, worldBounds);
      CHECK(actual == toBytes(expected));
    }
  }
}

} // namespace tb::render
