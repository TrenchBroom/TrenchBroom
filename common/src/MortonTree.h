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

#ifndef MortonTree_h
#define MortonTree_h

#include "NodeTree.h"
#include "MathUtils.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

template <typename V>
class VecCodeComputer {
public:
    using CodeType = uint64_t;
    static const size_t CodeTypeWidth = sizeof(CodeType)*8;
private:
    using T = typename V::Type;
    static const size_t S = V::Size;
    const BBox<CodeType,S> m_minMax;
public:
    VecCodeComputer(const BBox<T,S>& minMax) : m_minMax(makeIntegralMinMax(minMax)) {}
private:
    static BBox<uint64_t,S> makeIntegralMinMax(const BBox<T,S>& minMax) {
        const auto nonNeg = minMax.translated(minMax.size() / 2.0);
        return nonNeg.template makeIntegral<CodeType>();
    }
public:
    CodeType operator()(const Vec<T,S>& vec) const {
        const auto integral = vec.template makeIntegral<CodeType>();
        const auto translated = integral + m_minMax.size() / 2;
        const auto constrained = m_minMax.constrain(translated);

        return interleave(constrained);
    }
private:
    static CodeType interleave(const Vec<CodeType,S>& vec) {
        CodeType result = 0;
        for (CodeType i = 0; i < S; ++i) {
            const auto value = insertZeros(vec[i]);
            result |= (value << (static_cast<CodeType>(1) << (i-1)));
        }
        return result;
    }

    static CodeType insertZeros(const CodeType value) {
        CodeType result = 0;
        for (CodeType i = 0; i < CodeTypeWidth / S + 1; ++i) {
            const auto bit = value & (static_cast<CodeType>(1) << i);
            if (bit) {
                const auto offset = static_cast<CodeType>(S) * i;
                result |= (static_cast<CodeType>(1) << offset);
            }
        }
        return result;
    }
};

/**
 * A spacial data structure that uses morton codes to order the bounding boxes by the Z curve
 * of their center points.
 *
 * The nodes are structured as a binary tree with the following properties:
 * - The tree structure is defined by split nodes. Each split node represents a common prefix
 *   of the morton codes of all its children, i.e., all leafs of a split node have the same
 *   prefix of their morton codes. The length of this prefix is stored in the split node, and
 *   is called the split index.
 *   The split index indicates the highest bit in which the morton codes of its two subtrees
 *   differ. Thereby, the leafs in the left subtree have the bit set to 0, and the leafs in the
 *   right subtree have it set to 1.
 * - Nodes with identical morton codes are grouped under special set nodes. These nodes do not
 *   have any particular structure.
 *
 * @tparam T the floating point type
 * @tparam S the number of components of vectors
 * @tparam U the type of the data stored in this tree
 * @tparam CodeComp a functor used to compute morton codes
 * @tparam Cmp a comparator for the data
 */
