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
#include "MathUtils.h"
#include "bbox_decl.h"
#include "bbox_impl.h"
#include "Ray.h"
#include "intersection.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <memory>

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
    private:
        Box m_bounds;
    protected:
        explicit Node(const Box& bounds) : m_bounds(bounds) {}
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
         * Checks whether the given bounds are equal to this node's bounds. The bounds are compared
         * using the equality operator ==.
         *
         * @param bounds the bounds to check
         * @return true if the given bounds are equal to this node's bounds
         */
        bool hasBounds(const Box& bounds) const {
            return bounds == this->bounds();
        }

        /**
         * Indicates whether this node is a leaf.
         *
         * @return true if this node is a leaf and false otherwise
         */
        virtual bool leaf() const = 0;

        /**
         * Return the height of this node. A leaf always has a height of 1, and an inner node has a height equal to the
         * maximum of the heights of its children plus one.
         *
         * @return the height
         */
        virtual size_t height() const = 0;

        /**
         * Find a leaf containing the given bounds and data in this subtree.
         *
         * @param bounds the bounds to find
         * @param data the data to find
         * @return the leaf that contains the given bounds and data
         */
        virtual const LeafNode* find(const Box& bounds, const U& data) const = 0;

        /**
         * Inserts a new node with the given parameters into the subtree of which this node is the root. Returns the new
         * root of the subtree after insertion.
         *
         * @param bounds the bounds of the data to be inserted
         * @param data the data to be inserted
         * @return the new root
         */
        virtual Node* insert(const Box& bounds, const U& data) = 0;

        /**
         * Removes the node with the given parameters from the subtree of which this node is the root. Returns the new
         * root of the subtree after removal.
         *
         * @param bounds the bounds of the node to be removed
         * @param data the data associated with the node to be removed
         * @return a pair containing the new root and a boolean indicating whether or not a node was removed
         */
        virtual std::pair<Node*, bool> remove(const Box& bounds, const U& data) = 0;

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
            updateHeight();
        }

        ~InnerNode() override {
            delete m_left;
            delete m_right;
        }

        bool leaf() const override {
            return false;
        }

        size_t height() const override {
            return m_height;
        }

        const LeafNode* find(const Box& bounds, const U& data) const override {
            const LeafNode* result = nullptr;
            if (this->bounds().contains(bounds)) {
                result = m_left->find(bounds, data);
                if (result == nullptr) {
                    result = m_right->find(bounds, data);
                }
            }
            return result;
        }

        Node* insert(const Box& bounds, const U& data) override {
            // Select the subtree which is increased the least by inserting a node with the given bounds.
            // Then insert the node into that subtree and update our reference to it.
            auto*& subtree = selectLeastIncreaser(m_left, m_right, bounds);
            subtree = subtree->insert(bounds, data);

            // Update our data.
            updateBounds();
            updateHeight();

            return this;
        }

        std::pair<Node*, bool> remove(const Box& bounds, const U& data) override {
            const auto& [node, result] = doRemove(bounds, data, m_left, m_right);
            if (result) {
                return std::make_pair(node, result);
            } else {
                return doRemove(bounds, data, m_right, m_left);
            }
        }
    private:
        /**
         * Attempt to remove the node with the given bounds and data from the given child.
         *
         * @param bounds the bounds of the node to remove
         * @param data the data of the node to remove
         * @param child the child to remove the node from
         * @param sibling the sibling of the given child in this inner node
         * @return a pair of the node that should replace the given child in this inner node, or nullptr if the node to remove is
         * not a descendant of the given child, and a boolean indicating the node to remove was found in the given subtree
         */
        std::pair<Node*, bool> doRemove(const Box& bounds, const U& data, Node*& child, Node*& sibling) {
            if (child->bounds().contains(bounds)) {
                const auto&[newChild, result] = child->remove(bounds, data);
                if (result) {
                    if (newChild == nullptr) {
                        // child is a leaf, and it represents the node to remove; return sibling to the caller
                        auto *newChild = sibling;
                        // prevent the sibling to get deleted when this node gets deleted by the parent
                        sibling = nullptr;
                        // child will be deleted when this node gets deleted by the caller
                        return std::make_pair(newChild, true);
                    } else {
                        // child is an inner node
                        if (newChild != child) {
                            // the node to be removed was deleted from child's subtree, and we need to update our pointer
                            // with the new root of that subtree
                            delete child;
                            child = newChild;
                        }

                        // Update our data.
                        updateBounds();
                        updateHeight();

                        return std::make_pair(this, true);
                    }
                }
            }
            // the node to be removed was not found in the subtree
            return std::make_pair(nullptr, false);
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
            const auto new1 = merge(node1->bounds(), bounds);
            const auto new2 = merge(node2->bounds(), bounds);
            const auto vol1 = node1->bounds().volume();
            const auto vol2 = node2->bounds().volume();
            const auto diff1 = new1.volume() - vol1;
            const auto diff2 = new2.volume() - vol2;

            if (diff1 < diff2) {
                return node1;
            } else if (diff2 < diff1) {
                return node2;
            } else if (vol1 < vol2) {
                return node1;
            } else if (vol2 < vol1) {
                return node2;
            } else if (node1->height() < node2->height()) {
                return node1;
            } else if (node2->height() < node1->height()) {
                return node2;
            } else {
                // I give up!
                return node1;
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

        bool leaf() const override {
            return true;
        }

        size_t height() const override {
            return 1;
        }

        const LeafNode* find(const Box& bounds, const U& data) const override {
            if (this->hasBounds(bounds) && hasData(data)) {
                return this;
            } else {
                return nullptr;
            }
        }
        /**
         * Returns a new inner node that has this leaf as its left child and a new leaf representing the given bounds
         * and data as its right child.
         *
         * @param bounds the bounds to insert
         * @param data the data to insert
         * @return the newly created inner node which should replace this leaf in the parent
         */
        Node* insert(const Box& bounds, const U& data) override {
            return new InnerNode(this, new LeafNode(bounds, data));
        }

        /**
         * Tests whether this node equals the given data. If this node is a match, then this method returns a pair
         * of nullptr and true, otherwise it returns a pair this and false.
         *
         * @param bounds the bounds to remove
         * @param data the data to remove
         * @return a pair indicating whether this node was a match
         */
        std::pair<Node*, bool> remove(const Box& bounds, const U& data) override {
            if (hasData(data)) {
                return std::make_pair(nullptr, true);
            } else {
                return std::make_pair(this, false);
            }
        }

        /**
         * Checks whether the given data equals the data of this leaf. The given data is considered
         * equal to this node's data if and only if !(data < m_data) && !(m_data < data) where < is
         * implemented by the comparison operator Cmp.
         *
         * @param data the data to check
         * @return true if the given data is equal to the data of this leaf
         */
        bool hasData(const U& data) const {
            static const Cmp cmp;
            return !cmp(data, m_data) && !cmp(m_data, data);
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
    };
private:
    Node* m_root;
public:
    AABBTree() : m_root(nullptr) {}

    ~AABBTree() override {
        clear();
    }

    bool contains(const Box& bounds, const U& data) const override {
        return (!empty() && m_root->find(bounds, data) != nullptr);
    }

    void insert(const Box& bounds, const U& data) override {
        if (empty()) {
            m_root = new LeafNode(bounds, data);
        } else {
            m_root = m_root->insert(bounds, data);
        }
    }

    bool remove(const Box& bounds, const U& data) override {
        if (!empty() && m_root->bounds().contains(bounds)) {
            const auto& [newRoot, result] = m_root->remove(bounds, data);
            if (result) {
                if (newRoot != m_root) {
                    delete m_root;
                    m_root = newRoot;
                }
                return true;
            }
        }
        return false;
    }

    void update(const Box& oldBounds, const Box& newBounds, const U& data) override {
        if (!remove(oldBounds, data)) {
            NodeTreeException ex;
            ex << "AABB node not found with oldBounds [ ( " << oldBounds.min << " ) ( " << oldBounds.max << " ) ]: " << data;
            throw ex;
        }
        insert(newBounds, data);
    }

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
        static const auto EmptyBox = Box(vec<T,S>::NaN, vec<T,S>::NaN);

        assert(!empty());
        if (empty()) {
            return EmptyBox;
        } else {
            return m_root->bounds();
        }
    }

    List findIntersectors(const Ray<T,S>& ray) const override {
        List result;
        findIntersectors(ray, std::back_inserter(result));
        return std::move(result);
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
    void findIntersectors(const Ray<T,S>& ray, O out) const {
        if (!empty()) {
            LambdaVisitor visitor(
                    [&](const InnerNode* innerNode) {
                        return innerNode->bounds().contains(ray.origin) || !Math::isnan(intersect(ray, innerNode->bounds()));
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(ray.origin) || !Math::isnan(intersect(ray, leaf->bounds()))) {
                            out = leaf->data();
                            ++out;
                        }
                    }
            );
            m_root->accept(visitor);
        }
    }

     List findContainers(const vec<T,S>& point) const override {
         List result;
         findContainers(point, std::back_inserter(result));
         return std::move(result);
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
    void findContainers(const vec<T,S>& point, O out) const {
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
