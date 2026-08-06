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
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CompareHits.h"
#include "mdl/Hit.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"

#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

const auto TestHitType = HitType::freeType();

const auto worldBounds = vm::bbox3d{8192.0};

Hit makeHit(const HitType::Type type, const double distance, const auto& target)
{
  return Hit{type, distance, vm::vec3d{0, 0, distance}, target};
}

Hit makePlainHit(const double distance)
{
  return makeHit(TestHitType, distance, 1);
}

} // namespace

TEST_CASE("CompareHitsByDistance")
{
  const auto compare = CompareHitsByDistance{};

  CHECK(compare.compare(makePlainHit(1.0), makePlainHit(2.0)) == -1);
  CHECK(compare.compare(makePlainHit(2.0), makePlainHit(1.0)) == 1);
  CHECK(compare.compare(makePlainHit(1.0), makePlainHit(1.0)) == 0);
}

TEST_CASE("CompareHitsByType")
{
  const auto compare = CompareHitsByType{};

  auto brushNode = BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(32.0, "material")
    | kdl::value()};
  const auto brushHit =
    makeHit(BrushNode::BrushHitType, 1.0, BrushFaceHandle{&brushNode, 0});

  SECTION("sorts brush hits first")
  {
    CHECK(compare.compare(brushHit, makePlainHit(1.0)) == -1);
    CHECK(compare.compare(makePlainHit(1.0), brushHit) == 1);
  }

  SECTION("does not order hits that are not brush hits")
  {
    CHECK(compare.compare(makePlainHit(1.0), makePlainHit(2.0)) == 0);
  }
}

TEST_CASE("CompareHitsBySize")
{
  const auto compare = CompareHitsBySize{vm::axis::z};

  auto brushNode = BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(32.0, "material")
    | kdl::value()};

  // the top face of the cube has a projected area of 32 * 32 along the z axis
  const auto topFaceIndex = brushNode.brush().findFace(vm::vec3d{0, 0, 1});
  REQUIRE(topFaceIndex);
  REQUIRE(
    brushNode.brush().face(*topFaceIndex).projectedArea(vm::axis::z) == 32.0 * 32.0);

  const auto brushHit =
    makeHit(BrushNode::BrushHitType, 1.0, BrushFaceHandle{&brushNode, *topFaceIndex});

  SECTION("sorts by the projected area of the hit brush face")
  {
    // a hit that does not refer to a node has size 0
    CHECK(compare.compare(makePlainHit(1.0), brushHit) == -1);
    CHECK(compare.compare(brushHit, makePlainHit(1.0)) == 1);
  }

  SECTION("sorts by the projected area of the hit node")
  {
    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    const auto patchHit = makeHit(PatchNode::PatchHitType, 1.0, &patchNode);
    REQUIRE(patchNode.projectedArea(vm::axis::z) > 0.0);

    CHECK(compare.compare(makePlainHit(1.0), patchHit) == -1);
    CHECK(compare.compare(patchHit, makePlainHit(1.0)) == 1);
    CHECK(compare.compare(patchHit, brushHit) == -1);
  }

  SECTION("falls back to the distance if the sizes are equal")
  {
    CHECK(compare.compare(makePlainHit(1.0), makePlainHit(2.0)) == -1);
    CHECK(compare.compare(makePlainHit(2.0), makePlainHit(1.0)) == 1);
    CHECK(compare.compare(makePlainHit(1.0), makePlainHit(1.0)) == 0);
  }
}

TEST_CASE("CombineCompareHits")
{
  auto brushNode = BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(32.0, "material")
    | kdl::value()};
  const auto makeBrushHit = [&](const double distance) {
    return makeHit(BrushNode::BrushHitType, distance, BrushFaceHandle{&brushNode, 0});
  };

  const auto compare = CombineCompareHits{
    std::make_unique<CompareHitsByDistance>(), std::make_unique<CompareHitsByType>()};

  SECTION("uses the first comparator if it orders the hits")
  {
    // the closer hit comes first even though the other one is a brush hit
    CHECK(compare.compare(makePlainHit(1.0), makeBrushHit(2.0)) == -1);
    CHECK(compare.compare(makeBrushHit(2.0), makePlainHit(1.0)) == 1);
  }

  SECTION("falls back to the second comparator")
  {
    CHECK(compare.compare(makeBrushHit(1.0), makePlainHit(1.0)) == -1);
    CHECK(compare.compare(makePlainHit(1.0), makeBrushHit(1.0)) == 1);
  }
}

} // namespace tb::mdl
