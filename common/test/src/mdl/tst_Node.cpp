/*
 Copyright (C) 2010 Kristian Duske

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

#include "el/Expression.h"
#include "el/Value.h"
#include "io/NodeWriter.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/Node.h"
#include "mdl/Object.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

struct DoCanAddChild
{
  bool valueToReturn;
  const Node* expectedChild;
};

struct DoCanRemoveChild
{
  bool valueToReturn;
  const Node* expectedChild;
};

struct DoParentWillChange
{
};

struct DoParentDidChange
{
};

struct DoAncestorWillChange
{
};

struct DoAncestorDidChange
{
};

using ExpectedCall = std::variant<
  DoCanAddChild,
  DoCanRemoveChild,
  DoParentWillChange,
  DoParentDidChange,
  DoAncestorWillChange,
  DoAncestorDidChange>;

class MockNode : public Node
{
private:
  mutable std::vector<ExpectedCall> m_expectedCalls;

public:
  /**
   * Sets an expectation that the given member function will be called. Some of the
   * variant cases include a value to return when that function is called, or checks to
   * perform on the function arguments.
   *
   * The expectations set this way are all mandatory and must be called in the order
   * they are set.
   */
  void expectCall(ExpectedCall call) { m_expectedCalls.push_back(std::move(call)); }

  ~MockNode() override
  {
    // If this fails, it means a call that was expected was not made
    CHECK(m_expectedCalls.empty());
  }

private:
  template <class T>
  T popCall() const
  {
    REQUIRE_FALSE(m_expectedCalls.empty());
    T expectedCall = std::get<T>(kdl::vec_pop_front(m_expectedCalls));
    return expectedCall;
  }

private: // implement Node interface
  Node* doClone(const vm::bbox3d& /* worldBounds */) const override
  {
    return new MockNode{};
  }

  const std::string& doGetName() const override
  {
    static const auto name = std::string{"some name"};
    return name;
  }

  const vm::bbox3d& doGetLogicalBounds() const override
  {
    static const auto bounds = vm::bbox3d{};
    return bounds;
  }

  const vm::bbox3d& doGetPhysicalBounds() const override
  {
    static const auto bounds = vm::bbox3d{};
    return bounds;
  }

  double doGetProjectedArea(const vm::axis::type) const override { return double(0); }

  bool doCanAddChild(const Node* child) const override
  {
    auto call = popCall<DoCanAddChild>();
    CHECK(child == call.expectedChild);
    return call.valueToReturn;
  }

  bool doCanRemoveChild(const Node* child) const override
  {
    auto call = popCall<DoCanRemoveChild>();
    CHECK(child == call.expectedChild);
    return call.valueToReturn;
  }

  bool doRemoveIfEmpty() const override { return false; }

  bool doShouldAddToSpacialIndex() const override { return true; }

  void doParentWillChange() override { popCall<DoParentWillChange>(); }

  void doParentDidChange() override { popCall<DoParentDidChange>(); }

  bool doSelectable() const override { return false; }

  void doAncestorWillChange() override { popCall<DoAncestorWillChange>(); }

  void doAncestorDidChange() override { popCall<DoAncestorDidChange>(); }

  void doPick(
    const EditorContext&, const vm::ray3d& /*ray*/, PickResult& /*pickResult*/) override
  {
  }

  void doFindNodesContaining(
    const vm::vec3d& /*point*/, std::vector<Node*>& /*result*/) override
  {
  }

  void doAccept(NodeVisitor& /*visitor*/) override {}

  void doAccept(ConstNodeVisitor& /*visitor*/) const override {}

  void doAcceptTagVisitor(TagVisitor& /* visitor */) override {}
  void doAcceptTagVisitor(ConstTagVisitor& /* visitor */) const override {}
};

class TestNode : public Node
{
private: // implement Node interface
  Node* doClone(const vm::bbox3d& /* worldBounds */) const override
  {
    return new TestNode{};
  }

