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

#include "AABBTree.h"
#include "BBox.h"
#include "Vec.h"

using AABB = AABBTree<double, 3, const size_t>;

void assertTree(const std::string& exp, const AABB& actual);

TEST(AABBTreeTest, createEmptyTree) {
    AABB tree;

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(0u, tree.height());


    assertTree(R"(
)" , tree);
}

TEST(AABBTreeTest, insertSingleNode) {
    const BBox3d bounds(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));

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
    const BBox3d bounds1(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));

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
    const BBox3d bounds1(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));
    const BBox3d bounds3(Vec3d(-2.0, -2.0, -1.0), Vec3d(0.0, 0.0, 1.0));

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
    const BBox3d bounds1(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));
    const BBox3d bounds3(Vec3d(-2.0, -2.0, -1.0), Vec3d(0.0, 0.0, 1.0));

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
    const BBox3d bounds1(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));
    const BBox3d bounds3(Vec3d(-2.0, -2.0, -1.0), Vec3d(0.0, 0.0, 1.0));

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

TEST(AABBTreeTest, insertThreeContainedNodes) {
    const BBox3d bounds1(Vec3d(-3.0, -3, -3.0), Vec3d(3.0, 3.0, 3.0));
    const BBox3d bounds2(Vec3d(-2.0, -2.0, -2.0), Vec3d(2.0, 2.0, 2.0));
    const BBox3d bounds3(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);

    assertTree(R"(
O [ (-3 -3 -3) (3 3 3) ]
  L [ (-3 -3 -3) (3 3 3) ]: 1
  L [ (-2 -2 -2) (2 2 2) ]: 2
)" , tree);

    ASSERT_EQ(bounds1, tree.bounds());

    tree.insert(bounds3, 3u);

    assertTree(R"(
O [ (-3 -3 -3) (3 3 3) ]
  O [ (-3 -3 -3) (3 3 3) ]
    L [ (-3 -3 -3) (3 3 3) ]: 1
    L [ (-1 -1 -1) (1 1 1) ]: 3
  L [ (-2 -2 -2) (2 2 2) ]: 2
)" , tree);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(3u, tree.height());
    ASSERT_EQ(bounds1, tree.bounds());
}

TEST(AABBTreeTest, insertThreeContainedNodesInverse) {
    const BBox3d bounds1(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-2.0, -2.0, -2.0), Vec3d(2.0, 2.0, 2.0));
    const BBox3d bounds3(Vec3d(-3.0, -3.0, -3.0), Vec3d(3.0, 3.0, 3.0));

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

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(3u, tree.height());
    ASSERT_EQ(bounds3, tree.bounds());
}

void assertTree(const std::string& exp, const AABB& actual) {
    std::stringstream str;
    actual.print(str);
    ASSERT_EQ(exp, "\n" + str.str());
}
