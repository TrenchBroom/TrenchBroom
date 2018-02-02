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

#include "Vec.h"
#include "Ray.h"
#include "AABBTree.h"

using AABB = AABBTree<double, 3, size_t>;
using BOX = AABB::Box;
using RAY = Ray<AABB::FloatType, AABB::Components>;
using VEC = Vec<AABB::FloatType, AABB::Components>;

void assertTree(const std::string& exp, const AABB& actual);
void assertIntersectors(const AABB& tree, const Ray<AABB::FloatType, AABB::Components>& ray, std::initializer_list<AABB::DataType> items);

TEST(AABBTreeTest, createEmptyTree) {
    AABB tree;

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(0u, tree.height());


    assertTree(R"(
)" , tree);
}

TEST(AABBTreeTest, insertSingleNode) {
    const BOX bounds(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds, 1u);


    assertTree(R"(
L [ (0 0 0) (2 1 1) ]: 1
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(1u, tree.height());
    ASSERT_EQ(bounds, tree.bounds());
}

TEST(AABBTreeTest, insertTwoNodes) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);

    assertTree(R"(
O [ (-1 -1 -1) (2 1 1) ]
  L [ (0 0 0) (2 1 1) ]: 1
  L [ (-1 -1 -1) (1 1 1) ]: 2
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(2u, tree.height());
    ASSERT_EQ(bounds1.mergedWith(bounds2), tree.bounds());
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
O [ (-2 -2 -1) (2 1 1) ]
  L [ (0 0 0) (2 1 1) ]: 1
  O [ (-2 -2 -1) (1 1 1) ]
    L [ (-1 -1 -1) (1 1 1) ]: 2
    L [ (-2 -2 -1) (0 0 1) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(3u, tree.height());
    ASSERT_EQ(bounds1.mergedWith(bounds2).mergedWith(bounds3), tree.bounds());
}

TEST(AABBTreeTest, removeLeafsInInverseInsertionOrder) {
    const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
    const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
    const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ (-2 -2 -1) (2 1 1) ]
  L [ (0 0 0) (2 1 1) ]: 1
  O [ (-2 -2 -1) (1 1 1) ]
    L [ (-1 -1 -1) (1 1 1) ]: 2
    L [ (-2 -2 -1) (0 0 1) ]: 3
)" , tree);

    ASSERT_TRUE(tree.remove(bounds3, 3u));

    assertTree(R"(
O [ (-1 -1 -1) (2 1 1) ]
  L [ (0 0 0) (2 1 1) ]: 1
  L [ (-1 -1 -1) (1 1 1) ]: 2
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(2u, tree.height());
    ASSERT_EQ(bounds1.mergedWith(bounds2), tree.bounds());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_TRUE(tree.remove(bounds2, 2u));

    assertTree(R"(
L [ (0 0 0) (2 1 1) ]: 1
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(1u, tree.height());
    ASSERT_EQ(bounds1, tree.bounds());

    ASSERT_FALSE(tree.remove(bounds3, 3u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_TRUE(tree.remove(bounds1, 1u));

    assertTree(R"(
)" , tree);

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(0u, tree.height());

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

    assertTree(R"(
O [ (-2 -2 -1) (2 1 1) ]
  L [ (0 0 0) (2 1 1) ]: 1
  O [ (-2 -2 -1) (1 1 1) ]
    L [ (-1 -1 -1) (1 1 1) ]: 2
    L [ (-2 -2 -1) (0 0 1) ]: 3
)" , tree);

    ASSERT_TRUE(tree.remove(bounds1, 1u));

    assertTree(R"(
O [ (-2 -2 -1) (1 1 1) ]
  L [ (-1 -1 -1) (1 1 1) ]: 2
  L [ (-2 -2 -1) (0 0 1) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(2u, tree.height());
    ASSERT_EQ(bounds2.mergedWith(bounds3), tree.bounds());

    ASSERT_FALSE(tree.remove(bounds1, 1u));
    ASSERT_TRUE(tree.remove(bounds2, 2u));

    assertTree(R"(
L [ (-2 -2 -1) (0 0 1) ]: 3
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(1u, tree.height());
    ASSERT_EQ(bounds3, tree.bounds());

    ASSERT_FALSE(tree.remove(bounds1, 1u));
    ASSERT_FALSE(tree.remove(bounds2, 2u));
    ASSERT_TRUE(tree.remove(bounds3, 3u));

    assertTree(R"(
)" , tree);

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(0u, tree.height());

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
O [ (-4 -4 -4) (4 4 4) ]
  L [ (-4 -4 -4) (4 4 4) ]: 1
  L [ (-3 -3 -3) (3 3 3) ]: 2
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ (-4 -4 -4) (4 4 4) ]
  L [ (-4 -4 -4) (4 4 4) ]: 1
  O [ (-3 -3 -3) (3 3 3) ]
    L [ (-3 -3 -3) (3 3 3) ]: 2
    L [ (-2 -2 -2) (2 2 2) ]: 3
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    tree.insert(bounds4, 4u);

    assertTree(R"(
O [ (-4 -4 -4) (4 4 4) ]
  O [ (-4 -4 -4) (4 4 4) ]
    L [ (-4 -4 -4) (4 4 4) ]: 1
    L [ (-3 -3 -3) (3 3 3) ]: 2
  O [ (-2 -2 -2) (2 2 2) ]
    L [ (-2 -2 -2) (2 2 2) ]: 3
    L [ (-1 -1 -1) (1 1 1) ]: 4
)" , tree);

    ASSERT_EQ(4u, tree.height());
    ASSERT_EQ(bounds1, tree.bounds());
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
O [ (-2 -2 -2) (2 2 2) ]
  L [ (-1 -1 -1) (1 1 1) ]: 1
  L [ (-2 -2 -2) (2 2 2) ]: 2
)" , tree);

    ASSERT_EQ(bounds2, tree.bounds());

    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ (-3 -3 -3) (3 3 3) ]
  L [ (-1 -1 -1) (1 1 1) ]: 1
  O [ (-3 -3 -3) (3 3 3) ]
    L [ (-2 -2 -2) (2 2 2) ]: 2
    L [ (-3 -3 -3) (3 3 3) ]: 3
)" , tree);

    ASSERT_EQ(bounds3, tree.bounds());

    tree.insert(bounds4, 4u);

    assertTree(R"(
O [ (-4 -4 -4) (4 4 4) ]
  O [ (-2 -2 -2) (2 2 2) ]
    L [ (-1 -1 -1) (1 1 1) ]: 1
    L [ (-2 -2 -2) (2 2 2) ]: 2
  O [ (-4 -4 -4) (4 4 4) ]
    L [ (-3 -3 -3) (3 3 3) ]: 3
    L [ (-4 -4 -4) (4 4 4) ]: 4
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(4u, tree.height());
    ASSERT_EQ(bounds4, tree.bounds());
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

    assertTree(R"(
O [ (-4 -4 -4) (4 4 4) ]
  O [ (-2 -2 -2) (2 2 2) ]
    L [ (-1 -1 -1) (1 1 1) ]: 1
    L [ (-2 -2 -2) (2 2 2) ]: 2
  O [ (-4 -4 -4) (4 4 4) ]
    L [ (-3 -3 -3) (3 3 3) ]: 3
    L [ (-4 -4 -4) (4 4 4) ]: 4
)" , tree);


    tree.remove(bounds4, 4u);
    assertTree(R"(
O [ (-3 -3 -3) (3 3 3) ]
  O [ (-2 -2 -2) (2 2 2) ]
    L [ (-1 -1 -1) (1 1 1) ]: 1
    L [ (-2 -2 -2) (2 2 2) ]: 2
  L [ (-3 -3 -3) (3 3 3) ]: 3
)" , tree);


    tree.remove(bounds3, 3u);
    assertTree(R"(
O [ (-2 -2 -2) (2 2 2) ]
  L [ (-1 -1 -1) (1 1 1) ]: 1
  L [ (-2 -2 -2) (2 2 2) ]: 2
)" , tree);


    tree.remove(bounds2, 2u);
    assertTree(R"(
L [ (-1 -1 -1) (1 1 1) ]: 1
)" , tree);


    tree.remove(bounds1, 1u);
    assertTree(R"(
)" , tree);

}


template <typename K>
BOX makeBounds(const K min, const K max) {
    return BOX(VEC(static_cast<double>(min), -1.0, -1.0), VEC(static_cast<double>(max), 1.0, 1.0));
}

TEST(AABBTreeTest, rebalanceAfterRemoval) {
    AABB tree;
    tree.insert(makeBounds(1, 3), 1u);
    tree.insert(makeBounds(5, 7), 3u);
    tree.insert(makeBounds(2, 4), 2u);
    tree.insert(makeBounds(6, 8), 4u);
    tree.insert(makeBounds(7, 9), 5u);

    assertTree(R"(
O [ (1 -1 -1) (9 1 1) ]
  O [ (1 -1 -1) (4 1 1) ]
    L [ (1 -1 -1) (3 1 1) ]: 1
    L [ (2 -1 -1) (4 1 1) ]: 2
  O [ (5 -1 -1) (9 1 1) ]
    L [ (5 -1 -1) (7 1 1) ]: 3
    O [ (6 -1 -1) (9 1 1) ]
      L [ (6 -1 -1) (8 1 1) ]: 4
      L [ (7 -1 -1) (9 1 1) ]: 5
)" , tree);

    // Removing node 1 leads to the collapse of the first child of the root, makeing the root unbalanced.
    tree.remove(makeBounds(1, 3), 1u);

    // Rebalancig the tree should remove node 3 from the right subtree and insert it into the left, yielding the
    // following structure.
    assertTree(R"(
O [ (2 -1 -1) (9 1 1) ]
  O [ (2 -1 -1) (7 1 1) ]
    L [ (2 -1 -1) (4 1 1) ]: 2
    L [ (5 -1 -1) (7 1 1) ]: 3
  O [ (6 -1 -1) (9 1 1) ]
    L [ (6 -1 -1) (8 1 1) ]: 4
    L [ (7 -1 -1) (9 1 1) ]: 5
)" , tree);
}

TEST(AABBTreeTest, rebalanceAfterRemoval2) {
    AABB tree;
    tree.insert(makeBounds( 1,  2), 1u);
    tree.insert(makeBounds( 9, 10), 5u);
    tree.insert(makeBounds(10, 11), 6u);
    tree.insert(makeBounds( 4,  5), 3u);
    tree.insert(makeBounds( 2,  3), 2u);
    tree.insert(makeBounds( 5,  6), 4u);

    assertTree(R"(
O [ (1 -1 -1) (11 1 1) ]
  O [ (1 -1 -1) (6 1 1) ]
    O [ (1 -1 -1) (3 1 1) ]
      L [ (1 -1 -1) (2 1 1) ]: 1
      L [ (2 -1 -1) (3 1 1) ]: 2
    O [ (4 -1 -1) (6 1 1) ]
      L [ (4 -1 -1) (5 1 1) ]: 3
      L [ (5 -1 -1) (6 1 1) ]: 4
  O [ (9 -1 -1) (11 1 1) ]
    L [ (9 -1 -1) (10 1 1) ]: 5
    L [ (10 -1 -1) (11 1 1) ]: 6
)", tree);

    tree.remove(makeBounds(10, 11), 6u);

    assertTree(R"(
O [ (1 -1 -1) (10 1 1) ]
  O [ (1 -1 -1) (5 1 1) ]
    O [ (1 -1 -1) (3 1 1) ]
      L [ (1 -1 -1) (2 1 1) ]: 1
      L [ (2 -1 -1) (3 1 1) ]: 2
    L [ (4 -1 -1) (5 1 1) ]: 3
  O [ (5 -1 -1) (10 1 1) ]
    L [ (9 -1 -1) (10 1 1) ]: 5
    L [ (5 -1 -1) (6 1 1) ]: 4
)", tree);
}

TEST(AABBTreeTest, findIntersectorsOfEmptyTree) {
    AABB tree;
    assertIntersectors(tree, RAY(VEC::Null, VEC::PosX), {});
}

TEST(AABBTreeTest, findIntersectorsOfTreeWithOneNode) {
    AABB tree;
    tree.insert(BOX(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0)), 1u);

    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::NegX), {});
    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::PosX), { 1u });
}

