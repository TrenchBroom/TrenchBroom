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

#include "test_utils.h"

#include "vecmath/bbox.h"
#include "vecmath/bbox_io.h"
#include "vecmath/forward.h"
#include "vecmath/mat_ext.h"
#include "vecmath/vec.h"

#include <sstream>
#include <vector>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("bbox.constructor_default")
{
  constexpr auto bounds = bbox3f();
  CER_CHECK(bounds.min == vec3f::zero());
  CER_CHECK(bounds.max == vec3f::zero());
}

TEST_CASE("bbox.constructor_with_min_max_points")
{
  constexpr auto min = vec3f(-1, -2, -3);
  constexpr auto max = vec3f(1, 2, 3);

  constexpr auto bounds = bbox3f(min, max);
  CER_CHECK(bounds.min == min);
  CER_CHECK(bounds.max == max);
}

TEST_CASE("bbox.constructor_with_min_max_values")
{
  constexpr auto min = -16.f;
  constexpr auto max = +32.0f;

  constexpr auto bounds = bbox3f(min, max);
  CER_CHECK(bounds.min == vec3f::fill(min));
  CER_CHECK(bounds.max == vec3f::fill(max));
}

TEST_CASE("bbox.constructor_with_minmax_value")
{
  constexpr auto minMax = 16.f;

  constexpr auto bounds = bbox3f(minMax);
  CER_CHECK(-vec3f::fill(minMax) == bounds.min);
  CER_CHECK(+vec3f::fill(minMax) == bounds.max);
}

TEST_CASE("bbox.merge_all")
{
  constexpr auto points = std::array<vec3d, 6>{
    vec3d(-32, -16, -8),
    vec3d(0, -4, -4),
    vec3d(+4, +8, -16),
    vec3d(+32, +16, -4),
    vec3d(+16, +4, -8),
    vec3d(+24, +32, +4)};

  constexpr auto min =
    vm::min(points[0], points[1], points[2], points[3], points[4], points[5]);
  constexpr auto max =
    vm::max(points[0], points[1], points[2], points[3], points[4], points[5]);

  constexpr auto merged = bbox3d::merge_all(std::begin(points), std::end(points));
  CER_CHECK(merged.min == min);
  CER_CHECK(merged.max == max);
}

TEST_CASE("bbox.is_valid")
{
  CER_CHECK(bbox3d::is_valid(vec3d::zero(), vec3d::zero()));
  CER_CHECK(bbox3d::is_valid(vec3d(-1, -1, -1), vec3d(+1, +1, +1)));
  CER_CHECK_FALSE(bbox3d::is_valid(vec3d(+1, -1, -1), vec3d(-1, +1, +1)));
  CER_CHECK_FALSE(bbox3d::is_valid(vec3d(-1, +1, -1), vec3d(+1, -1, +1)));
  CER_CHECK_FALSE(bbox3d::is_valid(vec3d(-1, -1, +1), vec3d(+1, +1, -1)));
}

TEST_CASE("bbox.is_empty")
{
  CER_CHECK(bbox3d().is_empty())
  CER_CHECK_FALSE(bbox3d(1.0).is_empty())
  CER_CHECK(bbox3d(vec3d(-1, 0, -1), vec3d(+1, 0, +1)).is_empty());
}

TEST_CASE("bbox.center")
{
  constexpr auto min = vec3f(-1, -2, -3);
  constexpr auto max = vec3f(1, 4, 5);
  constexpr auto bounds = bbox3f(min, max);

  CER_CHECK(bounds.center() == vec3f(0, 1, 1));
}

TEST_CASE("bbox.size")
{
  constexpr auto min = vec3f(-1, -2, -3);
  constexpr auto max = vec3f(1, 3, 5);
  constexpr auto bounds = bbox3f(min, max);

  CER_CHECK(bounds.size() == vec3f(2, 5, 8));
}

TEST_CASE("bbox.volume")
{
  CER_CHECK(bbox3d().volume() == 0.0);
  CER_CHECK(bbox3d(2.0).volume() == 4.0 * 4.0 * 4.0);
}

