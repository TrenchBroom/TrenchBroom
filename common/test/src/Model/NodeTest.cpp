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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Exceptions.h"
#include "IO/NodeWriter.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/CollectTouchingNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/Object.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/ray.h>
#include <vecmath/mat_ext.h>

#include <vector>
#include <variant>

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

        // Visitors

        TEST_CASE("CollectTouchingNodesVisitor", "[NodeVisitorTest]") {
            const vm::bbox3 worldBounds(8192.0);
            EditorContext context;

            WorldNode map(Model::MapFormat::Standard);
            map.addOrUpdateAttribute("classname", "worldspawn");

            BrushBuilder builder(&map, worldBounds);
            BrushNode* brush1 = map.createBrush(builder.createCube(64.0, "none").value());
            BrushNode* brush2 = map.createBrush(builder.createCube(64.0, "none").value());
            BrushNode* brush3 = map.createBrush(builder.createCube(64.0, "none").value());

            brush2->transform(vm::translation_matrix(vm::vec3(10.0, 0.0, 0.0)), false, worldBounds);
            brush3->transform(vm::translation_matrix(vm::vec3(100.0, 0.0, 0.0)), false, worldBounds);

            map.defaultLayer()->addChild(brush1);
            map.defaultLayer()->addChild(brush2);
            map.defaultLayer()->addChild(brush3);

            CHECK(brush1->intersects(brush2));
            CHECK(brush2->intersects(brush1));

            CHECK(!brush1->intersects(brush3));
            CHECK(!brush3->intersects(brush1));

            const auto query = std::vector<BrushNode*>{brush1};
            auto visitor = CollectTouchingNodesVisitor(std::begin(query), std::end(query), context);
            map.acceptAndRecurse(visitor);

            CHECK(std::vector<Node*>{brush2} == visitor.nodes());
        }
    }
}
