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

#include "octree.h"

#include <kdl/string_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/forward.h>
#include <vecmath/ray.h>
#include <vecmath/vec.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace detail
{
TEST_CASE("node_address")
{
  SECTION("min")
  {
    CHECK(node_address{1, 2, 3, 0}.min() == vm::vec<int, 3>{1, 2, 3});
    CHECK(node_address{0, 2, 4, 1}.min() == vm::vec<int, 3>{0, 2, 4});
    CHECK(node_address{-2, -2, -2, 2}.min() == vm::vec<int, 3>{-2, -2, -2});
  }

  SECTION("max")
  {
    CHECK(node_address{1, 2, 3, 0}.max() == vm::vec<int, 3>{2, 3, 4});
    CHECK(node_address{0, 2, 4, 1}.max() == vm::vec<int, 3>{2, 4, 6});
    CHECK(node_address{0, 4, 8, 2}.max() == vm::vec<int, 3>{4, 8, 12});
    CHECK(node_address{-2, -2, -2, 2}.max() == vm::vec<int, 3>{2, 2, 2});
  }

  SECTION("contains")
  {
    CHECK(node_address{0, 0, 0, 0}.contains({0, 0, 0, 0}));
    CHECK(node_address{0, 0, 0, 1}.contains({0, 0, 0, 0}));
    CHECK(node_address{0, 0, 0, 2}.contains({0, 0, 0, 0}));
    CHECK(node_address{0, 0, 0, 2}.contains({0, 0, 0, 1}));
    CHECK(node_address{0, 0, 0, 2}.contains({0, 0, 0, 2}));
    CHECK(node_address{0, 0, 0, 2}.contains({0, 0, 0, 1}));
    CHECK(node_address{0, 0, 0, 2}.contains({2, 2, 2, 1}));
    CHECK(node_address{-4, -4, -4, 2}.contains({-2, -2, -2, 1}));

    CHECK_FALSE(node_address{0, 0, 0, 0}.contains({0, 0, 0, 1}));
    CHECK_FALSE(node_address{0, 0, 0, 2}.contains({2, 2, 4, 1}));
    CHECK_FALSE(node_address{0, 0, 0, 2}.contains({4, 0, 0, 2}));
    CHECK_FALSE(node_address{0, 0, 0, 2}.contains({0, 0, 0, 3}));

    CHECK_FALSE(node_address{0, 0, 0, 1}.contains({0, 3, 0, 0}));
  }

  SECTION("to_bounds")
  {
    CHECK(
      node_address{0, 0, 0, 0}.to_bounds(16.0) == vm::bbox3d{{0, 0, 0}, {16, 16, 16}});
    CHECK(
      node_address{0, 0, 0, 1}.to_bounds(16.0) == vm::bbox3d{{0, 0, 0}, {32, 32, 32}});
    CHECK(
      node_address{-2, 2, 4, 1}.to_bounds(16.0)
      == vm::bbox3d{{-32, 32, 64}, {0, 64, 96}});
  }

  SECTION("get_address")
  {
    CHECK(get_address({0, 0, 0}, 16.0) == node_address{0, 0, 0, 0});
    CHECK(get_address({8, 8, 8}, 16.0) == node_address{0, 0, 0, 0});
    CHECK(get_address({16, 0, 0}, 16.0) == node_address{1, 0, 0, 0});
    CHECK(get_address({16, 16, 16}, 16.0) == node_address{1, 1, 1, 0});

    CHECK(get_address({-1, -1, -1}, 16.0) == node_address{-1, -1, -1, 0});
    CHECK(get_address({-16, -16, -16}, 16.0) == node_address{-1, -1, -1, 0});
    CHECK(get_address({-17, -16, -16}, 16.0) == node_address{-2, -1, -1, 0});
  }

  SECTION("get_parent")
  {
    CHECK(get_parent({0, 0, 0, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({0, 0, 0, 1}) == node_address{0, 0, 0, 2});
    CHECK(get_parent({2, 0, 0, 1}) == node_address{0, 0, 0, 2});

    CHECK(get_parent({0, 0, 0, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({0, 0, 1, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({0, 1, 0, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({0, 1, 1, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({1, 0, 0, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({1, 0, 1, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({1, 1, 0, 0}) == node_address{0, 0, 0, 1});
    CHECK(get_parent({1, 1, 1, 0}) == node_address{0, 0, 0, 1});

    CHECK(get_parent({2, 2, 2, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({2, 2, 3, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({2, 3, 2, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({2, 3, 3, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({3, 2, 2, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({3, 2, 3, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({3, 3, 2, 0}) == node_address{2, 2, 2, 1});
    CHECK(get_parent({3, 3, 3, 0}) == node_address{2, 2, 2, 1});

    CHECK(get_parent({-1, -1, -1, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-1, -1, -2, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-1, -2, -1, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-1, -2, -2, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-2, -1, -1, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-2, -1, -2, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-2, -2, -1, 0}) == node_address{-2, -2, -2, 1});
    CHECK(get_parent({-2, -2, -2, 0}) == node_address{-2, -2, -2, 1});

    CHECK(get_parent({-3, -3, -3, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-3, -3, -4, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-3, -4, -3, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-3, -4, -4, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-4, -3, -3, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-4, -3, -4, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-4, -4, -3, 0}) == node_address{-4, -4, -4, 1});
    CHECK(get_parent({-3, -3, -3, 0}) == node_address{-4, -4, -4, 1});

    CHECK(get_parent({-4, 0, 2, 1}) == node_address{-4, 0, 0, 2});
    CHECK(get_parent({-2, 0, -2, 1}) == node_address{-4, 0, -4, 2});
  }

  SECTION("get_quadrant")
  {
    CHECK(get_quadrant({-4, -4, -4, 3}, {-1, -1, -1, 1}) == std::nullopt);

    CHECK(get_quadrant({-1, -1, -1, 1}, {-1, -1, -1, 0}) == 0);
    CHECK(get_quadrant({-1, -1, -1, 1}, {0, -1, -1, 0}) == 1);
    CHECK(get_quadrant({-1, -1, -1, 1}, {-1, 0, -1, 0}) == 2);
    CHECK(get_quadrant({-1, -1, -1, 1}, {0, 0, -1, 0}) == 3);
    CHECK(get_quadrant({-1, -1, -1, 1}, {-1, -1, 0, 0}) == 4);
    CHECK(get_quadrant({-1, -1, -1, 1}, {0, -1, 0, 0}) == 5);
    CHECK(get_quadrant({-1, -1, -1, 1}, {-1, 0, 0, 0}) == 6);
    CHECK(get_quadrant({-1, -1, -1, 1}, {0, 0, 0, 0}) == 7);

    CHECK(get_quadrant({-2, -2, -2, 2}, {-1, -1, -1, 0}) == 0);
    CHECK(get_quadrant({-2, -2, -2, 2}, {-2, -2, -2, 0}) == 0);
    CHECK(get_quadrant({-2, -2, -2, 2}, {0, -1, -1, 0}) == 1);
    CHECK(get_quadrant({-2, -2, -2, 2}, {-1, 0, -1, 0}) == 2);
    CHECK(get_quadrant({-2, -2, -2, 2}, {0, 0, -1, 0}) == 3);
    CHECK(get_quadrant({-2, -2, -2, 2}, {-1, -1, 0, 0}) == 4);
    CHECK(get_quadrant({-2, -2, -2, 2}, {0, -1, 0, 0}) == 5);
    CHECK(get_quadrant({-2, -2, -2, 2}, {-1, 0, 0, 0}) == 6);
    CHECK(get_quadrant({-2, -2, -2, 2}, {0, 0, 0, 0}) == 7);
    CHECK(get_quadrant({-2, -2, -2, 2}, {1, 1, 1, 0}) == 7);

    CHECK(get_quadrant({0, 0, 0, 1}, {0, 0, 0, 0}) == 0);
    CHECK(get_quadrant({0, 0, 0, 1}, {1, 1, 1, 0}) == 7);

    CHECK(get_quadrant({2, 2, 2, 1}, {2, 2, 2, 0}) == 0);
    CHECK(get_quadrant({2, 2, 2, 1}, {3, 3, 3, 0}) == 7);
  }

  SECTION("get_child")
  {
    CHECK(get_child({-2, -2, -2, 2}, 0) == node_address{-2, -2, -2, 1});
    CHECK(get_child({-2, -2, -2, 2}, 1) == node_address{0, -2, -2, 1});
    CHECK(get_child({-2, -2, -2, 2}, 2) == node_address{-2, 0, -2, 1});
    CHECK(get_child({-2, -2, -2, 2}, 3) == node_address{0, 0, -2, 1});
    CHECK(get_child({-2, -2, -2, 2}, 4) == node_address{-2, -2, 0, 1});
    CHECK(get_child({-2, -2, -2, 2}, 5) == node_address{0, -2, 0, 1});
    CHECK(get_child({-2, -2, -2, 2}, 6) == node_address{-2, 0, 0, 1});
    CHECK(get_child({-2, -2, -2, 2}, 7) == node_address{0, 0, 0, 1});

    CHECK(get_child({0, 0, 0, 2}, 0) == node_address{0, 0, 0, 1});
    CHECK(get_child({0, 0, 0, 2}, 1) == node_address{2, 0, 0, 1});
    CHECK(get_child({0, 0, 0, 2}, 2) == node_address{0, 2, 0, 1});
    CHECK(get_child({0, 0, 0, 2}, 3) == node_address{2, 2, 0, 1});
    CHECK(get_child({0, 0, 0, 2}, 4) == node_address{0, 0, 2, 1});
    CHECK(get_child({0, 0, 0, 2}, 5) == node_address{2, 0, 2, 1});
    CHECK(get_child({0, 0, 0, 2}, 6) == node_address{0, 2, 2, 1});
    CHECK(get_child({0, 0, 0, 2}, 7) == node_address{2, 2, 2, 1});

    CHECK(get_child({-4, -4, -4, 2}, 0) == node_address{-4, -4, -4, 1});
    CHECK(get_child({-4, -4, -4, 2}, 1) == node_address{-2, -4, -4, 1});
    CHECK(get_child({-4, -4, -4, 2}, 2) == node_address{-4, -2, -4, 1});
    CHECK(get_child({-4, -4, -4, 2}, 3) == node_address{-2, -2, -4, 1});
    CHECK(get_child({-4, -4, -4, 2}, 4) == node_address{-4, -4, -2, 1});
    CHECK(get_child({-4, -4, -4, 2}, 5) == node_address{-2, -4, -2, 1});
    CHECK(get_child({-4, -4, -4, 2}, 6) == node_address{-4, -2, -2, 1});
    CHECK(get_child({-4, -4, -4, 2}, 7) == node_address{-2, -2, -2, 1});
  }

  SECTION("is_root")
  {
    CHECK(is_root({-1, -1, -1, 1}));
    CHECK(is_root({-2, -2, -2, 2}));

    CHECK_FALSE(is_root({0, 0, 0, 0}));
    CHECK_FALSE(is_root({1, 2, 3, 0}));
  }

  SECTION("get_root")
  {
    CHECK(get_root({0, 0, 0, 0}) == node_address{-2, -2, -2, 2});
    CHECK(get_root({0, 0, 0, 1}) == node_address{-4, -4, -4, 3});
    CHECK(get_root({2, 2, 2, 1}) == node_address{-4, -4, -4, 3});
    CHECK(get_root({-1, -1, -1, 0}) == node_address{-2, -2, -2, 2});
    CHECK(get_root({-2, -2, -2, 0}) == node_address{-2, -2, -2, 2});
    CHECK(get_root({-3, -3, -3, 0}) == node_address{-4, -4, -4, 3});
    CHECK(get_root({-4, 0, 2, 1}) == node_address{-4, -4, -4, 3});
    CHECK(get_root({-3, 9, 0, 0}) == node_address{-16, -16, -16, 5});
  }

  SECTION("get_container")
  {
    CHECK(get_container({{2, 2, 2}, {6, 6, 6}}, 32.0) == node_address{0, 0, 0, 0});
    CHECK(
      get_container({{-4, -4, -4}, {-2, -2, -2}}, 32.0) == node_address{-1, -1, -1, 0});
    CHECK(get_container({{42, 42, 42}, {46, 46, 46}}, 32.0) == node_address{1, 1, 1, 0});

    CHECK(get_container({{-6, -6, -6}, {2, 2, 2}}, 32.0) == node_address{-1, -1, -1, 1});
    CHECK(get_container({{-2, -2, -2}, {2, 2, 2}}, 32.0) == node_address{-1, -1, -1, 1});
    CHECK(get_container({{-2, 2, 2}, {2, 4, 4}}, 32.0) == node_address{-1, -1, -1, 1});
    CHECK(
      get_container({{-42, -42, -42}, {2, 2, 2}}, 32.0) == node_address{-2, -2, -2, 2});
  }
}
} // namespace detail

using tree = octree<double, int>;
using node = tree::node;
using leaf_node = tree::leaf_node;
using inner_node = tree::inner_node;

TEST_CASE("octree.insert")
{
  auto tree = octree<double, int>{32.0};

  SECTION("inserting into root node")
  {
    tree.insert({{-2, 0, 0}, {5, 3, 6}}, 1);
    CHECK(tree == octree<double, int>{32.0, leaf_node{{-1, -1, -1, 1}, {1}}});

    tree.insert({{-32, -32, -32}, {32, 32, 32}}, 2);
    CHECK(tree == octree<double, int>{32.0, leaf_node{{-1, -1, -1, 1}, {1, 2}}});

    tree.insert({{-33, -32, -32}, {32, 32, 32}}, 3);
    CHECK(tree == octree<double, int>{32.0, leaf_node{{-2, -2, -2, 2}, {1, 2, 3}}});
  }


  SECTION("expanding root node")
  {
    tree.insert({{16, 16, -16}, {17, 17, -15}}, 1);
    CHECK(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -1, 0}, {1}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{0, 0, 0, 1}, {}}})}});

    tree.insert({{-120, 130, -48}, {-116, 140, -40}}, 2);
  }


  SECTION("inserting into quadrants")
  {
    SECTION("inserting skips unnecessary inner nodes")
    {
      tree.insert({{2, 2, 2}, {3, 3, 3}}, 1);
      CHECK(
        tree
        == octree<double, int>{
          32.0,
          inner_node{
            {-2, -2, -2, 2},
            {},
            kdl::vec_from(
              node{leaf_node{{-2, -2, -2, 1}, {}}},
              node{leaf_node{{0, -2, -2, 1}, {}}},
              node{leaf_node{{-2, 0, -2, 1}, {}}},
              node{leaf_node{{0, 0, -2, 1}, {}}},
              node{leaf_node{{-2, -2, 0, 1}, {}}},
              node{leaf_node{{0, -2, 0, 1}, {}}},
              node{leaf_node{{-2, 0, 0, 1}, {}}},
              node{leaf_node{{0, 0, 0, 0}, {1}}})}});

      tree.insert({{3, 3, 3}, {4, 4, 4}}, 2);
      CHECK(
        tree
        == octree<double, int>{
          32.0,
          inner_node{
            {-2, -2, -2, 2},
            {},
            kdl::vec_from(
              node{leaf_node{{-2, -2, -2, 1}, {}}},
              node{leaf_node{{0, -2, -2, 1}, {}}},
              node{leaf_node{{-2, 0, -2, 1}, {}}},
              node{leaf_node{{0, 0, -2, 1}, {}}},
              node{leaf_node{{-2, -2, 0, 1}, {}}},
              node{leaf_node{{0, -2, 0, 1}, {}}},
              node{leaf_node{{-2, 0, 0, 1}, {}}},
              node{leaf_node{{0, 0, 0, 0}, {1, 2}}})}});

      SECTION("skipped inner nodes are created as needed")
      {
        SECTION("when inserting into quadrant 7 of skipped node")
        {
          tree.insert({{33, 33, 33}, {34, 34, 34}}, 3);
          CHECK(
            tree
            == octree<double, int>{
              32.0,
              inner_node{
                {-2, -2, -2, 2},
                {},
                kdl::vec_from(
                  node{leaf_node{{-2, -2, -2, 1}, {}}},
                  node{leaf_node{{0, -2, -2, 1}, {}}},
                  node{leaf_node{{-2, 0, -2, 1}, {}}},
                  node{leaf_node{{0, 0, -2, 1}, {}}},
                  node{leaf_node{{-2, -2, 0, 1}, {}}},
                  node{leaf_node{{0, -2, 0, 1}, {}}},
                  node{leaf_node{{-2, 0, 0, 1}, {}}},
                  node{inner_node{
                    {0, 0, 0, 1},
                    {},
                    kdl::vec_from(
                      node{leaf_node{{0, 0, 0, 0}, {1, 2}}},
                      node{leaf_node{{1, 0, 0, 0}, {}}},
                      node{leaf_node{{0, 1, 0, 0}, {}}},
                      node{leaf_node{{1, 1, 0, 0}, {}}},
                      node{leaf_node{{0, 0, 1, 0}, {}}},
                      node{leaf_node{{1, 0, 1, 0}, {}}},
                      node{leaf_node{{0, 1, 1, 0}, {}}},
                      node{leaf_node{{1, 1, 1, 0}, {3}}})}})}});
        }

        SECTION("when inserting into quadrant 1 of skipped node")
        {
          tree.insert({{33, 3, 3}, {34, 4, 4}}, 3);
          CHECK(
            tree
            == octree<double, int>{
              32.0,
              inner_node{
                {-2, -2, -2, 2},
                {},
                kdl::vec_from(
                  node{leaf_node{{-2, -2, -2, 1}, {}}},
                  node{leaf_node{{0, -2, -2, 1}, {}}},
                  node{leaf_node{{-2, 0, -2, 1}, {}}},
                  node{leaf_node{{0, 0, -2, 1}, {}}},
                  node{leaf_node{{-2, -2, 0, 1}, {}}},
                  node{leaf_node{{0, -2, 0, 1}, {}}},
                  node{leaf_node{{-2, 0, 0, 1}, {}}},
                  node{inner_node{
                    {0, 0, 0, 1},
                    {},
                    kdl::vec_from(
                      node{leaf_node{{0, 0, 0, 0}, {1, 2}}},
                      node{leaf_node{{1, 0, 0, 0}, {3}}},
                      node{leaf_node{{0, 1, 0, 0}, {}}},
                      node{leaf_node{{1, 1, 0, 0}, {}}},
                      node{leaf_node{{0, 0, 1, 0}, {}}},
                      node{leaf_node{{1, 0, 1, 0}, {}}},
                      node{leaf_node{{0, 1, 1, 0}, {}}},
                      node{leaf_node{{1, 1, 1, 0}, {}}})}})}});
        }
        SECTION("when inserting into skipped node directly")
        {
          tree.insert({{31, 31, 31}, {34, 34, 34}}, 3);
          CHECK(
            tree
            == octree<double, int>{
              32.0,
              inner_node{
                {-2, -2, -2, 2},
                {},
                kdl::vec_from(
                  node{leaf_node{{-2, -2, -2, 1}, {}}},
                  node{leaf_node{{0, -2, -2, 1}, {}}},
                  node{leaf_node{{-2, 0, -2, 1}, {}}},
                  node{leaf_node{{0, 0, -2, 1}, {}}},
                  node{leaf_node{{-2, -2, 0, 1}, {}}},
                  node{leaf_node{{0, -2, 0, 1}, {}}},
                  node{leaf_node{{-2, 0, 0, 1}, {}}},
                  node{inner_node{
                    {0, 0, 0, 1},
                    {3},
                    kdl::vec_from(
                      node{leaf_node{{0, 0, 0, 0}, {1, 2}}},
                      node{leaf_node{{1, 0, 0, 0}, {}}},
                      node{leaf_node{{0, 1, 0, 0}, {}}},
                      node{leaf_node{{1, 1, 0, 0}, {}}},
                      node{leaf_node{{0, 0, 1, 0}, {}}},
                      node{leaf_node{{1, 0, 1, 0}, {}}},
                      node{leaf_node{{0, 1, 1, 0}, {}}},
                      node{leaf_node{{1, 1, 1, 0}, {}}})}})}});
        }
      }
    }
  }
}

TEST_CASE("octree.remove")
{
  auto tree = octree<double, int>{
    32.0,
    inner_node{
      {-2, -2, -2, 2},
      {},
      kdl::vec_from(
        node{leaf_node{{-2, -2, -2, 1}, {}}},
        node{leaf_node{{0, -2, -2, 1}, {}}},
        node{leaf_node{{-2, 0, -2, 1}, {}}},
        node{leaf_node{{0, 0, -2, 1}, {}}},
        node{leaf_node{{-2, -2, 0, 1}, {}}},
        node{leaf_node{{0, -2, 0, 1}, {}}},
        node{leaf_node{{-2, 0, 0, 1}, {}}},
        node{inner_node{
          {0, 0, 0, 1},
          {3},
          kdl::vec_from(
            node{leaf_node{{0, 0, 0, 0}, {1, 2}}},
            node{leaf_node{{1, 0, 0, 0}, {}}},
            node{leaf_node{{0, 1, 0, 0}, {}}},
            node{leaf_node{{1, 1, 0, 0}, {}}},
            node{leaf_node{{0, 0, 1, 0}, {}}},
            node{leaf_node{{1, 0, 1, 0}, {}}},
            node{leaf_node{{0, 1, 1, 0}, {}}},
            node{leaf_node{{1, 1, 1, 0}, {}}})}})}};

  SECTION("remove in insertion order")
  {
    tree.remove(1);
    CHECK(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{inner_node{
              {0, 0, 0, 1},
              {3},
              kdl::vec_from(
                node{leaf_node{{0, 0, 0, 0}, {2}}},
                node{leaf_node{{1, 0, 0, 0}, {}}},
                node{leaf_node{{0, 1, 0, 0}, {}}},
                node{leaf_node{{1, 1, 0, 0}, {}}},
                node{leaf_node{{0, 0, 1, 0}, {}}},
                node{leaf_node{{1, 0, 1, 0}, {}}},
                node{leaf_node{{0, 1, 1, 0}, {}}},
                node{leaf_node{{1, 1, 1, 0}, {}}})}})}});

    tree.remove(2);
    CHECK(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{0, 0, 0, 1}, {3}}})}});

    tree.remove(3);
    CHECK(tree == octree<double, int>{32.0});
  }

  SECTION("remove in inverse insertion order")
  {
    tree.remove(3);
    CHECK(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{0, 0, 0, 0}, {1, 2}}})}});

    tree.remove(2);
    CHECK(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{0, 0, 0, 0}, {1}}})}});

    tree.remove(1);
    CHECK(tree == octree<double, int>{32.0});
  }
}

TEST_CASE("octree.insert_duplicate")
{
  auto tree = octree<double, int>{32.0};

  tree.insert(vm::bbox3d{{0, 0, 0}, {2, 1, 1}}, 1);
  REQUIRE(tree.contains(1));

  CHECK_THROWS_AS(tree.insert(vm::bbox3d{{0, 0, 0}, {2, 1, 1}}, 1), NodeTreeException);

  CHECK(tree.contains(1));
  CHECK_FALSE(tree.empty());
}

TEST_CASE("octree.contains")
{
  auto tree = octree<double, int>{32.0};

  CHECK_FALSE(tree.contains(0));
  CHECK_FALSE(tree.contains(1));
  CHECK_FALSE(tree.contains(2));
  CHECK_FALSE(tree.contains(3));

  tree.insert(vm::bbox3d{{0, 0, 0}, {16, 16, 16}}, 1);
  tree.insert(vm::bbox3d{{16, 16, 16}, {32, 32, 32}}, 2);
  tree.insert(vm::bbox3d{{-16, -16, -16}, {0, 0, 0}}, 3);

  CHECK_FALSE(tree.contains(0));
  CHECK(tree.contains(1));
  CHECK(tree.contains(2));
  CHECK(tree.contains(3));
}

TEST_CASE("octree.find_intersectors-ray")
{
  auto tree = octree<double, int>{32.0};

  SECTION("empty tree")
  {
    CHECK(tree.find_intersectors(vm::ray3d{{0, 0, 0}, {1, 0, 0}}).empty());
  }

  SECTION("single node")
  {
    tree.insert({{32, 32, 32}, {64, 64, 64}}, 1);
    REQUIRE(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{1, 1, 1, 0}, {1}}})}});

    // the leaf that contains the data does not contain the ray origin
    CHECK(tree.find_intersectors(vm::ray3d{{48, 48, 0}, {0, 0, -1}}).empty());

    // the leaf that contains the data contains the ray origin
    CHECK(
      tree.find_intersectors(vm::ray3d{{48, 48, 48}, {0, 0, -1}}) == std::vector<int>{1});

    // the leaf that contains the data is hit by the ray
    CHECK(
      tree.find_intersectors(vm::ray3d{{48, 48, 0}, {0, 0, 1}}) == std::vector<int>{1});
  }
}