template <typename T, size_t S, typename U, typename CodeComp, typename Cmp = std::less<U>>
class MortonTree : public NodeTree<T,S,U,Cmp> {
public:
    using List = typename NodeTree<T,S,U,Cmp>::List;
    using Array = typename NodeTree<T,S,U,Cmp>::Array;
    using Box = typename NodeTree<T,S,U,Cmp>::Box;
    using DataType = typename NodeTree<T,S,U,Cmp>::DataType;
    using FloatType = typename NodeTree<T,S,U,Cmp>::FloatType;
    using GetBounds = typename NodeTree<T,S,U,Cmp>::GetBounds;
    using CodeType = typename CodeComp::CodeType;
    static const size_t CodeTypeWidth = CodeComp::CodeTypeWidth;
private:
    CodeComp computeMortonCode;

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
        void visit(const LeafNode* leaf)       override { m_leafVisitor(leaf); }
    };

    /**
     * Base class for any node in this tree.
     */
    class Node {
    protected:
        Box m_bounds;
    public:
        Node(const Box& bounds) : m_bounds(bounds) {}
        virtual ~Node() {}

        const Box& bounds() const {
            return m_bounds;
        }

        /**
         * Inserts a new node with the given parameters into the subtree of which this node is the root. Returns the new
         * root of the subtree after insertion.
         *
         * @param bounds the bounds of the data to be inserted
         * @param code the code of the data to be inserted
         * @param data the data to be inserted
         * @param parentIndex the split index of the parent node
         * @return the new root
         */
        virtual InnerNode* insert(const Box& bounds, const CodeType code, const U& data, const size_t parentIndex = CodeTypeWidth) = 0;

        /**
         * Attempts to remove the node with the given bounds, code, and data from this subtree.
         *
         * @param bounds the bounds of the node to remove
         * @param code the code of the node to remove
         * @param data the data of the node to remove
         * @return a pair of the replacement node of this node and a boolean indicating whether the node to remove was found in this subtree
         */
        virtual std::tuple<Node*, bool> remove(const Box& bounds, const CodeType code, const U& data) = 0;

        /**
         * Find a leaf containing the given bounds and data in this subtree.
         *
         * @param bounds the bounds to find
         * @param code the code to find
         * @param data the data to find
         * @return the leaf that contains the given bounds and data
         */
        virtual LeafNode* findLeaf(const Box& bounds, const CodeType code, const U& data) = 0;

        /**
         * Accepts the given visitor.
         *
         * @param visitor the visitor to accept
         */
        virtual void accept(Visitor& visitor) const = 0;

        /**
         * Checks the split index of the subtree starting at this node.
         *
         * @return true if the split index is correct and false otherwise
         */
        virtual bool checkSplitIndex() const = 0;

        /**
         * Checks the split index of the subtree starting at this node against the given parent index.
         *
         * @param parentIndex the split index of the parent node
         * @return true if the split index is correct and false otherwise
         */
        virtual bool doCheckSplitIndex(const size_t parentIndex) const = 0;

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
        virtual void appendTo(std::ostream& str, const std::string& indent, const size_t level) const = 0;
    protected:
        /**
         * Appends a textual representation of this node's bounds to the given output stream.
         *
         * @param str the stream to append to
         */
        void appendBounds(std::ostream& str) const {
            str << "[ (";
            m_bounds.min.write(str);
            str << ") (";
            m_bounds.max.write(str);
            str << ") ]";
        }
    };

    /**
     * Base class for all inner nodes of this tree. There are two types of inner nodes: split nodes
     * and set nodes. Split nodes structure the leafs according to their split index, while set nodes
     * simply group leafs with identical codes.
     */
    class InnerNode : public Node {
    private:
        using Node::m_bounds;
    protected:
        /**
         * The value of the identical bit prefix of all nodes in this subtree. Non-identical bits are 0.
         */
        CodeType m_identicalPrefix;
    protected:
        /**
         * Creates a new inner node with the given bounds and split index.
         *
         * @param bounds the bounds of this inner node
         * @param identicalPrefix the value of the identical bit prefix of all nodes in this subtree
         */
        InnerNode(const Box& bounds, const CodeType identicalPrefix) :
        Node(bounds),
        m_identicalPrefix(identicalPrefix) {}

        /**
         * Creates a new split node which has this node and a new leaf that represents the given data as children.
         *
         * @param bounds the bounds of the new leaf node
         * @param code the code of the new leaf node
         * @param data the data of the new leaf node
         * @param splitIndex the split index of the new split node
         * @return the new split node
         */
        InnerNode* insertSibling(const Box& bounds, const CodeType code, const U& data, const size_t splitIndex) {
            auto* leaf = new LeafNode(bounds, code, data);
            const auto identicalPrefix = Math::bitPrefix(code, splitIndex + 1);

            if (!Math::testBit(code, splitIndex)) {
                return new SplitNode(leaf, this, splitIndex, identicalPrefix);
            } else {
                return new SplitNode(this, leaf, splitIndex, identicalPrefix);
            }
        }

        /**
         * Checks whether the given bounds possibly contribute to this inner node's bounds.
         *
         * @param bounds the bounds to check, expected to be contained in this inner node's bounds
         * @return true if the given bounds possibly contribute to this inner node's bounds
         */
        bool contributesToBounds(const Box& bounds) const {
            assert(m_bounds.contains(bounds));

            for (size_t i = 0; i < S; ++i) {
                if (bounds.min[i] == m_bounds.min[i] || bounds.max[i] == m_bounds.max[i]) {
                    return true;
                }
            }
            return false;
        }
    public:
        virtual ~InnerNode() {}
    };

    /**
     * Split nodes structure the tree into a binary search tree.
     */
    class SplitNode : public InnerNode {
    private:
        Node* m_left;
        Node* m_right;
        size_t m_splitIndex;

        using Node::m_bounds;
        using Node::appendBounds;
        using InnerNode::m_identicalPrefix;
        using InnerNode::insertSibling;
        using InnerNode::contributesToBounds;
    public:
        /**
         * Creates a new inner node with the given left and right children and split index.
         *
         * @param left the left subtree, must not be null
         * @param right the right subtree, must not be null
         * @param splitIndex the split index, must not be greater than the number of bits of the CodeType
         * @param identicalPrefix  the value of the identical bit prefix of all nodes in this subtree
         */
        SplitNode(Node* left, Node* right, const size_t splitIndex, const CodeType identicalPrefix) :
        InnerNode(left->bounds().mergedWith(right->bounds()), identicalPrefix),
        m_left(left),
        m_right(right),
        m_splitIndex(splitIndex) {
            assert(m_left != nullptr);
            assert(m_right != nullptr);
        }

        /**
         * Creates a new inner node with the given left and right leafs and split index.
         *
         * Has the same effect as the other constructor, but performs an additional assertion.
         */
        SplitNode(LeafNode* left, LeafNode* right, const size_t splitIndex, const CodeType identicalPrefix) :
        SplitNode(static_cast<Node*>(left), static_cast<Node*>(right), splitIndex, identicalPrefix) {
            // If this is a split node, then the bit at the give split index must be unset for the left leaf
            // and set for the right leaf.
            assert(!Math::testBit( left->code(), splitIndex) &&
                    Math::testBit(right->code(), splitIndex));
        }

        ~SplitNode() override {
            delete m_left;
            m_left = nullptr;
            delete m_right;
            m_right = nullptr;
        }

        InnerNode* insert(const Box& bounds, const CodeType code, const U& data, const size_t parentIndex) override {
            // check whether this node needs a new parent
            if (parentIndex > m_splitIndex + 1) {
                // there is a gap between this node and its parent
                // we need to check all bits at the indices between the parent index and the index of this node
                const auto gapSplitIndex = Math::findHighestDifferingBit(code, m_identicalPrefix, parentIndex - 1);
                if (gapSplitIndex != CodeTypeWidth && gapSplitIndex > m_splitIndex) {
                    return insertSibling(bounds, code, data, gapSplitIndex);
                }
            }

            // test whether the bit at which this node splits the range of its subtree
            // is set or not
            if (!Math::testBit(code, m_splitIndex)) {
                m_left = m_left->insert(bounds, code, data);
            } else {
                m_right = m_right->insert(bounds, code, data);
            }

            m_bounds = m_left->bounds().mergedWith(m_right->bounds());

            return this;
        }

        std::tuple<Node*, bool> remove(const Box& bounds, const CodeType code, const U& data) override {
            // test whether the bit at which this node splits the range of its subtree
            // is set or not
            if (!Math::testBit(code, m_splitIndex)) {
                return doRemove(m_left, m_right, bounds, code, data);
            } else {
                return doRemove(m_right, m_left, bounds, code, data);
            }
        }
    private:
        std::tuple<Node*, bool> doRemove(Node*& child, Node*& other, const Box& bounds, const CodeType code, const U& data) {
            Node* newChild;
            bool result;
            std::tie(newChild, result) = child->remove(bounds, code, data);

            if (result) {
                // node was found
                if (newChild == nullptr) {
                    // child was a leaf and will be deleted when parent deletes me
                    auto* survivor = other;
                    other = nullptr; // prevent survivor from getting deleted by my destructor
                    
                    // parent must delete me
                    return std::make_tuple(survivor, result);
                } else {
                    if (newChild != child) {
                        // child was an inner node and needs to replaced
                        delete child;
                        child = newChild;
                    }
                    
                    // update my bounds if necessary
                    if (contributesToBounds(bounds)) {
                        m_bounds = child->bounds().mergedWith(other->bounds());
                    }

                    // the structure was adapted and the changes are contained in child's subtree
                    return std::make_tuple(this, result);
                }
            } else {
                // node was not found
                return std::make_tuple(this, result);
            }
        }
    public:
        LeafNode* findLeaf(const Box& bounds, const CodeType code, const U& data) override {
            // test whether the bit at which this node splits the range of its subtree
            // is set or not
            if (!Math::testBit(code, m_splitIndex)) {
                return m_left->findLeaf(bounds, code, data);
            } else {
                return m_right->findLeaf(bounds, code, data);
            }
        }

        bool checkSplitIndex() const override {
            return m_left->doCheckSplitIndex(m_splitIndex) && m_right->doCheckSplitIndex(m_splitIndex);
        }

        bool doCheckSplitIndex(const size_t parentIndex) const override {
            if (m_splitIndex >= parentIndex) {
                return false;
            } else {
                return m_left->doCheckSplitIndex(m_splitIndex) && m_right->doCheckSplitIndex(m_splitIndex);
            }
        }

        void accept(Visitor& visitor) const override {
            if (visitor.visit(this)) {
                m_left->accept(visitor);
                m_right->accept(visitor);
            }
        }

        void appendTo(std::ostream& str, const std::string& indent, const size_t level) const override {
            for (size_t i = 0; i < level; ++i) {
                str << indent;
            }

            str << "X ";
            appendBounds(str);
            str << std::endl;

            m_left->appendTo(str, indent, level + 1);
            m_right->appendTo(str, indent, level + 1);
        }
    };

    /**
     * Set nodes group nodes with identical morton codes, since those cannot be structured any further by
     * split nodes.
     */
    class SetNode : public InnerNode {
    private:
        std::list<LeafNode*> m_children;

        using Node::m_bounds;
        using Node::appendBounds;
        using InnerNode::m_identicalPrefix;
        using InnerNode::insertSibling;
        using InnerNode::contributesToBounds;
    public:
        /**
         * Creates a new set node that contains the given nodes.
         *
         * @param cur iterator to the first leaf to insert
         * @param end iterator past the last leaf to insert
         * @param identicalPrefix the values of the bits in the gap between the parents' split index and this node's split index
         */
        template <typename I>
        SetNode(I cur, I end, const CodeType identicalPrefix) :
        InnerNode(mergeBounds(cur, end, [](const auto* node){ return node->bounds(); }), identicalPrefix),
        m_children(cur, end) {}

        /**
         * Creates a new set node that contains the two nodes.
         *
         * @param child1 the first node to insert
         * @param child2 the second node to insert
         * @param identicalPrefix the values of the bits in the gap between the parents' split index and this node's split index
         */
        SetNode(LeafNode* child1, LeafNode* child2, const CodeType identicalPrefix) :
        InnerNode(child1->bounds().mergedWith(child2->bounds()), identicalPrefix),
        m_children( { child1, child2 } ){}

        ~SetNode() override {
            for (LeafNode* leaf : m_children) {
                delete leaf;
            }
            m_children.clear();
        }

        InnerNode* insert(const Box& bounds, const CodeType code, const U& data, const size_t parentIndex) override {
            // check whether this node needs a new parent
            if (parentIndex > 1) {
                // there is a gap between this node and its parent
                // we need to check all bits at the indices between the parent index and the index of this node
                const auto gapSplitIndex = Math::findHighestDifferingBit(code, m_identicalPrefix, parentIndex - 1);
                if (gapSplitIndex != CodeTypeWidth) {
                    return insertSibling(bounds, code, data, gapSplitIndex);
                }
            }

            m_children.push_back(new LeafNode(bounds, code, data));
            m_bounds.mergeWith(bounds);
            return this;
        }

        std::tuple<Node*, bool> remove(const Box& bounds, const CodeType code, const U& data) override {
            if (code != m_identicalPrefix) {
                return std::make_tuple(this, false);
            } else {
                const auto it = std::find_if(std::begin(m_children), std::end(m_children), [&](LeafNode* leaf) {
                    Node* newChild;
                    bool result;
                    std::tie(newChild, result) = leaf->remove(bounds, code, data);
                    if (newChild == nullptr) {
                        assert(result);
                        delete leaf;
                        return true;
                    } else {
                        return false;
                    }
                });
                if (it == std::end(m_children)) {
                    return std::make_tuple(this, false);
                } else {
                    m_children.erase(it);
                    if (m_children.size() == 1) {
                        auto* leaf = m_children.front();
                        m_children.clear();
                        return std::make_tuple(leaf, true);
                    } else {
                        if (contributesToBounds(bounds)) {
                            m_bounds = mergeBounds(std::begin(m_children), std::end(m_children), [](const auto* node){ return node->bounds(); });
                        }
                        return std::make_tuple(this, true);
                    }
                }
            }
        }

        LeafNode* findLeaf(const Box& bounds, const CodeType code, const U& data) override {
            LeafNode* result = nullptr;
            for (auto* leaf : m_children) {
                if (leaf->data() == data) {
                    return leaf;
                }
            }
            return result;
        }

        bool checkSplitIndex() const override {
            return true;
        }

        bool doCheckSplitIndex(const size_t parentIndex) const override {
            return true;
        }

        void accept(Visitor& visitor) const override {
            if (visitor.visit(this)) {
                for (const auto* leaf : m_children) {
                    visitor.visit(leaf);
                }
            }
        }

        void appendTo(std::ostream& str, const std::string& indent, const size_t level) const override {
            for (size_t i = 0; i < level; ++i) {
                str << indent;
            }

            str << "S ";
            appendBounds(str);
            str << std::endl;

            for (const auto* leaf : m_children) {
                leaf->appendTo(str, indent, level + 1);
            }
        }
    };

    /**
     * A coded node has a morton code. This class has been extracted for reuse as a parameter
     * in standard algorithms, e.g. std::lower_bound.
     */
    class CodedNode {
    private:
        CodeType m_code;
    public:
        CodedNode(const CodeType code) : m_code(code) {}

        CodeType code() const {
            return m_code;
        }
    };

    /**
     * Leaf nodes contain the actual data stored in the tree.
     */
    class LeafNode : public Node, public CodedNode {
    private:
        U m_data;

        using Node::appendBounds;
    public:
        LeafNode(const Box& bounds, const CodeType& code, const U& data) :
        Node(bounds),
        CodedNode(code),
        m_data(data) {}

        /**
         * Returns the data associated with this leaf.
         *
         * @return the data
         */
        U& data() {
            return m_data;
        }

        /**
         * Returns the data associated with this leaf.
         *
         * @return the data
         */
        const U& data() const {
            return m_data;
        }

        InnerNode* insert(const Box& bounds, const CodeType code, const U& data, const size_t parentIndex) override {
            InnerNode* result = nullptr;
            auto* newLeaf = new LeafNode(bounds, code, data);

            const auto splitIndex = Math::findHighestDifferingBit(this->code(), newLeaf->code(), parentIndex - 1);

            if (splitIndex == CodeTypeWidth) {
                // both leafs (this and newLeaf) have the same code
                result = new SetNode(this, newLeaf, code);
            } else {
                const auto identicalPrefix = Math::bitPrefix(code, splitIndex + 1);
                if (!Math::testBit(code, splitIndex)) {
                    result = new SplitNode(newLeaf, this, splitIndex, identicalPrefix);
                } else {
                    result = new SplitNode(this, newLeaf, splitIndex, identicalPrefix);
                }
            }
            return result;
        }

        std::tuple<Node*, bool> remove(const Box& bounds, const CodeType code, const U& data) override {
            if (data == m_data) {
                return std::make_tuple(nullptr, true);
            } else {
                return std::make_tuple(this, false);
            }
        }

        LeafNode* findLeaf(const Box& bounds, const CodeType code, const U& data) override {
            if (m_data == data) {
                assert(bounds == this->bounds());
                return this;
            } else {
                return nullptr;
            }
        }

        void accept(Visitor& visitor) const override {
            visitor.visit(this);
        }

        bool checkSplitIndex() const override {
            return true;
        }

        bool doCheckSplitIndex(const size_t parentIndexLeft) const override {
            return true;
        }

        void appendTo(std::ostream& str, const std::string& indent, const size_t level) const override {
            for (size_t i = 0; i < level; ++i) {
                str << indent;
            }

            str << "L ";
            appendBounds(str);
            str << ": " << m_data << std::endl;
        }
    };

