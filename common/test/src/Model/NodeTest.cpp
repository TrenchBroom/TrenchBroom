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

#include "Exceptions.h"
#include "IO/NodeWriter.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_io.h>
#include <vecmath/mat_ext.h>

#include <vector>
#include <variant>

#include "Catch2.h"
#include "GTestCompat.h"
#include "catch2/catch.hpp"

namespace TrenchBroom {
    namespace Model {
        struct DoCanAddChild { bool valueToReturn; const Node* expectedChild; };
        struct DoCanRemoveChild { bool valueToReturn; const Node* expectedChild; };
        struct DoParentWillChange {};
        struct DoParentDidChange {};
        struct DoAncestorWillChange {};
        struct DoAncestorDidChange {};

        using ExpectedCall = std::variant<
            DoCanAddChild,
            DoCanRemoveChild,
            DoParentWillChange,
            DoParentDidChange,
            DoAncestorWillChange,
            DoAncestorDidChange>;

        class MockNode : public Node {
        private:
            mutable std::vector<ExpectedCall> m_expectedCalls;
        public:
            /**
             * Sets an expectation that the given member function will be called. Some of the variant cases
             * include a value to return when that function is called, or checks to perform on the function arguments.
             *
             * The expectations set this way are all mandatory and must be called in the order they are set.
             */
            void expectCall(ExpectedCall call) {
                m_expectedCalls.push_back(std::move(call));
            }

            ~MockNode() {
                // If this fails, it means a call that was expected was not made
                ASSERT_TRUE(m_expectedCalls.empty());
            }
        private:
            template <class T>
            T popCall() const {
                ASSERT_FALSE(m_expectedCalls.empty());
                T expectedCall = std::get<T>(kdl::vec_pop_front(m_expectedCalls));
                return expectedCall;
            }
        private: // implement Node interface
            Node* doClone(const vm::bbox3& /* worldBounds */) const override {
                return new MockNode();
            }

            const std::string& doGetName() const override {
                static const std::string name("some name");
                return name;
            }

            const vm::bbox3& doGetLogicalBounds() const override {
                static const vm::bbox3 bounds;
                return bounds;
            }

            const vm::bbox3& doGetPhysicalBounds() const override {
                static const vm::bbox3 bounds;
                return bounds;
            }

            bool doCanAddChild(const Node* child) const override {
                auto call = popCall<DoCanAddChild>();
                ASSERT_EQ(call.expectedChild, child);
                return call.valueToReturn;
            }

            bool doCanRemoveChild(const Node* child) const override {
                auto call = popCall<DoCanRemoveChild>();
                ASSERT_EQ(call.expectedChild, child);
                return call.valueToReturn;
            }

            bool doRemoveIfEmpty() const override {
                return false;
            }

            bool doShouldAddToSpacialIndex() const override {
                return true;
            }

            void doParentWillChange() override {
                popCall<DoParentWillChange>();
            }

            void doParentDidChange() override {
                popCall<DoParentDidChange>();
            }

            bool doSelectable() const override {
                return false;
            }

            void doAncestorWillChange() override {
                popCall<DoAncestorWillChange>();
            }

            void doAncestorDidChange() override {
                popCall<DoAncestorDidChange>();
            }

            void doPick(const vm::ray3& /*ray*/, PickResult& /*pickResult*/) override {
            }

            void doFindNodesContaining(const vm::vec3& /*point*/, std::vector<Node*>& /*result*/) override {
            }

            void doAccept(NodeVisitor& /*visitor*/) override {
            }

            void doAccept(ConstNodeVisitor& /*visitor*/) const override {
            }

            void doGenerateIssues(const IssueGenerator* /* generator */, std::vector<Issue*>& /* issues */) override {}

            void doAcceptTagVisitor(TagVisitor& /* visitor */) override {}
            void doAcceptTagVisitor(ConstTagVisitor& /* visitor */) const override {}
        };