  const std::string& doGetName() const override
  {
    static const auto name = std::string{"some name"};
    return name;
  }

  const vm::bbox3d& doGetLogicalBounds() const override
  {
    static const auto bounds = vm::bbox3d{};
    return bounds;
  }

  const vm::bbox3d& doGetPhysicalBounds() const override
  {
    static const auto bounds = vm::bbox3d{};
    return bounds;
  }

  double doGetProjectedArea(const vm::axis::type) const override { return double(0); }

  bool doCanAddChild(const Node* /* child */) const override { return true; }

  bool doCanRemoveChild(const Node* /* child */) const override { return true; }

  bool doRemoveIfEmpty() const override { return false; }

  bool doShouldAddToSpacialIndex() const override { return true; }

  bool doSelectable() const override { return true; }

  void doParentWillChange() override {}
  void doParentDidChange() override {}
  void doAncestorWillChange() override {}
  void doAncestorDidChange() override {}

  void doPick(
    const EditorContext&,
    const vm::ray3d& /* ray */,
    PickResult& /* pickResult */) override
  {
  }
  void doFindNodesContaining(
    const vm::vec3d& /* point */, std::vector<Node*>& /* result */) override
  {
  }

  void doAccept(NodeVisitor& /* visitor */) override {}
  void doAccept(ConstNodeVisitor& /* visitor */) const override {}

  void doAcceptTagVisitor(TagVisitor& /* visitor */) override {}
  void doAcceptTagVisitor(ConstTagVisitor& /* visitor */) const override {}
};

class DestroyableNode : public TestNode
{
private:
  bool& m_destroyed;

public:
  explicit DestroyableNode(bool& destroyed)
    : m_destroyed(destroyed)
  {
  }

  ~DestroyableNode() override { m_destroyed = true; }
};

} // namespace

TEST_CASE("NodeTest.destroyChild")
{
  auto rootNode = std::make_unique<TestNode>();

  auto childDestroyed = false;
  rootNode->addChild(new DestroyableNode{childDestroyed});

  rootNode.reset();
  CHECK(childDestroyed);
}

