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

#include "render/LinkRenderer.h"

#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

namespace
{
using LineVertex = LinkRenderer::LineVertex;
using ArrowVertex = LinkRenderer::ArrowVertex;

std::vector<std::byte> toBytes(const std::vector<ArrowVertex>& vertices)
{
  const auto* bytes = reinterpret_cast<const std::byte*>(vertices.data());
  return {bytes, bytes + vertices.size() * sizeof(ArrowVertex)};
}

// mirrors LinkRenderer.cpp's addArrow(): a fixed local shape, repeated with the given
// color, position and direction
void addExpectedArrow(
  std::vector<ArrowVertex>& arrows,
  const vm::vec4f& color,
  const vm::vec3f& arrowPosition,
  const vm::vec3f& lineDir)
{
  arrows.emplace_back(vm::vec3f{0, 3, 0}, color, arrowPosition, lineDir);
  arrows.emplace_back(vm::vec3f{9, 0, 0}, color, arrowPosition, lineDir);
  arrows.emplace_back(vm::vec3f{9, 0, 0}, color, arrowPosition, lineDir);
  arrows.emplace_back(vm::vec3f{0, -3, 0}, color, arrowPosition, lineDir);
}

// mirrors LinkRenderer.cpp's getArrows(): places one arrowhead per given fraction of
// the way from start to end
std::vector<ArrowVertex> expectedArrows(
  const vm::vec3f& start,
  const vm::vec3f& end,
  const vm::vec4f& color,
  const std::vector<float>& fractions)
{
  const auto lineVec = end - start;
  const auto lineDir = vm::normalize(lineVec);

  auto arrows = std::vector<ArrowVertex>{};
  for (const auto fraction : fractions)
  {
    addExpectedArrow(arrows, color, start + lineVec * fraction, lineDir);
  }
  return arrows;
}
} // namespace

TEST_CASE("LinkRenderer")
{
  SECTION("getArrows")
  {
    const auto color = vm::vec4f{1, 0, 0, 1};

    SECTION("no lines produce no arrows")
    {
      CHECK(getArrows({}).empty());
    }

    SECTION("a line shorter than 512 units gets a single arrow at 60% of its length")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{400, 0, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, color},
        LineVertex{end, color},
      };

      const auto actual = getArrows(links);
      CHECK(toBytes(actual) == toBytes(expectedArrows(start, end, color, {0.6f})));
    }

    SECTION("a line of exactly 512 units gets two arrows (not the short-line count)")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{512, 0, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, color},
        LineVertex{end, color},
      };

      const auto actual = getArrows(links);
      CHECK(toBytes(actual) == toBytes(expectedArrows(start, end, color, {0.2f, 0.6f})));
    }

    SECTION("a line shorter than 1024 units gets two arrows at 20% and 60%")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{800, 0, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, color},
        LineVertex{end, color},
      };

      const auto actual = getArrows(links);
      CHECK(toBytes(actual) == toBytes(expectedArrows(start, end, color, {0.2f, 0.6f})));
    }

    SECTION("a line of exactly 1024 units gets three arrows (not the medium-line count)")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{1024, 0, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, color},
        LineVertex{end, color},
      };

      const auto actual = getArrows(links);
      CHECK(
        toBytes(actual)
        == toBytes(expectedArrows(start, end, color, {0.1f, 0.4f, 0.7f})));
    }

    SECTION("a line of at least 1024 units gets three arrows at 10%, 40% and 70%")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{2000, 0, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, color},
        LineVertex{end, color},
      };

      const auto actual = getArrows(links);
      CHECK(
        toBytes(actual)
        == toBytes(expectedArrows(start, end, color, {0.1f, 0.4f, 0.7f})));
    }

    SECTION("the arrow color is taken from the line's start vertex")
    {
      const auto start = vm::vec3f{0, 0, 0};
      const auto end = vm::vec3f{100, 0, 0};
      const auto startColor = vm::vec4f{0, 1, 0, 1};
      const auto endColor = vm::vec4f{0, 0, 1, 1};
      const auto links = std::vector<LineVertex>{
        LineVertex{start, startColor},
        LineVertex{end, endColor},
      };

      const auto actual = getArrows(links);
      CHECK(toBytes(actual) == toBytes(expectedArrows(start, end, startColor, {0.6f})));
    }

    SECTION("arrows for multiple lines are appended in order")
    {
      const auto start1 = vm::vec3f{0, 0, 0};
      const auto end1 = vm::vec3f{100, 0, 0};
      const auto start2 = vm::vec3f{0, 0, 0};
      const auto end2 = vm::vec3f{0, 2000, 0};
      const auto links = std::vector<LineVertex>{
        LineVertex{start1, color},
        LineVertex{end1, color},
        LineVertex{start2, color},
        LineVertex{end2, color},
      };

      auto expected = expectedArrows(start1, end1, color, {0.6f});
      const auto expected2 = expectedArrows(start2, end2, color, {0.1f, 0.4f, 0.7f});
      expected.insert(expected.end(), expected2.begin(), expected2.end());

      const auto actual = getArrows(links);
      CHECK(toBytes(actual) == toBytes(expected));
    }
  }
}

} // namespace tb::render
