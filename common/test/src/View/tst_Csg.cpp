/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "MapDocumentTest.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/ParallelUVCoordSystem.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include "kdl/result.h"

#include <filesystem>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgConvexMergeBrushes")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* brushNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{32, 64, 64}}, "material")
      .value()};
  auto* brushNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{32, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value()};
  document->addNodes({{entityNode, {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  CHECK(entityNode->children().size() == 1u);

  document->selectNodes({brushNode1, brushNode2});
  CHECK(document->csgConvexMerge());
  CHECK(entityNode->children().size() == 1u); // added to the parent of the first brush

  auto* brushNode3 = entityNode->children().front();
  CHECK(
    brushNode3->logicalBounds() == vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{64, 64, 64}});
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgConvexMergeFaces")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* brushNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{32, 64, 64}}, "material")
      .value()};
  auto* brushNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{32, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value()};
  document->addNodes({{entityNode, {brushNode1}}});
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  CHECK(entityNode->children().size() == 1u);

  const auto faceIndex = 0u;
  const auto& face1 = brushNode1->brush().face(faceIndex);
  const auto& face2 = brushNode2->brush().face(faceIndex);

  document->selectBrushFaces({{brushNode1, faceIndex}, {brushNode2, faceIndex}});
  CHECK(document->csgConvexMerge());
  CHECK(
    entityNode->children().size()
    == 2u); // added to the parent of the first brush, original brush is not deleted

  auto* brushNode3 = entityNode->children().back();

  // check our assumption about the order of the entities' children
  assert(brushNode3 != brushNode1);
  assert(brushNode3 != brushNode2);

  const auto face1Verts = face1.vertexPositions();
  const auto face2Verts = face2.vertexPositions();

  const auto bounds = vm::merge(
    vm::bbox3::merge_all(std::begin(face1Verts), std::end(face1Verts)),
    vm::bbox3::merge_all(std::begin(face2Verts), std::end(face2Verts)));

  CHECK(brushNode3->logicalBounds() == bounds);
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgConvexMergeTexturing")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto texAlignment = Model::ParallelUVCoordSystem{vm::vec3{1, 0, 0}, vm::vec3{0, 1, 0}};
  auto texAlignmentSnapshot = texAlignment.takeSnapshot();

  auto brush1 =
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{32, 64, 64}}, "material")
      .value();
  brush1.face(*brush1.findFace(vm::vec3::pos_z()))
    .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

  auto brush2 =
    builder.createCuboid(vm::bbox3{vm::vec3{32, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value();
  brush2.face(*brush2.findFace(vm::vec3::pos_z()))
    .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

  auto* brushNode1 = new Model::BrushNode{std::move(brush1)};
  auto* brushNode2 = new Model::BrushNode{std::move(brush2)};

  document->addNodes({{entityNode, {brushNode1}}});
  document->addNodes({{entityNode, {brushNode2}}});
  CHECK(entityNode->children().size() == 2u);

  document->selectNodes({brushNode1, brushNode2});
  CHECK(document->csgConvexMerge());
  CHECK(entityNode->children().size() == 1u);

  auto* brushNode3 = static_cast<Model::BrushNode*>(entityNode->children()[0]);
  const auto& brush3 = brushNode3->brush();

  const auto& top = brush3.face(*brush3.findFace(vm::vec3::pos_z()));
  CHECK(top.uAxis() == vm::vec3{1, 0, 0});
  CHECK(top.vAxis() == vm::vec3{0, 1, 0});
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgSubtractTexturing")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto texAlignment = Model::ParallelUVCoordSystem{vm::vec3{1, 0, 0}, vm::vec3{0, 1, 0}};
  auto texAlignmentSnapshot = texAlignment.takeSnapshot();

  auto brush1 =
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value();
  auto brush2 =
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{64, 64, 32}}, "material")
      .value();
  brush2.face(*brush2.findFace(vm::vec3::pos_z()))
    .restoreUVCoordSystemSnapshot(*texAlignmentSnapshot);

  auto* brushNode1 = new Model::BrushNode{std::move(brush1)};
  auto* brushNode2 = new Model::BrushNode{std::move(brush2)};

  document->addNodes({{entityNode, {brushNode1}}});
  document->addNodes({{entityNode, {brushNode2}}});
  CHECK(entityNode->children().size() == 2u);

  // we want to compute brush1 - brush2
  document->selectNodes({brushNode2});
  CHECK(document->csgSubtract());
  CHECK(entityNode->children().size() == 1u);

  auto* brushNode3 = static_cast<Model::BrushNode*>(entityNode->children()[0]);
  const auto& brush3 = brushNode3->brush();

  CHECK(
    brushNode3->logicalBounds() == vm::bbox3{vm::vec3{0, 0, 32}, vm::vec3{64, 64, 64}});

  // the material alignment from the top of brush2 should have transferred
  // to the bottom face of brush3
  const auto& top = brush3.face(*brush3.findFace(vm::vec3::neg_z()));
  CHECK(top.uAxis() == vm::vec3{1, 0, 0});
  CHECK(top.vAxis() == vm::vec3{0, 1, 0});
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgSubtractMultipleBrushes")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* minuendNode = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value()};
  auto* subtrahendNode1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{32, 32, 64}}, "material")
      .value()};
  auto* subtrahendNode2 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{32, 32, 0}, vm::vec3{64, 64, 64}}, "material")
      .value()};

  document->addNodes({{entityNode, {minuendNode, subtrahendNode1, subtrahendNode2}}});
  CHECK(entityNode->children().size() == 3u);

  // we want to compute minuend - {subtrahendNode1, subtrahendNode2}
  document->selectNodes({subtrahendNode1, subtrahendNode2});
  CHECK(document->csgSubtract());
  CHECK(entityNode->children().size() == 2u);

  auto* remainderNode1 = dynamic_cast<Model::BrushNode*>(entityNode->children()[0]);
  auto* remainderNode2 = dynamic_cast<Model::BrushNode*>(entityNode->children()[1]);
  CHECK(remainderNode1 != nullptr);
  CHECK(remainderNode2 != nullptr);

  const auto expectedBBox1 = vm::bbox3{vm::vec3{0, 32, 0}, vm::vec3{32, 64, 64}};
  const auto expectedBBox2 = vm::bbox3{vm::vec3{32, 0, 0}, vm::vec3{64, 32, 64}};

  if (remainderNode1->logicalBounds() != expectedBBox1)
  {
    std::swap(remainderNode1, remainderNode2);
  }

  CHECK(remainderNode1->logicalBounds() == expectedBBox1);
  CHECK(remainderNode2->logicalBounds() == expectedBBox2);
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgSubtractAndUndoRestoresSelection")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* subtrahend1 = new Model::BrushNode{
    builder.createCuboid(vm::bbox3{vm::vec3{0, 0, 0}, vm::vec3{64, 64, 64}}, "material")
      .value()};
  document->addNodes({{entityNode, {subtrahend1}}});

  document->selectNodes({subtrahend1});
  CHECK(document->csgSubtract());
  CHECK(entityNode->children().size() == 0u);
  CHECK(document->selectedNodes().empty());

  // check that the selection is restored after undo
  document->undoCommand();

  CHECK(document->selectedNodes().hasOnlyBrushes());
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<Model::BrushNode*>{subtrahend1}));
}

