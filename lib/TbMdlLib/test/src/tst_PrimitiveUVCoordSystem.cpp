/*
 Copyright (C) 2026 Thomas Jones

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

#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/PrimitiveUVCoordSystem.h"
#include "mdl/UVAttributes.h"

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

bool approxEqual(const UVAttributes& lhs, const UVAttributes& rhs)
{
  return lhs.offset == vm::approx{rhs.offset, 0.001f}
         && lhs.scale == vm::approx{rhs.scale, 0.001f}
         && lhs.rotation == vm::approx{rhs.rotation, 0.001f};
}

} // namespace

TEST_CASE("PrimitiveUVCoordSystem")
{
  SECTION("UV attributes round trip")
  {
    const auto normal = GENERATE(
      vm::vec3d{0, 0, 1},
      vm::vec3d{0, 0, -1},
      vm::vec3d{1, 0, 0},
      vm::vec3d{0, -1, 0},
      vm::normalize(vm::vec3d{1, 2, 3}));
    const auto textureSize = GENERATE(vm::vec2f{64, 64}, vm::vec2f{128, 64});
    const auto uvAttributes = GENERATE(
      UVAttributes{},
      UVAttributes{{16, -24}, {1, 1}, 0.0f},
      UVAttributes{{0, 0}, {0.5f, 0.25f}, 0.0f},
      UVAttributes{{8, 8}, {2, 4}, 30.0f},
      UVAttributes{{-16, 32}, {0.5f, 2}, 90.0f},
      UVAttributes{{0, 16}, {1, 2}, 180.0f},
      UVAttributes{{4, 4}, {3, 0.5f}, 270.0f});

    CAPTURE(normal, textureSize, uvAttributes);

    const auto coordSystem = PrimitiveUVCoordSystem{normal, uvAttributes, textureSize};
    CHECK(approxEqual(coordSystem.uvAttributes(textureSize), uvAttributes));
  }

  SECTION("setUVAttributes round trip")
  {
    const auto textureSize = vm::vec2f{64, 128};
    auto coordSystem =
      PrimitiveUVCoordSystem{vm::vec3d{0, 0, 1}, UVAttributes{}, textureSize};

    const auto uvAttributes = UVAttributes{{12, -8}, {2, 0.5f}, 45.0f};
    coordSystem.setUVAttributes(uvAttributes, textureSize);
    CHECK(approxEqual(coordSystem.uvAttributes(textureSize), uvAttributes));
  }

  SECTION("UV coordinates are independent of the texture size")
  {
    const auto uvAttributes = UVAttributes{{16, 8}, {2, 4}, 30.0f};
    const auto coordSystem =
      PrimitiveUVCoordSystem{vm::vec3d{0, 0, 1}, uvAttributes, vm::vec2f{64, 64}};

    const auto point = vm::vec3d{32, -16, 0};
    CHECK(
      coordSystem.uvCoords(point, vm::vec2f{64, 64})
      == vm::approx{coordSystem.uvCoords(point, vm::vec2f{128, 32})});
  }

  SECTION("UV coordinates match the derived attributes")
  {
    const auto textureSize = vm::vec2f{64, 128};
    const auto uvAttributes = UVAttributes{{16, 8}, {2, 4}, 30.0f};
    const auto coordSystem =
      PrimitiveUVCoordSystem{vm::vec3d{0, 0, 1}, uvAttributes, textureSize};

    // applying the derived attributes to the normalized axes must give the same UV
    // coordinates as the stored projection
    const auto point = vm::vec3d{32, -16, 0};
    CHECK(
      coordSystem.uvCoords(point, coordSystem.uvAttributes(textureSize), textureSize)
      == vm::approx{coordSystem.uvCoords(point, textureSize), 0.001f});
  }

  SECTION("toParallel preserves UV coordinates")
  {
    const auto textureSize = vm::vec2f{64, 64};
    const auto uvAttributes = UVAttributes{{16, 8}, {2, 0.5f}, 45.0f};
    const auto coordSystem =
      PrimitiveUVCoordSystem{vm::vec3d{0, 0, 1}, uvAttributes, textureSize};

    const auto parallel = coordSystem.toParallel(
      vm::vec3d{0, 0, 0}, vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, textureSize);

    const auto point = vm::vec3d{32, -16, 0};
    CHECK(
      parallel->uvCoords(point, textureSize)
      == vm::approx{coordSystem.uvCoords(point, textureSize), 0.001f});
  }

  SECTION("transform with texture lock preserves UV coordinates")
  {
    const auto textureSize = vm::vec2f{64, 64};
    const auto uvAttributes = UVAttributes{{16, 8}, {2, 0.5f}, 30.0f};
    auto coordSystem =
      PrimitiveUVCoordSystem{vm::vec3d{0, 0, 1}, uvAttributes, textureSize};

    const auto transformation =
      vm::translation_matrix(vm::vec3d{16, -32, 8})
      * vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(45.0));

    const auto point = vm::vec3d{32, -16, 0};
    const auto oldUVCoords = coordSystem.uvCoords(point, textureSize);

    const auto oldBoundary = vm::plane3d{0.0, vm::vec3d{0, 0, 1}};
    const auto newBoundary = vm::plane3d{8.0, vm::vec3d{0, 0, 1}};
    coordSystem.transform(
      oldBoundary, newBoundary, transformation, textureSize, true, vm::vec3d{0, 0, 0});

    CHECK(
      coordSystem.uvCoords(transformation * point, textureSize)
      == vm::approx{oldUVCoords, 0.001f});
  }

  SECTION("snapshot")
  {
    const auto textureSize = vm::vec2f{64, 64};
    auto coordSystem = PrimitiveUVCoordSystem{
      vm::vec3d{0, 0, 1}, UVAttributes{{16, 8}, {2, 0.5f}, 30.0f}, textureSize};
    const auto expected = coordSystem.uvAttributes(textureSize);

    const auto snapshot = coordSystem.takeSnapshot();
    REQUIRE(snapshot != nullptr);

    coordSystem.setUVAttributes(UVAttributes{}, textureSize);
    REQUIRE_FALSE(approxEqual(coordSystem.uvAttributes(textureSize), expected));

    snapshot->restore(coordSystem);
    CHECK(approxEqual(coordSystem.uvAttributes(textureSize), expected));
  }
}

} // namespace tb::mdl
