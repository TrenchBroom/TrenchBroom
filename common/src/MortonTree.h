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
#include <limits>
#include <vector>

template <typename V>
class VecCodeComputer {
public:
    using CodeType = uint64_t;
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
        for (CodeType i = 0; i < sizeof(CodeType)*8 / S + 1; ++i) {
            const auto bit = value & (static_cast<CodeType>(1) << i);
            if (bit) {
                const auto offset = static_cast<CodeType>(S) * i;
                result |= (static_cast<CodeType>(1) << offset);
            }
        }
        return result;
    }
};

template <typename T, size_t S, typename U, typename CodeComp, typename Cmp = std::less<U>>
class MortonTree : public NodeTree<T,S,U,Cmp> {
public:
    using List = typename NodeTree<T,S,U,Cmp>::List;
    using Box = typename NodeTree<T,S,U,Cmp>::Box;
    using DataType = typename NodeTree<T,S,U,Cmp>::DataType;
    using Pair = typename NodeTree<T,S,U,Cmp>::Pair;
    using PairList = typename NodeTree<T,S,U,Cmp>::PairList;
    using FloatType = typename NodeTree<T,S,U,Cmp>::FloatType;
private:
    using CodeType = typename CodeComp::CodeType;
    CodeComp computeMortonCode;

    class Node {
    private:
        Box m_bounds;
    public:
        Node(const Box& bounds) : m_bounds(bounds) {}
        virtual ~Node() {}

        const Box& bounds() const {
            return m_bounds;
        }
    };

    class InnerNode : public Node {
    private:
        Node* m_left;
        Node* m_right;
    public:
        InnerNode(Node* left, Node* right) : Node(left->bounds().mergedWith(right->bounds())), m_left(left), m_right(right){
            assert(m_left != nullptr);
            assert(m_right != nullptr);
        }

        ~InnerNode() override {
            delete m_left;
            m_left = nullptr;
            delete m_right;
            m_right = nullptr;
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
    };

private:
    Node* m_root;
public:
    MortonTree(const CodeComp& codeComp) : computeMortonCode(codeComp), m_root(nullptr) {}

    ~MortonTree() override {
        clear();
    }

    bool contains(const Box& bounds, const U& data) const override {}

    void clearAndBuild(const PairList& pairs) override {
        clear();
        build(pairs);
    }

private:
    void build(const PairList& pairs) {
        assert(empty());

        // initialize the list of leafs
        using LeafList = std::vector<LeafNode*>;
        LeafList leafList;
        leafList.reserve(pairs.size());

        for (const auto& pair : pairs) {
            const Box& bounds = std::get<0>(pair);
            const DataType& data = std::get<1>(pair);
            const CodeType mortonCode = computeMortonCode(bounds.center());
            leafList.push_back(new LeafNode(bounds, mortonCode, data));
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
        const auto hiBit = Math::findHighestDifferingBit(firstNode->code(), lastNode->code(), sizeof(CodeType)*8 - index - 1);

        // find midpoint for splitting the range
        const auto testValue = static_cast<decltype(hiBit)>(1) << hiBit;
        const auto testMask = (testValue - 1) | testValue; // has all bits greater than hiBit unset and all others set, e.g. 0000011111 if hiBit = 5;
        const CodedNode testNode(testValue);
        const auto midPoint = std::lower_bound(first, end, &testNode, [&testMask](const CodedNode* lhs, const CodedNode* rhs){ return (lhs->code() & testMask) < (rhs->code() & testMask); });

        auto* leftNode  = buildTree(first, midPoint, sizeof(CodeType)*8 - hiBit);
        auto* rightNode = buildTree(midPoint, end,   sizeof(CodeType)*8 - hiBit);

        return new InnerNode(leftNode, rightNode);
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

    List findIntersectors(const Ray<T,S>& ray) const override {}

    List findContainers(const Vec<T,S>& point) const override {}
};

#endif /* MortonTree_h */
