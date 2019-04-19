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

#ifndef TRENCHBROOM_AABBTREE_H
#define TRENCHBROOM_AABBTREE_H

#include "NodeTree.h"
#include "Exceptions.h"
#include <vecmath/scalar.h>
#include <vecmath/bbox.h>
#include <vecmath/ray.h>
#include <vecmath/intersection.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <unordered_map>

/**
 * An axis aligned bounding box tree that allows for quick ray intersection queries.
 *
 * @tparam T the floating point type
 * @tparam S the number of dimensions for vector types
 * @tparam U the node data to store in the leafs
 * @tparam Cmp the comparator used to compare nodes
 */
template <typename T, size_t S, typename U, typename Cmp = std::less<U>>
class AABBTree : public NodeTree<T,S,U,Cmp> {
public:
    using List = typename NodeTree<T,S,U,Cmp>::List;
    using Box = typename NodeTree<T,S,U,Cmp>::Box;
    using DataType = typename NodeTree<T,S,U,Cmp>::DataType;
    using FloatType = typename NodeTree<T,S,U,Cmp>::FloatType;
private:
    class InnerNode;
    class LeafNode;

    class Visitor {
    public:
        virtual ~Visitor() = default;

        virtual bool visit(const InnerNode* innerNode) = 0;
        virtual void visit(const LeafNode* leaf) = 0;
    };

    class LambdaVisitor : public Visitor {
    public:
        using InnerNodeVisitor = std::function<bool(const InnerNode*)>;
        using LeafVisitor = std::function<void(const LeafNode*)>;
    private:
        const InnerNodeVisitor m_innerNodeVisitor;
        const LeafVisitor m_leafVisitor;
    public:
        LambdaVisitor(const InnerNodeVisitor& innerNodeVisitor, const LeafVisitor& leafVisitor) :
        m_innerNodeVisitor(innerNodeVisitor),
        m_leafVisitor(leafVisitor) {}

        bool visit(const InnerNode* innerNode) override { return m_innerNodeVisitor(innerNode); }
        void visit(const LeafNode* leaf)           override { m_leafVisitor(leaf); }
    };

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
         * Inserts a new node with the given parameters into the subtree of which this node is the root. Returns the new
         * root of the subtree after insertion.
         *
         * @param bounds the bounds of the data to be inserted
         * @param data the data to be inserted
         * @return the new root
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

        virtual void debugParentPointers(const Node* expectedParent) const = 0;
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
         * Children (or grandchildren etc.) changed. Update the height and bounds
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
    public: // node removal public
        /**
         * One of our direct children is being deleted. `this` will turn into a LeafNode.
         *
         * @param child
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

            // don't delete our children
            m_left = nullptr;
            m_right = nullptr;

            // special case
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

            std::pair<Node*, LeafNode*> subtreeInsertionResult = subtree->insert(bounds, data);

            // need to connect the parent pointer
            subtreeInsertionResult.first->m_parent = this;

            // subtree is either a reference to m_left or m_right
            if (subtree == m_left) {
                m_left = subtreeInsertionResult.first;
            } else {
                assert(subtree == m_right);
                m_right = subtreeInsertionResult.first;
            }

            // Update our data.
            updateBounds();
            updateHeight();

            return std::make_pair(this, subtreeInsertionResult.second);
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

        virtual void debugParentPointers(const Node* expectedParent) const override {
            assert(this->m_parent == expectedParent);
            m_left->debugParentPointers(this);
            m_left->debugParentPointers(this);
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
         * @return the newly created inner node which should replace this leaf in the parent
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

        virtual void debugParentPointers(const Node* expectedParent) const override {
            assert(this->m_parent == expectedParent);
        }
    };
private:
    Node* m_root;
    std::unordered_map<U, LeafNode*> m_leafForData;
public:
    AABBTree() : m_root(nullptr) {}

    ~AABBTree() override {
        clear();
    }

    bool contains(const U& data) const override {
        auto it = m_leafForData.find(data);
        return it != m_leafForData.end();
    }

    void insert(const Box& bounds, const U& data) override {
        check(bounds, data);

        // FIXME:
        if (empty()) {
            auto* newLeaf = new LeafNode(bounds, data);

            m_root = newLeaf;
            m_leafForData[data] = newLeaf;
        } else {
            const std::pair<Node*, LeafNode*> newRootAndLeaf = m_root->insert(bounds, data);

            m_root = newRootAndLeaf.first;
            m_leafForData[data] = newRootAndLeaf.second;
        }

        m_root->debugParentPointers(nullptr);
    }

    bool remove(const U& data) override {
        auto it = m_leafForData.find(data);
        if (it == m_leafForData.end()) {
            return false;
        }

        LeafNode* leaf = it->second;
        assert(leaf->data() == data);
        m_leafForData.erase(it);

        m_root = leaf->deleteThis();

        if (m_root != nullptr) {
            m_root->debugParentPointers(nullptr);
        }
        return true;
    }

    void update(const Box& newBounds, const U& data) override {
        check(newBounds, data);

        if (!remove(data)) {
            NodeTreeException ex;
            ex << "AABB node not found: " << data;
            throw ex;
        }
        insert(newBounds, data);
    }
private:
    void check(const Box& bounds, const U& data) const {
        if (vm::isNaN(bounds.min) || vm::isNaN(bounds.max)) {
            NodeTreeException ex;
            ex << "Cannot add node to AABB with invalid bounds [ ( " << bounds.min << " ) ( " << bounds.max << " ) ]: " << data;
            throw ex;
        }
    }
public:
    void clear() override {
        if (!empty()) {
            delete m_root;
            m_root = nullptr;
        }
    }

    bool empty() const override {
        return m_root == nullptr;
    }

    const Box& bounds() const override {
        static const auto EmptyBox = Box(vm::vec<T,S>::NaN, vm::vec<T,S>::NaN);

        assert(!empty());
        if (empty()) {
            return EmptyBox;
        } else {
            return m_root->bounds();
        }
    }

    size_t height() const override {
        return empty() ? 0 : m_root->height();
    }

    List findIntersectors(const vm::ray<T,S>& ray) const override {
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
                        return innerNode->bounds().contains(ray.origin) || !vm::isnan(
                            intersectRayAndBBox(ray, innerNode->bounds()));
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(ray.origin) || !vm::isnan(intersectRayAndBBox(ray, leaf->bounds()))) {
                            out = leaf->data();
                            ++out;
                        }
                    }
            );
            m_root->accept(visitor);
        }
    }

     List findContainers(const vm::vec<T,S>& point) const override {
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
    void print(std::ostream& str = std::cout) const {
        if (!empty()) {
            m_root->appendTo(str);
        }
    }
};

#endif //TRENCHBROOM_AABBTREE_H