TEST_CASE("octree.find_intersectors-bbox")
{
  auto tree = octree<double, int>{32.0};

  SECTION("empty tree")
  {
    CHECK(tree.find_intersectors(vm::bbox3d{{0, 0, 0}, {1, 1, 1}}).empty());
  }

  SECTION("single node")
  {
    tree.insert({{32, 32, 32}, {64, 64, 64}}, 1);
    REQUIRE(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{1, 1, 1, 0}, {1}}})}});

    // non-intersection tests:

    // not touching
    CHECK(tree.find_intersectors(vm::bbox3d{{0, 0, 0}, {16, 16, 16}}).empty());

    // intersection tests:

    // share a corner
    CHECK(
      tree.find_intersectors(vm::bbox3d{{0, 0, 0}, {32, 32, 32}}) == std::vector<int>{1});

    // share a face
    CHECK(
      tree.find_intersectors(vm::bbox3d{{0, 32, 32}, {32, 32, 32}})
      == std::vector<int>{1});

    // fully inside leaf
    CHECK(
      tree.find_intersectors(vm::bbox3d{{40, 40, 40}, {48, 48, 48}})
      == std::vector<int>{1});

    // fully contains leaf
    CHECK(
      tree.find_intersectors(vm::bbox3d{{0, 0, 0}, {128, 128, 128}})
      == std::vector<int>{1});

    // partially contains leaf
    CHECK(
      tree.find_intersectors(vm::bbox3d{{48, 48, 48}, {128, 128, 128}})
      == std::vector<int>{1});
  }
}

