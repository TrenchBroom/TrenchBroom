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

#include <vecmath/approx.h>
#include <vecmath/constants.h>
#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/scalar.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "test_utils.h"

#include <iterator>
#include <vector>

#include <catch2/catch_all.hpp>

namespace vm
{
TEST_CASE("segment.constructor_default")
{
  constexpr auto s = segment3d();
  CER_CHECK(s.start() == vec3d::zero());
  CER_CHECK(s.end() == vec3d::zero());
}

TEST_CASE("segment.constructor_convert")
{
  constexpr auto start = vec3d(2, 0, 0);
  constexpr auto end = vec3d(3, 0, 0);
  constexpr auto s = segment3d(start, end);
  constexpr auto t = segment3f(s);
  CER_CHECK(t.start() == approx(vec3f(start)));
  CER_CHECK(t.end() == approx(vec3f(end)));
}

TEST_CASE("segment.constructor_with_points")
{
  constexpr auto start = vec3d(3, 0, 0);
  constexpr auto end = vec3d(2, 0, 0);
  constexpr auto s = segment3d(start, end);
  CER_CHECK(s.start() == end);
  CER_CHECK(s.end() == start);
}

TEST_CASE("segment.get_origin")
{
  constexpr auto s = segment3d(vec3d(3, 0, 0), vec3d(2, 0, 0));
  CER_CHECK(s.get_origin() == s.start());
}

TEST_CASE("segment.get_direction")
{
  const auto start = vec3d(3, 0, 0);
  const auto end = vec3d(2, 0, 0);
  const auto s = segment3d(start, end);
  CHECK(s.get_direction() == normalize(s.end() - s.start()));
}

TEST_CASE("segment.length")
{
  const auto s = segment3d(vec3d(4, 0, 0), vec3d(2, 0, 0));
  CHECK(s.length() == approx(2.0));
}

TEST_CASE("segment.length_c")
{
  constexpr auto s = segment3d(vec3d(4, 0, 0), vec3d(2, 0, 0));
  CER_CHECK(s.length_c() == approx(2.0));
}

TEST_CASE("segment.squared_length")
{
  constexpr auto s = segment3d(vec3d(4, 0, 0), vec3d(2, 0, 0));
  CER_CHECK(s.squared_length() == approx(4.0));
}

TEST_CASE("segment.contains1")
{
  constexpr auto z = vec3d::zero();
  constexpr auto o = vec3d(1.0, 0.0, 0.0);
  constexpr auto h = vec3d(0.5, 0.0, 0.0);
  constexpr auto n = vec3d(0.5, 1.0, 0.0);

  CHECK(segment3d(z, o).contains(z, Cd::almost_zero()));
  CHECK(segment3d(z, o).contains(h, Cd::almost_zero()));
  CHECK(segment3d(z, o).contains(o, Cd::almost_zero()));
  CHECK_FALSE(segment3d(z, o).contains(n, Cd::almost_zero()));
}

TEST_CASE("segment.contains2")
{
  const auto z = vec3d(-64.0, -64.0, 0.0);
  const auto o = vec3d(0.0, +64.0, 0.0);

  CHECK(segment3d(z, o).contains(z, Cd::almost_zero()));
  CHECK(segment3d(z, o).contains(o, Cd::almost_zero()));
}

TEST_CASE("segment.transform")
{
  constexpr auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
  constexpr auto sm = scaling_matrix(vec3d(2, 0.5, 3));
  constexpr auto tm = translation_matrix(vec3d::one());

  constexpr auto st = s.transform(sm * tm);
  CER_CHECK(st.start() == approx(sm * tm * s.start()));
  CER_CHECK(st.end() == approx(sm * tm * s.end()));
}

TEST_CASE("segment.translate")
{
  constexpr auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
  constexpr auto st = s.translate(vec3d::one());
  CER_CHECK(st.start() == approx(s.start() + vec3d::one()));
  CER_CHECK(st.end() == approx(s.end() + vec3d::one()));
}

TEST_CASE("segment.center")
{
  constexpr auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
  CER_CHECK(s.center() == approx(vec3d(2, 0, 0)));
}

TEST_CASE("segment.direction")
{
  const auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
  CHECK(s.direction() == approx(vec3d::pos_x()));
}

TEST_CASE("segment.get_vertices")
{
  const auto l = std::vector<segment3d>{
    segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0)), segment3d(vec3d(2, 0, 0), vec3d(6, 0, 0))};

  auto v = std::vector<vec3d>();
  segment3d::get_vertices(std::begin(l), std::end(l), std::back_inserter(v));

  const auto e =
    std::vector<vec3d>{vec3d(0, 0, 0), vec3d(4, 0, 0), vec3d(2, 0, 0), vec3d(6, 0, 0)};

  CHECK(v == e);
}

TEST_CASE("segment.compare")
{
  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)))
    == 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)))
    < 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)))
    < 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)))
    > 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)))
    > 0);

  // with large epsilon
  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      2.0)
    == 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
      2.0)
    == 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
      2.0)
    == 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
      2.0)
    == 0);

  CER_CHECK(
    compare(
      segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
      segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)))
    > 0);
}

TEST_CASE("segment.is_equal")
{
  CER_CHECK(is_equal(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
    0.0));
  CER_CHECK_FALSE(is_equal(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
    0.0));
  CER_CHECK(is_equal(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
    2.0));
}

TEST_CASE("segment.operator_equal")
{
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    == segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    == segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}

TEST_CASE("segment.operator_not_equal")
{
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    != segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    != segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}

TEST_CASE("segment.operator_less_than")
{
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3))
    < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3))
    < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3))
    < segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    < segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}

TEST_CASE("segment.operator_less_than_or_equal")
{
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3))
    <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3))
    <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3))
    <= segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    <= segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}

TEST_CASE("segment.operator_greater_than")
{
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3))
    > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3))
    > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3))
    > segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    > segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}

TEST_CASE("segment.operator_greater_than_or_equal")
{
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3))
    >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK(
    segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3))
    >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3))
    >= segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
  CER_CHECK_FALSE(
    segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
    >= segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
}
} // namespace vm
