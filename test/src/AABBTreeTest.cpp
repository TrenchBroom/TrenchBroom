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

#include <gtest/gtest.h>

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include "AABBTree.h"

using AABB = AABBTree<double, 3, size_t>;
using BOX = AABB::Box;
using RAY = vm::ray<AABB::FloatType, AABB::Components>;
using VEC = vm::vec<AABB::FloatType, AABB::Components>;

void assertTree(const std::string& exp, const AABB& actual);
void assertIntersectors(const AABB& tree, const RAY& ray, std::initializer_list<AABB::DataType> items);

TEST(AABBTreeTest, createEmptyTree) {
    AABB tree;

    ASSERT_TRUE(tree.empty());

    assertTree(R"(
)" , tree);
}

TEST(AABBTreeTest, insertSingleNode) {
    const BOX bounds(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds, 1u);


    assertTree(R"(
L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(bounds, tree.bounds());
    ASSERT_TRUE(tree.contains(bounds, 1u));
}

TEST(AABBTreeTest, insertTwoNodes) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);

    assertTree(R"(
O [ ( -1 -1 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(merge(bounds1, bounds2), tree.bounds());
    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
}

TEST(AABBTreeTest, insertThreeNodes) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ ( -2 -2 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
    L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(merge(merge(bounds1, bounds2), bounds3), tree.bounds());
    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));
}

TEST(AABBTreeTest, removeLeafsInInverseInsertionOrder) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));

    assertTree(R"(
O [ ( -2 -2 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
    L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

    ASSERT_TRUE(tree.remove(bounds3, 3u));

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));

    assertTree(R"(
O [ ( -1 -1 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(merge(bounds1, bounds2), tree.bounds());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_TRUE(tree.remove(bounds2, 2u));

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));

    assertTree(R"(
L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(bounds1, tree.bounds());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_TRUE(tree.remove(bounds1, 1u));

    ASSERT_FALSE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));

    assertTree(R"(
)" , tree);

    ASSERT_TRUE(tree.empty());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_FALSE(tree.remove(bounds1, 1u));
}

TEST(AABBTreeTest, removeLeafsInInsertionOrder) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));

    assertTree(R"(
O [ ( -2 -2 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
    L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

    ASSERT_TRUE(tree.remove(bounds1, 1u));

    ASSERT_FALSE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));

    assertTree(R"(
O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
  L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(merge(bounds2, bounds3), tree.bounds());

    ASSERT_FALSE(tree.remove(bounds1, 1u));
    ASSERT_TRUE(tree.remove(bounds2, 2u));

    ASSERT_FALSE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));

    assertTree(R"(
L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(bounds3, tree.bounds());

    ASSERT_FALSE(tree.remove(bounds1, 1u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_TRUE(tree.remove(bounds3, 3u));

    ASSERT_FALSE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));

    assertTree(R"(
)" , tree);

    ASSERT_TRUE(tree.empty());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_FALSE(tree.remove(bounds1, 1u));
}

TEST(AABBTreeTest, insertFourContainedNodes) {
    const BOX bounds1(VEC(-4.0, -4.0, -4.0), VEC(4.0, 4.0, 4.0));
    const BOX bounds2(VEC(-3.0, -3.0, -3.0), VEC(3.0, 3.0, 3.0));
    const BOX bounds3(VEC(-2.0, -2.0, -2.0), VEC(2.0, 2.0, 2.0));
    const BOX bounds4(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);

    assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 1
  L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 2
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
    L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 1
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 3
  L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 2
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    tree.insert(bounds4, 4u);

    assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
    L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 1
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 3
  O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
    L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 2
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 4
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));
    ASSERT_TRUE(tree.contains(bounds4, 4u));
}