TEST_CASE("NodeTest.addRemoveChild")
{
  auto rootNode = MockNode{};
  auto* childNode = new MockNode{};
  auto* grandChildNode1 = new MockNode{};
  auto* grandChildNode2 = new MockNode{};

  // NOTE: Node::doAddChild only calls canAddChild in debug builds
#ifndef NDEBUG
  childNode->expectCall(DoCanAddChild{true, grandChildNode1});
#endif
  grandChildNode1->expectCall(DoParentWillChange{});
  grandChildNode1->expectCall(DoAncestorWillChange{});
  grandChildNode1->expectCall(DoParentDidChange{});
  grandChildNode1->expectCall(DoAncestorDidChange{});
  childNode->addChild(grandChildNode1);
  CHECK(childNode->childCount() == 1u);
  CHECK(childNode->familySize() == 2u);
  CHECK(grandChildNode1->parent() == childNode);
  CHECK(kdl::vec_contains(childNode->children(), grandChildNode1));

#ifndef NDEBUG
  rootNode.expectCall(DoCanAddChild{true, childNode});
#endif
  childNode->expectCall(DoParentWillChange{});
  childNode->expectCall(DoAncestorWillChange{});
  childNode->expectCall(DoParentDidChange{});
  childNode->expectCall(DoAncestorDidChange{});
  grandChildNode1->expectCall(DoAncestorWillChange{});
  grandChildNode1->expectCall(DoAncestorDidChange{});

  rootNode.addChild(childNode);
  CHECK(rootNode.childCount() == 1u);
  CHECK(rootNode.familySize() == 3u);
  CHECK(childNode->parent() == &rootNode);
  CHECK(kdl::vec_contains(rootNode.children(), childNode));

#ifndef NDEBUG
  childNode->expectCall(DoCanAddChild{true, grandChildNode2});
#endif
  grandChildNode2->expectCall(DoParentWillChange{});
  grandChildNode2->expectCall(DoAncestorWillChange{});
  grandChildNode2->expectCall(DoParentDidChange{});
  grandChildNode2->expectCall(DoAncestorDidChange{});
  childNode->addChild(grandChildNode2);
  CHECK(rootNode.childCount() == 1u);
  CHECK(rootNode.familySize() == 4u);
  CHECK(childNode->childCount() == 2u);
  CHECK(childNode->familySize() == 3u);
  CHECK(grandChildNode2->parent() == childNode);
  CHECK(kdl::vec_contains(childNode->children(), grandChildNode2));

#ifndef NDEBUG
  rootNode.expectCall(DoCanRemoveChild{true, childNode});
#endif
  childNode->expectCall(DoParentWillChange{});
  childNode->expectCall(DoAncestorWillChange{});
  childNode->expectCall(DoParentDidChange{});
  childNode->expectCall(DoAncestorDidChange{});
  grandChildNode1->expectCall(DoAncestorWillChange{});
  grandChildNode1->expectCall(DoAncestorDidChange{});
  grandChildNode2->expectCall(DoAncestorWillChange{});
  grandChildNode2->expectCall(DoAncestorDidChange{});

  rootNode.removeChild(childNode);
  CHECK(childNode->parent() == nullptr);
  CHECK_FALSE(kdl::vec_contains(rootNode.children(), childNode));
  CHECK(rootNode.childCount() == 0u);
  CHECK(rootNode.familySize() == 1u);
  CHECK(childNode->childCount() == 2u);
  CHECK(childNode->familySize() == 3u);

#ifndef NDEBUG
  rootNode.expectCall(DoCanAddChild{true, childNode});
#endif
  childNode->expectCall(DoParentWillChange{});
  childNode->expectCall(DoAncestorWillChange{});
  childNode->expectCall(DoParentDidChange{});
  childNode->expectCall(DoAncestorDidChange{});
  grandChildNode1->expectCall(DoAncestorWillChange{});
  grandChildNode1->expectCall(DoAncestorDidChange{});
  grandChildNode2->expectCall(DoAncestorWillChange{});
  grandChildNode2->expectCall(DoAncestorDidChange{});

  rootNode.addChild(childNode);
  CHECK(childNode->parent() == &rootNode);
  CHECK(kdl::vec_contains(rootNode.children(), childNode));
  CHECK(rootNode.childCount() == 1u);
  CHECK(rootNode.familySize() == 4u);
  CHECK(childNode->childCount() == 2u);
  CHECK(childNode->familySize() == 3u);
}

TEST_CASE("NodeTest.replaceChildren")
{
  auto rootNode = TestNode{};
  auto* childNode1 = new TestNode{};
  auto* childNode2 = new TestNode{};

  rootNode.addChildren({childNode1, childNode2});

  auto childNode3Ptr = std::make_unique<TestNode>();
  auto* childNode3 = childNode3Ptr.get();

  auto newChildren = std::vector<std::unique_ptr<Node>>{};
  newChildren.push_back(std::move(childNode3Ptr));

  const auto oldChildren = rootNode.replaceChildren(std::move(newChildren));

  CHECK(oldChildren.size() == 2u);
  CHECK_THAT(
    kdl::vec_transform(oldChildren, [](const auto& c) { return c.get(); }),
    UnorderedEquals(std::vector<Node*>{childNode1, childNode2}));
  CHECK(childNode1->parent() == nullptr);
  CHECK(childNode2->parent() == nullptr);

  CHECK_THAT(rootNode.children(), UnorderedEquals(std::vector<Node*>{childNode3}));
  CHECK(childNode3->parent() == &rootNode);
}

