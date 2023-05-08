/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Assets/Texture.h"
#include "Catch2.h"
#include "Exceptions.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "TestUtils.h"

#include <kdl/collection_utils.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/approx.h>
#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/polygon.h>
#include <vecmath/ray.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("BrushNodeTest.entity")
{
  const auto worldBounds = vm::bbox3{4096.0};

  auto* brushNode = new BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(64.0, "testure").value()};
  auto entityNode = EntityNode{Entity{}};

  CHECK(brushNode->entity() == nullptr);

  entityNode.addChild(brushNode);
  CHECK(brushNode->entity() == &entityNode);
}

TEST_CASE("BrushNodeTest.hasSelectedFaces")
{
  const vm::bbox3 worldBounds(4096.0);

  // build a cube with length 16 at the origin
  BrushNode brush(
    Brush::create(
      worldBounds,
      {
        // left
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(0.0, 0.0, 1.0)),
        // right
        createParaxial(
          vm::vec3(16.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0)),
        // front
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0)),
        // back
        createParaxial(
          vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0)),
        // top
        createParaxial(
          vm::vec3(0.0, 0.0, 16.0), vm::vec3(0.0, 1.0, 16.0), vm::vec3(1.0, 0.0, 16.0)),
        // bottom
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
      })
      .value());

  CHECK(!brush.hasSelectedFaces());

  SECTION("Selecting faces correctly updates the node's face selection count")
  {
    brush.selectFace(0u);
    CHECK(brush.hasSelectedFaces());

    brush.selectFace(1u);
    CHECK(brush.hasSelectedFaces());

    brush.deselectFace(0u);
    CHECK(brush.hasSelectedFaces());

    brush.deselectFace(1u);
    CHECK(!brush.hasSelectedFaces());
  }

  SECTION(
    "Passing a brush with selected faces to constructor clears the brushes face "
    "selection")
  {
    REQUIRE(!brush.hasSelectedFaces());

    Brush copy = brush.brush();
    copy.face(0u).select();
    copy.face(1u).select();

    BrushNode another(std::move(copy));
    CHECK(!another.hasSelectedFaces());
  }

  SECTION(
    "Setting a brush with selected faces correctly updates the node's face selection "
    "count")
  {
    REQUIRE(!brush.hasSelectedFaces());

    Brush copy = brush.brush();
    copy.face(0u).select();
    copy.face(1u).select();

    brush.setBrush(std::move(copy));
    CHECK(brush.hasSelectedFaces());

    brush.deselectFace(0u);
    CHECK(brush.hasSelectedFaces());

    brush.deselectFace(1u);
    CHECK(!brush.hasSelectedFaces());
  }

  SECTION(
    "Cloning a brush node with selected faces returns a clone with no selected faces")
  {
    REQUIRE(!brush.hasSelectedFaces());

    brush.selectFace(0u);
    brush.selectFace(1u);
    REQUIRE(brush.hasSelectedFaces());

    auto clone = std::unique_ptr<BrushNode>(brush.clone(worldBounds));
    CHECK(!clone->hasSelectedFaces());
  }
}

TEST_CASE("BrushNodeTest.containsPatchNode")
{
  const auto worldBounds = vm::bbox3d{8192.0};

  auto builder = Model::BrushBuilder{MapFormat::Quake3, worldBounds};
  auto brushNode = Model::BrushNode{builder.createCube(64.0, "some_texture").value()};
  transformNode(
    brushNode, vm::rotation_matrix(0.0, 0.0, vm::to_radians(45.0)), worldBounds);

  // a half cylinder that, at this position, just sticks out of the brush
  // clang-format off
  auto patchNode = Model::PatchNode{Model::BezierPatch{3, 5, {
    { {32, 0,  16}, {32, 32,  16}, {0, 32,  16}, {-32,32,  16}, {-32, 0,  16},
      {32, 0,   0}, {32, 32,   0}, {0, 32,   0}, {-32,32,   0}, {-32, 0,   0},
      {32, 0, -16}, {32, 32, -16}, {0, 32, -16}, {-32,32, -16}, {-32, 0, -16}, }
  }, "some_texture"}};
  // clang-format on

  CHECK_FALSE(brushNode.contains(&patchNode));

  transformNode(patchNode, vm::translation_matrix(vm::vec3{0, -8, 0}), worldBounds);
  CHECK(brushNode.contains(&patchNode));

  transformNode(patchNode, vm::translation_matrix(vm::vec3{0, 0, 32}), worldBounds);
  CHECK_FALSE(brushNode.contains(&patchNode));
}

