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

#pragma once

#include "Exceptions.h"

#include <vecmath/scalar.h>
#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/ray.h>
#include <vecmath/intersection.h>

#include <cassert>
#include <iosfwd>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
/**
 * An axis aligned bounding box tree that allows for quick ray intersection queries.
 *
 * @tparam T the floating point type
 * @tparam S the number of dimensions for vector types
 * @tparam U the node data to store in the leafs
 */
    template <typename T, size_t S, typename U>
    class AABBTree {
    public:
        using List = std::vector<U>;
        using Box = vm::bbox<T,S>;
        using DataType = U;
        using FloatType = T;
        static constexpr size_t Components = S;
    private:
        class InnerNode;
        class LeafNode;

        class Visitor {
        public:
            virtual ~Visitor() = default;

            virtual bool visit(const InnerNode* innerNode) = 0;
            virtual void visit(const LeafNode* leaf) = 0;
        };

        template <typename I_V, typename L_V>
        class LambdaVisitor : public Visitor {
        private:
            I_V m_innerNodeVisitor;
            L_V m_leafNodeVisitor;
        public:
            LambdaVisitor(I_V innerNodeVisitor, L_V leafNodeVisitor) :
                m_innerNodeVisitor(std::move(innerNodeVisitor)),
                m_leafNodeVisitor(std::move(leafNodeVisitor)) {}

            bool visit(const InnerNode* innerNode) override { return m_innerNodeVisitor(innerNode); }
            void visit(const LeafNode* leafNode)   override { m_leafNodeVisitor(leafNode); }
        };

#if !defined __GNUC__ || defined __clang__
        /**
         * Deduction guide.
         *
         * Note that as of GCC 9.2, there appears to be a bug in GCC that disallows deduction guides in non-namespace scope.
         * Luckily, GCC does not need a guide to deduce the template parameters of LambdaVisitor, but clang does!
         */
        template <typename I_V, typename L_V>
        LambdaVisitor(I_V innerNodeVisitor, L_V outerNodeVisitor) -> LambdaVisitor<I_V, L_V>;
#endif

        class Node {
        public:
            Box m_bounds;
            InnerNode* m_parent;
        protected:
            explicit Node(const Box& bounds) :
                m_bounds(bounds),
                m_parent(nullptr) {}
        public:
            virtual ~Node() = default;

            /**
             * Return the bounds of this node.
             *
             * @return the bounds
             */
            const Box& bounds() const {
                return m_bounds;
            }

            /**
             * Return the height of this node. A leaf always has a height of 1, and an inner node has a height equal to the
             * maximum of the heights of its children plus one.
             *
             * @return the height
             */
            virtual size_t height() const = 0;

            /**
             * Inserts a new node into the subtree rooted at `this`.
             *
             * @param bounds the bounds of the data to be inserted
             * @param data the data to be inserted
             * @return a pair containing the new subtree root (may be `this`, or a new node), and the newly inserted LeafNode
             */
            virtual std::pair<Node*, LeafNode*> insert(const Box& bounds, const U& data) = 0;

            /**
             * Accepts the given visitor.
             *
             * @param visitor the visitor to accept
             */
            virtual void accept(Visitor& visitor) const = 0;
        public:
            /**
             * Appends a textual representation of this node to the given output stream.
             *
             * @param str the stream to append to
             */
            void appendTo(std::ostream& str) const {
                appendTo(str, "  ", 0);
            }

            /**
             * Appends a textual representation of this node to the given output stream using the given indent string and
             * the given level of indentation.
             *
             * @param str the stream to append to
             * @param indent the indent string
             * @param level the level of indentation
             */
            virtual void appendTo(std::ostream& str, const std::string& indent, size_t level) const = 0;

            virtual void checkParentPointers(const Node* expectedParent) const = 0;
        protected:
            /**
             * Updates the bounds of this node.
             *
             * @param bounds the new bounds
             */
            void setBounds(const Box& bounds) {
                m_bounds = bounds;
            }

            /**
             * Appends a textual representation of this node's bounds to the given output stream.
             *
             * @param str the stream to append to
             */
            void appendBounds(std::ostream& str) const {
                str << "[ ( " << m_bounds.min << " ) ( " << m_bounds.max  << " ) ]";
            }
        };

        /**
         * An inner node of an AABB tree does not carry data. It's only purpose is to structure the tree. Its bounds is
         * the smallest bounding box that contains the bounds of its children.
         */
        class InnerNode : public Node {
        private:
            Node* m_left;
            Node* m_right;
            size_t m_height;
        public:
            InnerNode(Node* left, Node* right) :
                Node(merge(left->bounds(), right->bounds())),
                m_left(left),
                m_right(right),
                m_height(0) {
                assert(m_left != nullptr);
                assert(m_right != nullptr);

                m_left->m_parent = this;
                m_right->m_parent = this;

                updateHeight();
            }

        private: // node removal private
            /**
             * Children (or grandchildren etc.) changed. Update the height and bounds.
             * Recurses up the tree along the m_parent chain until it reaches the root.
             *
             * @return the new root of the tree
             */
            Node* updateAndReturnRoot() {
                updateHeight();
                updateBounds();

                if (this->m_parent == nullptr) {
                    return this;
                }

                return this->m_parent->updateAndReturnRoot();
            }

            /**
             * One of our direct children is being swapped for a new node.
             *
             * @return the new root of the tree
             */
            Node* replaceChild(Node* child, Node* replacement) {
                replacement->m_parent = this;

                if (child == m_left) {
                    m_left = replacement;
                } else {
                    assert(child == m_right);
                    m_right = replacement;
                }

                return updateAndReturnRoot();
            }
        public: // Node removal public
            /**
             * One of our direct children is being deleted. `this` will turn into a LeafNode.
             *
             * @param child either m_left or m_right, which is being deleted
             * @return the new root of the tree
             */
            Node* handleChildDeletion(LeafNode* child) {
                Node* replacementForThis;
                if (child == m_left) {
                    replacementForThis = m_right;
                } else {
                    assert(child == m_right);
                    replacementForThis = m_left;
                }

                // Clear m_left/m_right so our destructor doesn't delete our children.
                // The caller is going to delete one of them (`child`),
                // and we will reuse the other one as the replacement for `this`.
                m_left = nullptr;
                m_right = nullptr;

                // Special case when `this` is already the tree root
                if (this->m_parent == nullptr) {
                    Node* newTreeRoot = replacementForThis;
                    newTreeRoot->m_parent = nullptr;
                    delete this;
                    return newTreeRoot;
                }

                Node* newTreeRoot = this->m_parent->replaceChild(this, replacementForThis);
                delete this;
                return newTreeRoot;
            }

        public: // Node overrides
            ~InnerNode() override {
                delete m_left;
                delete m_right;
            }

            size_t height() const override {
                return m_height;
            }

            std::pair<Node*, LeafNode*> insert(const Box& bounds, const U& data) override {
                // Select the subtree which is increased the least by inserting a node with the given bounds.
                // Then insert the node into that subtree and update our reference to it.
                auto*& subtree = selectLeastIncreaser(m_left, m_right, bounds);

                Node* newSubtree;
                LeafNode* insertedLeafNode;
                std::tie(newSubtree, insertedLeafNode) = subtree->insert(bounds, data);

                // Update the parent pointer
                newSubtree->m_parent = this;

                // Subtree is either a reference to m_left or m_right
                if (subtree == m_left) {
                    m_left = newSubtree;
                } else {
                    assert(subtree == m_right);
                    m_right = newSubtree;
                }

                // Update our data.
                updateBounds();
                updateHeight();

                return std::make_pair(this, insertedLeafNode);
            }

        private:
            /**
             * Selects one of the two given nodes such that it increases the given bounds the least.
             *
             * @tparam TT the type of the nodes
             * @param node1 the first node to test
             * @param node2 the second node to test
             * @param bounds the bounds to test against
             * @return node1 if it increases the given bounds volume by a smaller or equal amount than node2 would, and
             *     node2 otherwise
             */
            template <typename TT>
            static TT*& selectLeastIncreaser(TT*& node1, TT*& node2, const Box& bounds) {
                const auto node1Contains = node1->bounds().contains(bounds);
                const auto node2Contains = node2->bounds().contains(bounds);

                if (node1Contains && !node2Contains) {
                    return node1;
                } else if (!node1Contains && node2Contains) {
                    return node2;
                } else if (!node1Contains && !node2Contains) {
                    const auto new1 = vm::merge(node1->bounds(), bounds);
                    const auto new2 = vm::merge(node2->bounds(), bounds);
                    const auto vol1 = node1->bounds().volume();
                    const auto vol2 = node2->bounds().volume();
                    const auto diff1 = new1.volume() - vol1;
                    const auto diff2 = new2.volume() - vol2;

                    if (diff1 < diff2) {
                        return node1;
                    } else if (diff2 < diff1) {
                        return node2;
                    }
                }

                static auto choice = 0u;

                if (node1->height() < node2->height()) {
                    return node1;
                } else if (node2->height() < node1->height()) {
                    return node2;
                } else {
                    if (choice++ % 2 == 0) {
                        return node1;
                    } else {
                        return node2;
                    }
                }
            }

            void updateBounds() {
                this->setBounds(merge(m_left->bounds(), m_right->bounds()));
            }

            void updateHeight() {
                m_height = std::max(m_left->height(), m_right->height()) + 1;
                assert(m_height > 0);
            }

            void accept(Visitor& visitor) const override {
                if (visitor.visit(this)) {
                    m_left->accept(visitor);
                    m_right->accept(visitor);
                }
            }
        public:
            void appendTo(std::ostream& str, const std::string& indent, const size_t level) const override {
                for (size_t i = 0; i < level; ++i)
                    str << indent;

                str << "O ";
                this->appendBounds(str);
                str << std::endl;

                m_left->appendTo(str, indent, level + 1);
                m_right->appendTo(str, indent, level + 1);
            }

            void checkParentPointers([[maybe_unused]] const Node* expectedParent) const override {
                assert(this->m_parent == expectedParent);
                m_left->checkParentPointers(this);
                m_left->checkParentPointers(this);
            }
        };

        /**
         * A leaf node represents actual data. It does not have any children. Its bounds equals the bounds supplied when
         * the node was inserted into the tree. A leaf has a height of 1 and a balance of 0.
         */
        class LeafNode : public Node {
        private:
            U m_data;
        public:
            LeafNode(const Box& bounds, const U& data) : Node(bounds), m_data(data) {}

            /**
             * Deletes this. Returns the new root of the tree.
             */
            Node* deleteThis() {
                Node* newRoot = (this->m_parent != nullptr)
                                ? this->m_parent->handleChildDeletion(this)
                                : nullptr;
                delete this;
                return newRoot;
            }

            /**
             * Returns the data associated with this node.
             *
             * @return the data associated with this node
             */
            U& data() {
                return m_data;
            }

            /**
             * Returns the data associated with this node.
             *
             * @return the data associated with this node
             */
            const U& data() const {
                return m_data;
            }

        public: // Node overrides
            size_t height() const override {
                return 1;
            }

            /**
             * Returns a new inner node that has this leaf as its left child and a new leaf representing the given bounds
             * and data as its right child.
             *
             * @param bounds the bounds to insert
             * @param data the data to insert
             * @return a pair containing the new inner node that is the root of this subtree, and the newly inserted LeafNode
             */
            std::pair<Node*, LeafNode*> insert(const Box& bounds, const U& data) override {
                auto* newLeaf = new LeafNode(bounds, data);
                auto* newParent = new InnerNode(this, newLeaf);

                return std::make_pair(newParent, newLeaf);
            }

            void accept(Visitor& visitor) const override {
                visitor.visit(this);
            }

            void appendTo(std::ostream& str, const std::string& indent, const size_t level) const override {
                for (size_t i = 0; i < level; ++i)
                    str << indent;

                str << "L ";
                this->appendBounds(str);
                str << ": " << m_data << std::endl;
            }

            virtual void checkParentPointers([[maybe_unused]] const Node* expectedParent) const override {
                assert(this->m_parent == expectedParent);
            }
        };
    private:
        Node* m_root;
        std::unordered_map<U, LeafNode*> m_leafForData;
    public:
        AABBTree() : m_root(nullptr) {}

        ~AABBTree() {
            clear();
        }

        /**
         * Indicates whether a node with the given data exists in this tree.
         *
         * @param data the data to find
         * @return true if a node with the given data exists and false otherwise
         */
        bool contains(const U& data) const {
            auto it = m_leafForData.find(data);
            return it != m_leafForData.end();
        }

        /**
         * Clears this tree and rebuilds it by inserting given objects.
         *
         * @param objects the objects to insert, a list of DataType
         * @param getBounds a function from DataType -> Box to compute the bounds of each object
         */
        template <typename DataList, typename GetBounds>
        void clearAndBuild(const DataList& objects, GetBounds&& getBounds) {
            clear();
            for (const U& object : objects) {
                insert(getBounds(object), object);
            }
        }

        /**
         * Insert a node with the given bounds and data into this tree.
         *
         * @param bounds the bounds to insert
         * @param data the data to insert
         *
         * @throws NodeTreeException if a node with the given data already exists in this tree, or the bounds contains NaN
         */
        void insert(const Box& bounds, const U& data) {
            check(bounds);

            // Check that the data isn't already inserted
            if (m_leafForData.find(data) != m_leafForData.end()) {
                throw NodeTreeException("Data already in tree");
            }

            if (empty()) {
                auto* insertedLeafNode = new LeafNode(bounds, data);

                m_root = insertedLeafNode;
                m_leafForData[data] = insertedLeafNode;
            } else {
                LeafNode* insertedLeafNode;
                std::tie(m_root, insertedLeafNode) = m_root->insert(bounds, data);

                m_leafForData[data] = insertedLeafNode;
            }
        }

        /**
         * Removes the node with the given data from this tree.
         *
         * @param data the data to remove
         * @return true if a node with the given data was removed, and false otherwise
         */
        bool remove(const U& data) {
            auto it = m_leafForData.find(data);
            if (it == m_leafForData.end()) {
                return false;
            }

            LeafNode* leaf = it->second;
            assert(leaf->data() == data);
            m_leafForData.erase(it);

            m_root = leaf->deleteThis();

            return true;
        }

        /**
         * Updates the node with the given data with the given new bounds.
         *
         * @param newBounds the new bounds of the node
         * @param data the node data of the node to update
         *
         * @throws NodeTreeException if no node with the given data can be found in this tree
         */
        void update(const Box& newBounds, const U& data) {
            check(newBounds);

            if (!remove(data)) {
                throw NodeTreeException("AABB node not found");
            }
            insert(newBounds, data);
        }
    private:
        void check(const Box& bounds) const {
            if (vm::is_nan(bounds.min) || vm::is_nan(bounds.max)) {
                throw NodeTreeException("Cannot add node to AABB tree with invalid bounds");
            }
        }
    public:
        /**
         * Clears this node tree.
         */
        void clear() {
            if (!empty()) {
                delete m_root;
                m_root = nullptr;
            }
        }

        /**
         * Indicates whether this tree is empty.
         *
         * @return true if this tree is empty and false otherwise
         */
        bool empty() const {
            return m_root == nullptr;
        }

        /**
         * Returns the bounds of all nodes in this tree.
         *
         * @return the bounds of all nodes in this tree, or a bounding box made up of NaN values if this tree is empty
         */
        const Box& bounds() const {
            static constexpr auto EmptyBox = Box(vm::vec<T,S>::nan(), vm::vec<T,S>::nan());

            assert(!empty());
            if (empty()) {
                return EmptyBox;
            } else {
                return m_root->bounds();
            }
        }

        /**
         * Returns the height of this tree.
         *
         * @return the height of this tree
         */
        size_t height() const {
            return empty() ? 0 : m_root->height();
        }

        /**
         * Finds every data item in this tree whose bounding box intersects with the given ray and retuns a list of those items.
         *
         * @param ray the ray to test
         * @return a list containing all found data items
         */
        List findIntersectors(const vm::ray<T,S>& ray) const {
            List result;
            findIntersectors(ray, std::back_inserter(result));
            return result;
        }

        /**
         * Finds every data item in this tree whose bounding box intersects with the given ray and appends it to the given
         * output iterator.
         *
         * @tparam O the output iterator type
         * @param ray the ray to test
         * @param out the output iterator to append to
         */
        template <typename O>
        void findIntersectors(const vm::ray<T,S>& ray, O out) const {
            if (!empty()) {
                LambdaVisitor visitor(
                    [&](const InnerNode* innerNode) {
                        return innerNode->bounds().contains(ray.origin) || !vm::is_nan(
                            vm::intersect_ray_bbox(ray, innerNode->bounds()));
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(ray.origin) || !vm::is_nan(vm::intersect_ray_bbox(ray, leaf->bounds()))) {
                            out = leaf->data();
                            ++out;
                        }
                    }
                );
                m_root->accept(visitor);
            }
        }

        /**
         * Finds every data item in this tree whose bounding box contains the given point and returns a list of those items.
         *
         * @param point the point to test
         * @return a list containing all found data items
         */
        List findContainers(const vm::vec<T,S>& point) const {
            List result;
            findContainers(point, std::back_inserter(result));
            return result;
        }

        /**
         * Finds every data item in this tree whose bounding box contains the given point and appends it to the given
         * output iterator.
         *
         * @tparam O the output iterator type
         * @param point the point to test
         * @param out the output iterator to append to
         */
        template <typename O>
        void findContainers(const vm::vec<T,S>& point, O out) const {
            if (!empty()) {
                LambdaVisitor visitor(
                    [&](const InnerNode* innerNode) {
                        return innerNode->bounds().contains(point);
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(point)) {
                            out = leaf->data();
                            ++out;
                        }
                    }
                );
                m_root->accept(visitor);
            }
        }

        /**
         * Prints a textual representation of this tree to the given output stream.
         *
         * @param str the output stream to print to
         */
        void print(std::ostream& str) const {
            if (!empty()) {
                m_root->appendTo(str);
            }
        }
    };
}

#endif //TRENCHBROOM_AABBTREE_H
