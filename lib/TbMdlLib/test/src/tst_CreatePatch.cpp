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
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/CatchConfig.h"
#include "mdl/CreatePatch.h"
#include "mdl/MapFormat.h"

#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("createPatch")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  const auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

  SECTION("n=4")
  {
    const auto brush = builder.createCube(64.0, "material") | kdl::value();

    const auto faceIndex = brush.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto& face = brush.face(*faceIndex);

    CHECK(
      createPatch(face, 3, 3)
      == std::vector<BezierPatch>{BezierPatch{
        3,
        3,
        {
          {32, -32, 32, 32, 32},
          {0, -32, 32, 0, 32},
          {-32, -32, 32, -32, 32},
          {32, 0, 32, 32, 0},
          {0, 0, 32, 0, 0},
          {-32, 0, 32, -32, 0},
          {32, 32, 32, 32, -32},
          {0, 32, 32, 0, -32},
          {-32, 32, 32, -32, -32},
        },
        "material"}});
  }

  SECTION("n=3")
  {
    const auto brush = builder.createBrush(
                         std::vector<vm::vec3d>{
                           {-4, 4, -8},
                           {-4, -4, -8},
                           {4, -4, -8},
                           {-4, 4, 8},
                           {-4, -4, 8},
                           {4, -4, 8},
                         },
                         "material")
                       | kdl::value();

    const auto faceIndex = brush.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto& face = brush.face(*faceIndex);

    CHECK(
      createPatch(face, 3, 3)
      == std::vector<BezierPatch>{BezierPatch{
        3,
        3,
        {
          {-4, -4, 8, -4, 4},
          {-4, 0, 8, -4, 0},
          {-4, 4, 8, -4, -4},
          {0, -4, 8, 0, 4},
          {0, -2, 8, 0, 2},
          {0, 0, 8, 0, 0},
          {4, -4, 8, 4, 4},
          {4, -4, 8, 4, 4},
          {4, -4, 8, 4, 4},
        },
        "material"}});
  }

  SECTION("n=5")
  {
    const auto brush = builder.createBrush(
                         std::vector<vm::vec3d>{
                           {-6, 2, -8},
                           {-4, -4, -8},
                           {2, -5, -8},
                           {6, -1, -8},
                           {1, 5, -8},
                           {-6, 2, 8},
                           {-4, -4, 8},
                           {2, -5, 8},
                           {6, -1, 8},
                           {1, 5, 8},
                         },
                         "material")
                       | kdl::value();

    const auto faceIndex = brush.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto& face = brush.face(*faceIndex);

    CHECK(
      createPatch(face, 3, 3)
      == std::vector<BezierPatch>{
        BezierPatch{
          3,
          3,
          {
            {-4, -4, 8, -4, 4},
            {-5, -1, 8, -5, 1},
            {-6, 2, 8, -6, -2},
            {1, -2.5, 8, 1, 2.5},
            {-0.75, 0.5, 8, -0.75, -0.5},
            {-2.5, 3.5, 8, -2.5, -3.5},
            {6, -1, 8, 6, 1},
            {3.5, 2, 8, 3.5, -2},
            {1, 5, 8, 1, -5},
          },
          "material"},
        BezierPatch{
          3,
          3,
          {
            {6, -1, 8, 6, 1},
            {4, -3, 8, 4, 3},
            {2, -5, 8, 2, 5},
            {1, -2.5, 8, 1, 2.5},
            {0, -3.5, 8, 0, 3.5},
            {-1, -4.5, 8, -1, 4.5},
            {-4, -4, 8, -4, 4},
            {-4, -4, 8, -4, 4},
            {-4, -4, 8, -4, 4},
          },
          "material"},
      });
  }

  SECTION("n=6")
  {
    const auto brush = builder.createBrush(
                         std::vector<vm::vec3d>{
                           {-6, 0, -8},
                           {-3, -5, -8},
                           {3, -5, -8},
                           {6, 0, -8},
                           {3, 5, -8},
                           {-3, 5, -8},
                           {-6, 0, 8},
                           {-3, -5, 8},
                           {3, -5, 8},
                           {6, 0, 8},
                           {3, 5, 8},
                           {-3, 5, 8},
                         },
                         "material")
                       | kdl::value();

    const auto faceIndex = brush.findFace(vm::vec3d{0, 0, 1});
    REQUIRE(faceIndex);

    const auto& face = brush.face(*faceIndex);

    CHECK(
      createPatch(face, 3, 3)
      == std::vector<BezierPatch>{
        BezierPatch{
          3,
          3,
          {
            {3, -5, 8, 3, 5},
            {0, -5, 8, 0, 5},
            {-3, -5, 8, -3, 5},
            {0, 0, 8, 0, 0},
            {-2.25, -1.25, 8, -2.25, 1.25},
            {-4.5, -2.5, 8, -4.5, 2.5},
            {-3, 5, 8, -3, -5},
            {-4.5, 2.5, 8, -4.5, -2.5},
            {-6, 0, 8, -6, 0},
          },
          "material"},
        BezierPatch{
          3,
          3,
          {
            {-6, 0, 8, -6, 0},
            {-4.5, 2.5, 8, -4.5, -2.5},
            {-3, 5, 8, -3, -5},
            {0, 0, 8, 0, 0},
            {0, 2.5, 8, 0, -2.5},
            {0, 5, 8, 0, -5},
            {6, 0, 8, 6, 0},
            {4.5, 2.5, 8, 4.5, -2.5},
            {3, 5, 8, 3, -5},
          },
          "material"},
      });
  }
}

} // namespace tb::mdl
