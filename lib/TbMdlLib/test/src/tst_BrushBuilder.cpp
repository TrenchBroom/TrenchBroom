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

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/CatchConfig.h"
#include "mdl/MapFormat.h"
#include "mdl/Matchers.h"
#include "mdl/Polyhedron3.h"

#include "kd/range_fold.h"
#include "kd/ranges/to.h"
#include "kd/result.h"

#include "vm/constants.h"

#include <algorithm>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

auto makeFace(const std::tuple<vm::vec3d, vm::vec3d, vm::vec3d>& face)
{
  return BrushFace::create(
           std::get<0>(face),
           std::get<1>(face),
           std::get<2>(face),
           BrushFaceAttributes{"someName"},
           MapFormat::Standard)
         | kdl::value();
};

auto makeBrush(const std::vector<std::tuple<vm::vec3d, vm::vec3d, vm::vec3d>>& faces)
{
  return Brush::create(
           vm::bbox3d{8192.0},
           faces | std::views::transform(makeFace) | kdl::ranges::to<std::vector>())
         | kdl::value();
};

auto getMergedBounds(const std::vector<Brush>& brushes)
{
  return kdl::fold_left_first(
    brushes | std::views::transform([](const auto& brush) { return brush.bounds(); }),
    [](const auto& lhs, const auto& rhs) { return vm::merge(lhs, rhs); });
}

} // namespace

