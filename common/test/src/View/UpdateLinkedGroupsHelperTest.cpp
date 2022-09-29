/*
 Copyright (C) 2020 Kristian Duske

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

#include "View/UpdateLinkedGroupsHelper.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapDocumentTest.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <kdl/overload.h>
#include <kdl/result.h>

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
TEST_CASE("UpdateLinkedGroupsHelperTest.checkLinkedGroupsToUpdate") {
  auto groupNode1 = Model::GroupNode{Model::Group{"test"}};
  auto linkedGroupNode = Model::GroupNode{Model::Group{"test"}};
  setLinkedGroupId(groupNode1, "asdf");
  setLinkedGroupId(linkedGroupNode, "asdf");

  auto groupNode2 = Model::GroupNode{Model::Group{"test"}};
  setLinkedGroupId(groupNode2, "fdsa");

  CHECK(checkLinkedGroupsToUpdate({}));
  CHECK(checkLinkedGroupsToUpdate({&groupNode1}));
  CHECK(checkLinkedGroupsToUpdate({&groupNode1, &groupNode2}));
  CHECK(checkLinkedGroupsToUpdate({&linkedGroupNode, &groupNode2}));
  CHECK_FALSE(checkLinkedGroupsToUpdate({&groupNode1, &linkedGroupNode}));
}

class UpdateLinkedGroupsHelperTest : public MapDocumentTest {};

TEST_CASE_METHOD(UpdateLinkedGroupsHelperTest, "UpdateLinkedGroupsHelperTest.ownership") {
  class TestNode : public Model::EntityNode {
  private:
    bool& m_deleted;

  public:
    TestNode(Model::Entity entity, bool& deleted)
      : EntityNode(std::move(entity))
      , m_deleted(deleted) {
      m_deleted = false;
    }

    ~TestNode() override { m_deleted = true; };
  };

  auto* groupNode = new Model::GroupNode{Model::Group{""}};
  setLinkedGroupId(*groupNode, "asdf");

  bool deleted = false;
  auto* entityNode = new TestNode{Model::Entity{}, deleted};
  groupNode->addChild(entityNode);

  auto* linkedNode =
    static_cast<Model::GroupNode*>(groupNode->cloneRecursively(document->worldBounds()));

  document->addNodes({{document->parentForNodes(), {groupNode, linkedNode}}});

  SECTION("Helper takes ownership of replaced child nodes") {
    {
      auto helper = UpdateLinkedGroupsHelper{{linkedNode}};
      REQUIRE(
        helper.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));
    }
    CHECK(deleted);
  }

  SECTION("Helper relinquishes ownership of replaced child nodes when undoing updates") {
    {
      auto helper = UpdateLinkedGroupsHelper{{linkedNode}};
      REQUIRE(
        helper.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));
      helper.undoLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get()));
    }
    CHECK_FALSE(deleted);
  }

  // Need to clear the document and delete all nodes, otherwise the TestNode destructor will access
  // the deleted variable when its no longer valid.
  document.reset();
}

TEST_CASE_METHOD(
  UpdateLinkedGroupsHelperTest, "UpdateLinkedGroupsHelperTest.applyLinkedGroupUpdates") {
  auto* groupNode = new Model::GroupNode{Model::Group{"test"}};
  setLinkedGroupId(*groupNode, "asdf");

  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);

  auto* linkedGroupNode =
    static_cast<Model::GroupNode*>(groupNode->cloneRecursively(document->worldBounds()));

  REQUIRE(linkedGroupNode->children().size() == 1u);
  auto* linkedBrushNode = dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedBrushNode != nullptr);

  transformNode(
    *linkedGroupNode, vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)), document->worldBounds());
  REQUIRE(
    linkedBrushNode->physicalBounds() ==
    brushNode->physicalBounds().translate(vm::vec3(32.0, 0.0, 0.0)));

  document->addNodes({{document->parentForNodes(), {groupNode, linkedGroupNode}}});

  /*
  world
  +-defaultLayer
    +-groupNode
      +-brushNode
    +-linkedGroupNode (translated 32 0 0)
      +-linkedBrushNode (translated 32 0 0)
  */

  const auto originalBrushBounds = brushNode->physicalBounds();

  transformNode(
    *brushNode, vm::translation_matrix(vm::vec3(0.0, 16.0, 0.0)), document->worldBounds());
  REQUIRE(brushNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(0.0, 16.0, 0.0)));

  /*
  world
  +-defaultLayer
    +-groupNode
      +-brushNode (translated 0 16 0)
    +-linkedGroupNode (translated 32 0 0)
      +-linkedBrushNode (translated 32 0 0)
  */

  // propagate changes
  auto helper = UpdateLinkedGroupsHelper{{groupNode}};
  REQUIRE(helper.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));

  /*
  world
  +-defaultLayer
    +-groupNode
      +-brushNode (translated 0 16 0)
    +-linkedGroupNode (translated 32 0 0)
      +-newLinkedBrushNode (translated 32 16 0)
  */

  // changes were propagated
  REQUIRE(linkedGroupNode->childCount() == 1u);
  CHECK(linkedBrushNode->parent() == nullptr);
  auto* newLinkedBrushNode = linkedGroupNode->children().front();
  CHECK(
    newLinkedBrushNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 16.0, 0.0)));

  // undo change propagation
  helper.undoLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get()));

  /*
  world
  +-defaultLayer
    +-groupNode
      +-brushNode (translated 0 16 0)
    +-linkedGroupNode (translated 32 0 0)
      +-linkedBrushNode (translated 32 0 0)
  */

  REQUIRE(linkedGroupNode->childCount() == 1u);
  CHECK_THAT(
    linkedGroupNode->children(), Catch::Equals(std::vector<Model::Node*>{linkedBrushNode}));
  CHECK(linkedBrushNode->parent() == linkedGroupNode);
  CHECK(
    linkedBrushNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(32.0, 0.0, 0.0)));
}

