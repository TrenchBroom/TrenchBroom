/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2015 Eric Wasylishen

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

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/plane.h"
#include "vm/plane_io.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <array>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

namespace vm
{
TEST_CASE("plane")
{
  SECTION("constructor_default")
  {
    constexpr auto p = plane3f{};
    CER_CHECK(p.distance == 0.0f);
    CER_CHECK(p.normal == vec3f{0, 0, 0});
  }

  SECTION("constructor_convert")
  {
    constexpr auto p = plane3d{1.0, vec3d{0, 0, 1}};
    constexpr auto q = plane3f{p};
    CER_CHECK(q.distance == approx(1.0f));
    CER_CHECK(q.normal == approx(vec3f{0, 0, 1}));
  }

  SECTION("constructor_with_distance_and_normal")
  {
    constexpr auto d = 123.0f;
    constexpr auto n = normalize(vec3f{1.0f, 2.0f, 3.0f});
    constexpr auto p = plane3f{d, n};
    CER_CHECK(p.distance == approx(d));
    CER_CHECK(p.normal == approx(n));
  }

  SECTION("constructor_with_anchor_and_normal")
  {
    constexpr auto a = vec3f{-2038.034f, 0.0023f, 32.0f};
    constexpr auto n = normalize(vec3f{9.734f, -3.393f, 2.033f});
    constexpr auto p = plane3f{a, n};
    CER_CHECK(p.distance == approx(dot(a, n)));
    CER_CHECK(p.normal == approx(n));
  }

  SECTION("anchor")
  {
    constexpr auto a = vec3f{-2038.034f, 0.0023f, 32.0f};
    constexpr auto n = normalize(vec3f{9.734f, -3.393f, 2.033f});
    constexpr auto p = plane3f{a, n};
    CER_CHECK(p.anchor() == approx(p.distance * n));
  }

  SECTION("at")
  {
    constexpr auto a = vec3f{-2038.034f, 0.0023f, 32.0f};
    constexpr auto n = normalize(vec3f{9.734f, -3.393f, 2.033f});
    constexpr auto p = plane3f{a, n};
    constexpr auto point1 = vec2f{27.022f, -12.0123223f};

    CER_CHECK(
      p.at(point1, axis::x)
      == approx(
        (p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z())
        / p.normal[axis::x]));
    CER_CHECK(
      p.at(point1, axis::y)
      == approx(
        (p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z())
        / p.normal[axis::y]));
    CER_CHECK(
      p.at(point1, axis::z)
      == approx(
        (p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y())
        / p.normal[axis::z]));
  }

  SECTION("at_parallel_planes")
  {
    constexpr auto p1 = plane3f{10.0f, vec3f{1, 0, 0}};

    CER_CHECK(p1.at(vec2f{2.0f, 1.0f}, axis::x) == approx(p1.distance));
    CER_CHECK(p1.at(vec2f{22.0f, -34322.0232f}, axis::x) == approx(p1.distance));
    CER_CHECK(p1.at(vec2f{2.0f, 1.0f}, axis::y) == approx(0.0f));
    CER_CHECK(p1.at(vec2f{22.0f, -34322.0232f}, axis::y) == approx(0.0f));
    CER_CHECK(p1.at(vec2f{2.0f, 1.0f}, axis::z) == approx(0.0f));
    CER_CHECK(p1.at(vec2f{22.0f, -34322.0232f}, axis::z) == approx(0.0f));
  }

  SECTION("xyz_at")
  {
    constexpr auto a = vec3f{-2038.034f, 0.0023f, 32.0f};
    constexpr auto n = normalize(vec3f{9.734f, -3.393f, 2.033f});
    constexpr auto p = plane3f{a, n};
    constexpr auto point1 = vec2f{27.022f, -12.0123223f};

    CER_CHECK(p.xAt(point1) == approx(p.at(point1, axis::x)));
    CER_CHECK(p.yAt(point1) == approx(p.at(point1, axis::y)));
    CER_CHECK(p.zAt(point1) == approx(p.at(point1, axis::z)));
  }

  SECTION("point_distance")
  {
    constexpr auto a = vec3f{-2038.034f, 0.0023f, 32.0f};
    constexpr auto n = normalize(vec3f{9.734f, -3.393f, 2.033f});
    constexpr auto p = plane3f{a, n};
    constexpr auto point = vec3f{1.0f, -32.37873f, 32.0f};
    CER_CHECK(p.point_distance(point) == dot(point, p.normal) - p.distance);
  }

  SECTION("point_status")
  {
    constexpr auto p = plane3f{10.0f, vec3f{0, 0, 1}};
    CER_CHECK(p.point_status(vec3f{0.0f, 0.0f, 11.0f}) == plane_status::above);
    CER_CHECK(p.point_status(vec3f{0.0f, 0.0f, 9.0f}) == plane_status::below);
    CER_CHECK(p.point_status(vec3f{0.0f, 0.0f, 10.0f}) == plane_status::inside);
  }

  SECTION("flip")
  {
    constexpr auto p = plane3f{10.0f, vec3f{0, 0, 1}};
    CER_CHECK(p.flip() == plane3f{-10.0f, vec3f{0, 0, -1}});
  }

  SECTION("transform")
  {
    constexpr auto p = plane3d{vec3d{1, 1, 1}, vec3d{0, 0, 1}};
    constexpr auto tm = translation_matrix(vec3d{1, 1, 1});
    CER_CHECK(is_unit(p.normal, vm::Cd::almost_zero()));

    const auto rm =
      rotation_matrix(to_radians(15.0), to_radians(20.0), to_radians(-12.0));
    const auto rt = p.transform(rm * tm);
    CHECK(rt.point_status(rm * tm * p.anchor()) == plane_status::inside);
    CHECK(rt.normal == approx(rm * p.normal));

    constexpr auto sm = scaling_matrix(vec3d{2.0, 0.5, 3.0});
    constexpr auto st = p.transform(sm * tm);
    CER_CHECK(st.point_status(sm * tm * p.anchor()) == plane_status::inside);
    CER_CHECK(st.normal == approx(normalize(sm * p.normal)));

    // a point above the plane stays above it when the transformation mirrors it
    constexpr auto oblique = plane3d{vec3d{1, 1, 1}, normalize(vec3d{1, 2, 3})};
    constexpr auto mirror = scaling_matrix(vec3d{-1, 1, 1});
    CER_CHECK(
      oblique.transform(mirror).point_status(mirror * (oblique.anchor() + oblique.normal))
      == plane_status::above);

    // points on the plane stay on it when the transformation scales non-uniformly
    constexpr auto scaled = oblique.transform(sm);
    CER_CHECK(
      scaled.point_status(sm * (oblique.anchor() + vec3d{2, -1, 0}))
      == plane_status::inside);

    // points on the plane stay on it when the transformation scales non-uniformly and
    // shears it
    constexpr auto skew =
      shear_matrix(0.5, 0.0, 0.0, -0.25, 0.0, 0.0) * scaling_matrix(vec3d{2, 1, 3});
    constexpr auto skewed = oblique.transform(skew);
    CER_CHECK(
      skewed.point_status(skew * (oblique.anchor() + vec3d{2, -1, 0}))
      == plane_status::inside);
    CER_CHECK(
      skewed.point_status(skew * (oblique.anchor() + vec3d{3, 0, -1}))
      == plane_status::inside);
    CER_CHECK(
      skewed.point_status(skew * (oblique.anchor() + oblique.normal))
      == plane_status::above);
  }

  SECTION("project_point")
  {
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_point(vec3d{0, 0, 10})
      == approx(vec3d{0, 0, 0}));
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_point(vec3d{1, 2, 10})
      == approx(vec3d{1, 2, 0}));
    CER_CHECK(
      plane3d{0.0, normalize(vec3d{1, 1, 1})}.project_point(vec3d{10, 10, 10})
      == approx(vec3d{0, 0, 0}));
  }

  SECTION("project_point_direction")
  {
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_point(vec3d{0, 0, 10}, vec3d{0, 0, 1})
      == approx(vec3d{0, 0, 0}));
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_point(vec3d{1, 2, 10}, vec3d{0, 0, 1})
      == approx(vec3d{1, 2, 0}));
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_point(
        vec3d{10, 10, 10}, normalize(vec3d{1, 1, 1}))
      == approx(vec3d{0, 0, 0}));
  }

  SECTION("project_vector")
  {
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_vector(vec3d{1, 1, 1})
      == approx(vec3d{1, 1, 0}));
    CER_CHECK(
      plane3d{1.0, vec3d{0, 0, 1}}.project_vector(vec3d{1, 1, 1})
      == approx(vec3d{1, 1, 0}));
  }

  SECTION("project_vector_direction")
  {
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_vector(vec3d{1, 1, 1}, vec3d{0, 0, 1})
      == approx(vec3d{1, 1, 0}));
    CER_CHECK(
      plane3d{1.0, vec3d{0, 0, 1}}.project_vector(vec3d{1, 1, 1}, vec3d{0, 0, 1})
      == approx(vec3d{1, 1, 0}));
    CER_CHECK(
      plane3d{0.0, vec3d{0, 0, 1}}.project_vector(
        vec3d{1, 1, 1}, normalize(vec3d{1, 1, -1}))
      == approx(vec3d{2, 2, 0}));
  }

  SECTION("is_equal")
  {
    CER_CHECK(is_equal(
      plane3f{0.0f, vec3f{1, 0, 0}},
      plane3f{0.0f, vec3f{1, 0, 0}},
      constants<float>::almost_zero()));
    CER_CHECK(is_equal(
      plane3f{0.0f, vec3f{0, 1, 0}},
      plane3f{0.0f, vec3f{0, 1, 0}},
      constants<float>::almost_zero()));
    CER_CHECK(is_equal(
      plane3f{0.0f, vec3f{0, 0, 1}},
      plane3f{0.0f, vec3f{0, 0, 1}},
      constants<float>::almost_zero()));
    CER_CHECK_FALSE(is_equal(
      plane3f{0.0f, vec3f{1, 0, 0}},
      plane3f{0.0f, vec3f{-1, 0, 0}},
      constants<float>::almost_zero()));
    CER_CHECK_FALSE(is_equal(
      plane3f{0.0f, vec3f{1, 0, 0}},
      plane3f{0.0f, vec3f{0, 1, 0}},
      constants<float>::almost_zero()));
  }

  SECTION("operator_equal")
  {
    CER_CHECK(plane3d{} == plane3d{});
    CER_CHECK(plane3d{10.0, vec3d{0, 0, 1}} == plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK_FALSE(plane3d{20.0, vec3d{0, 0, 1}} == plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK_FALSE(plane3d{10.0, vec3d{0, 0, -1}} == plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK_FALSE(
      plane3d{10.0, normalize(vec3d{1, 1, 1})} == plane3d{10.0, vec3d{0, 0, 1}});
  }

  SECTION("operator_not_equal")
  {
    CER_CHECK_FALSE(plane3d{} != plane3d{});
    CER_CHECK_FALSE(plane3d{10.0, vec3d{0, 0, 1}} != plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK(plane3d{20.0, vec3d{0, 0, 1}} != plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK(plane3d{10.0, vec3d{0, 0, -1}} != plane3d{10.0, vec3d{0, 0, 1}});
    CER_CHECK(plane3d{10.0, normalize(vec3d{1, 1, 1})} != plane3d{10.0, vec3d{0, 0, 1}});
  }

  SECTION("plane_normal")
  {
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, vec3d{0, 1, 0}, vec3d{1, 0, 0}) == vec3d{0, 0, 1});
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, normalize(vec3d{1, 1, 0}), vec3d{1, 0, 0})
      == approx(vec3d{0, 0, 1}));
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, vec3d{0, 0, 0}, vec3d{1, 0, 0}) == std::nullopt);
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, vec3d{1, 0, 0}, vec3d{1, 0, 0}) == std::nullopt);
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, vec3d{-1, 0, 0}, vec3d{1, 0, 0}) == std::nullopt);
    CER_CHECK(
      plane_normal(vec3d{0, 0, 0}, vec3d{0, 0, 0}, vec3d{1, 0, 0}) == std::nullopt);
  }

  SECTION("from_points")
  {
    constexpr auto epsilon = constants<float>::point_status_epsilon();

    {
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f}, vec3f{1.0f, 0.0f, 0.0f}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(plane->normal == approx(vec3f{0, 0, 1}));
      CER_CHECK(plane->distance == approx(0.0f));
    }

    {
      // right angle, short vectors
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, epsilon, 0.0f}, vec3f{epsilon, 0.0f, 0.0f}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(plane->normal == approx(vec3f{0, 0, 1}));
      CER_CHECK(plane->distance == approx(0.0f));
    }

    {
      // plane point vectors at a 45 degree angle, short vectors
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f},
        vec3f{epsilon, epsilon, 0.0f},
        vec3f{epsilon, 0.0f, 0.0f}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(plane->normal == approx(vec3f{0, 0, 1}));
      CER_CHECK(plane->distance == approx(0.0f));
    }

    {
      // horizontal plane at z=length units above the origin
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, epsilon},
        vec3f{0.0f, epsilon, epsilon},
        vec3f{epsilon, 0.0f, epsilon}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(plane->normal == approx(vec3f{0, 0, 1}));
      CER_CHECK(plane->distance == approx(epsilon));
    }

    {
      // small angle (triangle 1000 units wide, length units tall)
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f},
        vec3f{1000.0f, epsilon, 0.0f},
        vec3f{1000.0f, 0.0f, 0.0f}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(plane->normal == approx(vec3f{0, 0, 1}));
      CER_CHECK(plane->distance == approx(0.0f));
    }

    {
      // small angle
      constexpr auto points = std::array{
        vec3f{224.0f, -400.0f, 1648.0f},
        vec3f{304.0f, -432.0f, 1248.0f + epsilon},
        vec3f{304.0f, -432.0f, 1248.0f}};

      constexpr auto plane = from_points(std::begin(points), std::end(points));
      CER_CHECK(plane);
      CER_CHECK(length(plane->normal) == approx(1.0f));
    }

    {
      // too-small angle (triangle 1000 units wide, length/100 units tall)
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f},
        vec3f{1000.0f, epsilon / 100.0f, 0.0f},
        vec3f{1000.0f, 0.0f, 0.0f}};

      CER_CHECK(from_points(std::begin(points), std::end(points)) == std::nullopt);
    }

    {
      // all zero
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 0.0f, 0.0f}};

      CER_CHECK(from_points(std::begin(points), std::end(points)) == std::nullopt);
    }

    {
      // same direction, short vectors
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f},
        vec3f{2 * epsilon, 0.0f, 0.0f},
        vec3f{epsilon, 0.0f, 0.0f}};

      CER_CHECK(from_points(std::begin(points), std::end(points)) == std::nullopt);
    }

    {
      // opposite, short vectors
      constexpr auto points = std::array{
        vec3f{0.0f, 0.0f, 0.0f}, vec3f{-epsilon, 0.0f, 0.0f}, vec3f{epsilon, 0.0f, 0.0f}};

      CER_CHECK(from_points(std::begin(points), std::end(points)) == std::nullopt);
    }
  }

  SECTION("horizontal_plane")
  {
    constexpr auto position = vec3f{322.0f, -122.2392f, 34.0f};
    constexpr auto p = horizontal_plane(position);
    CER_CHECK(p.point_status(position) == plane_status::inside)
    CER_CHECK(p.normal == approx(vec3f{0, 0, 1}));
  }

  SECTION("orthogonal_plane")
  {
    const auto position = vec3f{322.0f, -122.2392f, 34.0f};
    const auto direction = normalize(vec3f{1.0f, 2.0f, -3.0f});
    const auto p = orthogonal_plane(position, direction);
    CHECK(p.point_status(position) == plane_status::inside);
    CHECK(p.normal == approx(direction));
  }

  SECTION("aligned_orthogonal_plane")
  {
    constexpr auto position = vec3f{322.0f, -122.2392f, 34.0f};
    constexpr auto direction = normalize(vec3f{1.0f, 2.0f, -3.0f});
    constexpr auto p = aligned_orthogonal_plane(position, direction);
    CER_CHECK(p.point_status(position) == plane_status::inside)
    CER_CHECK(p.normal == approx(vec3f{0, 0, -1}));
  }

  SECTION("stream_insertion")
  {
    std::stringstream str;
    str << plane3d{10.0, vec3d{0, 0, 1}};
    CHECK(str.str() == "{ normal: (0 0 1), distance: 10 }");
  }
}
} // namespace vm
