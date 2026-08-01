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
#include "render/BoundsGuideRenderer.h"

#include "vm/bbox.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{
namespace
{

using Vertex = SpikeVertex;

std::vector<std::byte> capturePreparedVertices(BoundsGuideRenderer renderer)
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

// Independently derives the 24 expected spikes (8 corners x 3 axis-aligned rays,
// each pointing away from the box's center) without mirroring the production code's
// 24 individual, hand-written call sites -- exactly the shape of code where a
// copy-paste slip (wrong corner, wrong sign) is easy to make and easy to miss.
std::vector<Vertex> expectedBoundsGuideVertices(
  const vm::bbox3d& bounds, const Color& color)
{
  using C = vm::bbox3d::corner;

  auto expected = std::vector<Vertex>{};
  for (const auto cx : {C::min, C::max})
  {
    for (const auto cy : {C::min, C::max})
    {
      for (const auto cz : {C::min, C::max})
      {
        const auto origin = bounds.corner_position(cx, cy, cz);
        const auto sign = [](const C c) { return c == C::min ? -1.0 : 1.0; };

        for (const auto& direction :
             {vm::vec3d{sign(cx), 0, 0},
              vm::vec3d{0, sign(cy), 0},
              vm::vec3d{0, 0, sign(cz)}})
        {
          const auto spike = expectedSpikeVertices(vm::ray3d{origin, direction}, color);
          expected.insert(expected.end(), spike.begin(), spike.end());
        }
      }
    }
  }
  return expected;
}

} // namespace

TEST_CASE("BoundsGuideRenderer")
{
  const auto bounds = vm::bbox3d{{-10, -20, -30}, {10, 20, 30}};
  const auto color = RgbaF{1.0f, 0.5f, 0.0f, 1.0f};

  SECTION("setBounds")
  {
    SECTION("builds 24 outward-pointing spikes, one per corner axis")
    {
      auto renderer = BoundsGuideRenderer{};
      renderer.setColor(color);
      renderer.setBounds(bounds);

      const auto actual = capturePreparedVertices(renderer);
      CHECK(actual == toBytes(expectedBoundsGuideVertices(bounds, color)));
    }

    SECTION("with the same bounds again is a no-op")
    {
      auto renderer = BoundsGuideRenderer{};
      renderer.setColor(color);
      renderer.setBounds(bounds);
      renderer.setBounds(bounds);

      const auto actual = capturePreparedVertices(renderer);
      CHECK(actual == toBytes(expectedBoundsGuideVertices(bounds, color)));
    }
  }
}

} // namespace tb::render
