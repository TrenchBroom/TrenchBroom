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
    };

    class InnerNode : public Node {
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
    public:
        virtual ~InnerNode() {}
    };

    class SplitNode : public InnerNode {
    private:
        using InnerNode::m_identicalPrefix;
        Node* m_left;
        Node* m_right;
        size_t m_splitIndex;

        using InnerNode::insertSibling;
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
                m_left = m_left->insert(bounds, data, code);
            } else {
                m_right = m_right->insert(bounds, data, code);
            }

            return this;
        }

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
    };

    class SetNode : public InnerNode {
    private:
        using Node::m_bounds;
        using InnerNode::m_identicalPrefix;
        std::list<LeafNode*> m_children;

        using InnerNode::insertSibling;
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
    public:
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
    };

    class CodedNode {
    private:
        CodeType m_code;
    public:
        CodedNode(const CodeType code) : m_code(code) {}

        CodeType code() const {
            return m_code;
        }
    };

    class LeafNode : public Node, public CodedNode {
    private:
        U m_data;
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
        build(objects, getBounds);
    }
private:
    void build(const List& objects, const GetBounds& getBounds) {
        assert(empty());

        // initialize the list of leafs
        using LeafList = std::vector<LeafNode*>;
        LeafList leafList;
        leafList.reserve(objects.size());

        for (const auto& object : objects) {
            const auto& bounds = getBounds(object);
            const auto code = computeMortonCode(bounds.center());
            leafList.push_back(new LeafNode(bounds, code, object));
        }

        // sort the nodes by their morton codes
        std::sort(std::begin(leafList), std::end(leafList),
                  [](const CodedNode* lhs, const CodedNode* rhs) { return lhs->code() < rhs->code(); });

        // recursively build the tree
        m_root = buildTree(std::begin(leafList), std::end(leafList), CodeTypeWidth);
        assert(check());
    }

    template <typename I>
    Node* buildTree(I first, I end, const size_t parentIndex) {
        auto* firstNode = *first;

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

            auto* leftNode  = buildTree(first, midPoint, splitIndex);
            auto* rightNode = buildTree(midPoint, end,   splitIndex);

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
    }

    void update(const Box& oldBounds, const Box& newBounds, const U& data) override {
        assert(check());
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
            return checkSplitIndex();
        }
    }
private:
    bool checkSplitIndex() const {
        assert(!empty());
        return m_root->checkSplitIndex();
    }
};

#endif /* MortonTree_h */