TEST_CASE("NodeTest.partialSelection")
{
  auto rootNode = TestNode{};
  auto* childNode1 = new TestNode{};
  auto* childNode2 = new TestNode{};
  auto* grandChildNode1_1 = new TestNode{};
  auto* grandChildNode1_2 = new TestNode{};

  rootNode.addChild(childNode1);
  rootNode.addChild(childNode2);

  CHECK(rootNode.descendantSelectionCount() == 0u);
  childNode1->select();
  CHECK(childNode1->descendantSelectionCount() == 0u);
  CHECK(rootNode.descendantSelectionCount() == 1u);
  childNode2->select();
  CHECK(childNode1->descendantSelectionCount() == 0u);
  CHECK(childNode2->descendantSelectionCount() == 0u);
  CHECK(rootNode.descendantSelectionCount() == 2u);

  childNode1->deselect();
  CHECK(childNode1->descendantSelectionCount() == 0u);
  CHECK(rootNode.descendantSelectionCount() == 1u);

  grandChildNode1_1->select();
  childNode1->addChild(grandChildNode1_1);
  CHECK(childNode1->descendantSelectionCount() == 1u);
  CHECK(rootNode.descendantSelectionCount() == 2u);

  childNode1->addChild(grandChildNode1_2);
  CHECK(childNode1->descendantSelectionCount() == 1u);
  CHECK(rootNode.descendantSelectionCount() == 2u);
  grandChildNode1_2->select();
  CHECK(childNode1->descendantSelectionCount() == 2u);
  CHECK(rootNode.descendantSelectionCount() == 3u);

  grandChildNode1_1->deselect();
  CHECK(childNode1->descendantSelectionCount() == 1u);
  CHECK(rootNode.descendantSelectionCount() == 2u);
}

