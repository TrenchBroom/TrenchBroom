/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2020 Eric Wasylishen

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

#include "Matchers.h"
#include "gl/Camera.h"
#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PickResult.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/ExtrudeTool.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "kd/result.h"

#include "vm/approx.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <algorithm>
#include <filesystem>
#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

namespace
{

vm::vec3d n(const vm::vec3d& v)
{
  return vm::normalize(v);
}

vm::vec3f upFor(const vm::vec3f& direction)
{
  return vm::abs(direction.z()) < 0.9f ? vm::vec3f{0, 0, 1} : vm::vec3f{0, 1, 0};
}

/**
 * Build a camera positioned at the pick ray's origin looking along its direction, so
 * that the handle radius scaling used by the edge picking is realistic.
 */
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
    upFor(direction)};
}

gl::OrthographicCamera orthographicCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  return gl::OrthographicCamera{
    1.0f, 8000.0f, viewport, vm::vec3f{pickRay.origin}, direction, upFor(direction)};
}

mdl::PickResult performPick(mdl::Map& map, ExtrudeTool& tool, const vm::ray3d& pickRay)
{
  auto pickResult = mdl::PickResult::byDistance();
  pick(map, pickRay, pickResult);

  const auto hit = tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);
  CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
  CHECK_FALSE(vm::is_nan(hit.hitPoint()));

  REQUIRE(hit.isMatch());
  pickResult.addHit(hit);

  REQUIRE(tool.proposedDragHandles().empty());
  tool.updateProposedDragHandles(pickResult);
  REQUIRE_FALSE(tool.proposedDragHandles().empty());

  return pickResult;
}

} // namespace

