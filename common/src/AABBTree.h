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

#include <BBox.h>

#include <algorithm>
#include <cassert>
#include <memory>

template <typename T, size_t S, typename U>
class AABBTree {
private:
    using Box = BBox<T,S>;

    class Node {
    private:
        Box m_bounds;
    protected:
        Node(const Box& bounds) : m_bounds(bounds) {}
    public:
        virtual ~Node() {}

        const Box& bounds() const {
            return m_bounds;
        }

        virtual size_t height() const = 0;
        virtual Node* insert(const Box& bounds, U& data) = 0;
    };

    class InnerNode : public Node {
    private:
        Node* m_left;
        Node* m_right;
        size_t m_height;
    public:
        InnerNode(const Box& bounds, Node* left, Node* right) : Node(bounds), m_left(left), m_right(right), m_height(0) {
            assert(m_left != nullptr);
            assert(m_right != nullptr);
            updateHeight();
        }

        ~InnerNode() {
            delete m_left;
            delete m_right;
        }

        size_t height() const override {
            return m_height;
        }

        Node* insert(const Box& bounds, U& data) override {
            const auto newLeft = m_left->bounds().mergedWith(bounds);
            const auto newRight = m_right->bounds().mergedWith(bounds);
            const auto leftDiff = newLeft.volume() - m_left->bounds().volume();
            const auto rightDiff = newRight.volume() - m_right->bounds().volume();

            if (leftDiff <= rightDiff) {
                m_left = m_left->insert(bounds, data);
            } else {
                m_right = m_right->insert(bounds, data);
            }

            updateHeight();

            return this;
        }
    private:
        void updateHeight() {
            m_height = std::max(m_left->height(), m_right->height()) + 1;
            assert(m_height > 0);
        }
    };

    class Leaf : public Node {
    private:
        U m_data;
    public:
        Leaf(const Box& bounds, U& data) : Node(bounds), m_data(data) {}

        size_t height() const override {
            return 1;
        }

        Node* insert(const Box& bounds, U& data) override {
            return new InnerNode(bounds.mergedWith(bounds), this, new Leaf(bounds, data));
        }
    };
private:
    Node* m_root;
public:
    AABBTree() : m_root(nullptr) {}

    ~AABBTree() {
        delete m_root;
    }

    void insert(const Box& bounds, U& data) {
        if (empty()) {
            m_root = new Leaf(bounds, data);
        } else {
            m_root = m_root->insert(bounds, data);
        }
    }

    bool empty() const {
        return m_root == nullptr;
    }

    size_t height() const {
        if (empty()) {
            return 0;
        } else {
            return m_root->height();
        }
    }

    const Box& bounds() const {
        static const Box EmptyBox = Box(Vec<T,S>::NaN, Vec<T,S>::NaN);

        assert(!empty());
        if (empty()) {
            return EmptyBox;
        } else {
            return m_root->bounds();
        }
    }
};

#endif //TRENCHBROOM_AABBTREE_H
