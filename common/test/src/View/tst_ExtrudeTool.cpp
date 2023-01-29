/*
 Copyright (C) 2010-2017 Kristian Duske
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

#include "View/ExtrudeTool.h"

#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "Renderer/PerspectiveCamera.h"

#include <kdl/result.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/approx.h>
#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <memory>

#include "MapDocumentTest.h"
#include "TestLogger.h"
#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

namespace
{
vm::vec3 n(const vm::vec3& v)
{
  return vm::normalize(v);
}
} // namespace

TEST_CASE_METHOD(ValveMapDocumentTest, "ExtrudeToolTest.pick2D")
{
  constexpr auto brushBounds = vm::bbox3{16.0};

  auto tool = ExtrudeTool{document};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 =
    new Model::BrushNode{builder.createCuboid(brushBounds, "texture").value()};

  document->addNodes({{document->currentLayer(), {brushNode1}}});
  document->selectNodes({brushNode1});

  SECTION("Pick ray hits brush directly")
  {
    constexpr auto pickRay = vm::ray3{{0, 0, 32}, {0, 0, -1}};

    auto pickResult = Model::PickResult{};
    document->pick(pickRay, pickResult);

    REQUIRE(pickResult.all().size() == 1);

    const auto hit = tool.pick2D(pickRay, pickResult);
    CHECK_FALSE(hit.isMatch());
  }

  SECTION("Pick ray does not hit brush directly")
  {
    using T = std::tuple<vm::vec3, vm::vec3, vm::vec3, vm::vec3, vm::plane3, vm::vec3>;

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

    const auto hit = tool.pick2D(vm::ray3{origin, vm::normalize(direction)}, {});

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

TEST_CASE_METHOD(ValveMapDocumentTest, "ExtrudeToolTest.pick3D")
{
  constexpr auto brushBounds = vm::bbox3{16.0};

  auto tool = ExtrudeTool{document};

  auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  auto* brushNode1 =
    new Model::BrushNode{builder.createCuboid(brushBounds, "texture").value()};

  document->addNodes({{document->currentLayer(), {brushNode1}}});
  document->selectNodes({brushNode1});

  SECTION("Pick ray hits brush directly")
  {
    const auto pickRay = vm::ray3{{0, 0, 24}, vm::normalize(vm::vec3{-1, 0, -1})};

    auto pickResult = Model::PickResult{};
    document->pick(pickRay, pickResult);

    REQUIRE(pickResult.all().size() == 1);

    const auto hit = tool.pick3D(pickRay, pickResult);

    CHECK(hit.isMatch());
    CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
    CHECK(hit.hitPoint() == vm::vec3{-8, 0, 16});
    CHECK(hit.distance() == vm::approx{vm::length(hit.hitPoint() - pickRay.origin)});

    CHECK(
      hit.target<ExtrudeHitData>()
      == ExtrudeHitData{
        {brushNode1, *brushNode1->brush().findFace(vm::vec3{0, 0, 1})},
        vm::line3{hit.hitPoint(), {0, 0, 1}},
        hit.hitPoint()});
  }

  SECTION("Pick ray does not hit brush directly")
  {
    using T = std::tuple<vm::vec3, vm::vec3, vm::vec3, vm::vec3, vm::plane3, vm::vec3>;

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

    const auto hit = tool.pick3D(vm::ray3{origin, vm::normalize(direction)}, {});

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

/**
 * Boilerplate to perform picking
 */
static Model::PickResult performPick(
  View::MapDocument& document, ExtrudeTool& tool, const vm::ray3& pickRay)
{
  auto pickResult = Model::PickResult::byDistance();
  document.pick(pickRay, pickResult); // populate pickResult

  const auto hit = tool.pick3D(pickRay, pickResult);
  CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
  CHECK_FALSE(vm::is_nan(hit.hitPoint()));

  REQUIRE(hit.isMatch());
  pickResult.addHit(hit);

  REQUIRE(tool.proposedDragHandles().empty());
  tool.updateProposedDragHandles(pickResult);
  REQUIRE_FALSE(tool.proposedDragHandles().empty());

  return pickResult;
}

