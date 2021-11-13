/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <cstring>

#include "Catch2.h"

namespace TrenchBroom {
namespace Renderer {
struct TestVertex {
  vm::vec3f pos;
  vm::vec2f uv;
  vm::vec4f color;
};

TEST_CASE("VertexTest.memoryLayoutSingleVertex", "[VertexTest]") {
  using Vertex = GLVertexTypes::P3T2C4::Vertex;

  const auto pos = vm::vec3f(1.0f, 2.0f, 3.0f);
  const auto uv = vm::vec2f(4.0f, 5.0f);
  const auto color = vm::vec4f(7.0f, 8.0f, 9.0f, 10.0f);

  const auto expected = TestVertex{pos, uv, color};
  const auto actual = Vertex(pos, uv, color);

  REQUIRE(sizeof(Vertex) == sizeof(TestVertex));
  REQUIRE(std::memcmp(&expected, &actual, sizeof(expected)) == 0);
}

TEST_CASE("VertexTest.memoryLayoutVertexList", "[VertexTest]") {
  using Vertex = GLVertexTypes::P3T2C4::Vertex;

  auto expected = std::vector<TestVertex>();
  auto actual = std::vector<Vertex>();

  for (size_t i = 0; i < 3; ++i) {
    const auto f = static_cast<float>(i);
    const auto pos = f * vm::vec3f(1.0f, 2.0f, 3.0f);
    const auto uv = f * vm::vec2f(4.0f, 5.0f);
    const auto color = f * vm::vec4f(7.0f, 8.0f, 9.0f, 10.0f);

    expected.emplace_back(TestVertex{pos, uv, color});
    actual.emplace_back(pos, uv, color);
  }

  REQUIRE(sizeof(Vertex) == sizeof(TestVertex));
  REQUIRE(actual.size() == expected.size());
  REQUIRE(std::memcmp(expected.data(), actual.data(), sizeof(TestVertex) * 3) == 0);
}
} // namespace Renderer
} // namespace TrenchBroom