TEST_CASE("bbox.contains_point")
{
  constexpr auto bounds = bbox3f(vec3f(-12, -3, 4), vec3f(8, 9, 8));
  CER_CHECK(bounds.contains(vec3f(2, 1, 7)));
  CER_CHECK(bounds.contains(vec3f(-12, -3, 7)));
  CER_CHECK_FALSE(bounds.contains(vec3f(-13, -3, 7)));
}

TEST_CASE("bbox.contains_bbox")
{
  constexpr auto bounds1 = bbox3f(vec3f(-12, -3, 4), vec3f(8, 9, 8));
  constexpr auto bounds2 = bbox3f(vec3f(-10, -2, 5), vec3f(7, 8, 7));
  constexpr auto bounds3 = bbox3f(vec3f(-13, -2, 5), vec3f(7, 8, 7));
  CER_CHECK(bounds1.contains(bounds1))
  CER_CHECK(bounds1.contains(bounds2))
  CER_CHECK_FALSE(bounds1.contains(bounds3))
}

TEST_CASE("bbox.encloses")
{
  constexpr auto bounds1 = bbox3f(vec3f(-12, -3, 4), vec3f(8, 9, 8));
  constexpr auto bounds2 = bbox3f(vec3f(-10, -2, 5), vec3f(7, 8, 7));
  constexpr auto bounds3 = bbox3f(vec3f(-10, -3, 5), vec3f(7, 8, 7));
  CER_CHECK_FALSE(bounds1.encloses(bounds1))
  CER_CHECK(bounds1.encloses(bounds2))
  CER_CHECK_FALSE(bounds1.encloses(bounds3))
}

TEST_CASE("bbox.intersects")
{
  constexpr auto bounds1 = bbox3f(vec3f(-12.0f, -3.0f, 4.0f), vec3f(8.0f, 9.0f, 8.0f));
  constexpr auto bounds2 = bbox3f(vec3f(-10.0f, -2.0f, 5.0f), vec3f(7.0f, 8.0f, 7.0f));
  constexpr auto bounds3 = bbox3f(vec3f(-13.0f, -2.0f, 5.0f), vec3f(7.0f, 8.0f, 7.0f));
  constexpr auto bounds4 =
    bbox3f(vec3f(-15.0f, 10.0f, 9.0f), vec3f(-13.0f, 12.0f, 10.0f));
  constexpr auto bounds5 =
    bbox3f(vec3f(-15.0f, 10.0f, 9.0f), vec3f(-12.0f, 12.0f, 10.0f));
  CER_CHECK(bounds1.intersects(bounds1))
  CER_CHECK(bounds1.intersects(bounds2))
  CER_CHECK(bounds1.intersects(bounds3))
  CER_CHECK_FALSE(bounds1.intersects(bounds4))
  CER_CHECK_FALSE(bounds1.intersects(bounds5))
}

TEST_CASE("bbox.constrain")
{
  constexpr auto bounds = bbox3d(1024.0);
  CER_CHECK(bounds.constrain(vec3d::zero()) == vec3d::zero());
  CER_CHECK(bounds.constrain(bounds.min) == bounds.min);
  CER_CHECK(bounds.constrain(bounds.min + vec3d::neg_x()) == bounds.min);
  CER_CHECK(bounds.constrain(bounds.min + vec3d::neg_y()) == bounds.min);
  CER_CHECK(bounds.constrain(bounds.min + vec3d::neg_z()) == bounds.min);
  CER_CHECK(bounds.constrain(bounds.max + vec3d::pos_x()) == bounds.max);
  CER_CHECK(bounds.constrain(bounds.max + vec3d::pos_y()) == bounds.max);
  CER_CHECK(bounds.constrain(bounds.max + vec3d::pos_z()) == bounds.max);
}