TEST_CASE("NodeTest.isAncestorOf")
{
  auto rootNode = TestNode{};
  auto* childNode1 = new TestNode{};
  auto* childNode2 = new TestNode{};
  auto* grandChildNode1_1 = new TestNode{};
  auto* grandChildNode1_2 = new TestNode{};

  rootNode.addChild(childNode1);
  rootNode.addChild(childNode2);
  childNode1->addChild(grandChildNode1_1);
  childNode1->addChild(grandChildNode1_2);

  CHECK_FALSE(rootNode.isAncestorOf(&rootNode));
  CHECK(rootNode.isAncestorOf(childNode1));
  CHECK(rootNode.isAncestorOf(childNode2));
  CHECK(rootNode.isAncestorOf(grandChildNode1_1));
  CHECK(rootNode.isAncestorOf(grandChildNode1_2));

  CHECK_FALSE(childNode1->isAncestorOf(&rootNode));
  CHECK_FALSE(childNode1->isAncestorOf(childNode1));
  CHECK_FALSE(childNode1->isAncestorOf(childNode2));
  CHECK(childNode1->isAncestorOf(grandChildNode1_1));
  CHECK(childNode1->isAncestorOf(grandChildNode1_2));

  CHECK_FALSE(childNode2->isAncestorOf(&rootNode));
  CHECK_FALSE(childNode2->isAncestorOf(childNode1));
  CHECK_FALSE(childNode2->isAncestorOf(childNode2));
  CHECK_FALSE(childNode2->isAncestorOf(grandChildNode1_1));
  CHECK_FALSE(childNode2->isAncestorOf(grandChildNode1_2));

  CHECK_FALSE(grandChildNode1_1->isAncestorOf(&rootNode));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(childNode1));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(childNode2));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(grandChildNode1_1));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(grandChildNode1_2));

  CHECK_FALSE(grandChildNode1_2->isAncestorOf(&rootNode));
  CHECK_FALSE(grandChildNode1_2->isAncestorOf(childNode1));
  CHECK_FALSE(grandChildNode1_2->isAncestorOf(childNode2));
  CHECK_FALSE(grandChildNode1_2->isAncestorOf(grandChildNode1_1));
  CHECK_FALSE(grandChildNode1_2->isAncestorOf(grandChildNode1_2));

  CHECK(rootNode.isAncestorOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK(childNode1->isAncestorOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK_FALSE(childNode2->isAncestorOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK_FALSE(grandChildNode1_1->isAncestorOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
}

TEST_CASE("NodeTest.isDescendantOf")
{
  auto rootNode = TestNode{};
  auto* childNode1 = new TestNode{};
  auto* childNode2 = new TestNode{};
  auto* grandChildNode1_1 = new TestNode{};
  auto* grandChildNode1_2 = new TestNode{};

  rootNode.addChild(childNode1);
  rootNode.addChild(childNode2);
  childNode1->addChild(grandChildNode1_1);
  childNode1->addChild(grandChildNode1_2);

  CHECK_FALSE(rootNode.isDescendantOf(&rootNode));
  CHECK_FALSE(rootNode.isDescendantOf(childNode1));
  CHECK_FALSE(rootNode.isDescendantOf(childNode2));
  CHECK_FALSE(rootNode.isDescendantOf(grandChildNode1_1));
  CHECK_FALSE(rootNode.isDescendantOf(grandChildNode1_2));

  CHECK(childNode1->isDescendantOf(&rootNode));
  CHECK_FALSE(childNode1->isDescendantOf(childNode1));
  CHECK_FALSE(childNode1->isDescendantOf(childNode2));
  CHECK_FALSE(childNode1->isDescendantOf(grandChildNode1_1));
  CHECK_FALSE(childNode1->isDescendantOf(grandChildNode1_2));

  CHECK(childNode2->isDescendantOf(&rootNode));
  CHECK_FALSE(childNode2->isDescendantOf(childNode1));
  CHECK_FALSE(childNode2->isDescendantOf(childNode2));
  CHECK_FALSE(childNode2->isDescendantOf(grandChildNode1_1));
  CHECK_FALSE(childNode2->isDescendantOf(grandChildNode1_2));

  CHECK(grandChildNode1_1->isDescendantOf(&rootNode));
  CHECK(grandChildNode1_1->isDescendantOf(childNode1));
  CHECK_FALSE(grandChildNode1_1->isDescendantOf(childNode2));
  CHECK_FALSE(grandChildNode1_1->isDescendantOf(grandChildNode1_1));
  CHECK_FALSE(grandChildNode1_1->isDescendantOf(grandChildNode1_2));

  CHECK(grandChildNode1_2->isDescendantOf(&rootNode));
  CHECK(grandChildNode1_2->isDescendantOf(childNode1));
  CHECK_FALSE(grandChildNode1_2->isDescendantOf(childNode2));
  CHECK_FALSE(grandChildNode1_2->isDescendantOf(grandChildNode1_1));
  CHECK_FALSE(grandChildNode1_2->isDescendantOf(grandChildNode1_2));

  CHECK_FALSE(rootNode.isDescendantOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK(childNode1->isDescendantOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK(childNode2->isDescendantOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK(grandChildNode1_1->isDescendantOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
  CHECK(grandChildNode1_1->isDescendantOf(std::vector<Node*>{
    &rootNode, childNode1, childNode2, grandChildNode1_1, grandChildNode1_2}));
}

namespace
{
enum class Visited
{
  World,
  Layer,
  Group,
  Entity,
  Brush,
  Patch,
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& str, const Visited visited)
{
  switch (visited)
  {
  case Visited::World:
    return str << "World";
  case Visited::Layer:
    return str << "Layer";
  case Visited::Group:
    return str << "Group";
  case Visited::Entity:
    return str << "Entity";
  case Visited::Brush:
    return str << "Brush";
  case Visited::Patch:
    return str << "Patch";
    switchDefault();
  }
}

const auto nodeTestVisitor = kdl::overload(
  [](WorldNode*) { return Visited::World; },
  [](LayerNode*) { return Visited::Layer; },
  [](GroupNode*) { return Visited::Group; },
  [](EntityNode*) { return Visited::Entity; },
  [](BrushNode*) { return Visited::Brush; },
  [](PatchNode*) { return Visited::Patch; });

const auto constNodeTestVisitor = kdl::overload(
  [](const WorldNode*) { return Visited::World; },
  [](const LayerNode*) { return Visited::Layer; },
  [](const GroupNode*) { return Visited::Group; },
  [](const EntityNode*) { return Visited::Entity; },
  [](const BrushNode*) { return Visited::Brush; },
  [](const PatchNode*) { return Visited::Patch; });

} // namespace

TEST_CASE("NodeTest.accept")
{
  const auto worldBounds = vm::bbox3d{8192.0};

  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto layerNode = LayerNode{Layer{"name"}};
  auto groupNode = GroupNode{Group{"name"}};
  auto entityNode = EntityNode{{}};
  auto brushNode = BrushNode{
    BrushBuilder{worldNode.mapFormat(), worldBounds}.createCube(32.0, "material")
    | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, { 
    BezierPatch::Point{}, BezierPatch::Point{}, BezierPatch::Point{},
    BezierPatch::Point{}, BezierPatch::Point{}, BezierPatch::Point{},
    BezierPatch::Point{}, BezierPatch::Point{}, BezierPatch::Point{},
  }, "material"}};
  // clang-format on

  SECTION("Non const nodes accept non const visitor")
  {
    CHECK(worldNode.accept(nodeTestVisitor) == Visited::World);
    CHECK(layerNode.accept(nodeTestVisitor) == Visited::Layer);
    CHECK(groupNode.accept(nodeTestVisitor) == Visited::Group);
    CHECK(entityNode.accept(nodeTestVisitor) == Visited::Entity);
    CHECK(brushNode.accept(nodeTestVisitor) == Visited::Brush);
    CHECK(brushNode.accept(nodeTestVisitor) == Visited::Brush);
    CHECK(patchNode.accept(nodeTestVisitor) == Visited::Patch);
  }

  SECTION("Non const nodes accept const visitor")
  {
    CHECK(worldNode.accept(constNodeTestVisitor) == Visited::World);
    CHECK(layerNode.accept(constNodeTestVisitor) == Visited::Layer);
    CHECK(groupNode.accept(constNodeTestVisitor) == Visited::Group);
    CHECK(entityNode.accept(constNodeTestVisitor) == Visited::Entity);
    CHECK(brushNode.accept(constNodeTestVisitor) == Visited::Brush);
    CHECK(patchNode.accept(constNodeTestVisitor) == Visited::Patch);
  }

  SECTION("Const nodes accept const visitor")
  {
    CHECK(
      const_cast<const WorldNode&>(worldNode).accept(constNodeTestVisitor)
      == Visited::World);
    CHECK(
      const_cast<const LayerNode&>(layerNode).accept(constNodeTestVisitor)
      == Visited::Layer);
    CHECK(
      const_cast<const GroupNode&>(groupNode).accept(constNodeTestVisitor)
      == Visited::Group);
    CHECK(
      const_cast<const EntityNode&>(entityNode).accept(constNodeTestVisitor)
      == Visited::Entity);
    CHECK(
      const_cast<const BrushNode&>(brushNode).accept(constNodeTestVisitor)
      == Visited::Brush);
    CHECK(
      const_cast<const PatchNode&>(patchNode).accept(constNodeTestVisitor)
      == Visited::Patch);
  }
}

TEST_CASE("NodeTest.acceptAndVisitChildren")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* layerNode = worldNode.defaultLayer();

  auto* entityNode1 = new EntityNode{Entity{}};
  auto* entityNode2 = new EntityNode{Entity{}};
  auto* groupNode = new GroupNode(Group{"name"});
  auto* groupEntityNode = new EntityNode{Entity{}};

  layerNode->addChild(entityNode1);
  layerNode->addChild(entityNode2);
  layerNode->addChild(groupNode);
  groupNode->addChild(groupEntityNode);

  const auto collectRecursively = [](auto& node) {
    auto result = std::vector<Node*>{};
    node.accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* w) {
        result.push_back(w);
        w->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, LayerNode* l) {
        result.push_back(l);
        l->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* g) {
        result.push_back(g);
        g->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, EntityNode* e) {
        result.push_back(e);
        e->visitChildren(thisLambda);
      },
      [&](BrushNode* b) { result.push_back(b); },
      [&](PatchNode* p) { result.push_back(p); }));
    return result;
  };

  CHECK_THAT(
    collectRecursively(worldNode),
    Equals(std::vector<Node*>{
      &worldNode, layerNode, entityNode1, entityNode2, groupNode, groupEntityNode}));
  CHECK_THAT(
    collectRecursively(*groupNode),
    Equals(std::vector<Node*>{groupNode, groupEntityNode}));
  CHECK_THAT(collectRecursively(*entityNode1), Equals(std::vector<Node*>{entityNode1}));
}

TEST_CASE("NodeTest.visitParent")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* layerNode = worldNode.defaultLayer();

  CHECK(worldNode.visitParent(nodeTestVisitor) == std::nullopt);
  CHECK(worldNode.visitParent(constNodeTestVisitor) == std::nullopt);

  CHECK(layerNode->visitParent(nodeTestVisitor) == Visited::World);
  CHECK(layerNode->visitParent(constNodeTestVisitor) == Visited::World);

  CHECK(EntityNode{Entity{}}.visitParent(nodeTestVisitor) == std::nullopt);
  CHECK(EntityNode{Entity{}}.visitParent(constNodeTestVisitor) == std::nullopt);
}

namespace
{

auto makeCollectVisitedNodesVisitor(std::vector<Node*>& visited)
{
  return kdl::overload(
    [&](WorldNode* worldNode) { visited.push_back(worldNode); },
    [&](LayerNode* layerNode) { visited.push_back(layerNode); },
    [&](GroupNode* groupNode) { visited.push_back(groupNode); },
    [&](EntityNode* entityNode) { visited.push_back(entityNode); },
    [&](BrushNode* brushNode) { visited.push_back(brushNode); },
    [&](PatchNode* patchNode) { visited.push_back(patchNode); });
}

} // namespace

TEST_CASE("NodeTest.visitAll")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto layerNode = LayerNode{Layer{"name"}};
  auto groupNode = GroupNode{Group{"name"}};
  auto entityNode = EntityNode{Entity{}};

  const auto toVisit =
    std::vector<Node*>{&worldNode, &layerNode, &groupNode, &entityNode};
  auto visited = std::vector<Node*>{};
  Node::visitAll(toVisit, makeCollectVisitedNodesVisitor(visited));

  CHECK_THAT(visited, Equals(toVisit));
}

