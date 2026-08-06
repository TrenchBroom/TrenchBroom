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

#include "mdl/Hit.h"

#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <any>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

const auto TestHitType = HitType::freeType();
const auto OtherHitType = HitType::freeType();

Hit makeHit(const double distance, const int id = 1)
{
  return Hit{TestHitType, distance, vm::vec3d{0, 0, distance}, id};
}

} // namespace

TEST_CASE("Hit")
{
  SECTION("NoHit")
  {
    CHECK(!Hit::NoHit.isMatch());
    CHECK(Hit::NoHit.type() == HitType::NoType);
    CHECK(Hit::NoHit.distance() == 0.0);
    CHECK(Hit::NoHit.hitPoint() == vm::vec3d{0, 0, 0});
    CHECK(Hit::NoHit.error() == 0.0);
  }

  SECTION("isMatch")
  {
    CHECK(Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1}.isMatch());
    CHECK(!Hit{HitType::NoType, 1.0, vm::vec3d{0, 0, 1}, 1}.isMatch());
  }

  SECTION("type")
  {
    CHECK(Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1}.type() == TestHitType);
  }

  SECTION("hasType")
  {
    const auto hit = Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1};

    CHECK(hit.hasType(TestHitType));
    CHECK(hit.hasType(TestHitType | OtherHitType));
    CHECK(hit.hasType(HitType::AnyType));
    CHECK(!hit.hasType(OtherHitType));
    CHECK(!hit.hasType(HitType::NoType));
  }

  SECTION("distance")
  {
    CHECK(makeHit(2.0).distance() == 2.0);
  }

  SECTION("hitPoint")
  {
    CHECK(makeHit(2.0).hitPoint() == vm::vec3d{0, 0, 2});
  }

  SECTION("error")
  {
    CHECK(Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1}.error() == 0.0);
    CHECK(Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1, 0.5}.error() == 0.5);
  }

  SECTION("target")
  {
    const auto hit = Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, std::string{"target"}};

    CHECK(hit.target<std::string>() == "target");
    CHECK_THROWS_AS(hit.target<int>(), std::bad_any_cast);
  }
}

TEST_CASE("selectClosest")
{
  SECTION("returns no hit if neither hit matches")
  {
    CHECK(!selectClosest(Hit::NoHit, Hit::NoHit).isMatch());
  }

  SECTION("returns the matching hit if the other one does not match")
  {
    CHECK(selectClosest(Hit::NoHit, makeHit(2.0, 1)).target<int>() == 1);
    CHECK(selectClosest(makeHit(2.0, 1), Hit::NoHit).target<int>() == 1);
  }

  SECTION("returns the closer hit if both match")
  {
    CHECK(selectClosest(makeHit(1.0, 1), makeHit(2.0, 2)).target<int>() == 1);
    CHECK(selectClosest(makeHit(2.0, 2), makeHit(1.0, 1)).target<int>() == 1);
  }

  SECTION("returns the first hit if both match at the same distance")
  {
    CHECK(selectClosest(makeHit(1.0, 1), makeHit(1.0, 2)).target<int>() == 1);
  }

  SECTION("selects the closest of more than two hits")
  {
    CHECK(
      selectClosest(makeHit(3.0, 3), makeHit(1.0, 1), makeHit(2.0, 2)).target<int>()
      == 1);
    CHECK(selectClosest(makeHit(3.0, 3), Hit::NoHit, makeHit(2.0, 2)).target<int>() == 2);
  }
}

} // namespace tb::mdl
