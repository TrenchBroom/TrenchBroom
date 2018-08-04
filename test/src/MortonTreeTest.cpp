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
#include "MortonTree.h"

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

class GetBounds {
public:
    using Map = std::map<TREE::DataType, BOX>;
    using List = std::list<TREE::DataType>;
private:
    Map m_objects;
public:
    GetBounds(std::initializer_list<Map::value_type> init) : m_objects(init) {}
    
    List objects() const {
        List result;
        for (const auto& pair : m_objects) {
            result.push_back(pair.first);
        }
        return result;
    }

    TREE::GetBounds getBounds() const {
        return [&](const TREE::DataType& object){
            const auto it = m_objects.find(object);
            assert(it != std::end(m_objects));
            return it->second;
        };
    }
};

void assertIntersectors(const TREE& tree, const Ray<TREE::FloatType, TREE::Components>& ray, std::initializer_list<TREE::DataType> items);

TEST(MortonTreeTest, createEmptyTree) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    ASSERT_TRUE(tree.empty());
}

TEST(MortonTreeTest, buildTreeWithOneNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    
    GetBounds getBounds({
        { 1u, BBox3d(Vec3d::Null, Vec3d(16.0, 8.0, 8.0)) }
    });
    
    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(BBox3d(Vec3d::Null, Vec3d(16.0, 8.0, 8.0)), 1u));
    ASSERT_EQ(BBox3d(Vec3d::Null, Vec3d(16.0, 8.0, 8.0)), tree.bounds());
}

TEST(MortonTreeTest, buildTreeWithTwoNodes) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    
    GetBounds getBounds({
        { 1u, BBox3d(Vec3d( 0.0,  0.0,  0.0), Vec3d(16.0,  8.0,  8.0)) },
        { 2u, BBox3d(Vec3d(32.0, 32.0, 32.0), Vec3d(48.0, 48.0, 48.0)) }
    });
    
    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(BBox3d(Vec3d( 0.0,  0.0,  0.0), Vec3d(16.0,  8.0,  8.0)), 1u));
    ASSERT_TRUE(tree.contains(BBox3d(Vec3d(32.0, 32.0, 32.0), Vec3d(48.0, 48.0, 48.0)), 2u));
    ASSERT_EQ(BBox3d(Vec3d(0.0, 0.0, 0.0), Vec3d(48.0, 48.0, 48.0)), tree.bounds());
}

TEST(MortonTreeTest, buildTreeWithTwoNodesHavingIdenticalCodes) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    
    // These bounds have the same centers and will yield identical codes.
    GetBounds getBounds({
        { 1u, BBox3d(32.0) },
        { 2u, BBox3d(16.0) }
    });
    
    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(BBox3d(32.0), 1u));
    ASSERT_TRUE(tree.contains(BBox3d(16.0), 2u));
    ASSERT_EQ(BBox3d(32.0), tree.bounds());
}

TEST(MortonTreeTest, insertNodesIntoEmptyTree) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    const auto box1 = BOX(32.0);
    const auto box2 = BOX(16.0).translate(VEC(64.0, 0.0, 0.0));

    tree.insert(box1, 1u);
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_EQ(box1, tree.bounds());

    tree.insert(box2, 2u);
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));
    ASSERT_EQ(box1.mergedWith(box2), tree.bounds());
}

TEST(MortonTreeTest, insertNodeWithHigherSplitIndex) {
    const VecCodeComputer<Vec3d> computeMortonCode(BBox3d(4096.0));
    TREE tree(computeMortonCode);

    const auto box1 = BOX(16.0).translate(VEC(  0.0, 0.0, 0.0));
    const auto box2 = BOX(16.0).translate(VEC( 64.0, 0.0, 0.0));
    const auto box3 = BOX(16.0).translate(VEC(-64.0, 0.0, 0.0));

    const auto code1 = computeMortonCode(box1.center());
    const auto code2 = computeMortonCode(box2.center());
    const auto code3 = computeMortonCode(box3.center());

    // assert that the third node will be inserted into the gap before the root (the inner node created
    // when the second node was inserted)
    const auto split12 = Math::findHighestDifferingBit(code1, code2);
    const auto split23 = Math::findHighestDifferingBit(code2, code3);
    ASSERT_TRUE(split23 > split12);

    tree.insert(box1, 1u);
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_EQ(box1, tree.bounds());

    tree.insert(box2, 2u);
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));
    ASSERT_EQ(box1.mergedWith(box2), tree.bounds());

    tree.insert(box3, 3u);
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));
    ASSERT_TRUE(tree.contains(box3, 3u));
    ASSERT_EQ(box1.mergedWith(box2).mergedWith(box3), tree.bounds());
}