TEST_CASE("bbox.corner")
{
  constexpr auto min = vec3f(-1.0f, -2.0f, -3.0f);
  constexpr auto max = vec3f(1.0f, 3.0f, 5.0f);
  constexpr auto bounds = bbox3f(min, max);

  CER_CHECK(
    bounds.corner(bbox3f::Corner::min, bbox3f::Corner::min, bbox3f::Corner::min)
    == vec3f(-1.0f, -2.0f, -3.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::min, bbox3f::Corner::min, bbox3f::Corner::max)
    == vec3f(-1.0f, -2.0f, 5.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::min, bbox3f::Corner::max, bbox3f::Corner::min)
    == vec3f(-1.0f, 3.0f, -3.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::min, bbox3f::Corner::max, bbox3f::Corner::max)
    == vec3f(-1.0f, 3.0f, 5.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::max, bbox3f::Corner::min, bbox3f::Corner::min)
    == vec3f(1.0f, -2.0f, -3.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::max, bbox3f::Corner::min, bbox3f::Corner::max)
    == vec3f(1.0f, -2.0f, 5.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::max, bbox3f::Corner::max, bbox3f::Corner::min)
    == vec3f(1.0f, 3.0f, -3.0f));
  CER_CHECK(
    bounds.corner(bbox3f::Corner::max, bbox3f::Corner::max, bbox3f::Corner::max)
    == vec3f(1.0f, 3.0f, 5.0f));
}

TEST_CASE("bbox.relative_position")
{
  constexpr auto bounds = bbox3f(vec3f(-12.0f, -3.0f, 4.0f), vec3f(8.0f, 9.0f, 8.0f));
  constexpr auto point1 = vec3f(-1.0f, 0.0f, 0.0f);
  constexpr auto pos1 = bounds.relative_position(point1);
  CER_CHECK(pos1[0] == bbox3f::Range::within);
  CER_CHECK(pos1[1] == bbox3f::Range::within);
  CER_CHECK(pos1[2] == bbox3f::Range::less);
}

TEST_CASE("bbox.expand")
{
  constexpr auto bounds = bbox3f(vec3f(-12.0f, -3.0f, 4.0f), vec3f(8.0f, 9.0f, 8.0f));
  constexpr auto expanded =
    bbox3f(vec3f(-14.0f, -5.0f, 2.0f), vec3f(10.0f, 11.0f, 10.0f));
  CER_CHECK(bounds.expand(2.0f) == expanded);
}

TEST_CASE("bbox.translate")
{
  constexpr auto bounds = bbox3f(vec3f(-12.0f, -3.0f, 4.0f), vec3f(8.0f, 9.0f, 8.0f));
  constexpr auto translated =
    bbox3f(vec3f(-10.0f, -4.0f, 1.0f), vec3f(10.0f, 8.0f, 5.0f));
  CER_CHECK(bounds.translate(vec3f(2.0f, -1.0f, -3.0f)) == translated);
}

TEST_CASE("bbox.transform")
{
  constexpr auto bounds = bbox3d(-2.0, +10.0);
  constexpr auto transform = scaling_matrix(vec3d(0.5, 2, 3));
  constexpr auto points = bounds.vertices();
  constexpr auto transformedPoints = transform * points;
  constexpr auto transformed =
    bbox3d::merge_all(std::begin(transformedPoints), std::end(transformedPoints));
  CER_CHECK(bounds.transform(transform).min == transformed.min);
  CER_CHECK(bounds.transform(transform).max == transformed.max);
}

TEST_CASE("bbox.operator_equal")
{
  constexpr auto min = vec3f(-1, -2, -3);
  constexpr auto max = vec3f(1, 2, 3);
  constexpr auto bounds1 = bbox3f(min, max);
  constexpr auto bounds2 = bbox3f(min, max);
  constexpr auto bounds3 = bbox3f(22.0f);

  CER_CHECK(bounds1 == bounds2);
  CER_CHECK(bounds1 != bounds3);
}

TEST_CASE("bbox.operator_not_equal")
{
  constexpr auto min = vec3f(-1, -2, -3);
  constexpr auto max = vec3f(1, 2, 3);
  constexpr auto bounds1 = bbox3f(min, max);
  constexpr auto bounds2 = bbox3f(min, max);
  constexpr auto bounds3 = bbox3f(22.0f);

  CER_CHECK(bounds1 == bounds2);
  CER_CHECK(bounds1 != bounds3);
}

TEST_CASE("bbox.is_equal")
{
  constexpr auto bounds1 = bbox3f(vec3f(-12, -3, 4), vec3f(7, 8, 9));
  constexpr auto bounds2 = bbox3f(vec3f(-12, -3, 4), vec3f(7, 8, 10));

  CER_CHECK(is_equal(bounds1, bounds1, 0.0f));
  CER_CHECK(is_equal(bounds2, bounds2, 0.0f));
  CER_CHECK_FALSE(is_equal(bounds1, bounds2, 0.0f));
  CER_CHECK_FALSE(is_equal(bounds1, bounds2, 0.999f));
  CER_CHECK(is_equal(bounds1, bounds2, 1.0f));
}

