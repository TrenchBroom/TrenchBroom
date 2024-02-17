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

#include "vm/approx.h"
#include "vm/forward.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/mat_io.h"
#include "vm/polygon.h"
#include "vm/vec.h"
#include "vm/vec_ext.h"
#include "vm/vec_io.h"

#include <iterator>
#include <vector>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("polygon.constructor_default")
{
  CHECK(polygon3d().vertices().size() == 0u);
}

TEST_CASE("polygon.constructor_with_initializer_list")
{
  using Catch::Matchers::Equals;

  const auto expected = std::vector<vec3d>{
    vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)};
  CHECK_THAT(
    polygon3d({vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)})
      .vertices(),
    Equals(expected));
}

TEST_CASE("polygon.construct_with_vertex_list")
{
  using Catch::Matchers::Equals;

  const auto vertices = std::vector<vec3d>{
    vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)};
  const auto expected = std::vector<vec3d>{
    vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)};
  CHECK_THAT(polygon3d(vertices).vertices(), Equals(expected));
}

TEST_CASE("polygon.has_vertex")
{
  const auto vertices = std::vector<vec3d>{
    vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)};
  const auto p = polygon3d(vertices);

  for (const auto& v : vertices)
  {
    CHECK(p.hasVertex(v));
  }

  CHECK_FALSE(p.hasVertex(vec3d::one()));
}

TEST_CASE("polygon.vertex_count")
{
  const auto vertices = std::vector<vec3d>{
    vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)};
  const auto p = polygon3d(vertices);
  CHECK(p.vertexCount() == 4u);
  CHECK(polygon3d().vertexCount() == 0u);
}

TEST_CASE("polygon.vertices")
{
  using Catch::Matchers::Equals;

  const auto vertices = std::vector<vec3d>{
    vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)};
  const auto p = polygon3d(vertices);
  CHECK_THAT(p.vertices(), Equals(vertices));
}

TEST_CASE("polygon.center")
{
  const auto vertices = std::vector<vec3d>{
    vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)};
  const auto p = polygon3d(vertices);
  CHECK(p.center() == approx(vec3d::zero()));
}

TEST_CASE("polygon.invert")
{
  using Catch::Matchers::Equals;

  const auto p = polygon3d({
    vec3d(-1, -1, 0),
    vec3d(-1, +1, 0),
    vec3d(+1, +1, 0),
    vec3d(+1, -1, 0),
  });

  const auto exp = std::vector<vec3d>{
    vec3d(-1, -1, 0),
    vec3d(+1, -1, 0),
    vec3d(+1, +1, 0),
    vec3d(-1, +1, 0),
  };

  CHECK_THAT(p.invert().vertices(), Equals(exp));
}

TEST_CASE("polygon.translate")
{
  using Catch::Matchers::Equals;

  const auto p =
    polygon3d({vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)});
  const auto t = vec3d(1, 2, 3);
  CHECK_THAT(p.translate(t).vertices(), Equals(p.vertices() + t));
}

TEST_CASE("polygon.transform")
{
  using Catch::Matchers::Equals;

  const auto p =
    polygon3d({vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)});
  const auto t = rotation_matrix(to_radians(14.0), to_radians(13.0), to_radians(44.0))
                 * translation_matrix(vec3d(1, 2, 3));
  const auto exp = polygon3d(t * p.vertices());
  CHECK_THAT(p.transform(t).vertices(), Equals(exp.vertices()));
}

TEST_CASE("polygon.get_vertices")
{
  using Catch::Matchers::Equals;

  const auto p1 =
    polygon3d({vec3d(+1, +1, 0), vec3d(+1, -1, 0), vec3d(-1, -1, 0), vec3d(-1, +1, 0)});
  const auto p2 = p1.translate(vec3d(1, 2, 3));
  const auto ps = std::vector<polygon3d>{p1, p2};

  auto exp = p1.vertices();
  exp.insert(std::end(exp), std::begin(p2), std::end(p2));

  auto act = std::vector<vec3d>();
  polygon3d::get_vertices(std::begin(ps), std::end(ps), std::back_inserter(act));

  CHECK_THAT(act, Equals(exp));
}