static void setGroupName(Model::GroupNode& groupNode, const std::string& name) {
  auto group = groupNode.group();
  group.setName(name);
  groupNode.setGroup(std::move(group));
}

static Model::GroupNode* findGroupByName(Model::Node& node, const std::string& name) {
  const auto visitChildren = [&](auto&& lambda, Model::Node* n) -> Model::GroupNode* {
    for (auto* child : n->children()) {
      if (auto* result = child->accept(lambda)) {
        return result;
      }
    }
    return nullptr;
  };
  return node.accept(kdl::overload(
    [&](auto&& thisLambda, Model::WorldNode* worldNode) -> Model::GroupNode* {
      return visitChildren(thisLambda, worldNode);
    },
    [&](auto&& thisLambda, Model::LayerNode* layerNode) -> Model::GroupNode* {
      return visitChildren(thisLambda, layerNode);
    },
    [&](auto&& thisLambda, Model::GroupNode* groupNode) -> Model::GroupNode* {
      if (groupNode->name() == name) {
        return groupNode;
      }
      return visitChildren(thisLambda, groupNode);
    },
    [](Model::EntityNode*) -> Model::GroupNode* {
      return nullptr;
    },
    [](Model::BrushNode*) -> Model::GroupNode* {
      return nullptr;
    },
    [](Model::PatchNode*) -> Model::GroupNode* {
      return nullptr;
    }));
}

