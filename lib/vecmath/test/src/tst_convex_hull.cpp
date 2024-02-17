/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "vecmath/convex_hull.h"
#include "vecmath/forward.h"
#include "vecmath/vec.h"

#include <vector>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("convex_hull.convex_hull_simple")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(8.0, 8.0, 0.0);
  const vm::vec3d p3(8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);

  const std::vector<vm::vec3d> hull = vm::convex_hull<double>(points);
  CHECK(hull.size() == 4u);
  CHECK(hull[0] == p3);
  CHECK(hull[1] == p2);
  CHECK(hull[2] == p4);
  CHECK(hull[3] == p1);
}

TEST_CASE("convex_hull.convex_hull_simple_with_internal_point")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(8.0, 8.0, 0.0);
  const vm::vec3d p3(8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);
  const vm::vec3d p5(4.0, 4.0, 0.0);

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  points.push_back(p5);

  const std::vector<vm::vec3d> hull = vm::convex_hull<double>(points);
  CHECK(hull.size() == 4u);
  CHECK(hull[0] == p3);
  CHECK(hull[1] == p2);
  CHECK(hull[2] == p4);
  CHECK(hull[3] == p1);
}

TEST_CASE("convex_hull.convex_hull_simple_with_point_on_line")
{
  const vm::vec3d p1(0.0, 0.0, 0.0);
  const vm::vec3d p2(8.0, 8.0, 0.0);
  const vm::vec3d p3(8.0, 0.0, 0.0);
  const vm::vec3d p4(0.0, 8.0, 0.0);
  const vm::vec3d p5(4.0, 0.0, 0.0);

  std::vector<vm::vec3d> points;
  points.push_back(p1);
  points.push_back(p2);
  points.push_back(p3);
  points.push_back(p4);
  points.push_back(p5);

  const std::vector<vm::vec3d> hull = vm::convex_hull<double>(points);
  CHECK(hull.size() == 4u);
  CHECK(hull[0] == p3);
  CHECK(hull[1] == p2);
  CHECK(hull[2] == p4);
  CHECK(hull[3] == p1);
}
} // namespace vm
