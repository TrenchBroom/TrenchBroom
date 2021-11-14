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

#include "Model/BezierPatch.h"

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <tuple>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
namespace Model {
TEST_CASE("BezierPatch.evaluate") {
  using T = std::tuple<
    size_t, size_t, std::vector<BezierPatch::Point>, size_t, std::vector<BezierPatch::Point>>;

  // clang-format off
  const auto
  [w, h, controlPoints,                                     subdiv, expectedGrid ] = GENERATE(values<T>({
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

TEST_CASE("BezierPatch.transform") {
  // clang-format off
  auto patch = BezierPatch{3, 3, { {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                                   {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                                   {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, ""};
  // clang-format on

  patch.transform(vm::translation_matrix(vm::vec3d{2.0, 0.0, 0.0}));

  // clang-format off
  CHECK(patch.controlPoints() == std::vector<BezierPatch::Point>{ {2, 0, 0}, {3, 0, 1}, {4, 0, 0},
                                                                  {2, 1, 1}, {3, 1, 2}, {4, 1, 1},
                                                                  {2, 2, 0}, {3, 2, 1}, {4, 2, 0} });
  // clang-format on
}
} // namespace Model
} // namespace TrenchBroom
