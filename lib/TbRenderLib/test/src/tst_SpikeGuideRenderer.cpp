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

#include "TestUtils.h"
#include "base/Color.h"
#include "gl/MockGl.h"
#include "gl/TestUtils.h"
#include "gl/VboManager.h"
#include "render/SpikeGuideRenderer.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
using Vertex = SpikeVertex;

std::vector<std::byte> capturePreparedVertices(SpikeGuideRenderer renderer)
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
    // renderer must be destroyed (queuing its Vbo for destruction) before
    // destroyPendingVbos is called, and before vboManager itself is destroyed
    auto localRenderer = std::move(renderer);
    localRenderer.prepare(gl, vboManager);
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

TEST_CASE("SpikeGuideRenderer")
{
  SECTION("add")
  {
    SECTION("appends a faded spike from the ray's origin along its direction")
    {
      auto renderer = SpikeGuideRenderer{};
      const auto color = Color{RgbaF{1.0f, 0.0f, 0.0f, 1.0f}};
      renderer.setColor(color);

      const auto ray = vm::ray3d{vm::vec3d{1, 2, 3}, vm::vec3d{0, 0, 1}};
      renderer.add(ray);

      const auto actual = capturePreparedVertices(renderer);
      CHECK(actual == toBytes(expectedSpikeVertices(ray, color)));
    }

    SECTION("appends spikes for every ray added, in order")
    {
      auto renderer = SpikeGuideRenderer{};
      const auto color = Color{RgbaF{0.0f, 1.0f, 0.0f, 1.0f}};
      renderer.setColor(color);

      const auto ray1 = vm::ray3d{vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}};
      const auto ray2 = vm::ray3d{vm::vec3d{5, 5, 5}, vm::vec3d{0, 1, 0}};
      renderer.add(ray1);
      renderer.add(ray2);

      auto expected = expectedSpikeVertices(ray1, color);
      const auto expected2 = expectedSpikeVertices(ray2, color);
      expected.insert(expected.end(), expected2.begin(), expected2.end());

      const auto actual = capturePreparedVertices(renderer);
      CHECK(actual == toBytes(expected));
    }
  }

  SECTION("clear removes all previously added spikes")
  {
    auto renderer = SpikeGuideRenderer{};
    renderer.setColor(Color{RgbaF{1.0f, 1.0f, 1.0f, 1.0f}});
    renderer.add(vm::ray3d{vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}});
    renderer.clear();

    const auto actual = capturePreparedVertices(renderer);
    CHECK(actual.empty());
  }
}

} // namespace tb::render
