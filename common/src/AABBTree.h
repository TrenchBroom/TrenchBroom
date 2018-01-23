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
        Box m_box;
    protected:
        Node(const Box& box) : m_box(box) {}
    public:
        virtual ~Node() {}

        const Box& box() const {
            return m_box;
        }

        virtual size_t height() const = 0;
        virtual Node* insert(const Box& box, U& data) = 0;
    };

    class InnerNode : public Node {
    private:
        Node* m_left;
        Node* m_right;
        size_t m_height;
    public:
        InnerNode(const Box& box, Node* left, Node* right) : Node(box), m_left(left), m_right(right), m_height(0) {
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

        Node* insert(const Box& box, U& data) override {
            const auto newLeft = m_left->box().mergedWith(box);
            const auto newRight = m_right->box().mergedWith(box);
            const auto leftDiff = newLeft.volume() - m_left->box().volume();
            const auto rightDiff = newRight.volume() - m_right->box().volume();

            if (leftDiff <= rightDiff) {
                m_left = m_left->insert(data, box);
            } else {
                m_right = m_right->insert(data, box);
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
        Leaf(const Box& box, U& data) : Node(box), m_data(data) {}

        size_t height() const override {
            return 1;
        }

        Node* insert(const Box& box, U& data) override {
            return new InnerNode(box.mergedWith(box), this, new Leaf(box, data));
        }
    };
private:
    Node* m_root;

public:
    AABBTree() : m_root(nullptr) {}

    ~AABBTree() {
        delete m_root;
    }

    void insert(const Box& box, U& data) {
        if (m_root == nullptr) {
            m_root = new Leaf(box, data);
        } else {
            m_root = m_root->insert(box, data);
        }
    }

    size_t height() const {
        if (m_root == nullptr) {
            return 0;
        } else {
            return m_root->height();
        }
    }
};

#endif //TRENCHBROOM_AABBTREE_H