TEST_CASE("bbox.repair")
{
  auto actual = bbox3d(0.0);
  actual.min = vec3d(+8, -8, +8);
  actual.max = vec3d(-8, +8, -8);
  CHECK(repair(actual) == bbox3d(8.0));
}

TEST_CASE("bbox.merge_with_bbox")
{
  constexpr auto bounds1 = bbox3f(vec3f(-12, -3, 4), vec3f(7, 8, 9));
  constexpr auto bounds2 = bbox3f(vec3f(-10, -5, 3), vec3f(9, 9, 5));
  constexpr auto merged = bbox3f(vec3f(-12, -5, 3), vec3f(9, 9, 9));

  CER_CHECK(merge(bounds1, bounds2) == merged);
}

TEST_CASE("bbox.merge_with_vec")
{
  constexpr auto bounds = bbox3f(vec3f(-12, -3, 4), vec3f(7, 8, 9));
  constexpr auto vec = vec3f(-10, -6, 10);
  constexpr auto merged = bbox3f(vec3f(-12, -6, 4), vec3f(7, 8, 10));

  CER_CHECK(merge(bounds, vec) == merged);
}

TEST_CASE("bbox.intersect")
{
  constexpr auto b1 = bbox3d(vec3d(-10, -10, -10), vec3d(10, 10, 10));
  constexpr auto b2 = bbox3d(vec3d(-5, -5, -5), vec3d(20, 5, 10));
  constexpr auto b3 = bbox3d(vec3d(12, 12, 12), vec3d(15, 15, 15));

  CER_CHECK(intersect(b1, b2) == bbox3d(vec3d(-5, -5, -5), vec3d(10, 5, 10)));
  CER_CHECK(intersect(b2, b1) == bbox3d(vec3d(-5, -5, -5), vec3d(10, 5, 10)));
  CER_CHECK(intersect(b1, b3) == bbox3d(vec3d(0, 0, 0), vec3d(0, 0, 0)));
  CER_CHECK(intersect(b3, b1) == bbox3d(vec3d(0, 0, 0), vec3d(0, 0, 0)));
  CER_CHECK(intersect(b2, b3) == bbox3d(vec3d(0, 0, 0), vec3d(0, 0, 0)));
}

TEST_CASE("bbox.stream_insertion")
{
  std::stringstream str;
  str << bbox3d(vec3d(-10, -10, -10), vec3d(10, 10, 10));
  CHECK(str.str() == "{ min: (-10 -10 -10), max: (10 10 10) }");
}

TEST_CASE("bbox_builder.empty")
{
  constexpr auto builder = vm::bbox3f::builder();
  CER_CHECK_FALSE(builder.initialized())
}

TEST_CASE("bbox_builder.add_one_point")
{
  const auto point = vm::vec3f(10, 20, 30);

  vm::bbox3f::builder builder;
  builder.add(point);

  CHECK(builder.initialized());
  CHECK(builder.bounds() == vm::bbox3f(point, point));
}

TEST_CASE("bbox_builder.twoPoints")
{
  const auto point1 = vm::vec3f(10, 20, 30);
  const auto point2 = vm::vec3f(100, 200, 300);

  vm::bbox3f::builder builder;
  builder.add(point1);
  builder.add(point2);

  CHECK(builder.initialized());
  CHECK(builder.bounds() == vm::bbox3f(point1, point2));
}

TEST_CASE("bbox_builder.twoPointsReverseOrder")
{
  const auto point1 = vm::vec3f(10, 20, 30);
  const auto point2 = vm::vec3f(100, 200, 300);

  vm::bbox3f::builder builder;
  builder.add(point2);
  builder.add(point1);

  CHECK(builder.initialized());
  CHECK(builder.bounds() == vm::bbox3f(point1, point2));
}

TEST_CASE("bbox_builder.add_one_bbox")
{
  const auto bbox = vm::bbox3f(vec3f(2, 3, 4), vec3f(5, 6, 7));

  vm::bbox3f::builder builder;
  builder.add(bbox);

  CHECK(builder.initialized());
  CHECK(builder.bounds() == bbox);
}
} // namespace vm