// Test for https://github.com/TrenchBroom/TrenchBroom/issues/3755
TEST_CASE("CsgTest.csgSubtractFailure")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/csgSubtractFailure.map",
    "Quake",
    Model::MapFormat::Valve);

  REQUIRE(document->currentLayer()->childCount() == 2);
  auto* subtrahendNode =
    dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(1));
  REQUIRE(subtrahendNode);
  REQUIRE(subtrahendNode->brush().findFace("clip").has_value());

  // select the second object in the default layer (a clip brush) and subtract
  document->selectNodes({subtrahendNode});
  CHECK(document->csgSubtract());

  REQUIRE(document->currentLayer()->childCount() == 1);
  auto* result =
    dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(0));

  CHECK_THAT(
    result->brush().vertexPositions(),
    UnorderedApproxVecMatches(
      std::vector<vm::vec3>{
        {-2852, 372, 248},
        {-2854, 372, 256},
        {-2854, 364, 256},
        {-2852, 364, 248},
        {-2840, 372, 248},
        {-2843.2, 372, 256},
        {-2843.2, 364, 256},
        {-2840, 364, 248}},
      0.001));
}

TEST_CASE("CsgTest.csgHollow")
{
  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/MapDocumentTest/csgHollow.map", "Quake", Model::MapFormat::Valve);

  REQUIRE(document->currentLayer()->childCount() == 2);
  REQUIRE(!document->modified());

  SECTION("A brush too small to be hollowed doesn't block the command")
  {
    document->selectAllNodes();
    CHECK(document->csgHollow());

    // One cube is too small to hollow, so it's left untouched.
    // The other is hollowed into 6 brushes.
    CHECK(document->currentLayer()->childCount() == 7);
    CHECK(document->modified());
  }

  SECTION("If no brushes are hollowed, the transaction isn't committed")
  {
    auto* smallBrushNode = document->currentLayer()->children().at(0);
    document->selectNodes({smallBrushNode});

    CHECK(!document->csgHollow());
    CHECK(document->currentLayer()->childCount() == 2);
    CHECK(!document->modified());
  }
}

} // namespace TrenchBroom::View