TEST_CASE("BrushNodeTest.intersectsPatchNode")
{
  const auto worldBounds = vm::bbox3d{8192.0};

  auto builder = Model::BrushBuilder{MapFormat::Quake3, worldBounds};

  auto brushNode = Model::BrushNode{builder.createCube(64.0, "some_texture").value()};
  transformNode(
    brushNode, vm::rotation_matrix(0.0, 0.0, vm::to_radians(45.0)), worldBounds);

  // a half cylinder that, at this position, just sticks out of the brush
  // clang-format off
  auto patchNode = Model::PatchNode{Model::BezierPatch{3, 5, {
    { {32, 0,  16}, {32, 32,  16}, {0, 32,  16}, {-32,32,  16}, {-32, 0,  16},
      {32, 0,   0}, {32, 32,   0}, {0, 32,   0}, {-32,32,   0}, {-32, 0,   0},
      {32, 0, -16}, {32, 32, -16}, {0, 32, -16}, {-32,32, -16}, {-32, 0, -16}, }
  }, "some_texture"}};
  // clang-format on

  CHECK(brushNode.intersects(&patchNode));

  SECTION("Brush contains patch")
  {
    transformNode(patchNode, vm::translation_matrix(vm::vec3{0, -8, 0}), worldBounds);
    CHECK(brushNode.intersects(&patchNode));
  }

  SECTION("Patch sticks out of top of brush")
  {
    transformNode(patchNode, vm::translation_matrix(vm::vec3{0, -8, 32}), worldBounds);
    CHECK(brushNode.intersects(&patchNode));
  }

  SECTION("Patch is above brush")
  {
    transformNode(patchNode, vm::translation_matrix(vm::vec3{0, -8, 64}), worldBounds);
    CHECK_FALSE(brushNode.intersects(&patchNode));
  }

  SECTION("Patch doesn't touch brush, but bounds intersect")
  {
    transformNode(patchNode, vm::translation_matrix(vm::vec3{0, 32, 0}), worldBounds);
    CHECK_FALSE(brushNode.intersects(&patchNode));
  }

  SECTION("Brush does not contain any grid points, but patch intersects")
  {
    auto thinBrushNode = Model::BrushNode{
      builder
        .createCuboid(
          vm::bbox3d{vm::vec3d{1, -64, -64}, vm::vec3d{2, 64, 64}}, "some_texture")
        .value()};
    for (const auto& point : patchNode.grid().points)
    {
      REQUIRE_FALSE(thinBrushNode.brush().containsPoint(point.position));
    }
    CHECK(thinBrushNode.intersects(&patchNode));
  }
}

TEST_CASE("BrushNodeTest.pick")
{
  const vm::bbox3 worldBounds(4096.0);
  const auto editorContext = EditorContext{};

  // build a cube with length 16 at the origin
  BrushNode brush(
    Brush::create(
      worldBounds,
      {
        // left
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(0.0, 0.0, 1.0)),
        // right
        createParaxial(
          vm::vec3(16.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0)),
        // front
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0)),
        // back
        createParaxial(
          vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0)),
        // top
        createParaxial(
          vm::vec3(0.0, 0.0, 16.0), vm::vec3(0.0, 1.0, 16.0), vm::vec3(1.0, 0.0, 16.0)),
        // bottom
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
      })
      .value());

  PickResult hits1;
  brush.pick(editorContext, vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::pos_y()), hits1);
  CHECK(hits1.size() == 1u);

  Hit hit1 = hits1.all().front();
  CHECK(hit1.distance() == vm::approx(8.0));
  CHECK(hitToFaceHandle(hit1)->face().boundary().normal == vm::vec3::neg_y());

  PickResult hits2;
  brush.pick(editorContext, vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::neg_y()), hits2);
  CHECK(hits2.empty());
}

TEST_CASE("BrushNodeTest.clone")
{
  const vm::bbox3 worldBounds(4096.0);

  // build a cube with length 16 at the origin
  BrushNode original(
    Brush::create(
      worldBounds,
      {
        // left
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(0.0, 0.0, 1.0)),
        // right
        createParaxial(
          vm::vec3(16.0, 0.0, 0.0), vm::vec3(16.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0)),
        // front
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0)),
        // back
        createParaxial(
          vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0)),
        // top
        createParaxial(
          vm::vec3(0.0, 0.0, 16.0), vm::vec3(0.0, 1.0, 16.0), vm::vec3(1.0, 0.0, 16.0)),
        // bottom
        createParaxial(
          vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)),
      })
      .value());

  BrushNode* clone = original.clone(worldBounds);

  CHECK(clone->brush().faceCount() == original.brush().faceCount());
  for (const auto& originalFace : original.brush().faces())
  {
    const auto cloneFaceIndex = clone->brush().findFace(originalFace.boundary());
    CHECK(cloneFaceIndex.has_value());

    const auto& cloneFace = clone->brush().face(*cloneFaceIndex);
    CHECK(cloneFace == originalFace);
  }

  delete clone;
}
} // namespace Model
} // namespace TrenchBroom
