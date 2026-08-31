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

#include "base/Result.h"
#include "mdl/CatchConfig.h"
#include "mdl/ParallelUvCoordSystem.h"
#include "mdl/ParaxialUvCoordSystem.h"
#include "mdl/UvAttributes.h"
#include "mdl/UvCoordSystem.h"

#include "kd/result.h"

#include "vm/mat_ext.h"
#include "vm/plane.h"
#include "vm/scalar.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <limits>
#include <optional>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

auto createParaxial(const vm::vec3d& normal, const UvAttributes& uvAttributes = {})
{
  return ParaxialUvCoordSystem::createFromNormal(normal, uvAttributes) | kdl::value();
}

auto createParallel(
  const vm::vec3d& uAxis, const vm::vec3d& vAxis, const UvAttributes& uvAttributes = {})
{
  return ParallelUvCoordSystem::createFromAxes(uAxis, vAxis, uvAttributes) | kdl::value();
}

} // namespace

TEST_CASE("UvCoordSystem")
{
  SECTION("takeSnapshot")
  {
    SECTION("paraxial UV axes cannot be snapshotted")
    {
      const auto paraxial = UvCoordSystem{createParaxial(vm::vec3d{0, 0, 1})};
      CHECK(paraxial.takeSnapshot() == std::nullopt);
    }

    SECTION("parallel UV axes can be snapshotted and restored")
    {
      auto parallel =
        UvCoordSystem{createParallel(vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0})};
      const auto snapshot = parallel.takeSnapshot();
      REQUIRE(
        snapshot
        == std::optional{UvCoordSystemSnapshot{vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}}});

      parallel.reset(vm::vec3d{0, 0, 1});
      REQUIRE(parallel.takeSnapshot() != snapshot);

      parallel.restoreSnapshot(*snapshot);
      CHECK(parallel.uAxis() == vm::vec3d{0, 1, 0});
      CHECK(parallel.vAxis() == vm::vec3d{1, 0, 0});
    }
  }

  SECTION("setUvAttributes")
  {
    const auto normal = vm::vec3d{0, 0, 1};

    SECTION("updates the UV attributes and axes")
    {
      auto system = UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
      REQUIRE(
        system.setUvAttributes(normal, UvAttributes{{1, 2}, {3, 4}, 45.0f}).is_success());
      CHECK(system.uvAttributes() == UvAttributes{{1, 2}, {3, 4}, 45.0f});
    }

    SECTION("leaves the system unchanged if the given attributes are invalid")
    {
      auto system = UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
      const auto before = system;

      const auto nan = std::numeric_limits<float>::quiet_NaN();
      CHECK(
        system.setUvAttributes(normal, UvAttributes{{0, 0}, {1, 1}, nan})
        == Result<void>{Error{"UV attributes are invalid"}});
      CHECK(system == before);
    }

    SECTION("leaves the system unchanged if the resulting matrix is not invertible")
    {
      auto system = UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
      const auto before = system;

      const auto extremeScale = UvAttributes{{0, 0}, {1e30f, 1e30f}, 0.0f};
      CHECK(
        system.setUvAttributes(normal, extremeScale)
        == Result<void>{Error{"UV coordinate system is not invertible"}});
      CHECK(system == before);
    }
  }

  SECTION("rotate")
  {
    const auto normal = vm::vec3d{0, 0, 1};

    SECTION("leaves the system unchanged if the resulting rotation overflows")
    {
      const auto max = std::numeric_limits<float>::max();
      auto system = UvCoordSystem{createParallel(
        vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{{}, {1, 1}, max})};
      const auto before = system;

      // rotating by another huge angle overflows the resulting rotation to infinity --
      // this is the original crash this branch set out to fix
      CHECK(
        system.rotate(normal, max) == Result<void>{Error{"UV attributes are invalid"}});
      CHECK(system == before);
    }
  }

  SECTION("translate")
  {
    const auto normal = vm::vec3d{0, 0, 1};
    const auto offset = vm::vec2f{2, 3};

    SECTION("uses the U axis as horizontal if it is closer to the right axis")
    {
      auto system = UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{-2, -3});
    }

    SECTION("uses the V axis as horizontal if it is closer to the right axis")
    {
      auto system = UvCoordSystem{createParallel(vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0})};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{-3, -2});
    }

    SECTION(
      "falls back to comparing against the up axis if neither axis is clearly closer "
      "to the right axis")
    {
      const auto right = vm::normalize(vm::vec3d{1, 1, 0});

      SECTION("uses the V axis as horizontal if the U axis is closer to the up axis")
      {
        auto system =
          UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
        system.translate(normal, vm::vec3d{1, 0, 0}, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{-3, -2});
      }

      SECTION("uses the U axis as horizontal if the V axis is closer to the up axis")
      {
        auto system =
          UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
        system.translate(normal, vm::vec3d{0, 1, 0}, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{-2, -3});
      }

      SECTION("does nothing if neither axis can be clearly chosen")
      {
        auto system =
          UvCoordSystem{createParallel(vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0})};
        const auto up = vm::normalize(vm::vec3d{1, -1, 0});
        system.translate(normal, up, right, offset);
        CHECK(system.uvAttributes().offset == vm::vec2f{0, 0});
      }
    }

    SECTION("flips the offset direction for a negative scale")
    {
      auto system = UvCoordSystem{createParallel(
        vm::vec3d{1, 0, 0}, vm::vec3d{0, 1, 0}, UvAttributes{{}, {-1, -1}, 0.0f})};
      system.translate(normal, vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0}, offset);
      CHECK(system.uvAttributes().offset == vm::vec2f{2, 3});
    }
  }
}