TEST_CASE("ExtrudeTool")
{
  auto fixture = MapDocumentFixture{};

  SECTION("pick2D")
  {
    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    constexpr auto brushBounds = vm::bbox3d{16.0};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode1 =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1}}});
    selectNodes(map, {brushNode1});

    SECTION("Pick ray hits brush directly")
    {
      constexpr auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);

      REQUIRE(pickResult.all().size() == 1);

      const auto hit = tool.pick2D(pickRay, orthographicCameraFor(pickRay), pickResult);
      CHECK_FALSE(hit.isMatch());
    }

    SECTION("Pick ray does not hit brush directly")
    {
      using T =
        std::tuple<vm::vec3d, vm::vec3d, vm::vec3d, vm::vec3d, vm::plane3d, vm::vec3d>;

      // clang-format off
      const auto
      [origin,     direction,     expectedFaceNormal, expectedHitPoint, expectedDragReference,          expectedHandlePosition] = GENERATE(values<T>({
      // shoot from above downwards just past the top west edge, picking the west face
      {{-17, 0, 32}, { 0, 0, -1}, {-1, 0, 0},         {-17, 0, 16},     {{-16, 0, 16}, {0, 0, -1}},     {-16, 0, 16}},
      // shoot diagonally past the top west edge, picking the west face
      {{ -1, 0, 33}, {-1, 0, -1}, {-1, 0, 0},         {-17, 0, 17},     {{-16, 0, 16}, n({-1, 0, -1})}, {-16, 0, 16}},
      }));
      // clang-format on

      CAPTURE(brushBounds, origin, direction);

      const auto pickRay = vm::ray3d{origin, vm::normalize(direction)};
      const auto hit = tool.pick2D(pickRay, orthographicCameraFor(pickRay), {});

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == expectedHitPoint);
      CHECK(hit.distance() == vm::approx{vm::length(expectedHitPoint - origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(expectedFaceNormal)},
          expectedDragReference,
          expectedHandlePosition});
    }
  }

  SECTION("pick3D")
  {
    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    constexpr auto brushBounds = vm::bbox3d{16.0};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode1 =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1}}});
    selectNodes(map, {brushNode1});

    SECTION("Pick ray hits brush directly")
    {
      const auto pickRay = vm::ray3d{{0, 0, 24}, vm::normalize(vm::vec3d{-1, 0, -1})};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);

      REQUIRE(pickResult.all().size() == 1);

      const auto hit = tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == vm::vec3d{-8, 0, 16});
      CHECK(hit.distance() == vm::approx{vm::length(hit.hitPoint() - pickRay.origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(vm::vec3d{0, 0, 1})},
          vm::line3d{hit.hitPoint(), {0, 0, 1}},
          hit.hitPoint()});
    }

    SECTION("Pick ray does not hit brush directly")
    {
      using T =
        std::tuple<vm::vec3d, vm::vec3d, vm::vec3d, vm::vec3d, vm::plane3d, vm::vec3d>;

      // clang-format off
      const auto
      [origin,     direction,     expectedFaceNormal, expectedHitPoint, expectedDragReference,     expectedHandlePosition] = GENERATE(values<T>({
      // shoot from above downwards just past the top west edge, picking the west face
      {{-17, 0, 32}, { 0, 0, -1}, {-1, 0, 0},         {-17, 0, 16},     {{-16, 0, 16}, {0, 0, 1}}, {-16, 0, 16}},
      // shoot diagonally past the top west edge, picking the west face
      {{ -1, 0, 33}, {-1, 0, -1}, {-1, 0, 0},         {-17, 0, 17},     {{-16, 0, 16}, {0, 0, 1}}, {-16, 0, 16}},
      }));
      // clang-format on

      CAPTURE(brushBounds, origin, direction);

      const auto pickRay = vm::ray3d{origin, vm::normalize(direction)};
      const auto hit = tool.pick3D(pickRay, perspectiveCameraFor(pickRay), {});

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == expectedHitPoint);
      CHECK(hit.distance() == vm::approx{vm::length(expectedHitPoint - origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(expectedFaceNormal)},
          expectedDragReference,
          expectedHandlePosition});
    }
  }

  SECTION("Pick coplanar faces of two brushes")
  {
    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

    // two brushes side by side along the X axis, sharing a coplanar top face at z = 16
    auto* brushNode1 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "material")
      | kdl::value()};
    auto* brushNode2 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{16, -16, -16}, {48, 16, 16}}, "material")
      | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1, brushNode2}}});
    selectNodes(map, {brushNode1, brushNode2});

    // shoot straight down at the top face of the first brush
    const auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};

    const auto pickResult = performPick(map, tool, pickRay);

    // both coplanar top faces are chosen as drag handles, one per brush
    CHECK_THAT(
      tool.proposedDragHandles()
        | std::views::transform([](const auto& h) { return h.faceHandle.node(); }),
      UnorderedRangeEquals(std::vector<mdl::BrushNode*>{brushNode1, brushNode2}));

    CHECK_THAT(
      tool.proposedDragHandles() | std::views::transform([](const auto& h) {
        return h.faceAtDragStart().normal();
      }),
      RangeEquals(std::vector<vm::vec3d>{{0, 0, 1}, {0, 0, 1}}));
  }

  SECTION("Pick opposing coplanar faces of two brushes")
  {
    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

    // brushNode2 sits on top of brushNode1: brushNode1's top face (+Z) and brushNode2's
    // bottom face (-Z) are coincident at z = 16 but face in opposite directions.
    // brushNode2 is offset along +X so that the western half of brushNode1's top face
    // remains exposed and can be picked directly.
    auto* brushNode1 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "material")
      | kdl::value()};
    auto* brushNode2 = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{0, -16, 16}, {32, 16, 48}}, "material")
      | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1, brushNode2}}});
    selectNodes(map, {brushNode1, brushNode2});

    // shoot straight down at the exposed part of the first brush's top face
    const auto pickRay = vm::ray3d{{-8, 0, 32}, {0, 0, -1}};

    const auto pickResult = performPick(map, tool, pickRay);

    // both coincident faces are chosen as drag handles, one per brush
    CHECK_THAT(
      tool.proposedDragHandles()
        | std::views::transform([](const auto& h) { return h.faceHandle.node(); }),
      UnorderedRangeEquals(std::vector<mdl::BrushNode*>{brushNode1, brushNode2}));

    // the picked coplanar face faces +Z, the opposing face faces -Z
    CHECK_THAT(
      tool.proposedDragHandles() | std::views::transform([](const auto& h) {
        return h.faceAtDragStart().normal();
      }),
      UnorderedRangeEquals(std::vector<vm::vec3d>{{0, 0, 1}, {0, 0, -1}}));
  }

  SECTION("Pick a horizon edge handle directly")
  {
    using namespace mdl::HitFilters;

    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

    // Two brushes meeting at the plane y = 16 with identical vertices: frontBrush's +Y
    // face and backBrush's -Y face are coincident but face in opposite directions. This
    // shared seam is hidden between the brushes and cannot be picked as a face.
    auto* frontBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "material")
      | kdl::value()};
    auto* backBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, 16, -16}, {16, 48, 16}}, "material")
      | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {frontBrush, backBrush}}});
    selectNodes(map, {frontBrush, backBrush});

    SECTION("edge handle wins over the adjacent face it shares")
    {
      // Look at the seam from the -Y side and slightly above. The ray hits frontBrush's
      // top face just short of the seam, but passes within the handle radius of the
      // seam's top edge (a horizon edge: top face visible, +Y seam face hidden). The
      // adjacent top face is nearer than the edge, yet the edge handle still wins.
      const auto pickRay = vm::ray3d{{0, -556, 586}, vm::normalize(vm::vec3d{0, 1, -1})};

      const auto pickResult = performPick(map, tool, pickRay);

      // the hidden +Y seam face of frontBrush is chosen for extrusion
      const auto extrudeHit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
      CHECK(
        extrudeHit.target<ExtrudeHitData>().face
        == mdl::BrushFaceHandle{
          frontBrush, *frontBrush->brush().findFace(vm::vec3d{0, 1, 0})});

      // both coincident seam faces become drag handles, one per brush
      CHECK_THAT(
        tool.proposedDragHandles()
          | std::views::transform([](const auto& h) { return h.faceHandle.node(); }),
        UnorderedRangeEquals(std::vector<mdl::BrushNode*>{frontBrush, backBrush}));
      CHECK_THAT(
        tool.proposedDragHandles() | std::views::transform([](const auto& h) {
          return h.faceAtDragStart().normal();
        }),
        UnorderedRangeEquals(std::vector<vm::vec3d>{{0, 1, 0}, {0, -1, 0}}));
    }

    SECTION("an occluding face in front of the edge wins")
    {
      // A third brush sits in front of the seam, closer to the camera, so the ray hits
      // its face before reaching the seam edge. It is not adjacent to the edge and is
      // nearer, so it wins over the edge handle.
      auto* occluderBrush = new mdl::BrushNode{
        builder.createCuboid(vm::bbox3d{{-16, -100, 114}, {16, -68, 146}}, "material")
        | kdl::value()};
      addNodes(map, {{map.editorContext().currentLayer(), {occluderBrush}}});
      selectNodes(map, {occluderBrush});

      const auto pickRay = vm::ray3d{{0, -556, 586}, vm::normalize(vm::vec3d{0, 1, -1})};

      const auto pickResult = performPick(map, tool, pickRay);

      const auto extrudeHit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
      CHECK(
        extrudeHit.target<ExtrudeHitData>().face
        == mdl::BrushFaceHandle{
          occluderBrush, *occluderBrush->brush().findFace(vm::vec3d{0, -1, 0})});
    }

    SECTION("a face interior away from any edge is picked normally")
    {
      // Straight down onto the middle of frontBrush's top face, far from any edge.
      const auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};

      const auto pickResult = performPick(map, tool, pickRay);

      const auto extrudeHit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
      CHECK(
        extrudeHit.target<ExtrudeHitData>().face
        == mdl::BrushFaceHandle{
          frontBrush, *frontBrush->brush().findFace(vm::vec3d{0, 0, 1})});
    }
  }

  SECTION("Pick a horizon edge handle directly in 2D")
  {
    auto& document = fixture.create();
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

    // Two brushes meeting at the plane x = 16: leftBrush's +X face and rightBrush's -X
    // face are coincident but face in opposite directions.
    auto* leftBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{-16, -16, -16}, {16, 16, 16}}, "material")
      | kdl::value()};
    auto* rightBrush = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{{16, -16, -16}, {48, 16, 16}}, "material")
      | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {leftBrush, rightBrush}}});
    selectNodes(map, {leftBrush, rightBrush});

    // Top view (looking -Z). The ray hits leftBrush's top face just short of the seam at
    // x = 16, but passes within the handle radius of the seam's top edge.
    const auto pickRay = vm::ray3d{{12, 0, 32}, {0, 0, -1}};

    auto pickResult = mdl::PickResult::byDistance();
    pick(map, pickRay, pickResult);

    const auto hit = tool.pick2D(pickRay, orthographicCameraFor(pickRay), pickResult);
    REQUIRE(hit.isMatch());
    pickResult.addHit(hit);
    tool.updateProposedDragHandles(pickResult);

    // both coincident seam faces become drag handles, one per brush
    CHECK_THAT(
      tool.proposedDragHandles()
        | std::views::transform([](const auto& h) { return h.faceHandle.node(); }),
      UnorderedRangeEquals(std::vector<mdl::BrushNode*>{leftBrush, rightBrush}));
    CHECK_THAT(
      tool.proposedDragHandles() | std::views::transform([](const auto& h) {
        return h.faceAtDragStart().normal();
      }),
      UnorderedRangeEquals(std::vector<vm::vec3d>{{1, 0, 0}, {-1, 0, 0}}));
  }

  SECTION("findDragFaces")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/3726

    using T = std::tuple<std::filesystem::path, std::vector<std::string>>;

    // clang-format off
    const auto 
    [mapName,                              expectedDragFaceMaterialNames] = GENERATE(values<T>({
    {"findDragFaces_noCoplanarFaces.map",  {"larger_top_face"}},
    {"findDragFaces_twoCoplanarFaces.map", {"larger_top_face", "smaller_top_face"}}
    }));
    // clang-format on

    const auto mapPath = "test/ui/ExtrudeTool" / mapName;
    auto& document = fixture.load(mapPath, {.mapFormat = mdl::MapFormat::Valve});
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    selectAllNodes(map);

    auto brushes = map.selection().brushes;
    REQUIRE(brushes.size() == 2);

    const auto brushIt =
      std::ranges::find_if(brushes, [](const mdl::BrushNode* brushNode) {
        return brushNode->brush().findFace("larger_top_face").has_value();
      });
    REQUIRE(brushIt != std::end(brushes));

    const auto* brushNode = *brushIt;
    const auto& largerTopFace =
      brushNode->brush().face(brushNode->brush().findFace("larger_top_face").value());

    // Find the entity defining the camera position for our test
    auto* cameraEntity =
      (map.selection().entities | std::views::filter([](const auto* e) {
         return e->entity().classname() == "trigger_relay";
       }))
        .front();

    // Fire a pick ray at largerTopFace
    const auto pickRay = vm::ray3d{
      cameraEntity->entity().origin(),
      vm::normalize(largerTopFace.center() - cameraEntity->entity().origin())};

    const auto pickResult = performPick(map, tool, pickRay);
    REQUIRE(
      pickResult.all().front().target<mdl::BrushFaceHandle>().face() == largerTopFace);

    CHECK_THAT(
      tool.proposedDragHandles() | std::views::transform([](const auto& h) {
        return h.faceAtDragStart().attributes().materialName();
      }),
      UnorderedRangeEquals(expectedDragFaceMaterialNames));
  }

  SECTION("splitBrushes")
  {
    using namespace mdl::HitFilters;


    const auto mapPath = "test/ui/ExtrudeTool/splitBrushes.map";
    auto& document = fixture.load(mapPath, {.mapFormat = mdl::MapFormat::Valve});
    auto& map = document.map();

    auto tool = ExtrudeTool{document};

    selectAllNodes(map);

    auto brushes = map.selection().brushes;
    REQUIRE(brushes.size() == 2);

    // Find the entity defining the camera position for our test
    const auto* cameraEntity =
      (map.selection().entities | std::views::filter([](const auto* node) {
         return node->entity().classname() == "trigger_relay";
       }))
        .front();

    const auto* cameraTarget =
      (map.selection().entities | std::views::filter([](const auto* node) {
         return node->entity().classname() == "info_null";
       }))
        .front();

    const auto* funcDetailNode =
      (mdl::filterEntityNodes(mdl::collectDescendants({&map.worldNode()}))
       | std::views::filter(
         [](const auto* node) { return node->entity().classname() == "func_detail"; }))
        .front();

    // Fire a pick ray at cameraTarget
    const auto pickRay = vm::ray3d(
      cameraEntity->entity().origin(),
      vm::normalize(cameraTarget->entity().origin() - cameraEntity->entity().origin()));

    const auto pickResult = performPick(map, tool, pickRay);

    // We are going to drag the 2 faces with +Y normals
    CHECK_THAT(
      tool.proposedDragHandles() | std::views::transform([](const auto& h) {
        return h.faceAtDragStart().normal();
      }),
      RangeEquals(std::vector<vm::vec3d>{{0, 1, 0}, {0, 1, 0}}));

    const auto hit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
    auto dragState = ExtrudeDragState{
      tool.proposedDragHandles(),
      ExtrudeTool::getDragFaces(tool.proposedDragHandles()),
      false,
      vm::vec3d{0, 0, 0}};

    SECTION("split brushes inwards 32 units towards -Y")
    {
      const auto delta = vm::vec3d(0, -32, 0);

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 4);

      SECTION("check 2 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{
            {{-32, 144, 16}, {-16, 192, 32}},
            {{-32, 192, 16}, {-16, 224, 32}},
          }));
      }

      SECTION("check 2 resulting func_detail brushes")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{
            {{-16, 176, 16}, {16, 192, 32}},
            {{-16, 192, 16}, {16, 224, 32}},
          }));
      }

      CHECK_THAT(
        map.selection().brushes | std::views::transform([](const auto* brushNode) {
          return brushNode->linkId();
        }) | kdl::ranges::to<std::vector>(),
        AllDifferent<std::vector<std::string>>());
    }

    SECTION("split brushes inwards 48 units towards -Y")
    {
      const auto delta = vm::vec3d(0, -48, 0);

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 3);

      SECTION("check 2 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{
            {{-32, 144, 16}, {-16, 176, 32}},
            {{-32, 176, 16}, {-16, 224, 32}},
          }));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{{{-16, 176, 16}, {16, 224, 32}}}));
      }
    }

    SECTION("extrude inwards 32 units towards -Y")
    {
      const auto delta = vm::vec3d{0, -32, 0};

      dragState.splitBrushes = false;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 2);

      SECTION("check 1 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{
            {{-32, 144, 16}, {-16, 192, 32}},
          }));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = nodes | std::views::transform([](const auto* node) {
                              return node->logicalBounds();
                            });

        CHECK_THAT(
          bounds,
          UnorderedRangeEquals(std::vector<vm::bbox3d>{{{-16, 176, 16}, {16, 192, 32}}}));
      }
    }

    SECTION("split brushes outwards 16 units towards +Y")
    {
      const auto delta = vm::vec3d{0, 16, 0};

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 2);

      SECTION("check 1 resulting worldspawn brush")
      {
        const auto bounds =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children())
          | std::views::filter([](const auto* node) { return node->selected(); })
          | std::views::transform([](const auto* node) { return node->logicalBounds(); })
          | kdl::ranges::to<std::vector>();

        CHECK_THAT(
          bounds,
          UnorderedEquals(std::vector<vm::bbox3d>{
            {{-32, 224, 16}, {-16, 240, 32}},
          }));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        const auto bounds =
          mdl::filterBrushNodes(funcDetailNode->children())
          | std::views::filter([](const auto* node) { return node->selected(); })
          | std::views::transform([](const auto* node) { return node->logicalBounds(); })
          | kdl::ranges::to<std::vector>();

        CHECK_THAT(
          bounds,
          UnorderedEquals(std::vector<vm::bbox3d>{{{-16, 224, 16}, {16, 240, 32}}}));
      }

      CHECK_THAT(
        map.selection().brushes | std::views::transform([](const auto* brushNode) {
          return brushNode->linkId();
        }) | kdl::ranges::to<std::vector>(),
        AllDifferent<std::vector<std::string>>());
    }
  }
}

} // namespace tb::ui
