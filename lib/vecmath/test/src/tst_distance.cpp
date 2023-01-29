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

#include <vecmath/abstract_line.h>
#include <vecmath/approx.h>
#include <vecmath/constants.h>
#include <vecmath/distance.h>
#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("distance.distance_ray_point")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_z());

  // point is behind ray
  CER_CHECK(squared_distance(ray, vec3f(-1.0f, -1.0f, -1.0f)).position == approx(0.0f));
  CER_CHECK(squared_distance(ray, vec3f(-1.0f, -1.0f, -1.0f)).distance == approx(3.0f))

  // point is in front of ray
  CER_CHECK(squared_distance(ray, vec3f(1.0f, 1.0f, 1.0f)).position == approx(1.0f))
  CER_CHECK(squared_distance(ray, vec3f(1.0f, 1.0f, 1.0f)).distance == approx(2.0f))

  // point is in front of ray
  CER_CHECK(
    squared_distance(ray, vec3f(1.0f, 1.0f, 2.0f)).position
    == approx(2.0f)) // NOTE: position is not squared
  CER_CHECK(squared_distance(ray, vec3f(1.0f, 1.0f, 2.0f)).distance == approx(2.0f))

  // point is on ray
  CER_CHECK(squared_distance(ray, vec3f(0.0f, 0.0f, 1.0f)).position == approx(1.0f))
  CER_CHECK(squared_distance(ray, vec3f(0.0f, 0.0f, 1.0f)).distance == approx(0.0f))
}

TEST_CASE("distance.distance_segment_point")
{
  constexpr auto segment = segment3f(vec3f::zero(), vec3f::pos_z());

  // point is below start
  CHECK(squared_distance(segment, vec3f(-1.0f, -1.0f, -1.0f)).position == approx(0.0f));
  CHECK(squared_distance(segment, vec3f(-1.0f, -1.0f, -1.0f)).distance == approx(3.0f));

  // point is within segment
  CHECK(squared_distance(segment, vec3f(1.0f, 1.0f, 1.0f)).position == approx(1.0f));
  CHECK(squared_distance(segment, vec3f(1.0f, 1.0f, 1.0f)).distance == approx(2.0f));

  // point is above end
  CHECK(squared_distance(segment, vec3f(0.0f, 0.0f, 2.0f)).position == approx(1.0f));
  CHECK(squared_distance(segment, vec3f(0.0f, 0.0f, 2.0f)).distance == approx(1.0f));
}

template <class A, class B>
static void assert_line_distance_invariants(const A& lhs, const B& rhs)
{
  const auto linedist = squared_distance(lhs, rhs);

  const vec3f lhsPoint = point_at_distance(lhs, linedist.position1);
  const vec3f rhsPoint = point_at_distance(rhs, linedist.position2);
  const float dist = squared_distance(lhsPoint, rhsPoint);

  CHECK(linedist.distance == approx(dist));
}

template <class A, class B>
static void line_distance_extra_tests(const A& lhs, const B& rhs)
{
  assert_line_distance_invariants(lhs, rhs);
  assert_line_distance_invariants(
    lhs.transform(mat4x4f::mirror_x()), rhs.transform(mat4x4f::mirror_x()));
  assert_line_distance_invariants(
    lhs.transform(mat4x4f::mirror_y()), rhs.transform(mat4x4f::mirror_y()));
  assert_line_distance_invariants(
    lhs.transform(mat4x4f::mirror_z()), rhs.transform(mat4x4f::mirror_z()));
}

