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
private:
    using CodeType = typename CodeComp::CodeType;
    static const size_t CodeTypeWidth = CodeComp::CodeTypeWidth;
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
        void visit(const LeafNode* leaf)           override { m_leafVisitor(leaf); }
    };

    class Node {
    private:
        Box m_bounds;
    public:
        Node(const Box& bounds) : m_bounds(bounds) {}
        virtual ~Node() {}

        const Box& bounds() const {
            return m_bounds;
        }

        /**
         * Find a leaf containing the given bounds and data in this subtree.
         *
         * @param bounds the bounds to find
         * @param data the data to find
         * @param code the code to find
         * @return the leaf that contains the given bounds and data
         */
        virtual LeafNode* findLeaf(const Box& bounds, const U& data, const CodeType code) = 0;

        /**
         * Accepts the given visitor.
         *
         * @param visitor the visitor to accept
         */
        virtual void accept(Visitor& visitor) const = 0;
    };

    class InnerNode : public Node {
    private:
        Node* m_left;
        Node* m_right;
        CodeType m_splitIndex;
    public:
        /**
         * Creates a new inner node with the given left and right children and split index.
         *
         * The split index indicates the index of the highest bit at which the codes of the
         * two given subtrees differ. A split index which is equal to the number of bits of
         * the CodeType indicates that the codes of the two subtrees do not differ at all.
         *
         *
         * @param left the left subtree, must not be null
         * @param right the right subtree, must not be null
         * @param splitIndex the split index, must not be greater than the number of bits of the CodeType
         */
        InnerNode(Node* left, Node* right, const CodeType splitIndex) :
        Node(left->bounds().mergedWith(right->bounds())),
        m_left(left),
        m_right(right),
        m_splitIndex(splitIndex) {
            assert(m_left != nullptr);
            assert(m_right != nullptr);
        }

        ~InnerNode() override {
            delete m_left;
            m_left = nullptr;
            delete m_right;
            m_right = nullptr;
        }

        LeafNode* findLeaf(const Box& bounds, const U& data, const CodeType code) override {
            LeafNode* result = nullptr;
            if (isSplitNode()) {
                // test whether the bit at which this node splits the range of its subtree
                // is set or not, then
                const bool isSet = (code & (bitMask(reverseIndex(m_splitIndex))));
                if (!isSet) {
                    result = m_left->findLeaf(bounds, data, code);
                } else {
                    result =m_right->findLeaf(bounds, data, code);
                }
            } else {
                // we cannot use the morton codes to direct the search, but we can use the
                // bounding boxes
                if (m_left->bounds().contains(bounds)) {
                    result = m_left->findLeaf(bounds, data, code);
                }
                if (result == nullptr && m_right->bounds().contains(bounds)) {
                    result = m_right->findLeaf(bounds, data, code);
                }
            }
            return result;
        }

        void accept(Visitor& visitor) const override {
            if (visitor.visit(this)) {
                m_left->accept(visitor);
                m_right->accept(visitor);
            }
        }
    private:
        /**
         * Indicates whether the morton codes of the two subtrees differ in any bit.
         *
         * @return true if the morton codes of the subtree differ, and false otherwise
         */
        bool isSplitNode() const {
            return m_splitIndex != MortonTree::CodeTypeWidth;
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

        LeafNode* findLeaf(const Box& bounds, const U& data, const CodeType code) override {
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
            return m_root->findLeaf(bounds, data, computeMortonCode(bounds.center())) != nullptr;
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
            const Box& bounds = getBounds(object);
            const CodeType mortonCode = computeMortonCode(bounds.center());
            leafList.push_back(new LeafNode(bounds, mortonCode, object));
        }

        // sort the nodes by their morton codes
        std::sort(std::begin(leafList), std::end(leafList),
                  [](const CodedNode* lhs, const CodedNode* rhs) { return lhs->code() < rhs->code(); });

        // recursively build the tree
        m_root = buildTree(std::begin(leafList), std::end(leafList), 0);
    }

    template <typename I>
    Node* buildTree(I first, I end, const size_t index) {
        auto* firstNode = *first;

        const auto last = std::prev(end);
        if (first == last) {
            return firstNode;
        }

        auto* lastNode  = *last;

        // the highest bit in which first and last differ
        const auto hiBit = Math::findHighestDifferingBit(firstNode->code(), lastNode->code(), reverseIndex(index));
        const auto midPoint = findMidPoint(first, end, hiBit);

        auto* leftNode  = buildTree(first, midPoint, reverseIndex(hiBit) + 1);
        auto* rightNode = buildTree(midPoint, end,   reverseIndex(hiBit) + 1);

        return new InnerNode(leftNode, rightNode, reverseIndex(hiBit));
    }
    
    /**
     * Finds the mid point of the given range using the given high bit index. If the given index indicates that the codes of the given
     * range do not differ in any bit, then the range is split in the middle. Otherwise, an iterator to the first node at which the bit
     * with the given index is 1 is returned.
     *
     * @param first iterator to the beginning of the range
     * @param end iterator to the end of the range
     * @param hiBit the index from the right of the highest bit in which the first and last node of the given range differ
     *
     * @return an iterator to the mid point at which the given range is to be split
     */
    template <typename I>
    auto findMidPoint(I first, I end, const CodeType hiBit) const {
        if (hiBit == CodeTypeWidth) {
            // the morton codes are identical, split the range in the middle
            const auto length = end - first;
            return std::next(first, length / 2);
        } else {
            // find midpoint for splitting the range
            const auto testValue = bitMask(hiBit);
            const auto testMask = (testValue - 1) | testValue; // has all bits greater than hiBit unset and all others set, e.g. 0000011111 if hiBit = 5;
            const CodedNode testNode(testValue);
            return std::lower_bound(first, end, &testNode, [&testMask](const CodedNode* lhs, const CodedNode* rhs){ return (lhs->code() & testMask) < (rhs->code() & testMask); });
        }
    }
public:
    void insert(const Box& bounds, const U& data) override {}

    bool remove(const Box& bounds, const U& data) override {}

    void update(const Box& oldBounds, const Box& newBounds, const U& data) override {}

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
private: // utility functions for dealing with morton codes
    /**
     * Reverses the given 0 based bit index. If the index is from the left,
     * this function returns an equivalent index from the right, and vice versa.
     *
     * @param index the index to convert
     * @return the converted index, or CodeTypeSize if the given index could not be converted
     */
    static CodeType reverseIndex(const CodeType index) {
        if (index >= MortonTree::CodeTypeWidth) {
            return MortonTree::CodeTypeWidth;
        } else {
            return MortonTree::CodeTypeWidth - index - 1;
        }
    }

    /**
     * Returns a bit mask with only the bit at the given index set.
     *
     * @param index the 0-based index (from the right)
     * @return the bit mask
     */
    static CodeType bitMask(const CodeType index) {
        return static_cast<CodeType>(1) << index;
    }
};

#endif /* MortonTree_h */
