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

#include <memory>
#include <vector>

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

    };

    class InnerNode : public Node {

    };

    class LeafNode : public Node {
    private:
        CodeType m_code;
        U m_data;
    public:
        LeafNode(const CodeType& code, const U& data) :
                Node(),
                m_code(code),
                m_data(data) {}

        CodeType code() const {
            return m_code;
        }
    };

private:
    using NodePtr = std::unique_ptr<Node>;
    using LeafNodePtr = std::unique_ptr<LeafNode>;

    NodePtr m_root;
public:
    MortonTree(const CodeComp& codeComp) : computeMortonCode(codeComp) {}

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
        using LeafList = std::vector<LeafNodePtr>;
        LeafList leafList;
        leafList.reserve(pairs.size());

        for (const auto& pair : pairs) {
            const Box& bounds = std::get<0>(pair);
            const DataType& data = std::get<1>(pair);
            const CodeType mortonCode = computeMortonCode(bounds.center());
            leafList.push_back(std::make_unique(mortonCode, data));
        }

        // sort the nodes by their morton codes
        std::sort(std::begin(leafList), std::end(leafList),
                  [](const auto& lhs, const auto& rhs) { return lhs->code() < rhs->code(); });

        // recursively build the tree
        m_root = buildTree(std::begin(leafList), std::end(leafList));
    }

    template <typename I>
    NodePtr buildTree(I begin, I end) {
        // find the highest bit in which the morton codes of the first and last nodes differ

    }
public:
    void insert(const Box& bounds, const U& data) override {}

    bool remove(const Box& bounds, const U& data) override {}

    void update(const Box& oldBounds, const Box& newBounds, const U& data) override {}

    void clear() override {}

    bool empty() const override {}

    size_t height() const;

    const Box& bounds() const override {}

    List findIntersectors(const Ray<T,S>& ray) const override {}

    List findContainers(const Vec<T,S>& point) const override {}
};

#endif /* MortonTree_h */
