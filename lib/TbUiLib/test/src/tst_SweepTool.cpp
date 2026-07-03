/*
 Copyright (C) 2026 Jackson Palmer

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
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TestFactory.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/SweepTool.h"

#include "vm/approx.h"
#include "vm/bbox.h"
#include "vm/constants.h"
#include "vm/quat.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("SweepTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto tool = SweepTool{document};

  const auto parameters =
    SweepParameters{2, 1, SweepPathMode::Straight, SweepAlignment::Free};

  SECTION("applies")
  {
    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    CHECK(!tool.applies());

    selectBrushFaces(map, {{brushNode, 0}});
    CHECK(tool.applies());
  }

  SECTION("activation requires selected brush faces")
  {
    CHECK(!tool.activate());
  }

  SECTION("hasScaleHandle is false before activation")
  {
    CHECK(!tool.hasScaleHandle());
  }

  SECTION("with a selected face")
  {
    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    const auto faceIndex = *brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    selectBrushFaces(map, {{brushNode, faceIndex}});

    REQUIRE(tool.activate());
    tool.setParameters(parameters);

    SECTION("activation captures the source face")
    {
      CHECK(tool.transform().isNoOp());
      CHECK(tool.parameters() == parameters);
      CHECK(tool.destinationCenter() == vm::vec3d{16, 0, 0});
      CHECK(tool.hasScaleHandle());
    }

    SECTION("setTransform round-trips")
    {
      const auto transform = SweepTransform{vm::vec3d{32, 0, 0}};
      tool.setTransform(transform);

      CHECK(tool.transform() == transform);
    }

    SECTION("ring drags compose onto the rotation at the start of the drag")
    {
      tool.setTransform(
        SweepTransform{{}, vm::quatd{vm::vec3d{0, 0, 1}, vm::Cd::half_pi()}});

      auto tracker = tool.beginRingDrag();
      tracker->apply(tool.destinationCenter(), vm::vec3d{1, 0, 0}, vm::Cd::half_pi());

      // a quarter turn about z followed by a quarter turn about x maps {1,0,0} to {0,0,1}
      CHECK(
        tool.transform().rotation * vm::vec3d{1, 0, 0} == vm::approx{vm::vec3d{0, 0, 1}});

      tracker->cancel();
      CHECK(
        tool.transform().rotation * vm::vec3d{1, 0, 0} == vm::approx{vm::vec3d{0, 1, 0}});
    }

    SECTION("dragScaleHandleTo reads a uniform scale off the handle arm")
    {
      tool.setDestinationCenter(vm::vec3d{80, 0, 0});

      const auto arm = tool.scaleHandlePosition() - tool.destinationCenter();
      tool.dragScaleHandleTo(tool.destinationCenter() + arm * 2.0);
      CHECK(tool.transform().scale == vm::approx{vm::vec3d{2, 2, 2}});

      // dragging past the center clamps the scale instead of inverting the profile
      tool.dragScaleHandleTo(tool.destinationCenter() - arm);
      CHECK(
        tool.transform().scale == vm::approx{vm::vec3d::fill(SweepTool::MinScaleFactor)});
    }

    SECTION("setDestinationCenter translates towards the given position")
    {
      tool.setDestinationCenter(vm::vec3d{80, 0, 0});

      CHECK(tool.transform().translation == vm::vec3d{64, 0, 0});
      CHECK(tool.destinationCenter() == vm::vec3d{80, 0, 0});
    }

    SECTION("cancel resets the transform")
    {
      tool.setDestinationCenter(vm::vec3d{80, 0, 0});

      CHECK(tool.cancel());
      CHECK(tool.transform().isNoOp());
      CHECK(!tool.cancel());
    }

    SECTION("reactivation resets the transform")
    {
      tool.setDestinationCenter(vm::vec3d{80, 0, 0});

      REQUIRE(tool.deactivate());
      REQUIRE(tool.activate());
      CHECK(tool.transform().isNoOp());
    }

    SECTION("commitSweep")
    {
      auto* parent = brushNode->parent();
      const auto childCountBefore = parent->childCount();

      SECTION("adds the generated brushes in one transaction")
      {
        tool.setDestinationCenter(vm::vec3d{80, 0, 0});
        tool.commitSweep();

        CHECK(parent->childCount() == childCountBefore + 2);
        CHECK(map.selection().nodes.size() == 2);
        CHECK(!map.selection().hasBrushFaces());

        REQUIRE(map.undoCommandName());
        CHECK(*map.undoCommandName() == "Sweep");

        map.undoCommand();
        CHECK(parent->childCount() == childCountBefore);
      }

      SECTION("does nothing without a preview")
      {
        tool.commitSweep();

        CHECK(parent->childCount() == childCountBefore);
      }

      SECTION("zero segments produce no preview")
      {
        auto zeroSegmentParameters = parameters;
        zeroSegmentParameters.segments = 0;
        tool.setParameters(zeroSegmentParameters);
        tool.setDestinationCenter(vm::vec3d{80, 0, 0});

        tool.commitSweep();

        CHECK(parent->childCount() == childCountBefore);
      }
    }
  }

  SECTION("removing the source face's parent while the tool is active")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{entityNode, {brushNode}}});

    const auto faceIndex = *brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    selectBrushFaces(map, {{brushNode, faceIndex}});

    REQUIRE(tool.activate());
    tool.setParameters(parameters);
    tool.setDestinationCenter(vm::vec3d{80, 0, 0});

    removeNodes(map, {entityNode});

    // the generated brushes fall back to the map's default insertion parent
    auto* defaultParent = parentForNodes(map);
    const auto childCountBefore = defaultParent->childCount();

    tool.commitSweep();
    CHECK(defaultParent->childCount() == childCountBefore + 2);
  }

  SECTION(
    "sweeping faces with cancelling normals falls back to a straight path in S-Bend mode")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});

    auto* brushNodeA = createBrushNode(map);
    addNodes(map, {{entityNode, {brushNodeA}}});
    const auto entityChildCountBefore = entityNode->childCount();

    auto* defaultParent = parentForNodes(map);
    auto* brushNodeB = createBrushNode(map);
    addNodes(map, {{defaultParent, {brushNodeB}}});
    const auto defaultChildCountBefore = defaultParent->childCount();

    const auto faceIndexA = *brushNodeA->brush().findFace(vm::vec3d{1, 0, 0});
    const auto faceIndexB = *brushNodeB->brush().findFace(vm::vec3d{-1, 0, 0});
    selectBrushFaces(map, {{brushNodeA, faceIndexA}, {brushNodeB, faceIndexB}});

    REQUIRE(tool.activate());
    tool.setParameters(SweepParameters{2, 1, SweepPathMode::SBend, SweepAlignment::Free});
    tool.setDestinationCenter(tool.destinationCenter() + vm::vec3d{64, 0, 0});
    tool.commitSweep();

    // opposing normals cancel out, so the S-Bend tangent is zero and every station sits
    // on the straight line between the source and destination caps
    const auto& entityChildren = entityNode->children();
    REQUIRE(entityChildren.size() == entityChildCountBefore + 2);
    CHECK(
      entityChildren[entityChildCountBefore]->logicalBounds()
      == vm::bbox3d{{16, -16, -16}, {48, 16, 16}});
    CHECK(
      entityChildren[entityChildCountBefore + 1]->logicalBounds()
      == vm::bbox3d{{48, -16, -16}, {80, 16, 16}});

    const auto& defaultChildren = defaultParent->children();
    REQUIRE(defaultChildren.size() == defaultChildCountBefore + 2);
    CHECK(
      defaultChildren[defaultChildCountBefore]->logicalBounds()
      == vm::bbox3d{{-16, -16, -16}, {16, 16, 16}});
    CHECK(
      defaultChildren[defaultChildCountBefore + 1]->logicalBounds()
      == vm::bbox3d{{16, -16, -16}, {48, 16, 16}});
  }
}

} // namespace tb::ui