TEST_CASE("NodeTest.visitChildren")
{
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto* layerNode = worldNode.defaultLayer();

  auto* entityNode1 = new EntityNode{Entity{}};
  auto* entityNode2 = new EntityNode{Entity{}};
  layerNode->addChild(entityNode1);
  layerNode->addChild(entityNode2);

  SECTION("Visit children of world node")
  {
    auto visited = std::vector<Node*>{};
    worldNode.visitChildren(makeCollectVisitedNodesVisitor(visited));
    CHECK_THAT(visited, Equals(std::vector<Node*>{layerNode}));
  }

  SECTION("Visit children of layer node")
  {
    auto visited = std::vector<Node*>{};
    layerNode->visitChildren(makeCollectVisitedNodesVisitor(visited));
    CHECK_THAT(visited, Equals(std::vector<Node*>{entityNode1, entityNode2}));
  }

  SECTION("Visit children of entity node")
  {
    auto visited = std::vector<Node*>{};
    entityNode1->visitChildren(makeCollectVisitedNodesVisitor(visited));
    CHECK_THAT(visited, Equals(std::vector<Node*>{}));
  }
}

TEST_CASE("NodeTest.pathFrom")
{
  auto rootNode = TestNode{};
  auto& childNode1 = rootNode.addChild(new TestNode{});
  auto& childNode2 = rootNode.addChild(new TestNode{});
  auto& childNode1_1 = childNode1.addChild(new TestNode{});
  auto& childNode1_2 = childNode1.addChild(new TestNode{});
  auto& childNode1_1_1 = childNode1_1.addChild(new TestNode{});

  CHECK(childNode1_1_1.pathFrom(rootNode) == NodePath{{0, 0, 0}});
  CHECK(childNode1_1_1.pathFrom(childNode1) == NodePath{{0, 0}});
  CHECK(childNode1_1_1.pathFrom(childNode1_1) == NodePath{{0}});
  CHECK(childNode1_1_1.pathFrom(childNode1_1_1) == NodePath{{}});

  CHECK(childNode2.pathFrom(rootNode) == NodePath{{1}});
  CHECK(childNode1_2.pathFrom(rootNode) == NodePath{{0, 1}});
  CHECK(rootNode.pathFrom(rootNode) == NodePath{{}});
}

