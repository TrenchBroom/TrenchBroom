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
#include "MortonTree.h"
#include "TestUtils.h"

TEST(MortonTreeTest, testMortonCodeComputer) {
    VecCodeComputer<Vec3d> comp(BBox3d(2048.0));
    ASSERT_EQ(60129542144u, comp(Vec3d::Null));
    ASSERT_EQ(60129542151u, comp(Vec3d::One));
    ASSERT_EQ(60129542145u, comp(Vec3d::PosX));
    ASSERT_EQ(60129542146u, comp(Vec3d::PosY));
    ASSERT_EQ(60129542148u, comp(Vec3d::PosZ));
    ASSERT_EQ(52766741065u, comp(Vec3d::NegX));
    ASSERT_EQ(45403939986u, comp(Vec3d::NegY));
    ASSERT_EQ(30678337828u, comp(Vec3d::NegZ));
}

using TREE = MortonTree<double, 3, size_t, VecCodeComputer<Vec3d>>;
using BOX = TREE::Box;
using RAY = Ray<TREE::FloatType, TREE::Components>;
using VEC = Vec<TREE::FloatType, TREE::Components>;

TEST(MortonTreeTest, createEmptyTree) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    ASSERT_TRUE(tree.empty());
}

TEST(MortonTreeTest, buildTreeWithOneNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    
    TREE::PairList list;
    list.push_back(std::make_tuple(BBox3d(Vec3d::Null, Vec3d(16.0, 8.0, 8.0)), 1u));
    
    tree.clearAndBuild(list);
    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(BBox3d(Vec3d::Null, Vec3d(16.0, 8.0, 8.0)), tree.bounds());
}


TEST(MortonTreeTest, buildTreeWithTwoNodes) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    
    TREE::PairList list;
    list.push_back(std::make_tuple(BBox3d(Vec3d( 0.0,  0.0,  0.0), Vec3d(16.0,  8.0,  8.0)), 1u));
    list.push_back(std::make_tuple(BBox3d(Vec3d(32.0, 32.0, 32.0), Vec3d(48.0, 48.0, 48.0)), 2u));
    
    tree.clearAndBuild(list);
    ASSERT_FALSE(tree.empty());
    ASSERT_EQ(BBox3d(Vec3d(0.0, 0.0, 0.0), Vec3d(48.0, 48.0, 48.0)), tree.bounds());
}