        class TestNode : public Node {
        private: // implement Node interface
            Node* doClone(const vm::bbox3& /* worldBounds */) const override {
                return new TestNode();
            }

            const std::string& doGetName() const override {
                static const std::string name("some name");
                return name;
            }

            const vm::bbox3& doGetLogicalBounds() const override {
                static const vm::bbox3 bounds;
                return bounds;
            }

            const vm::bbox3& doGetPhysicalBounds() const override {
                static const vm::bbox3 bounds;
                return bounds;
            }

            bool doCanAddChild(const Node* /* child */) const override {
                return true;
            }

            bool doCanRemoveChild(const Node* /* child */) const override {
                return true;
            }

            bool doRemoveIfEmpty() const override {
                return false;
            }

            bool doShouldAddToSpacialIndex() const override {
                return true;
            }

            bool doSelectable() const override {
                return true;
            }

            void doParentWillChange() override {}
            void doParentDidChange() override {}
            void doAncestorWillChange() override {}
            void doAncestorDidChange() override {}

            void doPick(const vm::ray3& /* ray */, PickResult& /* pickResult */) override {}
            void doFindNodesContaining(const vm::vec3& /* point */, std::vector<Node*>& /* result */) override {}

            void doAccept(NodeVisitor& /* visitor */) override {}
            void doAccept(ConstNodeVisitor& /* visitor */) const override {}
            void doGenerateIssues(const IssueGenerator* /* generator */, std::vector<Issue*>& /* issues */) override {}

            void doAcceptTagVisitor(TagVisitor& /* visitor */) override {}
            void doAcceptTagVisitor(ConstTagVisitor& /* visitor */) const override {}
        };

        class DestroyableNode : public TestNode {
        private:
            bool& m_destroyed;
        public:
            explicit DestroyableNode(bool& destroyed) :
            m_destroyed(destroyed) {}

            ~DestroyableNode() override {
                m_destroyed = true;
            }
        };

        TEST_CASE("NodeTest.destroyChild", "[NodeTest]") {
            TestNode* root = new TestNode();
            bool childDestroyed = false;
            DestroyableNode* child = new DestroyableNode(childDestroyed);

            root->addChild(child);
            delete root;

            ASSERT_TRUE(childDestroyed);
        }