private:
    Node* m_root;
public:
    MortonTree(const CodeComp& codeComp) : computeMortonCode(codeComp), m_root(nullptr) {}

    ~MortonTree() override {
        clear();
    }

    bool contains(const Box& bounds, const U& data) const override {
        if (empty()) {
            return false;
        } else {
            const auto code = computeMortonCode(bounds.center());
            return m_root->findLeaf(bounds, code, data) != nullptr;
        }
    }

    void clearAndBuild(const List& objects, const GetBounds& getBounds) override {
        clear();
        build(std::begin(objects), std::end(objects), objects.size(), getBounds);
    }

    void clearAndBuild(const Array& objects, const GetBounds& getBounds) override {
        clear();
        build(std::begin(objects), std::end(objects), objects.size(), getBounds);
    }
private:
    /**
     * Builds the tree by creating leaf nodes for all objects, sorting the leaf nodes according to their
     * morton codes, and building the tree on top of this sorted array.
     *
     * @param cur the start of the range to insert
     * @param end the end of the range to insert
     * count the number of objects to insert
     * @param getBounds a function to obtain the bounds from each object
     */
    template <typename I>
    void build(I cur, I end, const size_t count, const GetBounds& getBounds) {
        assert(empty());

        if (cur != end) {
            // initialize the list of leafs
            using LeafList = std::vector<LeafNode*>;
            LeafList leafList;
            leafList.reserve(count);

            while (cur != end) {
                auto& object = *cur;
                const auto bounds = getBounds(object);
                const auto code = computeMortonCode(bounds.center());
                leafList.push_back(new LeafNode(bounds, code, object));
                ++cur;
            }

            // sort the nodes by their morton codes
            std::sort(std::begin(leafList), std::end(leafList),
                      [](const CodedNode* lhs, const CodedNode* rhs) { return lhs->code() < rhs->code(); });

            // recursively build the tree
            m_root = buildTree(std::begin(leafList), std::end(leafList), CodeTypeWidth);
            assert(check());
        }
    }

    /**
     * Recursively builds a tree from the given range of leaf nodes. The given parent index is
     * the split index of the parent node of this node.
     *
     * @tparam I the type of the range interators
     * @param first the start of the range of leafs
     * @param end the end of the range of leafs
     * @param parentIndex the split index of the parent node
     * @return the newly created tree
     */
    template <typename I>
    Node* buildTree(I first, I end, const size_t parentIndex) {
        auto* firstNode = *first;

        // if the range contains only one item, return that
        const auto last = std::prev(end);
        if (first == last) {
            return firstNode;
        }

        auto* lastNode = *last;

        // the highest bit in which first and last differ
        const auto splitIndex = Math::findHighestDifferingBit(firstNode->code(), lastNode->code(), parentIndex - 1);
        if (splitIndex == CodeTypeWidth) {
            // all nodes in the range have identical codes
            return new SetNode(first, end, firstNode->code());
        } else {
            // find midpoint for splitting the range
            const auto testValue = Math::bitMask<CodeType>(splitIndex);
            const auto testMask = (testValue - 1) | testValue; // has all bits greater than splitIndexRight unset and all others set, e.g. 0000011111 if splitIndexRight = 5;
            const CodedNode testNode(testValue);
            const auto midPoint = std::lower_bound(first, end, &testNode, [&testMask](const CodedNode* lhs, const CodedNode* rhs){ return (lhs->code() & testMask) < (rhs->code() & testMask); });

            // recursively create the two subtrees
            auto* leftNode  = buildTree(first, midPoint, splitIndex);
            auto* rightNode = buildTree(midPoint, end,   splitIndex);

            // compute the identical prefix of all nodes in the new tree
            const auto identicalPrefix = Math::bitPrefix(firstNode->code(), splitIndex + 1);
            return new SplitNode(leftNode, rightNode, splitIndex, identicalPrefix);
        }
    }