TEST_CASE("ParallelUvCoordSystem")
{
  SECTION("createFromPoints")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto point0 = vm::vec3d{0, 0, 0};
    const auto point1 = vm::vec3d{1, 0, 0};
    const auto point2 = vm::vec3d{0, 1, 0};

    SECTION("returns an orthonormal UV coordinate system with the given attributes")
    {
      const auto actual =
        ParallelUvCoordSystem::createFromPoints(point0, point1, point2, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uvAttributes() == uvAttributes);
      CHECK(vm::is_unit(actual.value().uAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_unit(actual.value().vAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_zero(
        vm::dot(actual.value().uAxis(), actual.value().vAxis()), vm::Cd::almost_zero()));
      CHECK(vm::is_zero(
        vm::dot(actual.value().uAxis(), actual.value().normal()), vm::Cd::almost_zero()));
      CHECK(vm::is_zero(
        vm::dot(actual.value().vAxis(), actual.value().normal()), vm::Cd::almost_zero()));
    }

    SECTION("returns an error if the points do not define a plane")
    {
      const auto degeneratePoint2 = vm::vec3d{2, 0, 0};
      CHECK(
        ParallelUvCoordSystem::createFromPoints(
          point0, point1, degeneratePoint2, uvAttributes)
        == Result<ParallelUvCoordSystem>{Error{"Face points do not define a plane"}});
    }
  }

  SECTION("createFromAxes")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto uAxis = vm::vec3d{1, 0, 0};
    const auto vAxis = vm::vec3d{0, 1, 0};

    SECTION("returns the given axes and attributes unchanged")
    {
      const auto actual =
        ParallelUvCoordSystem::createFromAxes(uAxis, vAxis, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uAxis() == uAxis);
      CHECK(actual.value().vAxis() == vAxis);
      CHECK(actual.value().uvAttributes() == uvAttributes);
    }

    SECTION("accepts a scale of 0")
    {
      const auto zeroScale = UvAttributes{{0, 0}, {0, 0}, 0.0f};
      CHECK(ParallelUvCoordSystem::createFromAxes(uAxis, vAxis, zeroScale).is_success());
    }

    SECTION("rejects invalid input")
    {
      const auto nan = std::numeric_limits<float>::quiet_NaN();

      using T = std::tuple<vm::vec3d, vm::vec3d, UvAttributes, Error>;

      const auto [testUAxis, testVAxis, testUvAttributes, expectedError] =
        GENERATE_COPY(values<T>({
          // non-finite rotation
          {uAxis,
           vAxis,
           UvAttributes{{0, 0}, {1, 1}, nan},
           Error{"UV attributes are invalid"}},
          // parallel axes
          {uAxis,
           uAxis,
           uvAttributes,
           Error{"UV axes do not form an invertible coordinate system"}},
          // a zero axis
          {vm::vec3d{0, 0, 0},
           vAxis,
           uvAttributes,
           Error{"UV axes do not form an invertible coordinate system"}},
          // otherwise-fine axes, but a scale this large makes the resulting matrix
          // look singular under the fixed pivot threshold used to invert it
          {uAxis,
           vAxis,
           UvAttributes{{0, 0}, {1e30f, 1e30f}, 0.0f},
           Error{"UV coordinate system is not invertible"}},
        }));

      CAPTURE(testUAxis, testVAxis, testUvAttributes);
      CHECK(
        ParallelUvCoordSystem::createFromAxes(testUAxis, testVAxis, testUvAttributes)
        == Result<ParallelUvCoordSystem>{expectedError});
    }
  }
}