TEST(AABBTreeTest, insertFourContainedNodesInverse) {
    const BOX bounds1(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds2(VEC(-2.0, -2.0, -2.0), VEC(2.0, 2.0, 2.0));
    const BOX bounds3(VEC(-3.0, -3.0, -3.0), VEC(3.0, 3.0, 3.0));
    const BOX bounds4(VEC(-4.0, -4.0, -4.0), VEC(4.0, 4.0, 4.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);

    assertTree(R"(
O [ ( -2 -2 -2 ) ( 2 2 2 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
)" , tree);

    ASSERT_EQ(bounds2, tree.bounds());

    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
)" , tree);

    ASSERT_EQ(bounds3, tree.bounds());

    tree.insert(bounds4, 4u);

    assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
      L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
      L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 4
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(bounds4, tree.bounds());

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));
    ASSERT_TRUE(tree.contains(bounds4, 4u));
}

TEST(AABBTreeTest, removeFourContainedNodes) {
    const BOX bounds1(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds2(VEC(-2.0, -2.0, -2.0), VEC(2.0, 2.0, 2.0));
    const BOX bounds3(VEC(-3.0, -3.0, -3.0), VEC(3.0, 3.0, 3.0));
    const BOX bounds4(VEC(-4.0, -4.0, -4.0), VEC(4.0, 4.0, 4.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);
    tree.insert(bounds4, 4u);

    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));
    ASSERT_TRUE(tree.contains(bounds4, 4u));

    assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
      L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
      L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 4
)" , tree);


    tree.remove(bounds4, 4u);
    assertTree(R"(
O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
)" , tree);


    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_TRUE(tree.contains(bounds3, 3u));
    ASSERT_FALSE(tree.contains(bounds4, 4u));

    tree.remove(bounds3, 3u);
    assertTree(R"(
O [ ( -2 -2 -2 ) ( 2 2 2 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
)" , tree);


    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_TRUE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));
    ASSERT_FALSE(tree.contains(bounds4, 4u));

    tree.remove(bounds2, 2u);
    assertTree(R"(
L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
)" , tree);


    ASSERT_TRUE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));
    ASSERT_FALSE(tree.contains(bounds4, 4u));

    tree.remove(bounds1, 1u);
    assertTree(R"(
)" , tree);

    ASSERT_FALSE(tree.contains(bounds1, 1u));
    ASSERT_FALSE(tree.contains(bounds2, 2u));
    ASSERT_FALSE(tree.contains(bounds3, 3u));
    ASSERT_FALSE(tree.contains(bounds4, 4u));

}


template <typename K>
BOX makeBounds(const K min, const K max) {
    return BOX(VEC(static_cast<double>(min), -1.0, -1.0), VEC(static_cast<double>(max), 1.0, 1.0));
}

TEST(AABBTreeTest, findIntersectorsOfEmptyTree) {
    AABB tree;
    assertIntersectors(tree, RAY(VEC::zero, VEC::pos_x), {});
}

TEST(AABBTreeTest, findIntersectorsOfTreeWithOneNode) {
    AABB tree;
    tree.insert(BOX(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0)), 1u);

    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::neg_x), {});
    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::pos_x), { 1u });
}

TEST(AABBTreeTest, findIntersectorsOfTreeWithTwoNodes) {
    AABB tree;
    tree.insert(BOX(VEC(-2.0, -1.0, -1.0), VEC(-1.0, +1.0, +1.0)), 1u);
    tree.insert(BOX(VEC(+1.0, -1.0, -1.0), VEC(+2.0, +1.0, +1.0)), 2u);

    assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::pos_x), {});
    assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::neg_x), {});
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::pos_z), {});
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::pos_x), { 2u });
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::neg_x), { 1u });
    assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::pos_x), { 1u, 2u });
    assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::neg_x), { 1u, 2u });
    assertIntersectors(tree, RAY(VEC(-1.5, -2.0,  0.0), VEC::pos_y), { 1u });
    assertIntersectors(tree, RAY(VEC(+1.5, -2.0,  0.0), VEC::pos_y), { 2u });
}

TEST(AABBTreeTest, findIntersectorFromInside) {
    AABB tree;
    tree.insert(BOX(VEC(-4.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)), 1u);

    assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::pos_x), { 1u });
}

TEST(AABBTreeTest, findIntersectorsFromInsideRootBBox) {
    AABB tree;
    tree.insert(BOX(VEC(-4.0, -1.0, -1.0), VEC(-2.0, +1.0, +1.0)), 1u);
    tree.insert(BOX(VEC(+2.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)), 2u);

    assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::pos_x), { 2u });
}

void assertTree(const std::string& exp, const AABB& actual) {
    std::stringstream str;
    actual.print(str);
    ASSERT_EQ(exp, "\n" + str.str());
}

void assertIntersectors(const AABB& tree, const RAY& ray, std::initializer_list<AABB::DataType> items) {
    const std::set<AABB::DataType> expected(items);
    std::set<AABB::DataType> actual;

    tree.findIntersectors(ray, std::inserter(actual, std::end(actual)));

    ASSERT_EQ(expected, actual);
}