/**
 * Test for https://github.com/TrenchBroom/TrenchBroom/issues/3726
 */
TEST_CASE("ExtrudeToolTest.findDragFaces")
{
  using T = std::tuple<IO::Path, std::vector<std::string>>;

  // clang-format off
  const auto 
  [mapName,                                        expectedDragFaceTextureNames] = GENERATE(values<T>({
  {IO::Path{"findDragFaces_noCoplanarFaces.map"},  {"larger_top_face"}},
  {IO::Path{"findDragFaces_twoCoplanarFaces.map"}, {"larger_top_face", "smaller_top_face"}}
  }));
  // clang-format on

  const auto mapPath = IO::Path{"fixture/test/View/ExtrudeToolTest"} + mapName;
  auto [document, game, gameConfig] =
    View::loadMapDocument(mapPath, "Quake", Model::MapFormat::Valve);

  document->selectAllNodes();

  auto brushes = document->selectedNodes().brushes();
  REQUIRE(brushes.size() == 2);

  const auto brushIt = std::find_if(
    std::begin(brushes), std::end(brushes), [](const Model::BrushNode* brushNode) {
      return brushNode->brush().findFace("larger_top_face").has_value();
    });
  REQUIRE(brushIt != std::end(brushes));

  const auto* brushNode = *brushIt;
  const auto& largerTopFace =
    brushNode->brush().face(brushNode->brush().findFace("larger_top_face").value());

  // Find the entity defining the camera position for our test
  auto* cameraEntity =
    kdl::vec_filter(document->selectedNodes().entities(), [](const auto* e) {
      return e->entity().classname() == "trigger_relay";
    }).front();

  // Fire a pick ray at largerTopFace
  const auto pickRay = vm::ray3{
    cameraEntity->entity().origin(),
    vm::normalize(largerTopFace.center() - cameraEntity->entity().origin())};

  auto tool = ExtrudeTool{document};

  const auto pickResult = performPick(*document, tool, pickRay);
  REQUIRE(
    pickResult.all().front().target<Model::BrushFaceHandle>().face() == largerTopFace);

  CHECK_THAT(
    kdl::vec_transform(
      tool.proposedDragHandles(),
      [](const auto& h) { return h.faceAtDragStart().attributes().textureName(); }),
    Catch::Matchers::UnorderedEquals(expectedDragFaceTextureNames));
}