        TEST_CASE("NodeTest.addRemoveChild", "[NodeTest]") {
            MockNode root;
            MockNode* child = new MockNode();
            MockNode* grandChild1 = new MockNode();
            MockNode* grandChild2 = new MockNode();

            // NOTE: Node::doAddChild only calls canAddChild in debug builds
#ifndef NDEBUG
            child->expectCall(DoCanAddChild{true, grandChild1});
#endif
            grandChild1->expectCall(DoParentWillChange{});
            grandChild1->expectCall(DoAncestorWillChange{});
            grandChild1->expectCall(DoParentDidChange{});
            grandChild1->expectCall(DoAncestorDidChange{});
            child->addChild(grandChild1);
            ASSERT_EQ(1u, child->childCount());
            ASSERT_EQ(2u, child->familySize());
            ASSERT_EQ(child, grandChild1->parent());
            ASSERT_TRUE(kdl::vec_contains(child->children(), grandChild1));

#ifndef NDEBUG
            root.expectCall(DoCanAddChild{true, child});
#endif
            child->expectCall(DoParentWillChange{});
            child->expectCall(DoAncestorWillChange{});
            child->expectCall(DoParentDidChange{});
            child->expectCall(DoAncestorDidChange{});
            grandChild1->expectCall(DoAncestorWillChange{});
            grandChild1->expectCall(DoAncestorDidChange{});

            root.addChild(child);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(3u, root.familySize());
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(kdl::vec_contains(root.children(), child));

#ifndef NDEBUG
            child->expectCall(DoCanAddChild{true,grandChild2});
#endif
            grandChild2->expectCall(DoParentWillChange{});
            grandChild2->expectCall(DoAncestorWillChange{});
            grandChild2->expectCall(DoParentDidChange{});
            grandChild2->expectCall(DoAncestorDidChange{});
            child->addChild(grandChild2);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
            ASSERT_EQ(child, grandChild2->parent());
            ASSERT_TRUE(kdl::vec_contains(child->children(), grandChild2));

#ifndef NDEBUG
            root.expectCall(DoCanRemoveChild{true,child});
#endif
            child->expectCall(DoParentWillChange{});
            child->expectCall(DoAncestorWillChange{});
            child->expectCall(DoParentDidChange{});
            child->expectCall(DoAncestorDidChange{});
            grandChild1->expectCall(DoAncestorWillChange{});
            grandChild1->expectCall(DoAncestorDidChange{});
            grandChild2->expectCall(DoAncestorWillChange{});
            grandChild2->expectCall(DoAncestorDidChange{});

            root.removeChild(child);
            ASSERT_EQ(nullptr, child->parent());
            ASSERT_FALSE(kdl::vec_contains(root.children(), child));
            ASSERT_EQ(0u, root.childCount());
            ASSERT_EQ(1u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());

#ifndef NDEBUG
            root.expectCall(DoCanAddChild{true,child});
#endif
            child->expectCall(DoParentWillChange{});
            child->expectCall(DoAncestorWillChange{});
            child->expectCall(DoParentDidChange{});
            child->expectCall(DoAncestorDidChange{});
            grandChild1->expectCall(DoAncestorWillChange{});
            grandChild1->expectCall(DoAncestorDidChange{});
            grandChild2->expectCall(DoAncestorWillChange{});
            grandChild2->expectCall(DoAncestorDidChange{});

            root.addChild(child);
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(kdl::vec_contains(root.children(), child));
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
        }

        TEST_CASE("NodeTest.partialSelection", "[NodeTest]") {
            TestNode root;
            TestNode* child1 = new TestNode();
            TestNode* child2 = new TestNode();
            TestNode* grandChild1_1 = new TestNode();
            TestNode* grandChild1_2 = new TestNode();

            root.addChild(child1);
            root.addChild(child2);

            ASSERT_EQ(0u, root.descendantSelectionCount());
            child1->select();
            ASSERT_EQ(0u, child1->descendantSelectionCount());
            ASSERT_EQ(1u, root.descendantSelectionCount());
            child2->select();
            ASSERT_EQ(0u, child1->descendantSelectionCount());
            ASSERT_EQ(0u, child2->descendantSelectionCount());
            ASSERT_EQ(2u, root.descendantSelectionCount());

            child1->deselect();
            ASSERT_EQ(0u, child1->descendantSelectionCount());
            ASSERT_EQ(1u, root.descendantSelectionCount());

            grandChild1_1->select();
            child1->addChild(grandChild1_1);
            ASSERT_EQ(1u, child1->descendantSelectionCount());
            ASSERT_EQ(2u, root.descendantSelectionCount());

            child1->addChild(grandChild1_2);
            ASSERT_EQ(1u, child1->descendantSelectionCount());
            ASSERT_EQ(2u, root.descendantSelectionCount());
            grandChild1_2->select();
            ASSERT_EQ(2u, child1->descendantSelectionCount());
            ASSERT_EQ(3u, root.descendantSelectionCount());

            grandChild1_1->deselect();
            ASSERT_EQ(1u, child1->descendantSelectionCount());
            ASSERT_EQ(2u, root.descendantSelectionCount());
        }