TEST_CASE("ParaxialUvCoordSystem")
{
  SECTION("createFromPoints")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto point0 = vm::vec3d{0, 0, 0};
    const auto point1 = vm::vec3d{1, 0, 0};
    const auto point2 = vm::vec3d{0, 1, 0};

    SECTION("returns an orthonormal UV coordinate system with the given attributes")
    {
      const auto actual =
        ParaxialUvCoordSystem::createFromPoints(point0, point1, point2, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uvAttributes() == uvAttributes);
      CHECK(vm::is_unit(actual.value().uAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_unit(actual.value().vAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_zero(
        vm::dot(actual.value().uAxis(), actual.value().vAxis()), vm::Cd::almost_zero()));
    }

    SECTION("returns an error if the points do not define a plane")
    {
      const auto degeneratePoint2 = vm::vec3d{2, 0, 0};
      CHECK(
        ParaxialUvCoordSystem::createFromPoints(
          point0, point1, degeneratePoint2, uvAttributes)
        == Result<ParaxialUvCoordSystem>{Error{"Face points do not define a plane"}});
    }
  }

  SECTION("createFromNormal")
  {
    const auto uvAttributes = UvAttributes{{1, 2}, {3, 4}, 45.0f};

    const auto normal = vm::vec3d{0, 0, 1};

    SECTION("returns an orthonormal UV coordinate system with the given attributes")
    {
      const auto actual = ParaxialUvCoordSystem::createFromNormal(normal, uvAttributes);
      REQUIRE(actual);
      CHECK(actual.value().uvAttributes() == uvAttributes);
      CHECK(vm::is_unit(actual.value().uAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_unit(actual.value().vAxis(), vm::Cd::almost_zero()));
      CHECK(vm::is_zero(
        vm::dot(actual.value().uAxis(), actual.value().vAxis()), vm::Cd::almost_zero()));
    }

    SECTION("accepts a scale of 0")
    {
      const auto zeroScale = UvAttributes{{0, 0}, {0, 0}, 0.0f};
      CHECK(ParaxialUvCoordSystem::createFromNormal(normal, zeroScale).is_success());
    }

    SECTION("rejects invalid input")
    {
      const auto nan = std::numeric_limits<float>::quiet_NaN();

      using T = std::tuple<UvAttributes, Error>;

      const auto [testUvAttributes, expectedError] = GENERATE_COPY(values<T>({
        // non-finite offset
        {UvAttributes{{nan, 0}, {1, 1}, 0.0f}, Error{"UV attributes are invalid"}},
        // otherwise-fine axes (Paraxial axes are always orthonormal), but a scale this
        // large makes the resulting matrix look singular under the fixed pivot
        // threshold used to invert it
        {UvAttributes{{0, 0}, {1e30f, 1e30f}, 0.0f},
         Error{"UV coordinate system is not invertible"}},
      }));

      CAPTURE(testUvAttributes);
      CHECK(
        ParaxialUvCoordSystem::createFromNormal(normal, testUvAttributes)
        == Result<ParaxialUvCoordSystem>{expectedError});
    }
  }

  SECTION("transform")
  {
    SECTION("does not produce a zero scale for a lock-textured translation")
    {
      // this face's UV attributes already carry pathological offset/scale magnitudes;
      // translating it further reunds the resulting V scale down to exactly 0
      auto system = createParaxial(
        vm::vec3d{0, 1, 0},
        UvAttributes{
          {-43460.609375f, -11065997.0f},
          {0.018230000510811806f, -0.00062000000616535544f},
          97.126823425292969f});

      const auto oldBoundary = vm::plane3d{-1960.0, vm::vec3d{0, 1, 0}};
      const auto newBoundary = vm::plane3d{-1960.0, vm::vec3d{0, 1, 0}};
      const auto transformation = vm::translation_matrix(vm::vec3d{16, 0, 0});
      const auto invariant = vm::vec3d{-6887.666666666667, -1960.0, -59.333333333333336};

      system.transform(
        oldBoundary, newBoundary, transformation, vm::vec2f{1, 1}, true, invariant);

      CHECK(!vm::is_zero(system.uvAttributes().scale.x(), vm::Cf::almost_zero()));
      CHECK(vm::is_zero(system.uvAttributes().scale.y(), vm::Cf::almost_zero()));
    }
  }
}

} // namespace tb::mdl
