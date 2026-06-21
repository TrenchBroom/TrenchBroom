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

#include "mdl/BrushBuilder.h"
#include "mdl/MapFormat.h"
#include "mdl/Matchers.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("MatchesBrushVertices")
{
  const auto builder = BrushBuilder{MapFormat::Standard, vm::bbox3d{8192.0}};

  const auto expected =
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {1, 1, 1}}, "material") | kdl::value();
  const auto actual =
    builder.createCuboid(vm::bbox3d{{0.01, 0.01, 0.01}, {1.01, 1.01, 1.01}}, "material")
    | kdl::value();

  const auto differentVertexCount =
    builder.createBrush(
      std::vector<vm::vec3d>{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, "material")
    | kdl::value();

  CHECK_THAT(actual, MatchesBrushVertices(expected, 0.02));
  CHECK_THAT(actual, !MatchesBrushVertices(expected, 0.005));
  CHECK_THAT(differentVertexCount, !MatchesBrushVertices(expected, 0.02));
}

} // namespace tb::mdl