TEST_CASE("polygon.compare")
{
  CHECK(compare(polygon3d(), polygon3d()) == 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)},
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    == 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)},
      polygon3d{vec3d(-2, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)},
      2.0)
    == 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)},
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    < 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)},
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    > 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)},
      polygon3d{vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    < 0);

  CHECK(
    compare(
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)},
      polygon3d{
        vec3d(+1, -1, 0),
        vec3d(-1, +1, 0),
      })
    < 0);

  CHECK(
    compare(
      polygon3d{vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)},
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    > 0);

  CHECK(
    compare(
      polygon3d{
        vec3d(+1, -1, 0),
        vec3d(-1, +1, 0),
      },
      polygon3d{vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    > 0);
}

TEST_CASE("polygon.operator_equal")
{
  CHECK(polygon3d() == polygon3d());

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    == polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    == polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    == polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    == polygon3d(
      {vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    == polygon3d({
      vec3d(+1, -1, 0),
      vec3d(-1, +1, 0),
    }));

  CHECK_FALSE(
    polygon3d({vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    == polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));

  CHECK_FALSE(
    polygon3d({
      vec3d(+1, -1, 0),
      vec3d(-1, +1, 0),
    })
    == polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));
}

TEST_CASE("polygon.operator_not_equal")
{
  CHECK_FALSE(polygon3d() != polygon3d());

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    != polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    != polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    != polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    != polygon3d(
      {vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)})
    != polygon3d({
      vec3d(+1, -1, 0),
      vec3d(-1, +1, 0),
    }));

  CHECK(
    polygon3d({vec3d(+1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    != polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));

  CHECK(
    polygon3d({
      vec3d(+1, -1, 0),
      vec3d(-1, +1, 0),
    })
    != polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}));
}

TEST_CASE("polygon.operator_less_than")
{
  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    < polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}) < polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    < polygon3d({
      vec3d(-1, -1, 0),
      vec3d(-1, +1, 0),
      vec3d(+1, +1, 0),
    }));
}

TEST_CASE("polygon.operator_less_than_or_equal")
{
  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    <= polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}) <= polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    <= polygon3d({
      vec3d(-1, -1, 0),
      vec3d(-1, +1, 0),
      vec3d(+1, +1, 0),
    }));
}

TEST_CASE("polygon.operator_greater_than")
{
  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    > polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}) > polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    > polygon3d({
      vec3d(-1, -1, 0),
      vec3d(-1, +1, 0),
      vec3d(+1, +1, 0),
    }));
}

TEST_CASE("polygon.operator_greater_than_or_equal")
{
  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    >= polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK_FALSE(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0)}) >= polygon3d(
      {vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)}));

  CHECK(
    polygon3d({vec3d(-1, -1, 0), vec3d(-1, +1, 0), vec3d(+1, +1, 0), vec3d(+1, -1, 0)})
    >= polygon3d({
      vec3d(-1, -1, 0),
      vec3d(-1, +1, 0),
      vec3d(+1, +1, 0),
    }));
}

TEST_CASE("polygon.compare_unoriented_empty_polygon")
{
  polygon3d p1{};
  CHECK(compareUnoriented(p1, polygon3d{}) == 0);
  CHECK(compareUnoriented(p1, polygon3d{vec3d::zero()}) == -1);

  polygon3d p2{vec3d::zero()};
  CHECK(compareUnoriented(p2, p1) == +1);
  CHECK(compareUnoriented(p2, polygon3d{vec3d::zero()}) == 0);
}

TEST_CASE("polygon.testBackwardComparePolygonWithOneVertex")
{
  polygon3d p2{vec3d::zero()};
  CHECK(compareUnoriented(p2, polygon3d{vec3d::zero()}) == 0);
  CHECK(compareUnoriented(p2, polygon3d{vec3d::zero(), vec3d::zero()}) == -1);
}

TEST_CASE("polygon.compare_unoriented")
{
  polygon3d p1{
    vec3d(-1.0, -1.0, 0.0),
    vec3d(+1.0, -1.0, 0.0),
    vec3d(+1.0, +1.0, 0.0),
    vec3d(-1.0, +1.0, 0.0),
  };
  polygon3d p2{
    vec3d(-1.0, +1.0, 0.0),
    vec3d(+1.0, +1.0, 0.0),
    vec3d(+1.0, -1.0, 0.0),
    vec3d(-1.0, -1.0, 0.0),
  };
  CHECK(compareUnoriented(p1, p1) == 0);
  CHECK(compareUnoriented(p1, p2) == 0);
  CHECK(compareUnoriented(p2, p1) == 0);
  CHECK(compareUnoriented(p2, p2) == 0);
}
} // namespace vm
