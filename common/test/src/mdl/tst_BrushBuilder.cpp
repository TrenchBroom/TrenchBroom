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
#include "mdl/MapFormat.h"
#include "mdl/Polyhedron3.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"

#include <string>

#include "Catch2.h"

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
           vm::bbox3d{8192.0}, faces | std::views::transform(makeFace) | kdl::to_vector)
         | kdl::value();
};

} // namespace

TEST_CASE("BrushBuilder")
{
  const auto worldBounds = vm::bbox3d{8192.0};

  SECTION("createCube")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    const auto cube = builder.createCube(128.0, "someName") | kdl::value();
    CHECK(cube.fullySpecified());
    CHECK(cube.bounds() == vm::bbox3d{-64.0, +64.0});

    const auto faces = cube.faces();
    CHECK(faces.size() == 6u);

    for (size_t i = 0; i < faces.size(); ++i)
    {
      CHECK(faces[i].attributes().materialName() == "someName");
    }
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
    defaultAttribs.setColor(Color{255, 255, 255, 255});

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds, defaultAttribs};

    const auto cube = builder.createCube(128.0, "someName") | kdl::value();
    CHECK(cube.fullySpecified());
    CHECK(cube.bounds() == vm::bbox3d{-64.0, +64.0});

    const auto faces = cube.faces();
    CHECK(faces.size() == 6u);

    for (size_t i = 0; i < faces.size(); ++i)
    {
      CHECK(faces[i].attributes().materialName() == "someName");
      CHECK(faces[i].attributes().offset() == vm::vec2f{0.5f, 0.5f});
      CHECK(faces[i].attributes().scale() == vm::vec2f{0.5f, 0.5f});
      CHECK(faces[i].attributes().rotation() == 45.0f);
      CHECK(faces[i].attributes().surfaceContents() == 1);
      CHECK(faces[i].attributes().surfaceFlags() == 2);
      CHECK(faces[i].attributes().surfaceValue() == 0.1f);
      CHECK(faces[i].attributes().color() == Color{255, 255, 255, 255});
    }
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
    defaultAttribs.setColor(Color{255, 255, 255, 255});

    auto builder = BrushBuilder{MapFormat::Standard, worldBounds, defaultAttribs};

    const auto brush = builder.createBrush(
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
                       | kdl::value();
    CHECK(brush.fullySpecified());
    CHECK(brush.bounds() == vm::bbox3d{-64.0, +64.0});

    const auto faces = brush.faces();
    CHECK(faces.size() == 6u);

    for (size_t i = 0; i < faces.size(); ++i)
    {
      CHECK(faces[i].attributes().materialName() == "someName");
      CHECK(faces[i].attributes().offset() == vm::vec2f{0.5f, 0.5f});
      CHECK(faces[i].attributes().scale() == vm::vec2f{0.5f, 0.5f});
      CHECK(faces[i].attributes().rotation() == 45.0f);
      CHECK(faces[i].attributes().surfaceContents() == 1);
      CHECK(faces[i].attributes().surfaceFlags() == 2);
      CHECK(faces[i].attributes().surfaceValue() == 0.1f);
      CHECK(faces[i].attributes().color() == Color{255, 255, 255, 255});
    }
  }

  SECTION("createCylinder")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};

    SECTION("Edge aligned cylinder")
    {
      const auto cylinder = builder.createCylinder(
        vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
        EdgeAlignedCircle{4},
        vm::axis::z,
        "someName");

      CHECK(
        cylinder
        == Result<Brush>{makeBrush({
          {{-32, -32, 32}, {-32, 32, -32}, {-32, 32, 32}},
          {{32, -32, 32}, {-32, -32, -32}, {-32, -32, 32}},
          {{32, 32, -32}, {-32, -32, -32}, {32, -32, -32}},
          {{32, 32, 32}, {-32, -32, 32}, {-32, 32, 32}},
          {{32, 32, 32}, {-32, 32, -32}, {32, 32, -32}},
          {{32, 32, 32}, {32, -32, -32}, {32, -32, 32}},
        })});
    }

    SECTION("Scalable Cylinder")
    {
      SECTION("In square bounds")
      {
        const auto cylinder = builder.createCylinder(
          vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
          ScalableCircle{0},
          vm::axis::z,
          "someName");

        CHECK(
          cylinder
          == Result<Brush>{makeBrush({
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
          })});
      }

      SECTION("In rectangular bounds")
      {
        const auto cylinder = builder.createCylinder(
          vm::bbox3d{{-64, -32, -32}, {64, 32, 32}},
          ScalableCircle{0},
          vm::axis::z,
          "someName");

        CHECK(
          cylinder
          == Result<Brush>{makeBrush({
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
          })});
      }
    }
  }


  SECTION("createHollowCylinder")
  {
    auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
    const auto cylinder = builder.createHollowCylinder(
      vm::bbox3d{{-32, -32, -32}, {32, 32, 32}},
      8.0,
      EdgeAlignedCircle{8},
      vm::axis::z,
      "someName");

    CHECK(cylinder.is_success());
    CHECK(cylinder.value().size() == 8);
  }
}

} // namespace tb::mdl