TEST_CASE("octree.find_containers")
{
  auto tree = octree<double, int>{32.0};

  SECTION("empty tree")
  {
    CHECK(tree.find_containers({0, 0, 0}).empty());
  }

  SECTION("single node")
  {
    tree.insert({{32, 32, 32}, {64, 64, 64}}, 1);
    REQUIRE(
      tree
      == octree<double, int>{
        32.0,
        inner_node{
          {-2, -2, -2, 2},
          {},
          kdl::vec_from(
            node{leaf_node{{-2, -2, -2, 1}, {}}},
            node{leaf_node{{0, -2, -2, 1}, {}}},
            node{leaf_node{{-2, 0, -2, 1}, {}}},
            node{leaf_node{{0, 0, -2, 1}, {}}},
            node{leaf_node{{-2, -2, 0, 1}, {}}},
            node{leaf_node{{0, -2, 0, 1}, {}}},
            node{leaf_node{{-2, 0, 0, 1}, {}}},
            node{leaf_node{{1, 1, 1, 0}, {1}}})}});

    // the leaf that contains the data does not contain the point
    CHECK(tree.find_containers({48, 48, 0}).empty());

    // the leaf that contains the data contains the point
    CHECK(tree.find_containers({48, 48, 48}) == std::vector<int>{1});

    // the leaf that contains the data contains the point as its min corner
    CHECK(tree.find_containers({32, 32, 32}) == std::vector<int>{1});

    // the leaf that contains the data contains the point as its max corner
    CHECK(tree.find_containers({64, 64, 64}) == std::vector<int>{1});
  }
}
} // namespace TrenchBroom
