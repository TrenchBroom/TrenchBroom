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

  SECTION("createArch")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    SECTION("Arch with different circle modes")
    {
      const auto bounds = vm::bbox3d{{-128, -64, 0}, {128, 64, 64}};

      SECTION("Edge aligned")
      {
        builder.createArch(bounds, 16.0, EdgeAlignedCircle{16}, vm::axis::y, "someName")
          | kdl::transform([&](const auto& brushes) {
              REQUIRE(brushes.size() == 9u);
              CHECK(getMergedBounds(brushes) == bounds);

              // Check only one brush to avoid clutter.
              const auto expectedBrush = makeBrush({
                {{112, -64, 6.9638421181958083},
                 {112, 64, 0},
                 {112, 64, 6.9638421181958083}},
                {{128, 64, 12.730391512298111},
                 {112, -64, 6.9638421181958083},
                 {112, 64, 6.9638421181958083}},
                {{128, -64, 12.730391512298111},
                 {112, -64, 0},
                 {112, -64, 6.9638421181958083}},
                {{128, 64, 0}, {112, -64, 0}, {128, -64, 0}},
                {{128, 64, 12.730391512298111}, {112, 64, 0}, {128, 64, 0}},
                {{128, 64, 12.730391512298111},
                 {128, -64, 0},
                 {128, -64, 12.730391512298111}},
              });

              CHECK_THAT(
                brushes, Contains(MatchesBrushVertices(expectedBrush, vertexEpsilon)));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }

      SECTION("Vertex aligned")
      {
        builder.createArch(bounds, 16.0, VertexAlignedCircle{16}, vm::axis::y, "someName")
          | kdl::transform([&](const auto& brushes) {
              REQUIRE(brushes.size() == 8u);
              CHECK(getMergedBounds(brushes) == bounds);

              // Check only one brush to avoid clutter.
              const auto expectedBrush = makeBrush({
                {{105.05776674599576, 64, 14.384730312563047},
                 {110.78036826717545, -64, 0},
                 {110.78036826717545, 64, 0}},
                {{118.2565801614447, 64, 24.491739671365746},
                 {105.05776674599576, -64, 14.384730312563047},
                 {105.05776674599576, 64, 14.384730312563047}},
                {{128, -64, 0},
                 {105.05776674599576, -64, 14.384730312563047},
                 {118.2565801614447, -64, 24.491739671365746}},
                {{128, 64, 0}, {110.78036826717545, -64, 0}, {128, -64, 0}},
                {{128, 64, 0},
                 {105.05776674599576, 64, 14.384730312563047},
                 {110.78036826717545, 64, 0}},
                {{128, 64, 0},
                 {118.2565801614447, -64, 24.491739671365746},
                 {118.2565801614447, 64, 24.491739671365746}},
              });

              CHECK_THAT(
                brushes, Contains(MatchesBrushVertices(expectedBrush, vertexEpsilon)));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }

      SECTION("Scalable")
      {
        builder.createArch(bounds, 16.0, ScalableCircle{0}, vm::axis::y, "someName")
          | kdl::transform([&](const auto& brushes) {
              REQUIRE(brushes.size() == 7u);
              CHECK(getMergedBounds(brushes) == bounds);

              // Check only one brush to avoid clutter.
              const auto expectedBrush = makeBrush({
                {{112, -64, 12}, {112, 64, 0}, {112, 64, 12}},
                {{128, 64, 16}, {112, -64, 12}, {112, 64, 12}},
                {{128, -64, 16}, {112, -64, 0}, {112, -64, 12}},
                {{128, 64, 0}, {112, -64, 0}, {128, -64, 0}},
                {{128, 64, 16}, {112, 64, 0}, {128, 64, 0}},
                {{128, 64, 16}, {128, -64, 0}, {128, -64, 16}},
              });

              CHECK(std::ranges::find(brushes, expectedBrush) != brushes.end());
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }
    }

    SECTION("Arch with different tunnel axes")
    {
      SECTION("X axis")
      {
        const auto bounds = vm::bbox3d{{-64, -128, 0}, {64, 128, 64}};
        builder.createArch(bounds, 16.0, EdgeAlignedCircle{8}, vm::axis::x, "someName")
          | kdl::transform([&](const auto& brushes) {
              REQUIRE(brushes.size() == 5u);
              CHECK(getMergedBounds(brushes) == bounds);

              // Check only one brush to avoid clutter.
              const auto expectedBrush = makeBrush({
                {{-64, 112, 16.621124171879767},
                 {-64, 128, 0},
                 {-64, 128, 26.509667991878086}},
                {{64, 112, 16.621124171879767},
                 {-64, 112, 0},
                 {-64, 112, 16.621124171879767}},
                {{64, 128, 26.509667991878086},
                 {-64, 112, 16.621124171879767},
                 {-64, 128, 26.509667991878086}},
                {{64, 128, 0}, {-64, 112, 0}, {64, 112, 0}},
                {{64, 128, 26.509667991878086}, {-64, 128, 0}, {64, 128, 0}},
                {{64, 128, 26.509667991878086},
                 {64, 112, 0},
                 {64, 112, 16.621124171879767}},
              });

              CHECK_THAT(
                brushes, Contains(MatchesBrushVertices(expectedBrush, vertexEpsilon)));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }

      SECTION("Y axis")
      {
        const auto bounds = vm::bbox3d{{-128, -64, 0}, {128, 64, 64}};
        builder.createArch(bounds, 16.0, EdgeAlignedCircle{8}, vm::axis::y, "someName")
          | kdl::transform([&](const auto& brushes) {
              REQUIRE(brushes.size() == 5u);
              CHECK(getMergedBounds(brushes) == bounds);

              // Check only one brush to avoid clutter.
              const auto expectedBrush = makeBrush({
                {{112, -64, 16.621124171879767},
                 {112, 64, 0},
                 {112, 64, 16.621124171879767}},
                {{128, 64, 26.509667991878086},
                 {112, -64, 16.621124171879767},
                 {112, 64, 16.621124171879767}},
                {{128, -64, 26.509667991878086},
                 {112, -64, 0},
                 {112, -64, 16.621124171879767}},
                {{128, 64, 0}, {112, -64, 0}, {128, -64, 0}},
                {{128, 64, 26.509667991878086}, {112, 64, 0}, {128, 64, 0}},
                {{128, 64, 26.509667991878086},
                 {128, -64, 0},
                 {128, -64, 26.509667991878086}},
              });

              CHECK_THAT(
                brushes, Contains(MatchesBrushVertices(expectedBrush, vertexEpsilon)));
            })
          | kdl::transform_error([](const auto& e) { FAIL(e); });
      }
    }

    SECTION("Degenerate bounds do not error")
    {
      SECTION("Zero height")
      {
        CHECK(
          builder.createArch(
            vm::bbox3d{{-128, -64, 0}, {128, 64, 0}},
            16.0,
            EdgeAlignedCircle{12},
            vm::axis::x,
            "someName")
          == Result<std::vector<Brush>>{std::vector<Brush>{}});
      }

      SECTION("Zero span")
      {
        CHECK(
          builder.createArch(
            vm::bbox3d{{-128, 0, 0}, {128, 0, 64}},
            16.0,
            EdgeAlignedCircle{12},
            vm::axis::x,
            "someName")
          == Result<std::vector<Brush>>{std::vector<Brush>{}});
      }

      SECTION("Zero extrusion depth")
      {
        CHECK(
          builder.createArch(
            vm::bbox3d{{0, -64, 0}, {0, 64, 64}},
            16.0,
            EdgeAlignedCircle{12},
            vm::axis::x,
            "someName")
          == Result<std::vector<Brush>>{std::vector<Brush>{}});
      }
    }
  }
}

} // namespace tb::mdl
