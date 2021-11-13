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

#include "Model/PatchNode.h"
#include "FloatType.h"
#include "Model/BezierPatch.h"
#include "Model/EditorContext.h"
#include "Model/PickResult.h"

#include <kdl/vector_utils.h>

#include <vecmath/approx.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace vm {
template <> class approx<TrenchBroom::Model::PatchGrid::Point> {
private:
  using GP = TrenchBroom::Model::PatchGrid::Point;
  const GP m_value;
  const FloatType m_epsilon;

public:
  constexpr explicit approx(const GP value, const FloatType epsilon)
    : m_value(value)
    , m_epsilon(epsilon) {
    assert(epsilon >= FloatType(0));
  }
  constexpr explicit approx(const GP value)
    : approx(value, vm::constants<FloatType>::almost_zero()) {}

  friend constexpr bool operator==(const GP& lhs, const approx<GP>& rhs) {
    return lhs.position == approx<vec3>{rhs.m_value.position, rhs.m_epsilon} &&
           lhs.texCoords == approx<vec2>{rhs.m_value.texCoords, rhs.m_epsilon} &&
           lhs.normal == approx<vec3>{rhs.m_value.normal, rhs.m_epsilon};
  }

  friend constexpr bool operator==(const approx<GP>& lhs, const GP& rhs) { return rhs == lhs; }

  friend constexpr bool operator!=(const GP& lhs, const approx<GP>& rhs) { return !(lhs == rhs); }

  friend constexpr bool operator!=(const approx<GP>& lhs, const GP& rhs) { return !(lhs == rhs); }

  friend std::ostream& operator<<(std::ostream& str, const approx<GP>& a) {
    str << a.m_value;
    return str;
  }
};
} // namespace vm

namespace TrenchBroom {
namespace Model {
TEST_CASE("PatchNode.computeGridNormals") {}

TEST_CASE("PatchNode.makePatchGrid") {
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
  CHECK(
    makePatchGrid(BezierPatch{r, c, controlPoints, "texture"}, sd).points ==
    kdl::vec_transform(expectedPoints, [](const auto& p) {
      return vm::approx{p};
    }));
}

TEST_CASE("PatchNode.pickFlatPatch") {
  using P = BezierPatch::Point;

  // clang-format off
            auto patchNode = PatchNode{BezierPatch{5, 5, {
                P{0.0, 4.0, 0.0}, P{1.0, 4.0, 0.0}, P{2.0, 4.0, 0.0}, P{3.0, 4.0, 0.0}, P{4.0, 4.0, 0.0},
                P{0.0, 3.0, 0.0}, P{1.0, 3.0, 0.0}, P{2.0, 3.0, 0.0}, P{3.0, 3.0, 0.0}, P{4.0, 3.0, 0.0},
                P{0.0, 2.0, 0.0}, P{1.0, 2.0, 0.0}, P{2.0, 2.0, 0.0}, P{3.0, 2.0, 0.0}, P{4.0, 2.0, 0.0},
                P{0.0, 1.0, 0.0}, P{1.0, 1.0, 0.0}, P{2.0, 1.0, 0.0}, P{3.0, 1.0, 0.0}, P{4.0, 1.0, 0.0},
                P{0.0, 0.0, 0.0}, P{1.0, 0.0, 0.0}, P{2.0, 0.0, 0.0}, P{3.0, 0.0, 0.0}, P{4.0, 0.0, 0.0},
            }, "texture"}};
  // clang-format on

  using T = std::tuple<vm::ray3, std::optional<vm::vec3>>;

  // clang-format off
            const auto 
            [pickRay,                                        expectedHitPoint  ] = GENERATE(values<T>({
            {vm::ray3{vm::vec3{2, 2,  1}, vm::vec3::neg_z()}, vm::vec3{2, 2, 0}},
            {vm::ray3{vm::vec3{2, 2, -1}, vm::vec3::pos_z()}, vm::vec3{2, 2, 0}},
            {vm::ray3{vm::vec3{2, 3,  1}, vm::vec3::neg_z()}, vm::vec3{2, 3, 0}},
            {vm::ray3{vm::vec3{2, 3,  1}, vm::vec3::pos_z()}, std::nullopt     },
            {vm::ray3{vm::vec3{0, -1, 1}, vm::vec3::neg_z()}, std::nullopt     },
            }));
  // clang-format on

  CAPTURE(pickRay);

  const auto editorContext = EditorContext{};
  auto pickResult = PickResult{};
  patchNode.pick(editorContext, pickRay, pickResult);

  if (expectedHitPoint.has_value()) {
    CHECK(pickResult.size() == 1u);

    const auto hit = pickResult.all().front();
    CHECK(hit.hitPoint() == expectedHitPoint);
  } else {
    CHECK(pickResult.size() == 0u);
  }
}
} // namespace Model
} // namespace TrenchBroom