TEST_CASE_METHOD(
  UpdateLinkedGroupsHelperTest,
  "UpdateLinkedGroupsHelperTest.applyLinkedGroupUpdatesWithNestedLinkedGroups") {
  document->deselectAll();

  auto* outerGroupNode = new Model::GroupNode{Model::Group{"outerGroupNode"}};
  setLinkedGroupId(*outerGroupNode, "outerGroupNode");

  auto* innerGroupNode = new Model::GroupNode{Model::Group{"innerGroupNode"}};
  setLinkedGroupId(*innerGroupNode, "innerGroupNode");

  auto* brushNode = createBrushNode();
  innerGroupNode->addChild(brushNode);
  outerGroupNode->addChild(innerGroupNode);

  document->addNodes({{document->parentForNodes(), {outerGroupNode}}});

  // create a linked group of the inner group node so that cloning the outer group node will create
  // a linked clone of the inner group node
  auto* linkedInnerGroupNode =
    static_cast<Model::GroupNode*>(innerGroupNode->cloneRecursively(document->worldBounds()));
  setGroupName(*linkedInnerGroupNode, "linkedInnerGroupNode");
  REQUIRE(linkedInnerGroupNode->group().linkedGroupId() == innerGroupNode->group().linkedGroupId());

  document->addNodes({{document->parentForNodes(), {linkedInnerGroupNode}}});

  auto* linkedOuterGroupNode =
    static_cast<Model::GroupNode*>(outerGroupNode->cloneRecursively(document->worldBounds()));
  setGroupName(*linkedOuterGroupNode, "linkedOuterGroupNode");
  REQUIRE(linkedOuterGroupNode->group().linkedGroupId() == outerGroupNode->group().linkedGroupId());

  document->addNodes({{document->parentForNodes(), {linkedOuterGroupNode}}});

  auto* nestedLinkedInnerGroupNode =
    static_cast<Model::GroupNode*>(linkedOuterGroupNode->children().front());
  setGroupName(*nestedLinkedInnerGroupNode, "nestedLinkedInnerGroupNode");
  REQUIRE(
    nestedLinkedInnerGroupNode->group().linkedGroupId() == innerGroupNode->group().linkedGroupId());

  /*
  world
  +-defaultLayer
    +-outerGroupNode--------+
      +-innerGroupNode------|-------+
        +-brushNode         |       |
    +-linkedInnerGroupNode--|-------+
      +-linkedBrushNode     |       |
    +-linkedOuterGroupNode--+       |
      +-nestedLinkedInnerGroupNode--+
        +-nestedLinkedBrushNode
  */

  const auto originalBrushBounds = brushNode->physicalBounds();

  transformNode(
    *linkedOuterGroupNode, vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)),
    document->worldBounds());

  REQUIRE(
    linkedOuterGroupNode->group().transformation() ==
    vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
  REQUIRE(
    linkedOuterGroupNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 0.0, 0.0)));

  REQUIRE(
    nestedLinkedInnerGroupNode->group().transformation() ==
    vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
  REQUIRE(
    nestedLinkedInnerGroupNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 0.0, 0.0)));

  auto* nestedLinkedBrushNode = nestedLinkedInnerGroupNode->children().front();
  REQUIRE(
    nestedLinkedBrushNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 0.0, 0.0)));

  /*
  world
  +-defaultLayer
    +-outerGroupNode
      +-innerGroupNode
        +-brushNode
    +-linkedInnerGroupNode
      +-linkedBrushNode
    +-linkedOuterGroupNode (translated 32 0 0)
      +-nestedLinkedInnerGroupNode (translated 32 0 0)
        +-nestedLinkedBrushNode (translated 32 0 0)
  */

  transformNode(
    *innerGroupNode, vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)), document->worldBounds());

  REQUIRE(
    innerGroupNode->group().transformation() == vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)));
  REQUIRE(
    innerGroupNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(0.0, 16.0, 0.0)));
  REQUIRE(brushNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(0.0, 16.0, 0.0)));

  /*
  world
  +-defaultLayer
    +-outerGroupNode
      +-innerGroupNode (translated 0 16 0)
        +-brushNode (translated 0 16 0)
    +-linkedInnerGroupNode
      +-linkedBrushNode
    +-linkedOuterGroupNode (translated 32 0 0)
      +-nestedLinkedInnerGroupNode (translated 32 0 0)
        +-nestedLinkedBrushNode (translated 32 0 0)
  */

  transformNode(
    *brushNode, vm::translation_matrix(vm::vec3d(0.0, 0.0, 8.0)), document->worldBounds());

  REQUIRE(brushNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(0.0, 16.0, 8.0)));

  /*
  world
  +-defaultLayer
    +-outerGroupNode
      +-innerGroupNode (translated 0 16 0)
        +-brushNode (translated 0 16 8)
    +-linkedInnerGroupNode
      +-linkedBrushNode
    +-linkedOuterGroupNode (translated 32 0 0)
      +-nestedLinkedInnerGroupNode (translated 32 0 0)
        +-nestedLinkedBrushNode (translated 32 0 0)
  */

  SECTION("First propagate changes to innerGroupNode, then outerGroupNode") {
    auto helper1 = UpdateLinkedGroupsHelper{{innerGroupNode}};
    CHECK(helper1.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));

    /*
    world
    +-defaultLayer
      +-outerGroupNode
        +-innerGroupNode (translated 0 16 0)
          +-brushNode (translated 0 16 8)
      +-linkedInnerGroupNode
        +-newLinkedBrushNode (translated 0 0 8)
      +-linkedOuterGroupNode (translated 32 0 0)
        +-nestedLinkedInnerGroupNode (translated 32 0 0)
          +-newNestedLinkedBrushNode (translated 32 0 8)
    */

    REQUIRE(linkedInnerGroupNode->childCount() == 1u);

    auto* newLinkedBrushNode = linkedInnerGroupNode->children().front();
    CHECK(
      newLinkedBrushNode->physicalBounds() ==
      originalBrushBounds.translate(vm::vec3(0.0, 0.0, 8.0)));

    REQUIRE(nestedLinkedInnerGroupNode != nullptr);
    CHECK(
      nestedLinkedInnerGroupNode->group().transformation() ==
      vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
    CHECK(
      nestedLinkedInnerGroupNode->physicalBounds() ==
      originalBrushBounds.translate(vm::vec3(32.0, 0.0, 8.0)));
    REQUIRE(nestedLinkedInnerGroupNode->childCount() == 1u);

    auto* newNestedLinkedBrushNode = nestedLinkedInnerGroupNode->children().front();
    CHECK(
      newNestedLinkedBrushNode->physicalBounds() ==
      originalBrushBounds.translate(vm::vec3(32.0, 0.0, 8.0)));

    auto helper2 = UpdateLinkedGroupsHelper{{outerGroupNode}};
    CHECK(helper2.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));

    // see end of test for assertions of final state
  }

  SECTION("First propagate changes to outerGroupNode, then innerGroupNode") {
    auto helper1 = UpdateLinkedGroupsHelper{{outerGroupNode}};
    REQUIRE(
      helper1.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));

    /*
    world
    +-defaultLayer
      +-outerGroupNode
        +-innerGroupNode (translated 0 16 0)
          +-brushNode (translated 0 16 8)
      +-linkedInnerGroupNode
        +-linkedBrushNode
      +-linkedOuterGroupNode (translated 32 0 0)
        +-newNestedLinkedInnerGroupNode (translated 32 16 0)
          +-newNestedLinkedBrushNode (translated 32 16 8)
    */

    CHECK(
      linkedOuterGroupNode->group().transformation() ==
      vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));

    auto* newNestedLinkedInnerGroupNode =
      findGroupByName(*document->world(), "nestedLinkedInnerGroupNode");
    CHECK(
      newNestedLinkedInnerGroupNode->group().transformation() ==
      vm::translation_matrix(vm::vec3(32.0, 16.0, 0.0)));
    CHECK(
      newNestedLinkedInnerGroupNode->physicalBounds() ==
      originalBrushBounds.translate(vm::vec3(32.0, 16.0, 8.0)));
    REQUIRE(newNestedLinkedInnerGroupNode->childCount() == 1u);

    auto* newNestedLinkedBrushNode = newNestedLinkedInnerGroupNode->children().front();
    CHECK(
      newNestedLinkedBrushNode->physicalBounds() ==
      originalBrushBounds.translate(vm::vec3(32.0, 16.0, 8.0)));

    auto helper2 = UpdateLinkedGroupsHelper{{innerGroupNode}};
    REQUIRE(
      helper2.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));

    // see end of test for assertions of final state
  }

  SECTION("Propagate both changes at once") {
    auto groupNodes = std::vector<Model::GroupNode*>{outerGroupNode, innerGroupNode};
    std::sort(std::begin(groupNodes), std::end(groupNodes));

    // The following code generates both permutations of the group nodes
    const auto permute = GENERATE(true, false);
    if (permute) {
      std::next_permutation(std::begin(groupNodes), std::end(groupNodes));
    }

    auto helper = UpdateLinkedGroupsHelper{groupNodes};
    REQUIRE(
      helper.applyLinkedGroupUpdates(*static_cast<MapDocumentCommandFacade*>(document.get())));
  }

  /*
  world
  +-defaultLayer
    +-outerGroupNode
      +-innerGroupNode (translated 0 16 0)
        +-brushNode (translated 0 16 8)
    +-linkedInnerGroupNode
      +-newLinkedBrushNode (translated 0 0 8)
    +-linkedOuterGroupNode (translated 32 0 0)
      +-newNestedLinkedInnerGroupNode (translated 32 16 8)
        +-newLinkedBrushNode (translated 32 16 8)
  */

  REQUIRE(linkedInnerGroupNode->childCount() == 1u);

  auto* newLinkedBrushNode = linkedInnerGroupNode->children().front();
  CHECK(
    newLinkedBrushNode->physicalBounds() == originalBrushBounds.translate(vm::vec3(0.0, 0.0, 8.0)));

  CHECK(
    linkedOuterGroupNode->group().transformation() ==
    vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));

  auto* newNestedLinkedInnerGroupNode =
    findGroupByName(*document->world(), "nestedLinkedInnerGroupNode");
  REQUIRE(newNestedLinkedInnerGroupNode != nullptr);
  CHECK(
    newNestedLinkedInnerGroupNode->group().transformation() ==
    vm::translation_matrix(vm::vec3(32.0, 16.0, 0.0)));
  CHECK(
    newNestedLinkedInnerGroupNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 16.0, 8.0)));
  REQUIRE(newNestedLinkedInnerGroupNode->childCount() == 1u);

  auto* newNestedLinkedBrushNode = newNestedLinkedInnerGroupNode->children().front();
  CHECK(
    newNestedLinkedBrushNode->physicalBounds() ==
    originalBrushBounds.translate(vm::vec3(32.0, 16.0, 8.0)));
}
} // namespace View
} // namespace TrenchBroom
