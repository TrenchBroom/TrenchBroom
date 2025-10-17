/*
 Copyright (C) 2021 Kristian Duske

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

#include "mdl/BezierPatch.h"
#include "mdl/EditorContext.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"

#include "vm/approx.h"
#include "vm/vec.h"

#include <ranges>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace vm
{

template <>
class approx<tb::mdl::PatchGrid::Point>
{
private:
  using GP = tb::mdl::PatchGrid::Point;
  const GP m_value;
  const double m_epsilon;

public:
  constexpr explicit approx(const GP value, const double epsilon)
    : m_value{value}
    , m_epsilon{epsilon}
  {
    assert(epsilon >= double(0));
  }
  constexpr explicit approx(const GP value)
    : approx{value, constants<double>::almost_zero()}
  {
  }

  constexpr std::strong_ordering operator<=>(const GP& rhs) const
  {
    if (const auto cmp = approx<vec3d>{m_value.position, m_epsilon} <=> rhs.position;
        cmp != 0)
    {
      return cmp;
    }

    if (const auto cmp = approx<vec2d>{m_value.uvCoords, m_epsilon} <=> rhs.uvCoords;
        cmp != 0)
    {
      return cmp;
    }

    return approx<vec3d>{m_value.normal, m_epsilon} <=> rhs.normal;
  }

  constexpr bool operator==(const GP& rhs) const { return *this <=> rhs == 0; }

  friend std::ostream& operator<<(std::ostream& str, const approx<GP>& a)
  {
    str << a.m_value;
    return str;
  }
};

} // namespace vm

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("PatchNode.computeGridNormals") {}

TEST_CASE("PatchNode.makePatchGrid")
{
  using CP = BezierPatch::Point;
  using GP = PatchGrid::Point;
  using T = std::tuple<size_t, size_t, size_t, std::vector<CP>, std::vector<GP>>;

  // clang-format off
  const auto 
  [r, c, sd, 
    controlPoints, 
    expectedPoints] = GENERATE(values<T>({
  {3, 3, 0, // flat surface on XY plane
    {CP{0.0, 2.0, 0.0, 0.0, 0.0}, CP{1.0, 2.0, 0.0, 0.5, 0.0}, CP{2.0, 2.0, 0.0, 1.0, 0.0},
    CP{0.0, 1.0, 0.0, 0.0, 0.5}, CP{1.0, 1.0, 0.0, 0.5, 0.5}, CP{2.0, 1.0, 0.0, 1.0, 0.5},
    CP{0.0, 0.0, 0.0, 0.0, 1.0}, CP{1.0, 0.0, 0.0, 0.5, 1.0}, CP{2.0, 0.0, 0.0, 1.0, 1.0}, },
    {GP{{0.0, 2.0, 0.0}, {0.0, 0.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 2.0, 0.0}, {1.0, 0.0}, {0.0, 0.0, 1.0}},
    GP{{0.0, 0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 0.0, 0.0}, {1.0, 1.0}, {0.0, 0.0, 1.0}}}},
  {3, 3, 0, // hill surface bulging towards +Z
    {CP{0.0, 2.0, 0.0, 0.0, 0.0}, CP{1.0, 2.0, 0.0, 0.5, 0.0}, CP{2.0, 2.0, 0.0, 1.0, 0.0},
    CP{0.0, 1.0, 0.0, 0.0, 0.5}, CP{1.0, 1.0, 4.0, 0.5, 0.5}, CP{2.0, 1.0, 0.0, 1.0, 0.5},
    CP{0.0, 0.0, 0.0, 0.0, 1.0}, CP{1.0, 0.0, 0.0, 0.5, 1.0}, CP{2.0, 0.0, 0.0, 1.0, 1.0}, },
    {GP{{0.0, 2.0, 0.0}, {0.0, 0.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 2.0, 0.0}, {1.0, 0.0}, {0.0, 0.0, 1.0}},
    GP{{0.0, 0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 0.0, 0.0}, {1.0, 1.0}, {0.0, 0.0, 1.0}}}},
  {3, 3, 1, // flat surface on XY plane
    {CP{0.0, 2.0, 0.0, 0.0, 0.0}, CP{1.0, 2.0, 0.0, 0.5, 0.0}, CP{2.0, 2.0, 0.0, 1.0, 0.0},
    CP{0.0, 1.0, 0.0, 0.0, 0.5}, CP{1.0, 1.0, 0.0, 0.5, 0.5}, CP{2.0, 1.0, 0.0, 1.0, 0.5},
    CP{0.0, 0.0, 0.0, 0.0, 1.0}, CP{1.0, 0.0, 0.0, 0.5, 1.0}, CP{2.0, 0.0, 0.0, 1.0, 1.0}, },
    {GP{{0.0, 2.0, 0.0}, {0.0, 0.0}, {0.0, 0.0, 1.0}}, GP{{1.0, 2.0, 0.0}, {0.5, 0.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 2.0, 0.0}, {1.0, 0.0}, {0.0, 0.0, 1.0}},
    GP{{0.0, 1.0, 0.0}, {0.0, 0.5}, {0.0, 0.0, 1.0}}, GP{{1.0, 1.0, 0.0}, {0.5, 0.5}, {0.0, 0.0, 1.0}}, GP{{2.0, 1.0, 0.0}, {1.0, 0.5}, {0.0, 0.0, 1.0}},
    GP{{0.0, 0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0, 1.0}}, GP{{1.0, 0.0, 0.0}, {0.5, 1.0}, {0.0, 0.0, 1.0}}, GP{{2.0, 0.0, 0.0}, {1.0, 1.0}, {0.0, 0.0, 1.0}}}},
  {3, 3, 1, // hill surface bulging towards +Z
    {CP{0.0, 2.0, 0.0, 0.0, 0.0}, CP{1.0, 2.0, 0.0, 0.5, 0.0}, CP{2.0, 2.0, 0.0, 1.0, 0.0},
    CP{0.0, 1.0, 0.0, 0.0, 0.5}, CP{1.0, 1.0, 4.0, 0.5, 0.5}, CP{2.0, 1.0, 0.0, 1.0, 0.5},
    CP{0.0, 0.0, 0.0, 0.0, 1.0}, CP{1.0, 0.0, 0.0, 0.5, 1.0}, CP{2.0, 0.0, 0.0, 1.0, 1.0}, },
    {GP{{0.0, 2.0, 0.0}, {0.0, 0.0}, {0.0, 0.0, 1.0}}, GP{{1.0, 2.0, 0.0}, {0.5, 0.0}, {0.0, 0.707107, 0.707107}}, GP{{2.0, 2.0, 0.0}, {1.0, 0.0}, {0.0, 0.0, 1.0}},
    GP{{0.0, 1.0, 0.0}, {0.0, 0.5}, {-0.707107, 0.0, 0.707107}}, GP{{1.0, 1.0, 1.0}, {0.5, 0.5}, {0.0, 0.0, 1.0}}, GP{{2.0, 1.0, 0.0}, {1.0, 0.5}, {0.707107, 0.0, 0.707107}},
    GP{{0.0, 0.0, 0.0}, {0.0, 1.0}, {0.0, 0.0, 1.0}}, GP{{1.0, 0.0, 0.0}, {0.5, 1.0}, {0.0, -0.707107, 0.707107}}, GP{{2.0, 0.0, 0.0}, {1.0, 1.0}, {0.0, 0.0, 1.0}}}},
  {5, 3, 1, // flat surface on XY plane with 5 rows
    {CP{0.0, 2.0, 0.0, 0.0, 0.0 }, CP{1.0, 2.0, 0.0, 0.5, 0.0 }, CP{2.0, 2.0, 0.0, 1.0, 0.0 },
    CP{0.0, 1.5, 0.0, 0.0, 0.25}, CP{1.0, 1.5, 0.0, 0.5, 0.25}, CP{2.0, 1.5, 0.0, 1.0, 0.25},
    CP{0.0, 1.0, 0.0, 0.0, 0.5 }, CP{1.0, 1.0, 0.0, 0.5, 0.5 }, CP{2.0, 1.0, 0.0, 1.0, 0.5 },
    CP{0.0, 0.5, 0.0, 0.0, 0.75}, CP{1.0, 0.5, 0.0, 0.5, 0.75}, CP{2.0, 0.5, 0.0, 1.0, 0.75},
    CP{0.0, 0.0, 0.0, 0.0, 1.0 }, CP{1.0, 0.0, 0.0, 0.5, 1.0 }, CP{2.0, 0.0, 0.0, 1.0, 1.0 }, },
    {GP{{0.0, 2.0, 0.0}, {0.0, 0.0 }, {0.0, 0.0, 1.0}}, GP{{1.0, 2.0, 0.0}, {0.5, 0.0 }, {0.0, 0.0, 1.0}}, GP{{2.0, 2.0, 0.0}, {1.0, 0.0 }, {0.0, 0.0, 1.0}},
    GP{{0.0, 1.5, 0.0}, {0.0, 0.25}, {0.0, 0.0, 1.0}}, GP{{1.0, 1.5, 0.0}, {0.5, 0.25}, {0.0, 0.0, 1.0}}, GP{{2.0, 1.5, 0.0}, {1.0, 0.25}, {0.0, 0.0, 1.0}},
    GP{{0.0, 1.0, 0.0}, {0.0, 0.5 }, {0.0, 0.0, 1.0}}, GP{{1.0, 1.0, 0.0}, {0.5, 0.5 }, {0.0, 0.0, 1.0}}, GP{{2.0, 1.0, 0.0}, {1.0, 0.5 }, {0.0, 0.0, 1.0}},
    GP{{0.0, 0.5, 0.0}, {0.0, 0.75}, {0.0, 0.0, 1.0}}, GP{{1.0, 0.5, 0.0}, {0.5, 0.75}, {0.0, 0.0, 1.0}}, GP{{2.0, 0.5, 0.0}, {1.0, 0.75}, {0.0, 0.0, 1.0}},
    GP{{0.0, 0.0, 0.0}, {0.0, 1.0 }, {0.0, 0.0, 1.0}}, GP{{1.0, 0.0, 0.0}, {0.5, 1.0 }, {0.0, 0.0, 1.0}}, GP{{2.0, 0.0, 0.0}, {1.0, 1.0 }, {0.0, 0.0, 1.0}}}},
  {9, 3, 1, // cylinder
    {CP{-1.0,  0.0,  1.0, 0.0, 0.0  }, CP{-1.0,  0.0,  0.0, 0.5, 0.0  }, CP{-1.0,  0.0, -1.0, 1.0, 0.0  },
    CP{-1.0,  1.0,  1.0, 0.0, 0.125}, CP{-1.0,  1.0,  0.0, 0.5, 0.125}, CP{-1.0,  1.0, -1.0, 1.0, 0.125},
    CP{ 0.0,  1.0,  1.0, 0.0, 0.25 }, CP{ 0.0,  1.0,  0.0, 0.5, 0.25 }, CP{ 0.0,  1.0, -1.0, 1.0, 0.25 },
    CP{ 1.0,  1.0,  1.0, 0.0, 0.375}, CP{ 1.0,  1.0,  0.0, 0.5, 0.375}, CP{ 1.0,  1.0, -1.0, 1.0, 0.375},
    CP{ 1.0,  0.0,  1.0, 0.0, 0.5  }, CP{ 1.0,  0.0,  0.0, 0.5, 0.5  }, CP{ 1.0,  0.0, -1.0, 1.0, 0.5  },
    CP{ 1.0, -1.0,  1.0, 0.0, 0.625}, CP{ 1.0, -1.0,  0.0, 0.5, 0.625}, CP{ 1.0, -1.0, -1.0, 1.0, 0.625},
    CP{ 0.0, -1.0,  1.0, 0.0, 0.75 }, CP{ 0.0, -1.0,  0.0, 0.5, 0.75 }, CP{ 0.0, -1.0, -1.0, 1.0, 0.75 },
    CP{-1.0, -1.0,  1.0, 0.0, 0.875}, CP{-1.0, -1.0,  0.0, 0.5, 0.875}, CP{-1.0, -1.0, -1.0, 1.0, 0.875},
    CP{-1.0,  0.0,  1.0, 0.0, 1.0  }, CP{-1.0,  0.0,  0.0, 0.5, 1.0  }, CP{-1.0,  0.0, -1.0, 1.0, 1.0  }},
    {GP{{-1.0,   0.0,   1.0}, {0.0, 0.0  }, {-1.0,       0.0,      0.0}}, GP{{-1.0,   0.0,  0.0}, {0.5, 0.0  }, {-1.0,       0.0,      0.0}}, GP{{-1.0,   0.0,  -1.0}, {1.0, 0.0  }, {-1.0,       0.0,      0.0}},
    GP{{-0.75,  0.75,  1.0}, {0.0, 0.125}, {-0.707107,  0.707107, 0.0}}, GP{{-0.75,  0.75, 0.0}, {0.5, 0.125}, {-0.707107,  0.707107, 0.0}}, GP{{-0.75,  0.75, -1.0}, {1.0, 0.125}, {-0.707107,  0.707107, 0.0}},
    GP{{ 0.0,   1.0,   1.0}, {0.0, 0.25 }, { 0.0,       1.0,      0.0}}, GP{{ 0.0,   1.0,  0.0}, {0.5, 0.25 }, { 0.0,       1.0,      0.0}}, GP{{ 0.0,   1.0,  -1.0}, {1.0, 0.25 }, { 0.0,       1.0,      0.0}},
    GP{{ 0.75,  0.75,  1.0}, {0.0, 0.375}, { 0.707107,  0.707107, 0.0}}, GP{{ 0.75,  0.75, 0.0}, {0.5, 0.375}, { 0.707107,  0.707107, 0.0}}, GP{{ 0.75,  0.75, -1.0}, {1.0, 0.375}, { 0.707107,  0.707107, 0.0}},
    GP{{ 1.0,   0.0,   1.0}, {0.0, 0.5  }, { 1.0,       0.0,      0.0}}, GP{{ 1.0,   0.0,  0.0}, {0.5, 0.5  }, { 1.0,       0.0,      0.0}}, GP{{ 1.0,   0.0,  -1.0}, {1.0, 0.5  }, { 1.0,       0.0,      0.0}},
    GP{{ 0.75, -0.75,  1.0}, {0.0, 0.625}, { 0.707107, -0.707107, 0.0}}, GP{{ 0.75, -0.75, 0.0}, {0.5, 0.625}, { 0.707107, -0.707107, 0.0}}, GP{{ 0.75, -0.75, -1.0}, {1.0, 0.625}, { 0.707107, -0.707107, 0.0}},
    GP{{ 0.0,  -1.0,   1.0}, {0.0, 0.75 }, { 0.0,      -1.0,      0.0}}, GP{{ 0.0,  -1.0,  0.0}, {0.5, 0.75 }, { 0.0,      -1.0,      0.0}}, GP{{ 0.0,  -1.0,  -1.0}, {1.0, 0.75 }, { 0.0,      -1.0,      0.0}},
    GP{{-0.75, -0.75,  1.0}, {0.0, 0.875}, {-0.707107, -0.707107, 0.0}}, GP{{-0.75, -0.75, 0.0}, {0.5, 0.875}, {-0.707107, -0.707107, 0.0}}, GP{{-0.75, -0.75, -1.0}, {1.0, 0.875}, {-0.707107, -0.707107, 0.0}},
    GP{{-1.0,   0.0,   1.0}, {0.0, 1.0  }, {-1.0,       0.0,      0.0}}, GP{{-1.0,   0.0,  0.0}, {0.5, 1.0  }, {-1.0,       0.0,      0.0}}, GP{{-1.0,   0.0,  -1.0}, {1.0, 1.0  }, {-1.0,       0.0,      0.0}}}},
  }));
  // clang-format on

  CAPTURE(r, c, sd, controlPoints);
  CHECK_THAT(
    makePatchGrid(BezierPatch{r, c, controlPoints, "material"}, sd).points,
    RangeEquals(expectedPoints | std::views::transform([](const auto& p) {
                  return vm::approx{p};
                })));
}

TEST_CASE("PatchNode.pickFlatPatch")
{
  using P = BezierPatch::Point;

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{5, 5, {
    P{0.0, 4.0, 0.0}, P{1.0, 4.0, 0.0}, P{2.0, 4.0, 0.0}, P{3.0, 4.0, 0.0}, P{4.0, 4.0, 0.0},
    P{0.0, 3.0, 0.0}, P{1.0, 3.0, 0.0}, P{2.0, 3.0, 0.0}, P{3.0, 3.0, 0.0}, P{4.0, 3.0, 0.0},
    P{0.0, 2.0, 0.0}, P{1.0, 2.0, 0.0}, P{2.0, 2.0, 0.0}, P{3.0, 2.0, 0.0}, P{4.0, 2.0, 0.0},
    P{0.0, 1.0, 0.0}, P{1.0, 1.0, 0.0}, P{2.0, 1.0, 0.0}, P{3.0, 1.0, 0.0}, P{4.0, 1.0, 0.0},
    P{0.0, 0.0, 0.0}, P{1.0, 0.0, 0.0}, P{2.0, 0.0, 0.0}, P{3.0, 0.0, 0.0}, P{4.0, 0.0, 0.0},
  }, "material"}};
  // clang-format on

  using T = std::tuple<vm::ray3d, std::optional<vm::vec3d>>;

  // clang-format off
  const auto 
  [pickRay,                                        expectedHitPoint  ] = GENERATE(values<T>({
  {vm::ray3d{vm::vec3d{2, 2,  1}, vm::vec3d{0, 0, -1}}, vm::vec3d{2, 2, 0}},
  {vm::ray3d{vm::vec3d{2, 2, -1}, vm::vec3d{0, 0, 1}}, vm::vec3d{2, 2, 0}},
  {vm::ray3d{vm::vec3d{2, 3,  1}, vm::vec3d{0, 0, -1}}, vm::vec3d{2, 3, 0}},
  {vm::ray3d{vm::vec3d{2, 3,  1}, vm::vec3d{0, 0, 1}}, std::nullopt     },
  {vm::ray3d{vm::vec3d{0, -1, 1}, vm::vec3d{0, 0, -1}}, std::nullopt     },
  }));
  // clang-format on

  CAPTURE(pickRay);

  const auto editorContext = EditorContext{};
  auto pickResult = PickResult{};
  patchNode.pick(editorContext, pickRay, pickResult);

  if (expectedHitPoint.has_value())
  {
    CHECK(pickResult.size() == 1u);

    const auto hit = pickResult.all().front();
    CHECK(hit.hitPoint() == expectedHitPoint);
  }
  else
  {
    CHECK(pickResult.size() == 0u);
  }
}

} // namespace tb::mdl