TEST_CASE("ExtrudeToolTest.splitBrushes")
{
  using namespace Model::HitFilters;

  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path{"fixture/test/View/ExtrudeToolTest/splitBrushes.map"},
    "Quake",
    Model::MapFormat::Valve);

  document->selectAllNodes();

  auto brushes = document->selectedNodes().brushes();
  REQUIRE(brushes.size() == 2);

  // Find the entity defining the camera position for our test
  const auto* cameraEntity =
    kdl::vec_filter(document->selectedNodes().entities(), [](const auto* node) {
      return node->entity().classname() == "trigger_relay";
    }).front();

  const auto* cameraTarget =
    kdl::vec_filter(document->selectedNodes().entities(), [](const auto* node) {
      return node->entity().classname() == "info_null";
    }).front();

  const auto* funcDetailNode =
    kdl::vec_filter(
      Model::filterEntityNodes(Model::collectDescendants({document->world()})),
      [](const auto* node) { return node->entity().classname() == "func_detail"; })
      .front();

  // Fire a pick ray at cameraTarget
  const auto pickRay = vm::ray3(
    cameraEntity->entity().origin(),
    vm::normalize(cameraTarget->entity().origin() - cameraEntity->entity().origin()));

  auto tool = ExtrudeTool{document};

  const auto pickResult = performPick(*document, tool, pickRay);

  // We are going to drag the 2 faces with +Y normals
  CHECK(
    kdl::vec_transform(
      tool.proposedDragHandles(),
      [](const auto& h) { return h.faceAtDragStart().normal(); })
    == std::vector<vm::vec3>{vm::vec3::pos_y(), vm::vec3::pos_y()});

  const auto hit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
  auto dragState = ExtrudeDragState{
    tool.proposedDragHandles(),
    ExtrudeTool::getDragFaces(tool.proposedDragHandles()),
    false,
    vm::vec3::zero()};

  SECTION("split brushes inwards 32 units towards -Y")
  {
    const auto delta = vm::vec3(0, -32, 0);

    dragState.splitBrushes = true;
    tool.beginExtrude();

    REQUIRE(tool.extrude(delta, dragState));
    tool.commit(dragState);

    CHECK(document->selectedNodes().brushes().size() == 4);

    SECTION("check 2 resulting worldspawn brushes")
    {
      const auto nodes = Model::filterBrushNodes(document->currentLayer()->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{
        {{-32, 144, 16}, {-16, 192, 32}}, {{-32, 192, 16}, {-16, 224, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }

    SECTION("check 2 resulting func_detail brushes")
    {
      const auto nodes = Model::filterBrushNodes(funcDetailNode->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{
        {{-16, 176, 16}, {16, 192, 32}}, {{-16, 192, 16}, {16, 224, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }
  }

  SECTION("split brushes inwards 48 units towards -Y")
  {
    const auto delta = vm::vec3(0, -48, 0);

    dragState.splitBrushes = true;
    tool.beginExtrude();

    REQUIRE(tool.extrude(delta, dragState));
    tool.commit(dragState);

    CHECK(document->selectedNodes().brushes().size() == 3);

    SECTION("check 2 resulting worldspawn brushes")
    {
      const auto nodes = Model::filterBrushNodes(document->currentLayer()->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{
        {{-32, 144, 16}, {-16, 176, 32}}, {{-32, 176, 16}, {-16, 224, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }

    SECTION("check 1 resulting func_detail brush")
    {
      const auto nodes = Model::filterBrushNodes(funcDetailNode->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{{{-16, 176, 16}, {16, 224, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }
  }

  SECTION("extrude inwards 32 units towards -Y")
  {
    const auto delta = vm::vec3{0, -32, 0};

    dragState.splitBrushes = false;
    tool.beginExtrude();

    REQUIRE(tool.extrude(delta, dragState));
    tool.commit(dragState);

    CHECK(document->selectedNodes().brushes().size() == 2);

    SECTION("check 1 resulting worldspawn brushes")
    {
      const auto nodes = Model::filterBrushNodes(document->currentLayer()->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{
        {{-32, 144, 16}, {-16, 192, 32}},
      };
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }

    SECTION("check 1 resulting func_detail brush")
    {
      const auto nodes = Model::filterBrushNodes(funcDetailNode->children());
      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{{{-16, 176, 16}, {16, 192, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }
  }

  SECTION("split brushes outwards 16 units towards +Y")
  {
    const auto delta = vm::vec3{0, 16, 0};

    dragState.splitBrushes = true;
    tool.beginExtrude();

    REQUIRE(tool.extrude(delta, dragState));
    tool.commit(dragState);

    CHECK(document->selectedNodes().brushes().size() == 2);

    SECTION("check 1 resulting worldspawn brush")
    {
      auto nodes = Model::filterBrushNodes(document->currentLayer()->children());
      nodes = kdl::vec_filter(
        std::move(nodes), [](const auto* node) { return node->selected(); });

      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{
        {{-32, 224, 16}, {-16, 240, 32}},
      };
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }

    SECTION("check 1 resulting func_detail brush")
    {
      auto nodes = Model::filterBrushNodes(funcDetailNode->children());
      nodes = kdl::vec_filter(
        std::move(nodes), [](const auto* node) { return node->selected(); });

      const auto bounds =
        kdl::vec_transform(nodes, [](const auto* node) { return node->logicalBounds(); });
      const auto expectedBounds = std::vector<vm::bbox3>{{{-16, 224, 16}, {16, 240, 32}}};
      CHECK_THAT(bounds, Catch::Matchers::UnorderedEquals(expectedBounds));
    }
  }
}
} // namespace TrenchBroom::View