TEST_CASE("distance.distance_ray_segment")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_z());
  line_distance<float> segDist;
  segment3f seg;

  // segment overlapping ray
  seg = segment3f(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f));
  CHECK(segDist.distance == approx(0.0f));
  CHECK(segDist.position2 == approx(0.0f));
  line_distance_extra_tests(ray, seg);

  // segment parallel to ray with XY offset
  seg = segment3f(vec3f(1.0f, 1.0f, 0.0f), vec3f(1.0f, 1.0f, 1.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f));
  CHECK(segDist.distance == approx(2.0f));
  CHECK(segDist.position2 == approx(0.0f));
  line_distance_extra_tests(ray, seg);

  // segment parallel, in front of ray
  seg = segment3f(vec3f(1.0f, 1.0f, 5.0f), vec3f(1.0f, 1.0f, 6.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(5.0f));
  CHECK(segDist.distance == approx(2.0f));
  CHECK(segDist.position2 == approx(0.0f));
  line_distance_extra_tests(ray, seg);

  // segment parallel, behind ray
  seg = segment3f(vec3f(1.0f, 1.0f, -2.0f), vec3f(1.0f, 1.0f, -1.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f));
  CHECK(segDist.distance == approx(3.0f));
  CHECK(segDist.position2 == approx(1.0f));
  line_distance_extra_tests(ray, seg);

  // segment parallel, in front of ray, degenerate segment
  seg = segment3f(vec3f(1.0f, 1.0f, 5.0f), vec3f(1.0f, 1.0f, 5.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(5.0f));
  CHECK(segDist.distance == approx(2.0f));
  CHECK(segDist.position2 == approx(0.0f));
  // assert_line_distance_squared_invariants(ray, seg); // can't do this check on a
  // degenerate segment

  // segment parallel, behind ray, degenerate segment
  seg = segment3f(vec3f(1.0f, 1.0f, -1.0f), vec3f(1.0f, 1.0f, -1.0f));
  segDist = squared_distance(ray, seg);
  CHECK(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f));
  CHECK(segDist.distance == approx(3.0f));
  CHECK(segDist.position2 == approx(0.0f));
  // assert_line_distance_squared_invariants(ray, seg); // can't do this check on a
  // degenerate segment

  // segment perpendicular to ray
  //   R = ray
  //   c = closest point
  //   s = segment start s
  //   e = segment end
  //
  // ^  s
  // |    c
  // Y  R   e
  //
  //    X ->
  //
  seg = segment3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
  segDist = squared_distance(ray, seg);
  CHECK_FALSE(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f)); // the ray origin is the closest point on R
  CHECK(segDist.distance == approx(0.5f));  // R to c distance, squared
  CHECK(segDist.position2 == approx(0.70710677f)); // s to c distance
  line_distance_extra_tests(ray, seg);

  // same as previous, but segment is below ray start
  seg = segment3f(vec3f(1.0f, 0.0f, -1.0f), vec3f(0.0f, 1.0f, -1.0f));
  segDist = squared_distance(ray, seg);
  CHECK_FALSE(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f)); // the ray origin is the closest point on R
  CHECK(segDist.distance == approx(1.5f));  // R to c distance, squared
  CHECK(segDist.position2 == approx(0.70710677f)); // s to c distance
  line_distance_extra_tests(ray, seg);

  seg = segment3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(2.0f, -1.0f, 0.0f));
  segDist = squared_distance(ray, seg);
  CHECK_FALSE(segDist.parallel);
  CHECK(segDist.position1 == approx(0.0f));
  CHECK(segDist.distance == approx(1.0f));
  CHECK(segDist.position2 == approx(0.0f));
  line_distance_extra_tests(ray, seg);

  seg = segment3f(vec3f(-1.0f, 1.5f, 2.0f), vec3f(+1.0f, 1.5f, 2.0f));
  segDist = distance(ray, seg);
  CHECK_FALSE(segDist.parallel);
  CHECK(segDist.position1 == approx(2.0f));
  CHECK(segDist.distance == approx(1.5f));
  CHECK(segDist.position2 == approx(1.0f));
  line_distance_extra_tests(ray, seg);
}

TEST_CASE("distance.distance_ray_ray")
{
  constexpr auto ray1 = ray3f(vec3f::zero(), vec3f::pos_z());

  // parallel, ray with itself
  constexpr auto segDist1 = squared_distance(ray1, ray1);
  CER_CHECK(segDist1.parallel);
  CER_CHECK(segDist1.position1 == approx(0.0f));
  CER_CHECK(segDist1.distance == approx(0.0f));
  CER_CHECK(segDist1.position2 == approx(0.0f));
  line_distance_extra_tests(ray1, ray1);

  // parallel, XY offset
  constexpr auto segDist2Ray = ray3f(vec3f(1.0f, 1.0, 0.0f), vec3f::pos_z());
  constexpr auto segDist2 = squared_distance(ray1, segDist2Ray);
  CER_CHECK(segDist2.parallel);
  CER_CHECK(segDist2.position1 == approx(0.0f));
  CER_CHECK(segDist2.distance == approx(2.0f));
  CER_CHECK(segDist2.position2 == approx(0.0f));
  line_distance_extra_tests(ray1, segDist2Ray);

  constexpr auto segDist3Ray =
    ray3f(vec3f(1.0f, 1.0f, 0.0f), normalize_c(vec3f(1.0f, 1.0f, 1.0f)));
  constexpr auto segDist3 = squared_distance(ray1, segDist3Ray);
  CER_CHECK_FALSE(segDist3.parallel);
  CHECK(segDist3.position1 == approx(0.0f));
  CHECK(segDist3.distance == approx(2.0f));
  CHECK(segDist3.position2 == approx(0.0f));
  line_distance_extra_tests(ray1, segDist3Ray);

  constexpr auto segDist4Ray =
    ray3f(vec3f(1.0f, 1.0f, 0.0f), normalize_c(vec3f(-1.0f, -1.0f, +1.0f)));
  constexpr auto segDist4 = squared_distance(ray1, segDist4Ray);
  CER_CHECK_FALSE(segDist4.parallel);
  CER_CHECK(segDist4.position1 == approx(1.0f));
  CER_CHECK(segDist4.distance == approx(0.0f));
  CER_CHECK(segDist4.position2 == approx(length_c(vec3f(1.0f, 1.0f, 1.0f))));
  line_distance_extra_tests(ray1, segDist4Ray);

  constexpr auto segDist5Ray =
    ray3f(vec3f(1.0f, 1.0f, 0.0f), normalize_c(vec3f(-1.0f, 0.0f, +1.0f)));
  constexpr auto segDist5 = squared_distance(ray1, segDist5Ray);
  CER_CHECK_FALSE(segDist5.parallel);
  CER_CHECK(segDist5.position1 == approx(1.0f));
  CER_CHECK(segDist5.distance == approx(1.0f));
  CER_CHECK(segDist5.position2 == approx(length_c(vec3f(1.0f, 0.0f, 1.0f))));
  line_distance_extra_tests(ray1, segDist5Ray);

  // parallel, second ray is in front
  constexpr auto segDist6Ray = ray3f(vec3f(1.0f, 1.0, 1.0f), vec3f::pos_z());
  constexpr auto segDist6 = squared_distance(ray1, segDist6Ray);
  CER_CHECK(segDist6.parallel);
  CER_CHECK(segDist6.position1 == approx(1.0f));
  CER_CHECK(segDist6.distance == approx(2.0f));
  CER_CHECK(segDist6.position2 == approx(0.0f));
  line_distance_extra_tests(ray1, segDist6Ray);

  // parallel, second ray is behind
  constexpr auto segDist7Ray = ray3f(vec3f(1.0f, 1.0, -1.0f), vec3f::pos_z());
  constexpr auto segDist7 = squared_distance(ray1, segDist7Ray);
  CER_CHECK(segDist7.parallel);
  CER_CHECK(segDist7.position1 == approx(0.0f));
  CER_CHECK(segDist7.distance == approx(2.0f));
  CER_CHECK(segDist7.position2 == approx(1.0f));
  line_distance_extra_tests(ray1, segDist7Ray);
}

