/*
 Copyright (C) 2010 Kristian Duske

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

#include "vm/approx.h"
#include "vm/mat_ext.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("BezierPatch")
{
  SECTION("evaluate")
  {
    using T = std::tuple<
      size_t,
      size_t,
      std::vector<BezierPatch::Point>,
      size_t,
      std::vector<BezierPatch::Point>>;

    // clang-format off
    const auto
    [w, h, controlPoints,                       subdiv, expectedGrid ] = GENERATE(values<T>({
    {3, 3, { {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
             {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
             {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, 2,      { {0, 0,   0},     {0.5, 0,   0.375}, {1, 0,   0.5},   {1.5, 0,   0.375}, {2, 0,   0}, 
                                                          {0, 0.5, 0.375}, {0.5, 0.5, 0.75},  {1, 0.5, 0.875}, {1.5, 0.5, 0.75},  {2, 0.5, 0.375}, 
                                                          {0, 1,   0.5},   {0.5, 1,   0.875}, {1, 1,   1},     {1.5, 1,   0.875}, {2, 1,   0.5}, 
                                                          {0, 1.5, 0.375}, {0.5, 1.5, 0.75},  {1, 1.5, 0.875}, {1.5, 1.5, 0.75},  {2, 1.5, 0.375}, 
                                                          {0, 2,   0},     {0.5, 2,   0.375}, {1, 2,   0.5},   {1.5, 2,   0.375}, {2, 2,   0} } }
    }));
    // clang-format on

    const auto patch = BezierPatch{w, h, controlPoints, ""};
    CHECK(patch.evaluate(subdiv) == expectedGrid);
  }

  SECTION("evaluateAt")
  {
    SECTION("matches evaluate for a single surface")
    {
      // clang-format off
      const auto patch = BezierPatch{3, 3, {
        {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
        {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
        {0, 2, 0}, {1, 2, 1}, {2, 2, 0},
      }, ""};
      // clang-format on

      // clang-format off
      const auto
      [u,    v,    expectedPoint] = GENERATE(table<double, double, BezierPatch::Point>({
      {0,    0,    {0,   0,   0    }},
      {0.25, 0,    {0.5, 0,   0.375}},
      {0.5,  0,    {1,   0,   0.5  }},
      {1,    0,    {2,   0,   0    }},
      {0,    0.5,  {0,   1,   0.5  }},
      {0.5,  0.5,  {1,   1,   1    }},
      {0.75, 0.25, {1.5, 0.5, 0.75 }},
      {1,    1,    {2,   2,   0    }},
      }));
      // clang-format on

      CAPTURE(u, v);
      CHECK(patch.evaluateAt(u, v) == expectedPoint);
    }

    SECTION("matches evaluate across multiple surfaces")
    {
      // clang-format off
      const auto patch = BezierPatch{5, 5, {
        {0, 0, 0}, {1, 0, 1}, {2, 0, 4}, {3, 0, 1}, {4, 0, 0},
        {0, 1, 1}, {1, 1, 2}, {2, 1, 3}, {3, 1, 2}, {4, 1, 1},
        {0, 2, 0}, {1, 2, 3}, {2, 2, 5}, {3, 2, 3}, {4, 2, 0},
        {0, 3, 1}, {1, 3, 2}, {2, 3, 3}, {3, 3, 2}, {4, 3, 1},
        {0, 4, 0}, {1, 4, 1}, {2, 4, 4}, {3, 4, 1}, {4, 4, 0},
      }, ""};
      // clang-format on

      const auto subdiv = size_t{2};
      const auto grid = patch.evaluate(subdiv);

      const auto gridPointRowCount = patch.surfaceRowCount() * (1u << subdiv) + 1u;
      const auto gridPointColumnCount = patch.surfaceColumnCount() * (1u << subdiv) + 1u;

      for (size_t row = 0; row < gridPointRowCount; ++row)
      {
        const auto v = double(row) / double(gridPointRowCount - 1u);
        for (size_t col = 0; col < gridPointColumnCount; ++col)
        {
          const auto u = double(col) / double(gridPointColumnCount - 1u);
          CAPTURE(row, col, u, v);
          CHECK(
            vm::approx{patch.evaluateAt(u, v), 1e-9}
            == grid[row * gridPointColumnCount + col]);
        }
      }
    }

    SECTION("matches evaluate for an asymmetric patch")
    {
      // clang-format off
      const auto patch = BezierPatch{3, 7, {
        {0, 0, 0}, {1, 0, 1}, {2, 0, 0}, {3, 0, 2}, {4, 0, 0}, {5, 0, 1}, {6, 0, 0},
        {0, 1, 1}, {1, 1, 2}, {2, 1, 1}, {3, 1, 3}, {4, 1, 1}, {5, 1, 2}, {6, 1, 1},
        {0, 2, 0}, {1, 2, 1}, {2, 2, 0}, {3, 2, 2}, {4, 2, 0}, {5, 2, 1}, {6, 2, 0},
      }, ""};
      // clang-format on

      const auto subdiv = size_t{2};
      const auto grid = patch.evaluate(subdiv);

      const auto gridPointRowCount = patch.surfaceRowCount() * (1u << subdiv) + 1u;
      const auto gridPointColumnCount = patch.surfaceColumnCount() * (1u << subdiv) + 1u;

      for (size_t row = 0; row < gridPointRowCount; ++row)
      {
        const auto v = double(row) / double(gridPointRowCount - 1u);
        for (size_t col = 0; col < gridPointColumnCount; ++col)
        {
          const auto u = double(col) / double(gridPointColumnCount - 1u);
          CAPTURE(row, col, u, v);
          CHECK(
            vm::approx{patch.evaluateAt(u, v), 1e-9}
            == grid[row * gridPointColumnCount + col]);
        }
      }
    }
  }

  SECTION("transform")
  {
    // clang-format off
    auto patch = BezierPatch{3, 3, { 
      {-1,  1,  0}, {0,  1,  1}, {1,  1,  2},
      {-1,  0, -1}, {0,  0,  0}, {1,  0,  1},
      {-1, -1, -2}, {0, -1, -1}, {1, -1,  0},
     }, ""};
    // clang-format on

    SECTION("translate")
    {
      patch.transform(vm::translation_matrix(vm::vec3d{2.0, 0.0, 0.0}));

      // clang-format off
      CHECK(patch.controlPoints() == std::vector<BezierPatch::Point>{
        {1,  1,  0}, {2,  1,  1}, {3,  1,  2},
        {1,  0, -1}, {2,  0,  0}, {3,  0,  1},
        {1, -1, -2}, {2, -1, -1}, {3, -1,  0},
          });
      // clang-format on
    }

    SECTION("mirror")
    {
      using T = std::tuple<vm::axis::type, std::vector<BezierPatch::Point>>;

      // clang-format off
      const auto
      [axis, expectedPoints] = GENERATE(values<T>({
      {vm::axis::x, {
        {-1,  1,  2}, {0,  1,  1}, {1,  1,  0},
        {-1,  0,  1}, {0,  0,  0}, {1,  0, -1},
        {-1, -1,  0}, {0, -1, -1}, {1, -1, -2},
        }},
      {vm::axis::y, {
        { 1, -1,  2}, {0, -1,  1}, {-1, -1,  0},
        { 1,  0,  1}, {0,  0,  0}, {-1,  0, -1},
        { 1,  1,  0}, {0,  1, -1}, {-1,  1, -2},
        }},
      {vm::axis::z, {
        { 1,  1, -2}, {0,  1, -1}, {-1,  1,  0},
        { 1,  0, -1}, {0,  0,  0}, {-1,  0,  1},
        { 1, -1,  0}, {0, -1,  1}, {-1, -1,  2},
        }},
      }));
      // clang-format on

      CAPTURE(axis);

      patch.transform(vm::mirror_matrix<double>(axis));

      CHECK(patch.controlPoints() == expectedPoints);
    }
  }

  SECTION("transformControlPoints")
  {
    // clang-format off
    auto patch = BezierPatch{3, 3, {
      {-1,  1,  0}, {0,  1,  1}, {1,  1,  2},
      {-1,  0, -1}, {0,  0,  0}, {1,  0,  1},
      {-1, -1, -2}, {0, -1, -1}, {1, -1,  0},
    }, ""};
    // clang-format on

    SECTION("updates matching control points and recomputes bounds")
    {
      patch.transformControlPoints(
        std::set<vm::vec3d>{{1, 1, 2}}, vm::translation_matrix(vm::vec3d{2, 0, 0}));

      // clang-format off
      CHECK(patch.controlPoints() == std::vector<BezierPatch::Point>{
        {-1,  1,  0}, {0,  1,  1}, {3,  1,  2},
        {-1,  0, -1}, {0,  0,  0}, {1,  0,  1},
        {-1, -1, -2}, {0, -1, -1}, {1, -1,  0},
      });
      // clang-format on

      CHECK(patch.bounds() == vm::bbox3d{{-1, -1, -2}, {3, 1, 2}});
    }

    SECTION("does nothing when no control point has the given position")
    {
      const auto originalControlPoints = patch.controlPoints();
      const auto originalBounds = patch.bounds();

      patch.transformControlPoints(
        std::set<vm::vec3d>{{100, 100, 100}}, vm::translation_matrix(vm::vec3d{2, 0, 0}));

      CHECK(patch.controlPoints() == originalControlPoints);
      CHECK(patch.bounds() == originalBounds);
    }

    SECTION("does nothing when the given position set is empty")
    {
      const auto originalControlPoints = patch.controlPoints();
      const auto originalBounds = patch.bounds();

      patch.transformControlPoints(
        std::set<vm::vec3d>{}, vm::translation_matrix(vm::vec3d{2, 0, 0}));

      CHECK(patch.controlPoints() == originalControlPoints);
      CHECK(patch.bounds() == originalBounds);
    }
  }
}

} // namespace tb::mdl