TEST_CASE("BrushBuilder")
{
  using Catch::Matchers::Contains;
  using Catch::Matchers::RangeEquals;

  const auto worldBounds = vm::bbox3d{8192.0};
  const auto vertexEpsilon = vm::Cd::almost_zero();

  SECTION("createCube")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    builder.createCube(128.0, "someName") | kdl::transform([](const auto& cube) {
      CHECK(cube.fullySpecified());
      CHECK(cube.bounds() == vm::bbox3d{-64.0, +64.0});

      CHECK_THAT(
        cube.faces() | std::views::transform([](const auto& face) {
          return face.attributes().materialName();
        }),
        RangeEquals(std::vector<std::string>{6u, "someName"}));
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("createCubeDefaults")
  {
    auto defaultAttribs = BrushFaceAttributes{"defaultMaterial"};
    defaultAttribs.setOffset({0.5f, 0.5f});
    defaultAttribs.setScale({0.5f, 0.5f});
    defaultAttribs.setRotation(45.0f);
    defaultAttribs.setSurfaceContents(1);
    defaultAttribs.setSurfaceFlags(2);
    defaultAttribs.setSurfaceValue(0.1f);
    defaultAttribs.setColor(RgbB{255, 255, 255});

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds, defaultAttribs};

    builder.createCube(128.0, "someName") | kdl::transform([&](const auto& cube) {
      CHECK(cube.fullySpecified());
      CHECK(cube.bounds() == vm::bbox3d{-64.0, +64.0});

      CHECK_THAT(
        cube.faces()
          | std::views::transform([](const auto& face) { return face.attributes(); }),
        RangeEquals(std::vector{6u, BrushFaceAttributes{"someName", defaultAttribs}}));
    }) | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("createBrushDefaults")
  {
    auto defaultAttribs = BrushFaceAttributes{"defaultMaterial"};
    defaultAttribs.setOffset({0.5f, 0.5f});
    defaultAttribs.setScale({0.5f, 0.5f});
    defaultAttribs.setRotation(45.0f);
    defaultAttribs.setSurfaceContents(1);
    defaultAttribs.setSurfaceFlags(2);
    defaultAttribs.setSurfaceValue(0.1f);
    defaultAttribs.setColor(RgbB{255, 255, 255});

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds, defaultAttribs};

    builder.createBrush(
      Polyhedron3{
        vm::vec3d{-64, -64, -64},
        vm::vec3d{-64, -64, +64},
        vm::vec3d{-64, +64, -64},
        vm::vec3d{-64, +64, +64},
        vm::vec3d{+64, -64, -64},
        vm::vec3d{+64, -64, +64},
        vm::vec3d{+64, +64, -64},
        vm::vec3d{+64, +64, +64},
      },
      "someName")
      | kdl::transform([&](const auto& brush) {
          CHECK(brush.fullySpecified());
          CHECK(brush.bounds() == vm::bbox3d{-64.0, +64.0});

          CHECK_THAT(
            brush.faces()
              | std::views::transform([](const auto& face) { return face.attributes(); }),
            RangeEquals(
              std::vector{6u, BrushFaceAttributes{"someName", defaultAttribs}}));
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("createCylinder")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    SECTION("Edge aligned cylinder")
    {
      builder.createCylinder(
        vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
        EdgeAlignedCircle{4},
        vm::axis::z,
        "someName")
        | kdl::transform([](const auto& cylinder) {
            CHECK(cylinder.bounds() == vm::bbox3d{{-32, -32, -32}, {32, 32, 32}});

            CHECK(
              cylinder
              == makeBrush({
                {{-32, -32, 32}, {-32, 32, -32}, {-32, 32, 32}},
                {{32, -32, 32}, {-32, -32, -32}, {-32, -32, 32}},
                {{32, 32, -32}, {-32, -32, -32}, {32, -32, -32}},
                {{32, 32, 32}, {-32, -32, 32}, {-32, 32, 32}},
                {{32, 32, 32}, {-32, 32, -32}, {32, 32, -32}},
                {{32, 32, 32}, {32, -32, -32}, {32, -32, 32}},
              }));
          })
        | kdl::transform_error([](const auto& e) { FAIL(e); });
    }

    SECTION("Scalable Cylinder")
    {
      SECTION("In square bounds")
      {
        builder.createCylinder(
          vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
          ScalableCircle{0},
          vm::axis::z,
          "someName")
          | kdl::transform([](const auto& cylinder) {
              CHECK(cylinder.bounds() == vm::bbox3d{{-32, -32, -32}, {32, 32, 32}});

              CHECK(
                cylinder
                == makeBrush({
                  {{-32, -8, 32}, {-32, 8, -32}, {-32, 8, 32}},
                  {{-24, -24, 32}, {-32, -8, -32}, {-32, -8, 32}},
                  {{-24, 24, 32}, {-32, 8, -32}, {-24, 24, -32}},
                  {{-8, -32, 32}, {-24, -24, -32}, {-24, -24, 32}},
                  {{-8, 32, 32}, {-24, 24, -32}, {-8, 32, -32}},
                  {{8, -32, 32}, {-8, -32, -32}, {-8, -32, 32}},
                  {{32, 8, -32}, {24, -24, -32}, {32, -8, -32}},
                  {{32, 8, 32}, {8, 32, 32}, {24, 24, 32}},
                  {{8, 32, 32}, {-8, 32, -32}, {8, 32, -32}},
                  {{24, -24, 32}, {8, -32, -32}, {8, -32, 32}},
                  {{24, 24, 32}, {8, 32, -32}, {24, 24, -32}},
                  {{32, -8, 32}, {24, -24, -32}, {24, -24, 32}},
                  {{32, 8, 32}, {24, 24, -32}, {32, 8, -32}},
                  {{32, 8, 32}, {32, -8, -32}, {32, -8, 32}},
                }));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }

      SECTION("In rectangular bounds")
      {
        builder.createCylinder(
          vm::bbox3d{{-64, -32, -32}, {64, 32, 32}},
          ScalableCircle{0},
          vm::axis::z,
          "someName")
          | kdl::transform([](const auto& cylinder) {
              CHECK(cylinder.bounds() == vm::bbox3d{{-64, -32, -32}, {64, 32, 32}});

              CHECK(
                cylinder
                == makeBrush({
                  {{-64, -8, 32}, {-64, 8, -32}, {-64, 8, 32}},
                  {{-56, -24, 32}, {-64, -8, -32}, {-64, -8, 32}},
                  {{-56, 24, 32}, {-64, 8, -32}, {-56, 24, -32}},
                  {{-40, -32, 32}, {-56, -24, -32}, {-56, -24, 32}},
                  {{-40, 32, 32}, {-56, 24, -32}, {-40, 32, -32}},
                  {{40, -32, 32}, {-40, -32, -32}, {-40, -32, 32}},
                  {{64, 8, -32}, {56, -24, -32}, {64, -8, -32}},
                  {{64, 8, 32}, {40, 32, 32}, {56, 24, 32}},
                  {{40, 32, 32}, {-40, 32, -32}, {40, 32, -32}},
                  {{56, -24, 32}, {40, -32, -32}, {40, -32, 32}},
                  {{56, 24, 32}, {40, 32, -32}, {56, 24, -32}},
                  {{64, -8, 32}, {56, -24, -32}, {56, -24, 32}},
                  {{64, 8, 32}, {56, 24, -32}, {64, 8, -32}},
                  {{64, 8, 32}, {64, -8, -32}, {64, -8, 32}},
                }));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }
    }
  }

  SECTION("createHollowCylinder")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    const auto bounds = vm::bbox3d{{-32, -32, -32}, {32, 32, 32}};
    builder.createHollowCylinder(
      bounds, 8.0, EdgeAlignedCircle{8}, vm::axis::z, "someName")
      | kdl::transform([&](const auto& brushes) {
          REQUIRE(brushes.size() == 8u);
          CHECK(getMergedBounds(brushes) == bounds);

          // Check only one brush to avoid clutter.
          const auto outerOffset = 13.254833995939043;
          const auto innerMin = -9.9411254969542853;
          const auto innerMax = 9.9411254969542835;
          const auto expectedBrush = makeBrush({
            {{24, innerMin, 32}, {24, innerMax, -32}, {24, innerMax, 32}},
            {{24, innerMax, -32}, {32, outerOffset, 32}, {24, innerMax, 32}},
            {{32, -outerOffset, 32}, {24, innerMin, -32}, {24, innerMin, 32}},
            {{32, outerOffset, -32}, {24, innerMin, -32}, {32, -outerOffset, -32}},
            {{32, outerOffset, 32}, {24, innerMin, 32}, {24, innerMax, 32}},
            {{32, outerOffset, 32}, {32, -outerOffset, -32}, {32, -outerOffset, 32}},
          });

          CHECK_THAT(
            brushes, Contains(MatchesBrushVertices(expectedBrush, vertexEpsilon)));
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }
}

} // namespace tb::mdl