TEST_CASE("distance.distance_ray_line")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_z());

  constexpr auto segDist1Line = line3f(vec3f(0.0f, 0.0f, 0.0f), vec3f::pos_z());
  constexpr auto segDist1 = squared_distance(ray, segDist1Line);
  CER_CHECK(segDist1.parallel);
  CER_CHECK(segDist1.position1 == approx(0.0f));
  CER_CHECK(segDist1.distance == approx(0.0f));
  CER_CHECK(segDist1.position2 == approx(0.0f));
  line_distance_extra_tests(ray, segDist1Line);

  constexpr auto segDist2Line = line3f(vec3f(1.0f, 1.0f, 0.0f), vec3f::pos_z());
  constexpr auto segDist2 = squared_distance(ray, segDist2Line);
  CER_CHECK(segDist2.parallel);
  CER_CHECK(segDist2.position1 == approx(0.0f));
  CER_CHECK(segDist2.distance == approx(2.0f));
  CER_CHECK(segDist2.position2 == approx(0.0f));
  line_distance_extra_tests(ray, segDist2Line);

  constexpr auto segDist3Line =
    line3f(vec3f(1.0f, 0.0f, 0.0f), normalize_c(vec3f(-1.0f, 1.0f, 0.0f)));
  constexpr auto segDist3 = squared_distance(ray, segDist3Line);
  CER_CHECK_FALSE(segDist3.parallel);
  CER_CHECK(segDist3.position1 == approx(0.0f));
  CER_CHECK(segDist3.distance == approx(0.5f));
  CER_CHECK(segDist3.position2 == approx(sqrt_c(2.0f) / 2.0f));
  line_distance_extra_tests(ray, segDist3Line);

  constexpr auto segDist4Line =
    line3f(vec3f(1.0f, 0.0f, 0.0f), normalize_c(vec3f(1.0f, -1.0f, 0.0f)));
  constexpr auto segDist4 = squared_distance(ray, segDist4Line);
  CER_CHECK_FALSE(segDist4.parallel);
  CER_CHECK(segDist4.position1 == approx(0.0f));
  CER_CHECK(segDist4.distance == approx(0.5f));
  CER_CHECK(segDist4.position2 == approx(-sqrt_c(2.0f) / 2.0f));
  line_distance_extra_tests(ray, segDist4Line);

  // parallel, ray is in front of line
  constexpr auto segDist5Line = line3f(vec3f(1.0f, 1.0f, -1.0f), vec3f::pos_z());
  constexpr auto segDist5 = squared_distance(ray, segDist5Line);
  CER_CHECK(segDist5.parallel);
  CER_CHECK(segDist5.position1 == approx(0.0f));
  CER_CHECK(segDist5.distance == approx(2.0f));
  CER_CHECK(
    segDist5.position2 == approx(1.0f)); // we use the ray origin as the closest point, so
                                         // this is the corresponding position on the line
  line_distance_extra_tests(ray, segDist5Line);

  // parallel, ray is behind line
  constexpr auto segDist6Line = line3f(vec3f(1.0f, 1.0f, 1.0f), vec3f::pos_z());
  constexpr auto segDist6 = squared_distance(ray, segDist6Line);
  CER_CHECK(segDist6.parallel);
  CER_CHECK(segDist6.position1 == approx(0.0f));
  CER_CHECK(segDist6.distance == approx(2.0f));
  CER_CHECK(
    segDist6.position2
    == approx(-1.0f)); // we use the ray origin as the closest point, so
                       // this is the corresponding position on the line
  line_distance_extra_tests(ray, segDist6Line);
}
} // namespace vm
