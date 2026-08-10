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

#include "gl/Camera.h"
#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Grid.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/ExtrudeTool.h"
#include "ui/ExtrudeToolController.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

namespace
{

gl::PerspectiveCamera perspectiveCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  return gl::PerspectiveCamera{
    90.0f,
    1.0f,
    8000.0f,
    viewport,
    vm::vec3f{pickRay.origin},
    direction,
    vm::vec3f{0, 0, 1}};
}

gl::OrthographicCamera orthographicCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  return gl::OrthographicCamera{
    1.0f, 8000.0f, viewport, vm::vec3f{pickRay.origin}, direction, vm::vec3f{0, 1, 0}};
}

} // namespace

TEST_CASE("ExtrudeToolController")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
  addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
  selectNodes(map, {brushNode});

  auto tool = ExtrudeTool{document};

  SECTION("3D controller")
  {
    auto extrudeController = ExtrudeToolController3D{tool};
    auto& controller = static_cast<ToolController&>(extrudeController);

    // shoot straight down at the top face
    const auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};
    const auto camera = perspectiveCameraFor(pickRay);

    const auto inputStateFor = [&](ModifierKeyState modifiers) {
      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(PickRequest{pickRay, camera});
      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      inputState.setModifierKeys(modifiers);
      controller.pick(inputState, pickResult);
      inputState.setPickResult(std::move(pickResult));
      return inputState;
    };

    SECTION("tool returns the wrapped tool")
    {
      CHECK(&controller.tool() == static_cast<Tool*>(&tool));
    }

    SECTION("cancel always returns false")
    {
      CHECK(!controller.cancel());
    }

    SECTION("pick")
    {
      SECTION("adds a hit with Shift held")
      {
        const auto inputState = inputStateFor(ModifierKeys::Shift);
        const auto& hit = inputState.pickResult().first(
          mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType));
        CHECK(hit.isMatch());
      }

      SECTION("adds a hit with Shift+CtrlCmd held")
      {
        const auto inputState =
          inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        const auto& hit = inputState.pickResult().first(
          mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType));
        CHECK(hit.isMatch());
      }

      SECTION("adds a hit with Shift+CtrlCmd+Alt held")
      {
        const auto inputState =
          inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        const auto& hit = inputState.pickResult().first(
          mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType));
        CHECK(hit.isMatch());
      }

      SECTION("adds nothing without Shift")
      {
        const auto inputState = inputStateFor(ModifierKeys::None);
        const auto& hit = inputState.pickResult().first(
          mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType));
        CHECK(!hit.isMatch());
      }

      SECTION("adds nothing when the tool does not apply")
      {
        deselectAll(map);
        const auto inputState = inputStateFor(ModifierKeys::Shift);
        const auto& hit = inputState.pickResult().first(
          mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType));
        CHECK(!hit.isMatch());
      }
    }

    SECTION("modifierKeyChange")
    {
      auto inputState = inputStateFor(ModifierKeys::Shift);
      REQUIRE(tool.proposedDragHandles().empty());

      SECTION("updates the proposed drag handles when no drag is in progress")
      {
        controller.modifierKeyChange(inputState);
        CHECK(!tool.proposedDragHandles().empty());
      }

      SECTION("does nothing while a drag is in progress")
      {
        inputState.setAnyToolDragging(true);
        controller.modifierKeyChange(inputState);
        CHECK(tool.proposedDragHandles().empty());
      }
    }

    SECTION("mouseMove")
    {
      SECTION("updates the proposed drag handles when input is handled")
      {
        auto inputState = inputStateFor(ModifierKeys::Shift);
        REQUIRE(tool.proposedDragHandles().empty());

        controller.mouseMove(inputState);
        CHECK(!tool.proposedDragHandles().empty());
      }

      SECTION("does nothing without the required modifiers")
      {
        auto inputState = inputStateFor(ModifierKeys::None);
        controller.mouseMove(inputState);
        CHECK(tool.proposedDragHandles().empty());
      }

      SECTION("does nothing while a drag is in progress")
      {
        auto inputState = inputStateFor(ModifierKeys::Shift);
        inputState.setAnyToolDragging(true);
        controller.mouseMove(inputState);
        CHECK(tool.proposedDragHandles().empty());
      }
    }

    SECTION("acceptMouseDrag")
    {
      SECTION("returns nullptr without the required modifiers")
      {
        auto inputState = inputStateFor(ModifierKeys::None);
        inputState.mouseDown(MouseButtons::Left);
        CHECK(controller.acceptMouseDrag(inputState) == nullptr);
      }

      SECTION("returns nullptr without the left mouse button")
      {
        auto inputState = inputStateFor(ModifierKeys::Shift);
        inputState.mouseDown(MouseButtons::Right);
        CHECK(controller.acceptMouseDrag(inputState) == nullptr);
      }

      SECTION("returns nullptr when the tool does not apply")
      {
        deselectAll(map);
        auto inputState = inputStateFor(ModifierKeys::Shift);
        inputState.mouseDown(MouseButtons::Left);
        CHECK(controller.acceptMouseDrag(inputState) == nullptr);
      }

      SECTION("starts an extrude drag with Shift")
      {
        map.grid().toggleSnap(); // disable snapping for a deterministic delta

        auto inputState = inputStateFor(ModifierKeys::Shift);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = controller.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        const auto boundsBefore = brushNode->logicalBounds();

        // the extrude handle picker projects the drag ray onto a vertical line
        // through the hit point (the face normal); a ray parallel to that line has
        // no well-defined projection, so angle this one away from straight-down
        auto dragInputState = InputState{0.0f, 0.0f};
        const auto dragRay =
          vm::ray3d{pickRay.origin, vm::normalize(vm::vec3d{0, 0.5, -1})};
        dragInputState.setPickRequest(PickRequest{dragRay, camera});
        dragInputState.setModifierKeys(ModifierKeys::Shift);
        dragInputState.mouseDown(MouseButtons::Left);
        CHECK(tracker->update(dragInputState));

        tracker->end(dragInputState);
        CHECK(brushNode->logicalBounds() != boundsBefore);
      }

      SECTION("starts a split extrude drag with Shift+CtrlCmd")
      {
        auto inputState = inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = controller.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        tracker->cancel();
        // cancelling reverts the transaction; the tool no longer considers itself
        // mid-drag, so a fresh drag can be accepted again
        CHECK(controller.acceptMouseDrag(inputState) != nullptr);
      }

      SECTION("starts a stamp drag with Shift+CtrlCmd+Alt")
      {
        map.grid().toggleSnap(); // disable snapping for a deterministic delta

        auto inputState =
          inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        inputState.mouseDown(MouseButtons::Left);

        auto tracker = controller.acceptMouseDrag(inputState);
        REQUIRE(tracker != nullptr);

        const auto boundsBefore = brushNode->logicalBounds();

        // same picker as plain extrude, see above
        auto dragInputState = InputState{0.0f, 0.0f};
        const auto dragRay =
          vm::ray3d{pickRay.origin, vm::normalize(vm::vec3d{0, 0.5, -1})};
        dragInputState.setPickRequest(PickRequest{dragRay, camera});
        dragInputState.setModifierKeys(
          ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        dragInputState.mouseDown(MouseButtons::Left);
        CHECK(tracker->update(dragInputState));

        tracker->end(dragInputState);

        // stamping leaves the original brush untouched and deselected; a new brush
        // is created and selected instead
        CHECK(brushNode->logicalBounds() == boundsBefore);
        CHECK(!brushNode->selected());

        const auto brushes = map.selection().brushes;
        REQUIRE(brushes.size() == 1);
        CHECK(brushes.front() != brushNode);
      }
    }
  }

  SECTION("2D controller")
  {
    // pick2D only ever hits a "horizon edge" (a silhouette edge of the selection as
    // seen from this view), never a face directly -- unlike a lone brush's top face,
    // two brushes meeting at a seam give pick2D a real edge to find. leftBrush's +X
    // face and rightBrush's -X face are coincident but face in opposite directions.
    deselectAll(map);

    auto* leftBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "material")
      | kdl::value()};
    auto* rightBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{16, -16, -16}, {48, 16, 16}}, "material")
      | kdl::value()};
    addNodes(map, {{map.editorContext().currentLayer(), {leftBrush, rightBrush}}});
    selectNodes(map, {leftBrush, rightBrush});

    auto extrudeController = ExtrudeToolController2D{tool};
    auto& controller = static_cast<ToolController&>(extrudeController);

    // top view, just short of the seam at x = 16, within the edge handle's radius
    const auto pickRay = vm::ray3d{{12, 0, 32}, {0, 0, -1}};
    const auto camera = orthographicCameraFor(pickRay);

    const auto inputStateFor = [&](ModifierKeyState modifiers) {
      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(PickRequest{pickRay, camera});
      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);
      inputState.setModifierKeys(modifiers);
      controller.pick(inputState, pickResult);
      inputState.setPickResult(std::move(pickResult));
      return inputState;
    };

    SECTION("pick requires Shift, Shift+CtrlCmd, Shift+Alt, or Shift+CtrlCmd+Alt")
    {
      SECTION("Shift")
      {
        const auto inputState = inputStateFor(ModifierKeys::Shift);
        CHECK(inputState.pickResult()
                .first(mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType))
                .isMatch());
      }

      SECTION("Shift+Alt")
      {
        const auto inputState = inputStateFor(ModifierKeys::Shift | ModifierKeys::Alt);
        CHECK(inputState.pickResult()
                .first(mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType))
                .isMatch());
      }

      SECTION("Shift+CtrlCmd+Alt")
      {
        const auto inputState =
          inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        CHECK(inputState.pickResult()
                .first(mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType))
                .isMatch());
      }

      SECTION("Alt alone is not enough")
      {
        const auto inputState = inputStateFor(ModifierKeys::Alt);
        CHECK(!inputState.pickResult()
                 .first(mdl::HitFilters::type(ExtrudeTool::ExtrudeHitType))
                 .isMatch());
      }
    }

    SECTION("acceptMouseDrag starts a move drag with Shift+Alt in an orthographic view")
    {
      map.grid().toggleSnap(); // disable snapping for a deterministic delta

      auto inputState = inputStateFor(ModifierKeys::Shift | ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      const auto faceBefore =
        leftBrush->brush().face(*leftBrush->brush().findFace(vm::vec3d{1, 0, 0}));
      const auto boundaryBefore = faceBefore.boundary();

      // the move handle picker intersects a horizontal plane through the initial hit
      // point; shifting the ray's origin (not just its direction) moves the
      // intersection point, simulating the mouse moving across the view. Shift along
      // X, perpendicular to the dragged (+X-facing) seam face, so its plane actually
      // moves -- a shift along Y would just slide the face within its own plane.
      auto dragInputState = InputState{0.0f, 0.0f};
      const auto dragRay =
        vm::ray3d{pickRay.origin + vm::vec3d{8, 0, 0}, pickRay.direction};
      dragInputState.setPickRequest(PickRequest{dragRay, camera});
      dragInputState.setModifierKeys(ModifierKeys::Shift | ModifierKeys::Alt);
      dragInputState.mouseDown(MouseButtons::Left);
      CHECK(tracker->update(dragInputState));

      tracker->end(dragInputState);

      const auto faceAfter =
        leftBrush->brush().face(*leftBrush->brush().findFace(vm::vec3d{1, 0, 0}));
      CHECK(faceAfter.boundary() != boundaryBefore);
    }

    SECTION(
      "acceptMouseDrag starts a stamp drag with Shift+CtrlCmd+Alt in an orthographic "
      "view")
    {
      // A side view from positive X looking in -X, positioned above both brushes (z=20 >
      // z_top=16), so the 3D pick misses all faces and pick2D falls through to
      // findClosestHorizonEdge. The seam edge at x=16 is the closest horizon edge; from
      // this view direction its makeEdgeHit logic picks leftFaceHandle = top face (+Z).
      // collectCoplanarFaces then finds both brushes' top faces -- both have normal +Z at
      // z=16 -- so both handles are outward for a +Z drag and the stamp succeeds.
      map.grid().toggleSnap(); // disable snapping for a deterministic delta

      const auto sidePickRay = vm::ray3d{{64, 0, 20}, {-1, 0, 0}};
      const auto sideCamera = orthographicCameraFor(sidePickRay);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(PickRequest{sidePickRay, sideCamera});
      auto pickResult = mdl::PickResult{};
      pick(map, sidePickRay, pickResult);
      inputState.setModifierKeys(
        ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
      controller.pick(inputState, pickResult);
      inputState.setPickResult(std::move(pickResult));
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);
      REQUIRE(tool.proposedDragHandles().size() == 2);

      const auto leftBoundsBefore = leftBrush->logicalBounds();
      const auto rightBoundsBefore = rightBrush->logicalBounds();

      // shift the pick ray origin in +Z to drag the top faces upward
      auto dragInputState = InputState{0.0f, 0.0f};
      const auto dragRay =
        vm::ray3d{sidePickRay.origin + vm::vec3d{0, 0, 8}, sidePickRay.direction};
      dragInputState.setPickRequest(PickRequest{dragRay, sideCamera});
      dragInputState.setModifierKeys(
        ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
      dragInputState.mouseDown(MouseButtons::Left);
      CHECK(tracker->update(dragInputState));

      tracker->end(dragInputState);

      // stamp succeeds: both originals untouched and deselected; two new brushes added
      CHECK(leftBrush->logicalBounds() == leftBoundsBefore);
      CHECK(rightBrush->logicalBounds() == rightBoundsBefore);
      CHECK(!leftBrush->selected());
      CHECK(!rightBrush->selected());

      const auto newBrushBounds =
        map.selection().brushes
        | std::views::transform([](const auto* node) { return node->logicalBounds(); });
      CHECK_THAT(
        newBrushBounds,
        UnorderedRangeEquals(std::vector<vm::bbox3d>{
          {{-16, -16, 16}, {16, 16, 24}},
          {{16, -16, 16}, {48, 16, 24}},
        }));
    }

    SECTION(
      "acceptMouseDrag stamp drag with Shift+CtrlCmd+Alt is denied when a handle "
      "would be dragged inward in an orthographic view")
    {
      map.grid().toggleSnap(); // disable snapping for a deterministic delta

      auto inputState =
        inputStateFor(ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      const auto leftBoundsBefore = leftBrush->logicalBounds();
      const auto rightBoundsBefore = rightBrush->logicalBounds();

      // same drag ray shift technique as the move-drag test above
      auto dragInputState = InputState{0.0f, 0.0f};
      const auto dragRay =
        vm::ray3d{pickRay.origin + vm::vec3d{8, 0, 0}, pickRay.direction};
      dragInputState.setPickRequest(PickRequest{dragRay, camera});
      dragInputState.setModifierKeys(
        ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
      dragInputState.mouseDown(MouseButtons::Left);
      // The seam faces have opposing normals (+X for leftBrush, -X for rightBrush). The
      // drag delta is in the +X direction, which is inward for rightBrush's -X face, so
      // the stamp is denied to prevent overlapping brushes.
      CHECK(tracker->update(dragInputState));

      tracker->end(dragInputState);

      // stamp denied: original brushes are unchanged and still selected; no new brushes
      // were added to the scene
      CHECK(leftBrush->logicalBounds() == leftBoundsBefore);
      CHECK(rightBrush->logicalBounds() == rightBoundsBefore);
      CHECK(leftBrush->selected());
      CHECK(rightBrush->selected());
      CHECK(map.selection().brushes.size() == 2);
    }
  }
}

} // namespace tb::ui