TEST(AABBTreeTest, findIntersectorsOfTreeWithTwoNodes) {
    AABB tree;
    tree.insert(BOX(VEC(-2.0, -1.0, -1.0), VEC(-1.0, +1.0, +1.0)), 1u);
    tree.insert(BOX(VEC(+1.0, -1.0, -1.0), VEC(+2.0, +1.0, +1.0)), 2u);

    assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::PosX), {});
    assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::NegX), {});
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::PosZ), {});
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::PosX), { 2u });
    assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::NegX), { 1u });
    assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::PosX), { 1u, 2u });
    assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::NegX), { 1u, 2u });
    assertIntersectors(tree, RAY(VEC(-1.5, -2.0,  0.0), VEC::PosY), { 1u });
    assertIntersectors(tree, RAY(VEC(+1.5, -2.0,  0.0), VEC::PosY), { 2u });
}

void assertTree(const std::string& exp, const AABB& actual) {
    std::stringstream str;
    actual.print(str);
    ASSERT_EQ(exp, "\n" + str.str());
}

void assertIntersectors(const AABB& tree, const Ray<AABB::FloatType, AABB::Components>& ray, std::initializer_list<AABB::DataType> items) {
    const std::set<AABB::DataType> expected(items);
    std::set<AABB::DataType> actual;

    tree.findIntersectors(ray, std::inserter(actual, std::end(actual)));

    ASSERT_EQ(expected, actual);
}
