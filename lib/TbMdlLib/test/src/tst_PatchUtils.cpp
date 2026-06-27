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

#include "mdl/BezierPatch.h"
#include "mdl/CatchConfig.h"
#include "mdl/PatchUtils.h"

#include "vm/approx.h"
#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("resamplePatch")
{
  // clang-format off
  const auto patch = BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0},
  }, "material"};
  // clang-format on

  SECTION("same point counts returns the patch unchanged")
  {
    // Resampling at the same resolution is a degenerate case of exact subdivision: every
    // new sub-surface boundary coincides with an old one, so the fit must recover the
    // original control points (modulo floating point error from numeric quadrature)
    // rather than flattening them.
    const auto resampled = resamplePatch(patch, 3, 3);

    REQUIRE(resampled.pointRowCount() == patch.pointRowCount());
    REQUIRE(resampled.pointColumnCount() == patch.pointColumnCount());
    CHECK(resampled.materialName() == patch.materialName());

    for (size_t row = 0; row < patch.pointRowCount(); ++row)
    {
      for (size_t col = 0; col < patch.pointColumnCount(); ++col)
      {
        CAPTURE(row, col);
        CHECK(
          vm::approx{resampled.controlPoint(row, col), 1e-9}
          == patch.controlPoint(row, col));
      }
    }
  }

  SECTION("more point counts refines the patch without changing its shape")
  {
    // These control points are an exact Bezier subdivision (verified by hand via the de
    // Casteljau split formula).
    // clang-format off
    const auto expected = BezierPatch{5, 5, {
      {0, 0,   0},   {0.5, 0,   0.5}, {1, 0,   0.5}, {1.5, 0,   0.5}, {2, 0,   0},
      {0, 0.5, 0.5}, {0.5, 0.5, 1},   {1, 0.5, 1},    {1.5, 0.5, 1},   {2, 0.5, 0.5},
      {0, 1,   0.5}, {0.5, 1,   1},   {1, 1,   1},    {1.5, 1,   1},   {2, 1,   0.5},
      {0, 1.5, 0.5}, {0.5, 1.5, 1},   {1, 1.5, 1},    {1.5, 1.5, 1},   {2, 1.5, 0.5},
      {0, 2,   0},   {0.5, 2,   0.5}, {1, 2,   0.5},  {1.5, 2,   0.5}, {2, 2,   0},
    }, "material"};
    // clang-format on

    const auto resampled = resamplePatch(patch, 5, 5);

    REQUIRE(resampled.pointRowCount() == expected.pointRowCount());
    REQUIRE(resampled.pointColumnCount() == expected.pointColumnCount());
    CHECK(resampled.materialName() == expected.materialName());

    for (size_t row = 0; row < expected.pointRowCount(); ++row)
    {
      for (size_t col = 0; col < expected.pointColumnCount(); ++col)
      {
        CAPTURE(row, col);
        CHECK(
          vm::approx{resampled.controlPoint(row, col), 1e-9}
          == expected.controlPoint(row, col));
      }
    }

    // The exact control points above guarantee that the refined patch must trace out
    // precisely the same surface as the original at every parameter, not just at its own
    // control points.
    for (size_t i = 0; i <= 10; ++i)
    {
      const auto u = double(i) / 10.0;
      for (size_t j = 0; j <= 10; ++j)
      {
        const auto v = double(j) / 10.0;
        CAPTURE(u, v);
        CHECK(vm::approx{resampled.evaluateAt(u, v), 1e-9} == patch.evaluateAt(u, v));
      }
    }
  }

  SECTION("fewer point counts still preserves the patch's corners exactly")
  {
    // clang-format off
    const auto largePatch = BezierPatch{5, 5, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0}, {3, 0, 1}, {4, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1}, {3, 1, 2}, {4, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0}, {3, 2, 1}, {4, 2, 0},
      {0, 3, 1}, {1, 3, 2}, {2, 3, 1}, {3, 3, 2}, {4, 3, 1},
      {0, 4, 0}, {1, 4, 1}, {2, 4, 0}, {3, 4, 1}, {4, 4, 0},
    }, "material"};
    // clang-format on

    const auto resampled = resamplePatch(largePatch, 3, 3);

    CHECK(resampled.pointRowCount() == 3u);
    CHECK(resampled.pointColumnCount() == 3u);
    CHECK(resampled.materialName() == "material");
    CHECK(resampled.controlPoint(0, 0) == largePatch.controlPoint(0, 0));
    CHECK(resampled.controlPoint(0, 2) == largePatch.controlPoint(0, 4));
    CHECK(resampled.controlPoint(2, 0) == largePatch.controlPoint(4, 0));
    CHECK(resampled.controlPoint(2, 2) == largePatch.controlPoint(4, 4));

    // A single quadratic sub-surface cannot exactly reproduce a 2x2 grid of independent
    // quadratic sub-surfaces, so this fit is necessarily lossy, but it should still stay
    // visibly close to the original surface rather than diverge wildly.
    for (size_t i = 0; i <= 10; ++i)
    {
      const auto u = double(i) / 10.0;
      for (size_t j = 0; j <= 10; ++j)
      {
        const auto v = double(j) / 10.0;
        CAPTURE(u, v);
        CHECK(vm::approx{resampled.evaluateAt(u, v), 1.0} == largePatch.evaluateAt(u, v));
      }
    }
  }

  SECTION("refining an asymmetric patch preserves its shape")
  {
    // The row and column directions use independent code paths throughout (separate
    // sub-surface counts, separate fit calls), so a patch with the same number of rows
    // and columns cannot tell those paths apart if one is accidentally used for the
    // other. This patch refines rows by 1 -> 2 and columns by 2 -> 4 sub-surfaces, two
    // different exact multiples, so a row/column mix-up would distort the surface.

    // clang-format off
    const auto asymmetricPatch = BezierPatch{3, 5, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0}, {3, 0, 1}, {4, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1}, {3, 1, 2}, {4, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0}, {3, 2, 1}, {4, 2, 0},
    }, "material"};
    // clang-format on

    const auto resampled = resamplePatch(asymmetricPatch, 5, 9);

    REQUIRE(resampled.pointRowCount() == 5u);
    REQUIRE(resampled.pointColumnCount() == 9u);

    for (size_t i = 0; i <= 10; ++i)
    {
      const auto u = double(i) / 10.0;
      for (size_t j = 0; j <= 10; ++j)
      {
        const auto v = double(j) / 10.0;
        CAPTURE(u, v);
        CHECK(
          vm::approx{resampled.evaluateAt(u, v), 1e-9}
          == asymmetricPatch.evaluateAt(u, v));
      }
    }
  }

  SECTION(
    "fewer point counts across multiple old sub-surfaces only fits to the relevant "
    "boundaries")
  {
    // The old patch has 3 column sub-surfaces (boundaries at 1/3 and 2/3) but only 1 row
    // sub-surface, so the row direction stays unchanged while resampling to 2 column
    // sub-surfaces forces each new column sub-surface to fit against exactly one of the
    // two old boundaries while excluding the other (the new boundary at u = 0.5 lies
    // between them), exercising the boundary-splitting logic that the other sections
    // above never reach.

    // clang-format off
    const auto patchWithManySubSurfaces = BezierPatch{3, 7, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0}, {3, 0, 2}, {4, 0, 0}, {5, 0, 1}, {6, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1}, {3, 1, 3}, {4, 1, 1}, {5, 1, 2}, {6, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0}, {3, 2, 2}, {4, 2, 0}, {5, 2, 1}, {6, 2, 0},
    }, "material"};
    // clang-format on

    const auto resampled = resamplePatch(patchWithManySubSurfaces, 3, 5);

    REQUIRE(resampled.pointRowCount() == 3u);
    REQUIRE(resampled.pointColumnCount() == 5u);

    // u = 0 and u = 1 are the patch's own domain boundaries, so every row's fitted curve
    // must reduce to the original boundary control point there, modulo floating point
    // error from the quadrature-based fit (only rows 0 and 2 are exact corners, computed
    // by direct surface evaluation; row 1 is itself fitted, so it can only match
    // approximately).
    for (size_t row = 0; row < 3u; ++row)
    {
      CAPTURE(row);
      CHECK(
        vm::approx{resampled.controlPoint(row, 0), 1e-9}
        == patchWithManySubSurfaces.controlPoint(row, 0));
      CHECK(
        vm::approx{resampled.controlPoint(row, 4), 1e-9}
        == patchWithManySubSurfaces.controlPoint(row, 6));
    }

    // The middle column of the new grid (u = 0.5) is a true corner only for the even
    // (row-corner) rows; there it must lie exactly on the original surface.
    for (const auto row : {size_t{0}, size_t{2}})
    {
      const auto v = double(row) / 2.0;
      CAPTURE(row);
      CHECK(
        vm::approx{resampled.controlPoint(row, 2), 1e-9}
        == patchWithManySubSurfaces.evaluateAt(0.5, v));
    }

    // The two new edge midpoints (u = 0.25 and u = 0.75) are independently computed below
    // by exact symbolic integration of the same L2 fit, deliberately not reusing the
    // production code's own quadrature. The new boundary at u = 0.5 sits strictly between
    // the old boundaries at 1/3 and 2/3, so the [0, 0.5] fit must include only the 1/3
    // boundary and the [0.5, 1] fit must include only the 2/3 boundary; using the wrong
    // sub-surface count (e.g. mixing up rows and columns) would silently skip that split
    // and miss this exact value while staying within the very loose surface-closeness
    // tolerance used below.
    for (const auto row : {size_t{0}, size_t{2}})
    {
      const auto y = double(row);
      CAPTURE(row);
      CHECK(
        vm::approx{resampled.controlPoint(row, 1), 1e-9}
        == BezierPatch::Point{1.5, y, 7.0 / 27.0});
      CHECK(
        vm::approx{resampled.controlPoint(row, 3), 1e-9}
        == BezierPatch::Point{4.5, y, 7.0 / 27.0});
    }

    for (size_t i = 0; i <= 10; ++i)
    {
      const auto u = double(i) / 10.0;
      for (size_t j = 0; j <= 10; ++j)
      {
        const auto v = double(j) / 10.0;
        CAPTURE(u, v);
        CHECK(
          vm::approx{resampled.evaluateAt(u, v), 0.5}
          == patchWithManySubSurfaces.evaluateAt(u, v));
      }
    }
  }
}

} // namespace tb::mdl
