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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "CollectionUtils.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"

namespace TrenchBroom {
    namespace Model {
        class MockNode : public Node {
        private: // implement Node interface
            Node* doClone(const bbox3& worldBounds) const override {
                return new MockNode();
            }
            
            const String& doGetName() const override {
                static const String name("some name");
                return name;
            }
            
            const bbox3& doGetBounds() const override {
                static const bbox3 bounds;
                return bounds;
            }
            
            bool doCanAddChild(const Node* child) const override {
                return mockDoCanAddChild(child);
            }
            
            bool doCanRemoveChild(const Node* child) const override {
                return mockDoCanRemoveChild(child);
            }
            
            bool doRemoveIfEmpty() const override {
                return false;
            }
            
            void doParentWillChange() override {
                mockDoParentWillChange();
            }
            
            void doParentDidChange() override {
                mockDoParentDidChange();
            }

            bool doSelectable() const override {
                return mockDoSelectable();
            }
            
            void doAncestorWillChange() override {
                mockDoAncestorWillChange();
            }
            
            void doAncestorDidChange() override {
                mockDoAncestorDidChange();
            }
            
            void doPick(const Ray3& ray, PickResult& pickResult) const override {
                mockDoPick(ray, pickResult);
            }
            
            void doFindNodesContaining(const vec3& point, NodeList& result) override {
                mockDoFindNodesContaining(point, result);
            }

            FloatType doIntersectWithRay(const Ray3& ray) const override {
                return mockDoIntersectWithRay(ray);
            }

            void doAccept(NodeVisitor& visitor) override {
                mockDoAccept(visitor);
            }
            
            void doAccept(ConstNodeVisitor& visitor) const override {
                mockDoAccept(visitor);
            }
        
            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override {}
        public:
            MOCK_CONST_METHOD1(mockDoCanAddChild, bool(const Node*));
            MOCK_CONST_METHOD1(mockDoCanRemoveChild, bool(const Node*));
            MOCK_CONST_METHOD0(mockDoSelectable, bool());
            
            MOCK_METHOD0(mockDoParentWillChange, void());
            MOCK_METHOD0(mockDoParentDidChange, void());
            MOCK_METHOD0(mockDoAncestorWillChange, void());
            MOCK_METHOD0(mockDoAncestorDidChange, void());
            
            MOCK_CONST_METHOD2(mockDoPick, void(const Ray3&, PickResult&));
            MOCK_CONST_METHOD2(mockDoFindNodesContaining, void(const vec3&, NodeList&));
            MOCK_CONST_METHOD1(mockDoIntersectWithRay, FloatType(const Ray3&));
            
            MOCK_METHOD1(mockDoAccept, void(NodeVisitor&));
            MOCK_CONST_METHOD1(mockDoAccept, void(ConstNodeVisitor&));
        };
        
        class TestNode : public Node {
        private: // implement Node interface
            Node* doClone(const bbox3& worldBounds) const override {
                return new TestNode();
            }
            
            const String& doGetName() const override {
                static const String name("some name");
                return name;
            }
            
            const bbox3& doGetBounds() const override {
                static const bbox3 bounds;
                return bounds;
            }
            
            bool doCanAddChild(const Node* child) const override {
                return true;
            }
            
            bool doCanRemoveChild(const Node* child) const override {
                return true;
            }
            
            bool doRemoveIfEmpty() const override {
                return false;
            }
            
            bool doSelectable() const override {
                return true;
            }
            
            void doParentWillChange() override {}
            void doParentDidChange() override {}
            void doAncestorWillChange() override {}
            void doAncestorDidChange() override {}
            
            void doPick(const Ray3& ray, PickResult& pickResult) const override {}
            void doFindNodesContaining(const vec3& point, NodeList& result) override {}
            FloatType doIntersectWithRay(const Ray3& ray) const override { return Math::nan<FloatType>(); }

            void doAccept(NodeVisitor& visitor) override {}
            void doAccept(ConstNodeVisitor& visitor) const override {}
            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override {}
        };
        
        class DestroyableNode : public TestNode {
        private:
            bool& m_destroyed;
        public:
            DestroyableNode(bool& destroyed) :
            m_destroyed(destroyed) {}
            
            ~DestroyableNode() override {
                m_destroyed = true;
            }
        };
        
        TEST(NodeTest, destroyChild) {
            using namespace ::testing;
            
            TestNode* root = new TestNode();
            bool childDestroyed = false;
            DestroyableNode* child = new DestroyableNode(childDestroyed);
            
            root->addChild(child);
            delete root;
            
            ASSERT_TRUE(childDestroyed);
        }
        
        TEST(NodeTest, addRemoveChild) {
            using namespace ::testing;
            
            MockNode root;
            MockNode* child = new MockNode();
            MockNode* grandChild1 = new MockNode();
            MockNode* grandChild2 = new MockNode();
            
#ifndef NDEBUG
            EXPECT_CALL(*child, mockDoCanAddChild(grandChild1)).WillOnce(Return(true));
#endif
            EXPECT_CALL(*grandChild1, mockDoParentWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoParentDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            child->addChild(grandChild1);
            ASSERT_EQ(1u, child->childCount());
            ASSERT_EQ(2u, child->familySize());
            ASSERT_EQ(child, grandChild1->parent());
            ASSERT_TRUE(VectorUtils::contains(child->children(), grandChild1));
            
#ifndef NDEBUG
            EXPECT_CALL(root, mockDoCanAddChild(child)).WillOnce(Return(true));
#endif
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            
            root.addChild(child);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(3u, root.familySize());
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(VectorUtils::contains(root.children(), child));
            
#ifndef NDEBUG
            EXPECT_CALL(*child, mockDoCanAddChild(grandChild2)).WillOnce(Return(true));
#endif
            EXPECT_CALL(*grandChild2, mockDoParentWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoParentDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());
            child->addChild(grandChild2);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
            ASSERT_EQ(child, grandChild2->parent());
            ASSERT_TRUE(VectorUtils::contains(child->children(), grandChild2));
            
#ifndef NDEBUG
            EXPECT_CALL(root, mockDoCanRemoveChild(child)).WillOnce(Return(true));
#endif
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());

            root.removeChild(child);
            ASSERT_EQ(nullptr, child->parent());
            ASSERT_FALSE(VectorUtils::contains(root.children(), child));
            ASSERT_EQ(0u, root.childCount());
            ASSERT_EQ(1u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
            
#ifndef NDEBUG
            EXPECT_CALL(root, mockDoCanAddChild(child)).WillOnce(Return(true));
#endif
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());
            
            root.addChild(child);
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(VectorUtils::contains(root.children(), child));
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
        }
        
        TEST(NodeTest, partialSelection) {
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
        
        TEST(NodeTest, isAncestorOf) {
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
            
            ASSERT_TRUE(root.isAncestorOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child1->isAncestorOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(child2->isAncestorOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_FALSE(grandChild1_1->isAncestorOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
        }
        
        TEST(NodeTest, isDescendantOf) {
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
            
            ASSERT_FALSE(root.isDescendantOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child1->isDescendantOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(child2->isDescendantOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(grandChild1_1->isDescendantOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
            ASSERT_TRUE(grandChild1_1->isDescendantOf(NodeList{ &root, child1, child2, grandChild1_1, grandChild1_2 }));
        }
    }
}
