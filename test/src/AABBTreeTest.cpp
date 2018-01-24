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

TEST(AABBTreeTest, createEmptyTree) {
    AABB tree;

    ASSERT_TRUE(tree.empty());
    ASSERT_EQ(0u, tree.height());
}

TEST(AABBTreeTest, insertSingleNode) {
    const BBox3d bounds(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));

    AABB tree;
    tree.insert(bounds, 1u);

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

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(3u, tree.height());
    ASSERT_EQ(bounds1.mergedWith(bounds2).mergedWith(bounds3), tree.bounds());
}

TEST(AABBTreeTest, removeLeaf) {
    const BBox3d bounds1(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    const BBox3d bounds2(Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0));
    const BBox3d bounds3(Vec3d(-2.0, -2.0, -1.0), Vec3d(0.0, 0.0, 1.0));

    AABB tree;
    tree.insert(bounds1, 1u);
    tree.insert(bounds2, 2u);
    tree.insert(bounds3, 3u);

    ASSERT_TRUE(tree.remove(bounds3, 3u));

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(2u, tree.height());
    ASSERT_EQ(bounds1.mergedWith(bounds2), tree.bounds());
}