        TEST_CASE("NodeTest.isAncestorOf", "[NodeTest]") {
            TestNode root;
            TestNode* child1 = new TestNode();
            TestNode* child2 = new TestNode();
            TestNode* grandChild1_1 = new TestNode();
            TestNode* grandChild1_2 = new TestNode();

            root.addChild(child1);
            root.addChild(child2);
            child1->addChild(grandChild1_1);
            child1->addChild(grandChild1_2);

            ASSERT_FALSE(root.isAncestorOf(&root));
            ASSERT_TRUE(root.isAncestorOf(child1));
            ASSERT_TRUE(root.isAncestorOf(child2));
            ASSERT_TRUE(root.isAncestorOf(grandChild1_1));
            ASSERT_TRUE(root.isAncestorOf(grandChild1_2));

            ASSERT_FALSE(child1->isAncestorOf(&root));
            ASSERT_FALSE(child1->isAncestorOf(child1));
            ASSERT_FALSE(child1->isAncestorOf(child2));
            ASSERT_TRUE(child1->isAncestorOf(grandChild1_1));
            ASSERT_TRUE(child1->isAncestorOf(grandChild1_2));

            ASSERT_FALSE(child2->isAncestorOf(&root));
            ASSERT_FALSE(child2->isAncestorOf(child1));
            ASSERT_FALSE(child2->isAncestorOf(child2));
            ASSERT_FALSE(child2->isAncestorOf(grandChild1_1));
            ASSERT_FALSE(child2->isAncestorOf(grandChild1_2));

            ASSERT_FALSE(grandChild1_1->isAncestorOf(&root));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(child1));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(child2));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(grandChild1_1));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(grandChild1_2));

            ASSERT_FALSE(grandChild1_2->isAncestorOf(&root));
            ASSERT_FALSE(grandChild1_2->isAncestorOf(child1));
            ASSERT_FALSE(grandChild1_2->isAncestorOf(child2));
            ASSERT_FALSE(grandChild1_2->isAncestorOf(grandChild1_1));
            ASSERT_FALSE(grandChild1_2->isAncestorOf(grandChild1_2));

            ASSERT_TRUE(root.isAncestorOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child1->isAncestorOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(child2->isAncestorOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
        }

        TEST_CASE("NodeTest.isDescendantOf", "[NodeTest]") {
            TestNode root;
            TestNode* child1 = new TestNode();
            TestNode* child2 = new TestNode();
            TestNode* grandChild1_1 = new TestNode();
            TestNode* grandChild1_2 = new TestNode();

            root.addChild(child1);
            root.addChild(child2);
            child1->addChild(grandChild1_1);
            child1->addChild(grandChild1_2);

            ASSERT_FALSE(root.isDescendantOf(&root));
            ASSERT_FALSE(root.isDescendantOf(child1));
            ASSERT_FALSE(root.isDescendantOf(child2));
            ASSERT_FALSE(root.isDescendantOf(grandChild1_1));
            ASSERT_FALSE(root.isDescendantOf(grandChild1_2));

            ASSERT_TRUE(child1->isDescendantOf(&root));
            ASSERT_FALSE(child1->isDescendantOf(child1));
            ASSERT_FALSE(child1->isDescendantOf(child2));
            ASSERT_FALSE(child1->isDescendantOf(grandChild1_1));
            ASSERT_FALSE(child1->isDescendantOf(grandChild1_2));

            ASSERT_TRUE(child2->isDescendantOf(&root));
            ASSERT_FALSE(child2->isDescendantOf(child1));
            ASSERT_FALSE(child2->isDescendantOf(child2));
            ASSERT_FALSE(child2->isDescendantOf(grandChild1_1));
            ASSERT_FALSE(child2->isDescendantOf(grandChild1_2));

            ASSERT_TRUE(grandChild1_1->isDescendantOf(&root));
            ASSERT_TRUE(grandChild1_1->isDescendantOf(child1));
            ASSERT_FALSE(grandChild1_1->isDescendantOf(child2));
            ASSERT_FALSE(grandChild1_1->isDescendantOf(grandChild1_1));
            ASSERT_FALSE(grandChild1_1->isDescendantOf(grandChild1_2));

            ASSERT_TRUE(grandChild1_2->isDescendantOf(&root));
            ASSERT_TRUE(grandChild1_2->isDescendantOf(child1));
            ASSERT_FALSE(grandChild1_2->isDescendantOf(child2));
            ASSERT_FALSE(grandChild1_2->isDescendantOf(grandChild1_1));
            ASSERT_FALSE(grandChild1_2->isDescendantOf(grandChild1_2));

            ASSERT_FALSE(root.isDescendantOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child1->isDescendantOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child2->isDescendantOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(grandChild1_1->isDescendantOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(grandChild1_1->isDescendantOf(std::vector<Node*>{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
        }

        enum class Visited {
            World,
            Layer,
            Group,
            Entity,
            Brush
        };

        std::ostream& operator<<(std::ostream& str, const Visited visited) {
            switch (visited) {
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
                switchDefault()
            }
        }

        const auto nodeTestVisitor = kdl::overload(
            [](WorldNode*)  { return Visited::World; },
            [](LayerNode*)  { return Visited::Layer; },
            [](GroupNode*)  { return Visited::Group; },
            [](EntityNode*) { return Visited::Entity; },
            [](BrushNode*)  { return Visited::Brush; }
        );

        const auto constNodeTestVisitor = kdl::overload(
            [](const WorldNode*)  { return Visited::World; },
            [](const LayerNode*)  { return Visited::Layer; },
            [](const GroupNode*)  { return Visited::Group; },
            [](const EntityNode*) { return Visited::Entity; },
            [](const BrushNode*)  { return Visited::Brush; }
        );

        TEST_CASE("NodeTest.accept", "[NodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);

            WorldNode world(Entity(), MapFormat::Standard);
            LayerNode layer("name");
            GroupNode group("name");
            EntityNode entity;
            BrushNode brush(BrushBuilder(&world, worldBounds).createCube(32.0, "texture").value());

            SECTION("Non const nodes accept non const visitor") {
                CHECK(world.accept(nodeTestVisitor) == Visited::World);
                CHECK(layer.accept(nodeTestVisitor) == Visited::Layer);
                CHECK(group.accept(nodeTestVisitor) == Visited::Group);
                CHECK(entity.accept(nodeTestVisitor) == Visited::Entity);
                CHECK(brush.accept(nodeTestVisitor) == Visited::Brush);
            }

            SECTION("Non const nodes accept const visitor") {
                CHECK(world.accept(constNodeTestVisitor) == Visited::World);
                CHECK(layer.accept(constNodeTestVisitor) == Visited::Layer);
                CHECK(group.accept(constNodeTestVisitor) == Visited::Group);
                CHECK(entity.accept(constNodeTestVisitor) == Visited::Entity);
                CHECK(brush.accept(constNodeTestVisitor) == Visited::Brush);
            }

            SECTION("Const nodes accept const visitor") {
                CHECK(const_cast<const WorldNode&> (world).accept(constNodeTestVisitor) == Visited::World);
                CHECK(const_cast<const LayerNode&> (layer).accept(constNodeTestVisitor) == Visited::Layer);
                CHECK(const_cast<const GroupNode&> (group).accept(constNodeTestVisitor) == Visited::Group);
                CHECK(const_cast<const EntityNode&>(entity).accept(constNodeTestVisitor) == Visited::Entity);
                CHECK(const_cast<const BrushNode&> (brush).accept(constNodeTestVisitor) == Visited::Brush);
            }
        }

        TEST_CASE("NodeTest.acceptAndVisitChildren", "[NodeTest]") {
            WorldNode world(Entity(), MapFormat::Standard);
            auto* layer = world.defaultLayer();

            auto* entity1 = world.createEntity(Entity());
            auto* entity2 = world.createEntity(Entity());
            auto* group = world.createGroup("name");
            auto* groupEntity = world.createEntity(Entity());

            layer->addChild(entity1);
            layer->addChild(entity2);
            layer->addChild(group);
            group->addChild(groupEntity);

            const auto collectRecursively = [](auto& node) {
                auto result = std::vector<Node*>{};
                node.accept(kdl::overload(
                    [&](auto&& thisLambda, WorldNode* w)  { result.push_back(w); w->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, LayerNode* l)  { result.push_back(l); l->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, GroupNode* g)  { result.push_back(g); g->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, EntityNode* e) { result.push_back(e); e->visitChildren(thisLambda); },
                    [&](BrushNode* b)                     { result.push_back(b); }
                ));
                return result;
            };

            CHECK_THAT(collectRecursively(world), Catch::Equals(std::vector<Node*>{&world, layer, entity1, entity2, group, groupEntity}));
            CHECK_THAT(collectRecursively(*group), Catch::Equals(std::vector<Node*>{group, groupEntity}));
            CHECK_THAT(collectRecursively(*entity1), Catch::Equals(std::vector<Node*>{entity1}));
        }

        TEST_CASE("NodeTest.visitParent", "[NodeTest]") {
            WorldNode world(Entity(), MapFormat::Standard);
            auto* layer = world.defaultLayer();

            CHECK(world.visitParent(nodeTestVisitor) == std::nullopt);
            CHECK(world.visitParent(constNodeTestVisitor) == std::nullopt);

            CHECK(layer->visitParent(nodeTestVisitor) == Visited::World);
            CHECK(layer->visitParent(constNodeTestVisitor) == Visited::World);

            CHECK(EntityNode().visitParent(nodeTestVisitor) == std::nullopt);
            CHECK(EntityNode().visitParent(constNodeTestVisitor) == std::nullopt);
        }

        static auto makeCollectVisitedNodesVisitor(std::vector<Node*>& visited) {
            return kdl::overload(
                [&](WorldNode* world)   { visited.push_back(world); },
                [&](LayerNode* layer)   { visited.push_back(layer); },
                [&](GroupNode* group)   { visited.push_back(group); },
                [&](EntityNode* entity) { visited.push_back(entity); },
                [&](BrushNode* brush)   { visited.push_back(brush); }
            );
        }

        TEST_CASE("NodeTest.visitAll", "[NodeTest]") {
            WorldNode world(Entity(), MapFormat::Standard);
            LayerNode layer("name");
            GroupNode group("name");
            EntityNode entity;

            const auto toVisit = std::vector<Node*>{&world, &layer, &group, &entity};
            auto visited = std::vector<Node*>{};
            Node::visitAll(toVisit, makeCollectVisitedNodesVisitor(visited));

            CHECK_THAT(visited, Catch::Equals(toVisit));
        }

        TEST_CASE("NodeTest.visitChildren", "[NodeTest]") {
            WorldNode world(Entity(), MapFormat::Standard);
            auto* layer = world.defaultLayer();
            
            auto* entity1 = world.createEntity(Entity());
            auto* entity2 = world.createEntity(Entity());
            layer->addChild(entity1);
            layer->addChild(entity2);

            SECTION("Visit children of world node") {
                auto visited = std::vector<Node*>{};
                world.visitChildren(makeCollectVisitedNodesVisitor(visited));
                CHECK_THAT(visited, Catch::Equals(std::vector<Node*>{layer}));
            }

            SECTION("Visit children of layer node") {
                auto visited = std::vector<Node*>{};
                layer->visitChildren(makeCollectVisitedNodesVisitor(visited));
                CHECK_THAT(visited, Catch::Equals(std::vector<Node*>{entity1, entity2}));
            }

            SECTION("Visit children of entity node") {
                auto visited = std::vector<Node*>{};
                entity1->visitChildren(makeCollectVisitedNodesVisitor(visited));
                CHECK_THAT(visited, Catch::Equals(std::vector<Node*>{}));
            }
        }
    }
}
