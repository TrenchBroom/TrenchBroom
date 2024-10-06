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
#include "View/SelectionTool.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include "vm/approx.h"

#include <vector>

#include "Catch2.h"

namespace tb::View
{

TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSingleBrush")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* brushNode1 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto pickResult = mdl::PickResult{};
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{1, 0, 0}}, pickResult);

  auto hits = pickResult.all();
  CHECK(hits.size() == 1u);

  const auto& brush1 = brushNode1->brush();
  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});

  pickResult.clear();
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{-1, 0, 0}}, pickResult);
  CHECK(pickResult.all().empty());
}

TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSingleEntity")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* entityNode1 = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  const auto origin = entityNode1->entity().origin();
  const auto bounds = entityNode1->logicalBounds();

  const auto rayOrigin =
    origin + vm::vec3d{-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0};

  auto pickResult = mdl::PickResult{};
  document->pick(vm::ray3d{rayOrigin, vm::vec3d{1, 0, 0}}, pickResult);

  auto hits = pickResult.all();
  CHECK(hits.size() == 1u);

  CHECK(hits.front().target<mdl::EntityNode*>() == entityNode1);
  CHECK(hits.front().distance() == vm::approx{32.0 - bounds.size().x() / 2.0});

  pickResult.clear();
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{-1, 0, 0}}, pickResult);
  CHECK(pickResult.all().empty());
}

TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSimpleGroup")
{
  using namespace mdl::HitFilters;

  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* brushNode1 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(
      vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
    | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectAllNodes();
  auto* group = document->groupSelection("test");

  auto pickResult = mdl::PickResult{};
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{1, 0, 0}}, pickResult);

  // picking a grouped object when the containing group is closed should return the
  // object, which is converted to the group when hitsToNodesWithGroupPicking() is used.
  auto hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  const auto& brush1 = brushNode1->brush();
  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});

  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<mdl::Node*>{group}));

  // hitting both objects in the group should return the group only once
  pickResult.clear();
  document->pick(vm::ray3d{vm::vec3d{32, 32, -32}, vm::vec3d{0, 0, 1}}, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 2u);

  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<mdl::Node*>{group}));

  // hitting the group bounds doesn't count as a hit
  pickResult.clear();
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 96}, vm::vec3d{1, 0, 0}}, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.empty());

  // hitting a grouped object when the containing group is open should return the object
  // only
  document->openGroup(group);

  pickResult.clear();
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{1, 0, 0}}, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});

  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits),
    Catch::Equals(std::vector<mdl::Node*>{brushNode1}));
}

TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickNestedGroup")
{
  using namespace mdl::HitFilters;

  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* brushNode1 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(
      vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
    | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectAllNodes();
  auto* innerGroup = document->groupSelection("inner");

  document->deselectAll();
  auto* brushNode3 = new mdl::BrushNode{
    builder.createCuboid(
      vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 256}), "material")
    | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode3}}});

  document->selectAllNodes();
  auto* outerGroup = document->groupSelection("outer");

  const vm::ray3d highRay(vm::vec3d{-32, 0, 256 + 32}, vm::vec3d{1, 0, 0});
  const vm::ray3d lowRay(vm::vec3d{-32, 0, +32}, vm::vec3d{1, 0, 0});

  /*
   *          Z
   *         /|\
   *          |
   *          | ______________
   *          | |   ______   |
   *  hiRay *-->|   | b3 |   |
   *          | |   |____|   |
   *          | |            |
   *          | |   outer    |
   *          | | __________ |
   *          | | | ______ | |
   *          | | | | b2 | | |
   *          | | | |____| | |
   *          | | |        | |
   *          | | |  inner | |
   *          | | | ______ | |
   * lowRay *-->| | | b1 | | |
   *        0_| | | |____| | |
   *          | | |________| |
   *          | |____________|
   * ---------|--------------------> X
   *                |
   *                0
   */

  /*
   * world
   * * outer (closed)
   *   * inner (closed)
   *     * brush1
   *     * brush2
   *   * brush3
   */

  auto pickResult = mdl::PickResult{};

  // hitting a grouped object when the containing group is open should return the object
  // only
  document->openGroup(outerGroup);

  /*
   * world
   * * outer (open)
   *   * inner (closed)
   *     * brush1
   *     * brush2
   *   * brush3
   */

  pickResult.clear();
  document->pick(highRay, pickResult);

  auto hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  const auto& brush3 = brushNode3->brush();
  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush3.face(*brush3.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});

  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits),
    Catch::Equals(std::vector<mdl::Node*>{brushNode3}));

  // hitting the brush in the inner group should return the inner group when
  // hitsToNodesWithGroupPicking() is used
  pickResult.clear();
  document->pick(lowRay, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  const auto& brush1 = brushNode1->brush();
  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});
  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits),
    Catch::Equals(std::vector<mdl::Node*>{innerGroup}));

  // open the inner group, too. hitsToNodesWithGroupPicking() should no longer return
  // groups, since all groups are open.
  document->openGroup(innerGroup);

  /*
   * world
   * * outer (open)
   *   * inner (open)
   *     * brush1
   *     * brush2
   *   * brush3
   */

  CHECK(innerGroup->opened());
  CHECK_FALSE(outerGroup->opened());
  CHECK(outerGroup->hasOpenedDescendant());

  // pick a brush in the outer group
  pickResult.clear();
  document->pick(highRay, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush3.face(*brush3.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});
  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits),
    Catch::Equals(std::vector<mdl::Node*>{brushNode3}));

  // pick a brush in the inner group
  pickResult.clear();
  document->pick(lowRay, pickResult);

  hits = pickResult.all(type(mdl::BrushNode::BrushHitType));
  CHECK(hits.size() == 1u);

  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});
  CHECK_THAT(
    hitsToNodesWithGroupPicking(hits),
    Catch::Equals(std::vector<mdl::Node*>{brushNode1}));
}

TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickBrushEntity")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* brushNode1 = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* brushNode2 = new mdl::BrushNode{
    builder.createCuboid(
      vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
    | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode2}}});

  document->selectAllNodes();

  document->createBrushEntity(m_brushEntityDef);
  document->deselectAll();

  auto pickResult = mdl::PickResult{};

  // picking entity brushes should only return the brushes and not the entity
  document->pick(vm::ray3d{vm::vec3d{-32, 0, 0}, vm::vec3d{1, 0, 0}}, pickResult);

  auto hits = pickResult.all();
  CHECK(hits.size() == 1u);

  const auto& brush1 = brushNode1->brush();
  CHECK(
    mdl::hitToFaceHandle(hits.front())->face()
    == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
  CHECK(hits.front().distance() == vm::approx{32.0});
}

} // namespace tb::View
