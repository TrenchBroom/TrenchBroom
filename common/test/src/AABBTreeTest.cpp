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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include "AABBTree.h"

#include <set>
#include <sstream>

namespace TrenchBroom {
    using AABB = AABBTree<double, 3, size_t>;
    using BOX = AABB::Box;
    using RAY = vm::ray<AABB::FloatType, AABB::Components>;
    using VEC = vm::vec<AABB::FloatType, AABB::Components>;

    void assertTree(const std::string& exp, const AABB& actual);
    void assertIntersectors(const AABB& tree, const RAY& ray, std::initializer_list<AABB::DataType> items);
    void assertTreeContains(const AABB& tree, const BOX& box, AABB::DataType data);
    void assertTreeDoesNotContain(const AABB& tree, const BOX& box, AABB::DataType data);

    TEST_CASE("AABBTreeTest.createEmptyTree", "[AABBTreeTest]") {
        AABB tree;

        ASSERT_TRUE(tree.empty());

        assertTree(R"(
)" , tree);
    }

    TEST_CASE("AABBTreeTest.insertSingleNode", "[AABBTreeTest]") {
        const BOX bounds(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));

        AABB tree;
        tree.insert(bounds, 1u);


        assertTree(R"(
L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
)" , tree);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(bounds, tree.bounds());
        assertTreeContains(tree, bounds, 1u);
    }

    TEST_CASE("AABBTreeTest.insertDuplicateNode", "[AABBTreeTest]") {
        const BOX bounds(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));

        AABB tree;
        tree.insert(bounds, 1u);

        ASSERT_THROW(tree.insert(bounds, 1u), NodeTreeException);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(bounds, tree.bounds());
        assertTreeContains(tree, bounds, 1u);
    }

    TEST_CASE("AABBTreeTest.insertTwoNodes", "[AABBTreeTest]") {
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
        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
    }

    TEST_CASE("AABBTreeTest.insertThreeNodes", "[AABBTreeTest]") {
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
        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);
    }

    TEST_CASE("AABBTreeTest.removeLeafsInInverseInsertionOrder", "[AABBTreeTest]") {
        const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
        const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
        const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

        AABB tree;
        tree.insert(bounds1, 1u);
        tree.insert(bounds2, 2u);
        tree.insert(bounds3, 3u);

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);

        assertTree(R"(
O [ ( -2 -2 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
    L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

        ASSERT_TRUE(tree.remove(3u));

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);

        assertTree(R"(
O [ ( -1 -1 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
)" , tree);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(merge(bounds1, bounds2), tree.bounds());

        ASSERT_FALSE(tree.remove(3u));
        ASSERT_TRUE(tree.remove(2u));

        assertTreeContains(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);

        assertTree(R"(
L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
)" , tree);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(bounds1, tree.bounds());

        ASSERT_FALSE(tree.remove(3u));
        ASSERT_FALSE(tree.remove(2u));
        ASSERT_TRUE(tree.remove(1u));

        assertTreeDoesNotContain(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);

        assertTree(R"(
)" , tree);

        ASSERT_TRUE(tree.empty());

        ASSERT_FALSE(tree.remove(3u));
        ASSERT_FALSE(tree.remove(2u));
        ASSERT_FALSE(tree.remove(1u));
    }

    TEST_CASE("AABBTreeTest.removeLeafsInInsertionOrder", "[AABBTreeTest]") {
        const BOX bounds1(VEC(0.0, 0.0, 0.0), VEC(2.0, 1.0, 1.0));
        const BOX bounds2(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
        const BOX bounds3(VEC(-2.0, -2.0, -1.0), VEC(0.0, 0.0, 1.0));

        AABB tree;
        tree.insert(bounds1, 1u);
        tree.insert(bounds2, 2u);
        tree.insert(bounds3, 3u);

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);

        assertTree(R"(
O [ ( -2 -2 -1 ) ( 2 1 1 ) ]
  L [ ( 0 0 0 ) ( 2 1 1 ) ]: 1
  O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
    L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
    L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

        ASSERT_TRUE(tree.remove(1u));

        assertTreeDoesNotContain(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);

        assertTree(R"(
O [ ( -2 -2 -1 ) ( 1 1 1 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 2
  L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(merge(bounds2, bounds3), tree.bounds());

        ASSERT_FALSE(tree.remove(1u));
        ASSERT_TRUE(tree.remove(2u));

        assertTreeDoesNotContain(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);

        assertTree(R"(
L [ ( -2 -2 -1 ) ( 0 0 1 ) ]: 3
)" , tree);

        ASSERT_FALSE(tree.empty());
        ASSERT_EQ(bounds3, tree.bounds());

        ASSERT_FALSE(tree.remove(1u));
        ASSERT_FALSE(tree.remove(2u));
        ASSERT_TRUE(tree.remove(3u));

        assertTreeDoesNotContain(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);

        assertTree(R"(
)" , tree);

        ASSERT_TRUE(tree.empty());

        ASSERT_FALSE(tree.remove(3u));
        ASSERT_FALSE(tree.remove(2u));
        ASSERT_FALSE(tree.remove(1u));
    }

    TEST_CASE("AABBTreeTest.insertFourContainedNodes", "[AABBTreeTest]") {
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

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);
        assertTreeContains(tree, bounds4, 4u);
    }

    TEST_CASE("AABBTreeTest.insertFourContainedNodesInverse", "[AABBTreeTest]") {
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

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);
        assertTreeContains(tree, bounds4, 4u);
    }

    TEST_CASE("AABBTreeTest.removeFourContainedNodes", "[AABBTreeTest]") {
        const BOX bounds1(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0));
        const BOX bounds2(VEC(-2.0, -2.0, -2.0), VEC(2.0, 2.0, 2.0));
        const BOX bounds3(VEC(-3.0, -3.0, -3.0), VEC(3.0, 3.0, 3.0));
        const BOX bounds4(VEC(-4.0, -4.0, -4.0), VEC(4.0, 4.0, 4.0));

        AABB tree;
        tree.insert(bounds1, 1u);
        tree.insert(bounds2, 2u);
        tree.insert(bounds3, 3u);
        tree.insert(bounds4, 4u);

        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);
        assertTreeContains(tree, bounds4, 4u);

        assertTree(R"(
O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    O [ ( -4 -4 -4 ) ( 4 4 4 ) ]
      L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
      L [ ( -4 -4 -4 ) ( 4 4 4 ) ]: 4
)" , tree);


        tree.remove(4u);
        assertTree(R"(
O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  O [ ( -3 -3 -3 ) ( 3 3 3 ) ]
    L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
    L [ ( -3 -3 -3 ) ( 3 3 3 ) ]: 3
)" , tree);


        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeContains(tree, bounds3, 3u);
        assertTreeDoesNotContain(tree, bounds4, 4u);

        tree.remove(3u);
        assertTree(R"(
O [ ( -2 -2 -2 ) ( 2 2 2 ) ]
  L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
  L [ ( -2 -2 -2 ) ( 2 2 2 ) ]: 2
)" , tree);


        assertTreeContains(tree, bounds1, 1u);
        assertTreeContains(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);
        assertTreeDoesNotContain(tree, bounds4, 4u);

        tree.remove(2u);
        assertTree(R"(
L [ ( -1 -1 -1 ) ( 1 1 1 ) ]: 1
)" , tree);


        assertTreeContains(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);
        assertTreeDoesNotContain(tree, bounds4, 4u);

        tree.remove(1u);
        assertTree(R"(
)" , tree);

        assertTreeDoesNotContain(tree, bounds1, 1u);
        assertTreeDoesNotContain(tree, bounds2, 2u);
        assertTreeDoesNotContain(tree, bounds3, 3u);
        assertTreeDoesNotContain(tree, bounds4, 4u);

    }


    template <typename K>
    BOX makeBounds(const K min, const K max) {
        return BOX(VEC(static_cast<double>(min), -1.0, -1.0), VEC(static_cast<double>(max), 1.0, 1.0));
    }

    TEST_CASE("AABBTreeTest.findIntersectorsOfEmptyTree", "[AABBTreeTest]") {
        AABB tree;
        assertIntersectors(tree, RAY(VEC::zero(), VEC::pos_x()), {});
    }

    TEST_CASE("AABBTreeTest.findIntersectorsOfTreeWithOneNode", "[AABBTreeTest]") {
        AABB tree;
        tree.insert(BOX(VEC(-1.0, -1.0, -1.0), VEC(1.0, 1.0, 1.0)), 1u);

        assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::neg_x()), {});
        assertIntersectors(tree, RAY(VEC(-2.0, 0.0, 0.0), VEC::pos_x()), { 1u });
    }

    TEST_CASE("AABBTreeTest.findIntersectorsOfTreeWithTwoNodes", "[AABBTreeTest]") {
        AABB tree;
        tree.insert(BOX(VEC(-2.0, -1.0, -1.0), VEC(-1.0, +1.0, +1.0)), 1u);
        tree.insert(BOX(VEC(+1.0, -1.0, -1.0), VEC(+2.0, +1.0, +1.0)), 2u);

        assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::pos_x()), {});
        assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::neg_x()), {});
        assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::pos_z()), {});
        assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::pos_x()), { 2u });
        assertIntersectors(tree, RAY(VEC( 0.0,  0.0,  0.0), VEC::neg_x()), { 1u });
        assertIntersectors(tree, RAY(VEC(-3.0,  0.0,  0.0), VEC::pos_x()), { 1u, 2u });
        assertIntersectors(tree, RAY(VEC(+3.0,  0.0,  0.0), VEC::neg_x()), { 1u, 2u });
        assertIntersectors(tree, RAY(VEC(-1.5, -2.0,  0.0), VEC::pos_y()), { 1u });
        assertIntersectors(tree, RAY(VEC(+1.5, -2.0,  0.0), VEC::pos_y()), { 2u });
    }

    TEST_CASE("AABBTreeTest.findIntersectorFromInside", "[AABBTreeTest]") {
        AABB tree;
        tree.insert(BOX(VEC(-4.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)), 1u);

        assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::pos_x()), { 1u });
    }

    TEST_CASE("AABBTreeTest.findIntersectorsFromInsideRootBBox", "[AABBTreeTest]") {
        AABB tree;
        tree.insert(BOX(VEC(-4.0, -1.0, -1.0), VEC(-2.0, +1.0, +1.0)), 1u);
        tree.insert(BOX(VEC(+2.0, -1.0, -1.0), VEC(+4.0, +1.0, +1.0)), 2u);

        assertIntersectors(tree, RAY(VEC(0.0,  0.0,  0.0), VEC::pos_x()), { 2u });
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

    void assertTreeContains(const AABB& tree, const BOX& box, AABB::DataType data) {
        ASSERT_TRUE(tree.contains(data));

        // Check that the the AABB tree can retrieve `data` by doing a spatial search
        bool found = false;
        for (const AABB::DataType dataAtBoxCenter : tree.findContainers(box.center())) {
            if (dataAtBoxCenter == data) {
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found);

        // Check that a spatial search of a point outside `box` doesn't return `data`
        const auto pointOutsideBox = box.center() + box.size();
        ASSERT_FALSE(box.contains(pointOutsideBox));
        for (const AABB::DataType dataOutsideBox : tree.findContainers(pointOutsideBox)) {
            ASSERT_FALSE(dataOutsideBox == data);
        }
    }

    void assertTreeDoesNotContain(const AABB& tree, const BOX& box, AABB::DataType data) {
        ASSERT_FALSE(tree.contains(data));

        // Check that a spatial search doesn't return `data`
        for (const AABB::DataType dataAtBoxCenter : tree.findContainers(box.center())) {
            ASSERT_NE(dataAtBoxCenter, data);
        }
    }
}
