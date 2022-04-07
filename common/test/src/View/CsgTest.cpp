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
#include "TestUtils.h"

#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgConvexMergeBrushes") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  auto* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  auto* brushNode1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
  auto* brushNode2 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
  addNode(*document, entity, brushNode1);
  addNode(*document, document->parentForNodes(), brushNode2);
  CHECK(entity->children().size() == 1u);

  document->selectNodes({brushNode1, brushNode2});
  CHECK(document->csgConvexMerge());
  CHECK(entity->children().size() == 1u); // added to the parent of the first brush

  auto* brush3 = entity->children().front();
  CHECK(brush3->logicalBounds() == vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)));
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgConvexMergeFaces") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  auto* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  auto* brushNode1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
  auto* brushNode2 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
  addNode(*document, entity, brushNode1);
  addNode(*document, document->parentForNodes(), brushNode2);
  CHECK(entity->children().size() == 1u);

  const auto faceIndex = 0u;
  const auto& face1 = brushNode1->brush().face(faceIndex);
  const auto& face2 = brushNode2->brush().face(faceIndex);

  document->selectBrushFaces({{brushNode1, faceIndex}, {brushNode2, faceIndex}});
  CHECK(document->csgConvexMerge());
  CHECK(
    entity->children().size() ==
    2u); // added to the parent of the first brush, original brush is not deleted

  auto* brush3 = entity->children().back();

  // check our assumption about the order of the entities' children
  assert(brush3 != brushNode1);
  assert(brush3 != brushNode2);

  const auto face1Verts = face1.vertexPositions();
  const auto face2Verts = face2.vertexPositions();

  const auto bounds = vm::merge(
    vm::bbox3::merge_all(std::begin(face1Verts), std::end(face1Verts)),
    vm::bbox3::merge_all(std::begin(face2Verts), std::end(face2Verts)));

  CHECK(brush3->logicalBounds() == bounds);
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgConvexMergeTexturing") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
  auto texAlignmentSnapshot = texAlignment.takeSnapshot();

  Model::Brush brush1 =
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value();
  brush1.face(*brush1.findFace(vm::vec3::pos_z()))
    .restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

  Model::Brush brush2 =
    builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
  brush2.face(*brush2.findFace(vm::vec3::pos_z()))
    .restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

  Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
  Model::BrushNode* brushNode2 = new Model::BrushNode(std::move(brush2));

  addNode(*document, entity, brushNode1);
  addNode(*document, entity, brushNode2);
  CHECK(entity->children().size() == 2u);

  document->selectNodes({brushNode1, brushNode2});
  CHECK(document->csgConvexMerge());
  CHECK(entity->children().size() == 1u);

  Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
  const Model::Brush& brush3 = brushNode3->brush();

  const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::pos_z()));
  CHECK(top.textureXAxis() == vm::vec3(1, 0, 0));
  CHECK(top.textureYAxis() == vm::vec3(0, 1, 0));
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgSubtractTexturing") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
  auto texAlignmentSnapshot = texAlignment.takeSnapshot();

  Model::Brush brush1 =
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
  Model::Brush brush2 =
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 32)), "texture").value();
  brush2.face(*brush2.findFace(vm::vec3::pos_z()))
    .restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

  Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
  Model::BrushNode* brushNode2 = new Model::BrushNode(std::move(brush2));

  addNode(*document, entity, brushNode1);
  addNode(*document, entity, brushNode2);
  CHECK(entity->children().size() == 2u);

  // we want to compute brush1 - brush2
  document->selectNodes({brushNode2});
  CHECK(document->csgSubtract());
  CHECK(entity->children().size() == 1u);

  Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
  const Model::Brush& brush3 = brushNode3->brush();

  CHECK(brushNode3->logicalBounds() == vm::bbox3(vm::vec3(0, 0, 32), vm::vec3(64, 64, 64)));

  // the texture alignment from the top of brush2 should have transferred
  // to the bottom face of brush3
  const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::neg_z()));
  CHECK(top.textureXAxis() == vm::vec3(1, 0, 0));
  CHECK(top.textureYAxis() == vm::vec3(0, 1, 0));
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgSubtractMultipleBrushes") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  auto* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  Model::BrushNode* minuend = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
  Model::BrushNode* subtrahend1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 32, 64)), "texture").value());
  Model::BrushNode* subtrahend2 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(32, 32, 0), vm::vec3(64, 64, 64)), "texture").value());

  document->addNodes({{entity, {minuend, subtrahend1, subtrahend2}}});
  CHECK(entity->children().size() == 3u);

  // we want to compute minuend - {subtrahend1, subtrahend2}
  document->selectNodes({subtrahend1, subtrahend2});
  CHECK(document->csgSubtract());
  CHECK(entity->children().size() == 2u);

  auto* remainder1 = dynamic_cast<Model::BrushNode*>(entity->children()[0]);
  auto* remainder2 = dynamic_cast<Model::BrushNode*>(entity->children()[1]);
  CHECK(remainder1 != nullptr);
  CHECK(remainder2 != nullptr);

  const auto expectedBBox1 = vm::bbox3(vm::vec3(0, 32, 0), vm::vec3(32, 64, 64));
  const auto expectedBBox2 = vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 32, 64));

  if (remainder1->logicalBounds() != expectedBBox1) {
    std::swap(remainder1, remainder2);
  }

  CHECK(remainder1->logicalBounds() == expectedBBox1);
  CHECK(remainder2->logicalBounds() == expectedBBox2);
}