public:
    void insert(const Box& bounds, const U& data) override {
        const auto code = computeMortonCode(bounds.center());
        if (empty()) {
            m_root = new LeafNode(bounds, code, data);
        } else {
            m_root = m_root->insert(bounds, code, data);
        }
        assert(check());
    }

    bool remove(const Box& bounds, const U& data) override {
        assert(check());
        if (empty()) {
            return false;
        } else {
            const auto code = computeMortonCode(bounds.center());

            Node* newRoot;
            bool result;
            std::tie(newRoot, result) = m_root->remove(bounds, code, data);
            if (newRoot != m_root) {
                delete m_root;
                m_root = newRoot;
            }
            return result;
        }
    }

    void update(const Box& oldBounds, const Box& newBounds, const U& data) override {
        if (!remove(oldBounds, data)) {
            NodeTreeException ex;
            ex << "Node not found with oldBounds [ (" << oldBounds.min.asString(S) << ") (" << oldBounds.max.asString(S) << ") ]: " << data;
            throw ex;
        }
        insert(newBounds, data);
    }

    void clear() override {
        delete m_root;
        m_root = nullptr;
    }

    bool empty() const override {
        return m_root == nullptr;
    }

    const Box& bounds() const override {
        assert(!empty());
        return m_root->bounds();
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
                        return innerNode->bounds().contains(ray.origin) || !Math::isnan(innerNode->bounds().intersectWithRay(ray));
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(ray.origin) || !Math::isnan(leaf->bounds().intersectWithRay(ray))) {
                            out = leaf->data();
                        }
                    }
            );
            m_root->accept(visitor);
        }
    }

    List findIntersectors(const Ray<T,S>& ray) const override {
        List result;
        findIntersectors(ray, std::back_inserter(result));
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
    void findContainers(const Vec<T,S>& point, O out) const {
        if (!empty()) {
            LambdaVisitor visitor(
                    [&](const InnerNode* innerNode) {
                        return innerNode->bounds().contains(point);
                    },
                    [&](const LeafNode* leaf) {
                        if (leaf->bounds().contains(point)) {
                            out = leaf->data();
                        }
                    }
            );
            m_root->accept(visitor);
        }
    }

    List findContainers(const Vec<T,S>& point) const override {
        List result;
        findContainers(point, std::back_inserter(result));
        return std::move(result);
    }

    bool check() const {
        if (empty()) {
            return true;
        } else {
            return checkSplitIndex() && checkUniqueData();
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
private:
    bool checkSplitIndex() const {
        assert(!empty());
        return m_root->checkSplitIndex();
    }
    
    bool checkUniqueData() const {
        assert(!empty());
        
        std::set<U> dataSet;
        bool unique = true;
        
        LambdaVisitor visitor(
            [&](const InnerNode* innerNode) {
              return true;
            },
            [&](const LeafNode* leaf) {
                unique &= dataSet.insert(leaf->data()).second;
            }
        );
        
        m_root->accept(visitor);
        return unique;
    }
};

#endif /* MortonTree_h */
