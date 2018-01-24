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
    AABB tree;
    const BBox3d bounds(Vec3d(0.0, 0.0, 0.0), Vec3d(2.0, 1.0, 1.0));
    tree.insert(bounds, 1u);

    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(1u, tree.height());
    ASSERT_EQ(bounds, tree.bounds());
}
