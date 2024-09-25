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

#include "Error.h" // IWYU pragma: keep
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/MapFormat.h"
#include "Model/Polyhedron.h"
#include "Model/Polyhedron3.h"

#include "kdl/range_to_vector.h"
#include "kdl/result.h"

#include <string>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::Model
{
namespace
{
auto makeFace(const std::tuple<vm::vec3, vm::vec3, vm::vec3>& face)
{
  return BrushFace::create(
           std::get<0>(face),
           std::get<1>(face),
           std::get<2>(face),
           BrushFaceAttributes{"someName"},
           MapFormat::Standard)
         | kdl::value();
};

auto makeBrush(const std::vector<std::tuple<vm::vec3, vm::vec3, vm::vec3>>& faces)
{
  return Brush::create(
           vm::bbox3{8192.0}, faces | std::views::transform(makeFace) | kdl::to_vector)
         | kdl::value();
};
} // namespace

TEST_CASE("BrushBuilderTest.createCube")
{
  const auto worldBounds = vm::bbox3{8192.0};

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

TEST_CASE("BrushBuilderTest.createCubeDefaults")
{
  const auto worldBounds = vm::bbox3{8192.0};

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

TEST_CASE("BrushBuilderTest.createBrushDefaults")
{
  const auto worldBounds = vm::bbox3{8192.0};

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
                         vm::vec3{-64, -64, -64},
                         vm::vec3{-64, -64, +64},
                         vm::vec3{-64, +64, -64},
                         vm::vec3{-64, +64, +64},
                         vm::vec3{+64, -64, -64},
                         vm::vec3{+64, -64, +64},
                         vm::vec3{+64, +64, -64},
                         vm::vec3{+64, +64, +64},
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

TEST_CASE("BrushBuilderTest.createCylinder")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  const auto cylinder = builder.createCylinder(
    vm::bbox3{{-32, -32, -32}, {32, 32, 32}},
    4,
    RadiusMode::ToEdge,
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

TEST_CASE("BrushBuilderTest.createHollowCylinder")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  const auto cylinder = builder.createHollowCylinder(
    vm::bbox3{{-32, -32, -32}, {32, 32, 32}},
    8.0,
    8,
    RadiusMode::ToEdge,
    vm::axis::z,
    "someName");

  CHECK(cylinder.is_success());
  CHECK(cylinder.value().size() == 8);
}

} // namespace TrenchBroom::Model