TEST_CASE("Nodepath.resolvePath")
{
  auto rootNode = TestNode{};
  auto& childNode1 = rootNode.addChild(new TestNode{});
  auto& childNode2 = rootNode.addChild(new TestNode{});
  auto& childNode1_1 = childNode1.addChild(new TestNode{});
  auto& childNode1_2 = childNode1.addChild(new TestNode{});
  auto& childNode1_1_1 = childNode1_1.addChild(new TestNode{});

  CHECK(rootNode.resolvePath(NodePath{{}}) == &rootNode);
  CHECK(rootNode.resolvePath(NodePath{{0}}) == &childNode1);
  CHECK(rootNode.resolvePath(NodePath{{1}}) == &childNode2);
  CHECK(rootNode.resolvePath(NodePath{{2}}) == nullptr);
  CHECK(rootNode.resolvePath(NodePath{{0, 0}}) == &childNode1_1);
  CHECK(rootNode.resolvePath(NodePath{{0, 0, 0}}) == &childNode1_1_1);
  CHECK(rootNode.resolvePath(NodePath{{0, 1}}) == &childNode1_2);
  CHECK(childNode1.resolvePath(NodePath{{0, 0}}) == &childNode1_1_1);
  CHECK(childNode1_1.resolvePath(NodePath{{0}}) == &childNode1_1_1);
  CHECK(childNode1_1_1.resolvePath(NodePath{{}}) == &childNode1_1_1);
}

TEST_CASE("NodeTest.entityPropertyConfig")
{
  class RootNode : public TestNode
  {
  private:
    EntityPropertyConfig m_entityPropertyConfig;

  public:
    explicit RootNode(EntityPropertyConfig entityPropertyConfig)
      : m_entityPropertyConfig{std::move(entityPropertyConfig)}
    {
    }

  private:
    const EntityPropertyConfig& doGetEntityPropertyConfig() const override
    {
      return m_entityPropertyConfig;
    }
  };

  const auto config =
    EntityPropertyConfig{{el::ExpressionNode{el::LiteralExpression{el::Value{2.0}}}}};
  auto root = std::make_unique<RootNode>(config);
  REQUIRE(root->entityPropertyConfig() == config);

  auto node = std::make_unique<TestNode>();
  CHECK(node->entityPropertyConfig() == EntityPropertyConfig{});

  root->addChild(node.release());

  auto nodePtr = root->children().front();
  CHECK(nodePtr->entityPropertyConfig() == config);
}

} // namespace tb::mdl
