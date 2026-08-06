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
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

const auto TestHitType = HitType::freeType();

const auto worldBounds = vm::bbox3d{8192.0};

BrushNode* createBrushNode()
{
  return new BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(32.0, "material")
    | kdl::value()};
}

PatchNode* createPatchNode()
{
  // clang-format off
  return new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on
}

Hit makeHit(const HitType::Type type, const auto& target)
{
  return Hit{type, 1.0, vm::vec3d{0, 0, 1}, target};
}

Hit makeBrushHit(BrushNode* brushNode, const size_t faceIndex = 0)
{
  return makeHit(BrushNode::BrushHitType, BrushFaceHandle{brushNode, faceIndex});
}

} // namespace

TEST_CASE("HitFilter")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Quake3};

  SECTION("any")
  {
    const auto filter = HitFilters::any();

    CHECK(filter(makeHit(TestHitType, 1)));
    CHECK(filter(Hit::NoHit));
  }

  SECTION("none")
  {
    const auto filter = HitFilters::none();

    CHECK(!filter(makeHit(TestHitType, 1)));
    CHECK(!filter(Hit::NoHit));
  }

  SECTION("type")
  {
    const auto otherHitType = HitType::freeType();

    CHECK(HitFilters::type(TestHitType)(makeHit(TestHitType, 1)));
    CHECK(!HitFilters::type(otherHitType)(makeHit(TestHitType, 1)));

    SECTION("matches any hit type by default")
    {
      const auto filter = HitFilters::type();

      CHECK(filter(makeHit(TestHitType, 1)));
      CHECK(filter(makeHit(otherHitType, 1)));
      CHECK(!filter(Hit::NoHit));
    }

    SECTION("matches a mask of hit types")
    {
      const auto filter = HitFilters::type(TestHitType | otherHitType);

      CHECK(filter(makeHit(TestHitType, 1)));
      CHECK(filter(makeHit(otherHitType, 1)));
      CHECK(!filter(makeHit(HitType::NoType, 1)));
    }
  }

  SECTION("selected")
  {
    const auto filter = HitFilters::selected();

    SECTION("brush hits")
    {
      auto* brushNode = createBrushNode();
      worldNode.defaultLayer()->addChild(brushNode);

      const auto hit = makeBrushHit(brushNode);
      CHECK(!filter(hit));

      SECTION("with a selected brush node")
      {
        brushNode->select();
        CHECK(filter(hit));
      }

      SECTION("with a selected face")
      {
        brushNode->selectFace(0);
        CHECK(filter(hit));

        SECTION("but a different face is hit")
        {
          CHECK(!filter(makeBrushHit(brushNode, 1)));
        }
      }
    }

    SECTION("entity hits")
    {
      auto* entityNode = new EntityNode{Entity{}};
      worldNode.defaultLayer()->addChild(entityNode);

      const auto hit = makeHit(EntityNode::EntityHitType, entityNode);
      CHECK(!filter(hit));

      entityNode->select();
      CHECK(filter(hit));
    }

    SECTION("patch hits")
    {
      auto* patchNode = createPatchNode();
      worldNode.defaultLayer()->addChild(patchNode);

      const auto hit = makeHit(PatchNode::PatchHitType, patchNode);
      CHECK(!filter(hit));

      patchNode->select();
      CHECK(filter(hit));
    }

    SECTION("hits that do not refer to a node")
    {
      CHECK(!filter(makeHit(TestHitType, 1)));
      CHECK(!filter(Hit::NoHit));
    }

    SECTION("does not consider the selection of the containing group")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode();
      groupNode->addChild(brushNode);
      worldNode.defaultLayer()->addChild(groupNode);

      groupNode->select();
      CHECK(!filter(makeBrushHit(brushNode)));
    }
  }

  SECTION("transitivelySelected")
  {
    const auto filter = HitFilters::transitivelySelected();

    SECTION("brush hits")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode();
      groupNode->addChild(brushNode);
      worldNode.defaultLayer()->addChild(groupNode);

      const auto hit = makeBrushHit(brushNode);
      CHECK(!filter(hit));

      SECTION("with a selected brush node")
      {
        brushNode->select();
        CHECK(filter(hit));
      }

      SECTION("with a selected containing group")
      {
        groupNode->select();
        CHECK(filter(hit));
      }

      SECTION("with a selected face")
      {
        brushNode->selectFace(0);
        CHECK(filter(hit));

        SECTION("but a different face is hit")
        {
          CHECK(!filter(makeBrushHit(brushNode, 1)));
        }
      }
    }

    SECTION("entity hits")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* entityNode = new EntityNode{Entity{}};
      groupNode->addChild(entityNode);
      worldNode.defaultLayer()->addChild(groupNode);

      const auto hit = makeHit(EntityNode::EntityHitType, entityNode);
      CHECK(!filter(hit));

      groupNode->select();
      CHECK(filter(hit));
    }

    SECTION("patch hits")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* patchNode = createPatchNode();
      groupNode->addChild(patchNode);
      worldNode.defaultLayer()->addChild(groupNode);

      const auto hit = makeHit(PatchNode::PatchHitType, patchNode);
      CHECK(!filter(hit));

      groupNode->select();
      CHECK(filter(hit));
    }

    SECTION("hits that do not refer to a node")
    {
      CHECK(!filter(makeHit(TestHitType, 1)));
      CHECK(!filter(Hit::NoHit));
    }
  }

  SECTION("minDistance")
  {
    const auto filter = HitFilters::minDistance(2.0);

    CHECK(!filter(Hit{TestHitType, 1.0, vm::vec3d{0, 0, 1}, 1}));
    CHECK(filter(Hit{TestHitType, 2.0, vm::vec3d{0, 0, 2}, 1}));
    CHECK(filter(Hit{TestHitType, 3.0, vm::vec3d{0, 0, 3}, 1}));
  }

  SECTION("operator&&")
  {
    const auto hit = makeHit(TestHitType, 1);

    CHECK((HitFilters::any() && HitFilters::any())(hit));
    CHECK(!(HitFilters::any() && HitFilters::none())(hit));
    CHECK(!(HitFilters::none() && HitFilters::any())(hit));
    CHECK(!(HitFilters::none() && HitFilters::none())(hit));
  }

  SECTION("operator||")
  {
    const auto hit = makeHit(TestHitType, 1);

    CHECK((HitFilters::any() || HitFilters::any())(hit));
    CHECK((HitFilters::any() || HitFilters::none())(hit));
    CHECK((HitFilters::none() || HitFilters::any())(hit));
    CHECK(!(HitFilters::none() || HitFilters::none())(hit));
  }

  SECTION("operator!")
  {
    const auto hit = makeHit(TestHitType, 1);

    CHECK(!(!HitFilters::any())(hit));
    CHECK((!HitFilters::none())(hit));
  }
}

} // namespace tb::mdl