TEST_CASE_METHOD(MapDocumentTest, "CsgTest.csgSubtractAndUndoRestoresSelection") {
  const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

  auto* entity = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entity);

  Model::BrushNode* subtrahend1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
  addNode(*document, entity, subtrahend1);

  document->selectNodes({subtrahend1});
  CHECK(document->csgSubtract());
  CHECK(entity->children().size() == 0u);
  CHECK(document->selectedNodes().empty());

  // check that the selection is restored after undo
  document->undoCommand();

  CHECK(document->selectedNodes().hasOnlyBrushes());
  CHECK_THAT(
    document->selectedNodes().brushes(),
    Catch::Equals(std::vector<Model::BrushNode*>{subtrahend1}));
}

// Test for https://github.com/TrenchBroom/TrenchBroom/issues/3755
TEST_CASE("CsgTest.csgSubtractFailure", "[MapDocumentTest]") {
  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path("fixture/test/View/MapDocumentTest/csgSubtractFailure.map"), "Quake",
    Model::MapFormat::Valve);

  REQUIRE(document->currentLayer()->childCount() == 2);
  auto* subtrahend = dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(1));
  REQUIRE(subtrahend);
  REQUIRE(subtrahend->brush().findFace("clip").has_value());

  // select the second object in the default layer (a clip brush) and subtract
  document->selectNodes({subtrahend});
  CHECK(document->csgSubtract());

  REQUIRE(document->currentLayer()->childCount() == 1);
  auto* result = dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(0));

  CHECK_THAT(
    result->brush().vertexPositions(), UnorderedApproxVecMatches(
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

TEST_CASE("CsgTest.csgHollow", "[MapDocumentTest]") {
  auto [document, game, gameConfig] = View::loadMapDocument(
    IO::Path("fixture/test/View/MapDocumentTest/csgHollow.map"), "Quake", Model::MapFormat::Valve);

  REQUIRE(document->currentLayer()->childCount() == 2);
  REQUIRE(!document->modified());

  SECTION("A brush too small to be hollowed doesn't block the command") {
    document->selectAllNodes();
    CHECK(document->csgHollow());

    // One cube is too small to hollow, so it's left untouched.
    // The other is hollowed into 6 brushes.
    CHECK(document->currentLayer()->childCount() == 7);
    CHECK(document->modified());
  }
  SECTION("If no brushes are hollowed, the transaction isn't committed") {
    auto* smallBrushNode = document->currentLayer()->children().at(0);
    document->selectNodes({smallBrushNode});

    CHECK(!document->csgHollow());
    CHECK(document->currentLayer()->childCount() == 2);
    CHECK(!document->modified());
  }
}
} // namespace View
} // namespace TrenchBroom