TEST(MortonTreeTest, insertNodesWithIdenticalCodeAndNodeWithHigherSplitIndex) {
    const VecCodeComputer<Vec3d> computeMortonCode(BBox3d(4096.0));
    TREE tree(computeMortonCode);

    const auto box1 = BOX(16.0).translate(VEC(  0.0, 0.0, 0.0));
    const auto box2 = BOX(16.0).translate(VEC(  0.0, 0.0, 0.0));
    const auto box3 = BOX(16.0).translate(VEC(-64.0, 0.0, 0.0));

    const auto code1 = computeMortonCode(box1.center());
    const auto code2 = computeMortonCode(box2.center());
    const auto code3 = computeMortonCode(box3.center());

    // assert that the third node will be inserted into the gap before the root (the inner node created
    // when the second node was inserted)
    const auto split12 = Math::findHighestDifferingBit(code1, code2);
    const auto split23 = Math::findHighestDifferingBit(code2, code3);
    ASSERT_EQ(sizeof(TREE::CodeType)*8, split12);
    ASSERT_NE(sizeof(TREE::CodeType)*8, split23);

    // inserting the first two nodes will create a set node
    tree.insert(box1, 1u);
    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_EQ(box1, tree.bounds());

    tree.insert(box2, 2u);
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));
    ASSERT_EQ(box1.mergedWith(box2), tree.bounds());

    // this should be inserted above of the set node
    tree.insert(box3, 3u);
    ASSERT_TRUE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));
    ASSERT_TRUE(tree.contains(box3, 3u));
    ASSERT_EQ(box1.mergedWith(box2).mergedWith(box3), tree.bounds());
}

TEST(MortonTreeTest, removeNonExistingNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));


    const auto box1 = BOX(32.0);
    const auto box2 = BOX(16.0);

    ASSERT_FALSE(tree.remove(box1, 1u));

    tree.insert(box1, 1u);

    ASSERT_FALSE(tree.remove(box2, 2u));
}

TEST(MortonTreeTest, removeSingleNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    const auto box1 = BOX(32.0);

    tree.insert(box1, 1u);

    ASSERT_TRUE(tree.remove(box1, 1u));
    ASSERT_TRUE(tree.empty());
    ASSERT_FALSE(tree.contains(box1, 1u));
}

TEST(MortonTreeTest, removeNodeFromSingleSetNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    const auto box1 = BOX(32.0);
    const auto box2 = BOX(16.0);

    tree.insert(box1, 1u);
    tree.insert(box2, 2u);

    ASSERT_TRUE(tree.remove(box1, 1u));
    ASSERT_FALSE(tree.empty());
    ASSERT_FALSE(tree.contains(box1, 1u));
    ASSERT_TRUE(tree.contains(box2, 2u));

    ASSERT_TRUE(tree.remove(box2, 2u));
    ASSERT_TRUE(tree.empty());
    ASSERT_FALSE(tree.contains(box2, 2u));
}

TEST(MortonTreeTest, findIntersectorsOfEmptyTree) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));
    assertIntersectors(tree, RAY(VEC::Null, VEC::PosX), {});
}

TEST(MortonTreeTest, findIntersectorsOfTreeWithOneNode) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    GetBounds getBounds({
        { 1u, BOX(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0)) }
    });

    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());

    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::NegX), {});
    assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::PosX), { 1u });
}

TEST(MortonTreeTest, findIntersectorsOfTreeWithTwoNodes) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    GetBounds getBounds({
        { 1u, BOX(VEC(-2.0, -1.0, -1.0), VEC(-1.0, +1.0, +1.0)) },
        { 2u, BOX(VEC(+1.0, -1.0, -1.0), VEC(+2.0, +1.0, +1.0)) },
    });

    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());

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

TEST(MortonTreeTest, findIntersectorFromInside) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    GetBounds getBounds({
        { 1u, BOX(VEC(-4.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)) },
    });

    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());

    assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::PosX), { 1u });
}

TEST(MortonTreeTest, findIntersectorsFromInsideRootBBox) {
    TREE tree(VecCodeComputer<Vec3d>(BBox3d(4096.0)));

    GetBounds getBounds({
        { 1u, BOX(VEC(-4.0, -1.0, -1.0), VEC(-2.0, +1.0, +1.0)) },
        { 2u, BOX(VEC(+2.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)) },
    });

    tree.clearAndBuild(getBounds.objects(), getBounds.getBounds());

    assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::PosX), { 2u });
}

void assertIntersectors(const TREE& tree, const Ray<TREE::FloatType, TREE::Components>& ray, std::initializer_list<TREE::DataType> items) {
    const std::set<TREE::DataType> expected(items);
    std::set<TREE::DataType> actual;

    tree.findIntersectors(ray, std::inserter(actual, std::end(actual)));

    ASSERT_EQ(expected, actual);
}
