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
#include "vm/scalar.h"

#include <array>
#include <limits>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("scalar.identity")
{
  constexpr auto id = identity();
  CER_CHECK(id(1) == 1);
  CER_CHECK(id(-1) == -1);
  CER_CHECK(id(1.234) == 1.234);
}

TEST_CASE("scalar.is_nan")
{
  CER_CHECK(is_nan(std::numeric_limits<double>::quiet_NaN()));
  CER_CHECK(is_nan(std::numeric_limits<float>::quiet_NaN()));
  CER_CHECK_FALSE(is_nan(1.0));
  CER_CHECK_FALSE(is_nan(1.0f));
}

TEST_CASE("scalar.is_inf")
{
  CER_CHECK(is_inf(+std::numeric_limits<double>::infinity()));
  CER_CHECK(is_inf(-std::numeric_limits<double>::infinity()));
  CER_CHECK(is_inf(+std::numeric_limits<float>::infinity()));
  CER_CHECK(is_inf(-std::numeric_limits<float>::infinity()));
  CER_CHECK_FALSE(is_inf(0.0));
  CER_CHECK_FALSE(is_inf(0.0f));
}

TEST_CASE("scalar.nan")
{
  CER_CHECK(is_nan(nan<double>()));
  CER_CHECK(is_nan(nan<float>()));
}

TEST_CASE("scalar.min")
{
  CER_CHECK(min(+1.0, +1.0) == +1.0);
  CER_CHECK(min(+1.0, +2.0) == +1.0);
  CER_CHECK(min(+2.0, +1.0) == +1.0);
  CER_CHECK(min(-1.0, +2.0) == -1.0);
  CER_CHECK(min(+1.0, -2.0) == -2.0);
  CER_CHECK(min(-1.0, -2.0) == -2.0);
  CER_CHECK(min(-1.0, -2.0, -3.0) == -3.0);
  CER_CHECK(min(-1.0, -3.0, -2.0) == -3.0);
  CER_CHECK(min(-2.0, -1.0, -3.0) == -3.0);
  CER_CHECK(min(-2.0, -3.0, -1.0) == -3.0);
  CER_CHECK(min(-3.0, -1.0, -2.0) == -3.0);
  CER_CHECK(min(-3.0, -2.0, -1.0) == -3.0);
}

TEST_CASE("scalar.max")
{
  CER_CHECK(max(+1.0, +1.0) == +1.0);
  CER_CHECK(max(+1.0, +2.0) == +2.0);
  CER_CHECK(max(+2.0, +1.0) == +2.0);
  CER_CHECK(max(-1.0, +2.0) == +2.0);
  CER_CHECK(max(+1.0, -2.0) == +1.0);
  CER_CHECK(max(-1.0, -2.0) == -1.0);
  CER_CHECK(max(-1.0, -2.0, -3.0) == -1.0);
  CER_CHECK(max(-1.0, -3.0, -2.0) == -1.0);
  CER_CHECK(max(-2.0, -1.0, -3.0) == -1.0);
  CER_CHECK(max(-2.0, -3.0, -1.0) == -1.0);
  CER_CHECK(max(-3.0, -1.0, -2.0) == -1.0);
  CER_CHECK(max(-3.0, -2.0, -1.0) == -1.0);
}

TEST_CASE("scalar.abs_min")
{
  CER_CHECK(abs_min(+1.0, +1.0) == +1.0);
  CER_CHECK(abs_min(+1.0, +2.0) == +1.0);
  CER_CHECK(abs_min(+2.0, +1.0) == +1.0);
  CER_CHECK(abs_min(-1.0, +2.0) == -1.0);
  CER_CHECK(abs_min(+1.0, -2.0) == +1.0);
  CER_CHECK(abs_min(-1.0, -2.0) == -1.0);
  CER_CHECK(abs_min(+1.0, -2.0, +3.0) == +1.0);
}

TEST_CASE("scalar.abs_max")
{
  CER_CHECK(abs_max(+1.0, +1.0) == +1.0);
  CER_CHECK(abs_max(+1.0, +2.0) == +2.0);
  CER_CHECK(abs_max(+2.0, +1.0) == +2.0);
  CER_CHECK(abs_max(-1.0, +2.0) == +2.0);
  CER_CHECK(abs_max(+1.0, -2.0) == -2.0);
  CER_CHECK(abs_max(-1.0, -2.0) == -2.0);
  CER_CHECK(abs_max(-1.0, -2.0, -3.0) == -3.0);
}

TEST_CASE("scalar.safe_min")
{
  CER_CHECK(safe_min(+1.0, +1.0) == +1.0);
  CER_CHECK(safe_min(+1.0, +2.0) == +1.0);
  CER_CHECK(safe_min(+2.0, +1.0) == +1.0);
  CER_CHECK(safe_min(-1.0, +2.0) == -1.0);
  CER_CHECK(safe_min(+1.0, -2.0) == -2.0);
  CER_CHECK(safe_min(-1.0, -2.0) == -2.0);
  CER_CHECK(safe_min(-1.0, -2.0, -3.0) == -3.0);

  CER_CHECK(safe_min(+1.0, nan<double>()) == +1.0);
  CER_CHECK(safe_min(nan<double>(), -1.0) == -1.0);
  CER_CHECK(is_nan(safe_min(nan<double>(), nan<double>())));

  CER_CHECK(safe_min(nan<double>(), +1.0, -2.0) == -2.0)
  CER_CHECK(safe_min(+1.0, nan<double>(), -2.0) == -2.0)
  CER_CHECK(safe_min(+1.0, -2.0, nan<double>()) == -2.0)
  CER_CHECK(safe_min(+1.0, nan<double>(), nan<double>()) == +1.0)
  CER_CHECK(is_nan(safe_min(nan<double>(), nan<double>(), nan<double>())));
}

TEST_CASE("scalar.safe_max")
{
  CER_CHECK(safe_max(+1.0, +1.0) == +1.0);
  CER_CHECK(safe_max(+1.0, +2.0) == +2.0);
  CER_CHECK(safe_max(+2.0, +1.0) == +2.0);
  CER_CHECK(safe_max(-1.0, +2.0) == +2.0);
  CER_CHECK(safe_max(+1.0, -2.0) == +1.0);
  CER_CHECK(safe_max(-1.0, -2.0) == -1.0);

  CER_CHECK(safe_max(+1.0, nan<double>()) == +1.0);
  CER_CHECK(safe_max(nan<double>(), -1.0) == -1.0);
  CER_CHECK(is_nan(safe_max(nan<double>(), nan<double>())));

  CER_CHECK(safe_max(nan<double>(), +1.0, -2.0) == +1.0);
  CER_CHECK(safe_max(+1.0, nan<double>(), -2.0) == +1.0);
  CER_CHECK(safe_max(+1.0, -2.0, nan<double>()) == +1.0);
  CER_CHECK(safe_max(+1.0, nan<double>(), nan<double>()) == +1.0);
  CER_CHECK(is_nan(safe_max(nan<double>(), nan<double>(), nan<double>())));
}

TEST_CASE("scalar.abs_difference")
{
  CER_CHECK(abs_difference(+4, +7) == 3);
  CER_CHECK(abs_difference(+7, +4) == 3);
  CER_CHECK(abs_difference(+7, -1) == 6);
  CER_CHECK(abs_difference(-7, +1) == 6);
  CER_CHECK(abs_difference(-7, -1) == 6);
  CER_CHECK(abs_difference(7u, 1u) == 6u);
  CER_CHECK(abs_difference(1u, 7u) == 6u);
}

TEST_CASE("scalar.clamp")
{
  CER_CHECK(clamp(0.0, 0.0, +1.0) == 0.0);
  CER_CHECK(clamp(+1.0, 0.0, +1.0) == +1.0);
  CER_CHECK(clamp(-1.0, 0.0, +1.0) == 0.0);
  CER_CHECK(clamp(+2.0, 0.0, +1.0) == +1.0);
  CER_CHECK(clamp(+0.5, 0.0, +1.0) == +0.5);

  CER_CHECK(clamp(0.0, -1.0, 0.0) == 0.0);
  CER_CHECK(clamp(-1.0, -1.0, 0.0) == -1.0);
  CER_CHECK(clamp(+1.0, -1.0, 0.0) == 0.0);
  CER_CHECK(clamp(-2.0, -1.0, 0.0) == -1.0);
  CER_CHECK(clamp(-0.5, -1.0, 0.0) == -0.5);

  CER_CHECK(clamp(0.0, -1.0, +1.0) == 0.0);
  CER_CHECK(clamp(-1.0, -1.0, +1.0) == -1.0);
  CER_CHECK(clamp(+1.0, -1.0, +1.0) == +1.0);
  CER_CHECK(clamp(-2.0, -1.0, +1.0) == -1.0);
  CER_CHECK(clamp(+2.0, -1.0, +1.0) == +1.0);
}

TEST_CASE("scalar.sign")
{
  CER_CHECK(sign(-2) == -1);
  CER_CHECK(sign(-1) == -1);
  CER_CHECK(sign(0) == 0);
  CER_CHECK(sign(1) == +1);
  CER_CHECK(sign(2) == +1);
}

TEST_CASE("scalar.step")
{
  CER_CHECK(step(1, -1) == 0);
  CER_CHECK(step(1, 0) == 0);
  CER_CHECK(step(1, 1) == 1);
  CER_CHECK(step(1, 2) == 1);
}

TEST_CASE("scalar.smoothstep")
{
  CER_CHECK(smoothstep(0.0, 1.0, -1.0) == approx(0.0));
  CER_CHECK(smoothstep(0.0, 1.0, 0.0) == approx(0.0));
  CER_CHECK(smoothstep(0.0, 1.0, 0.25) == approx(0.15625));
  CER_CHECK(smoothstep(0.0, 1.0, 0.5) == approx(0.5));
  CER_CHECK(smoothstep(0.0, 1.0, 0.75) == approx(0.84375));
  CER_CHECK(smoothstep(0.0, 1.0, 1.0) == approx(1.0));
  CER_CHECK(smoothstep(0.0, 1.0, 2.0) == approx(1.0));
}

TEST_CASE("scalar.mod")
{
  CER_CHECK(mod(+4.0, +2.0) == approx(0.0));
  CER_CHECK(mod(+5.0, +2.0) == approx(+1.0));
  CER_CHECK(mod(-5.0, +2.0) == approx(-1.0));
  CER_CHECK(mod(+5.0, -2.0) == approx(+1.0));
  CER_CHECK(mod(-5.0, -2.0) == approx(-1.0));
  CER_CHECK(mod(+5.5, +2.0) == approx(+1.5));
}

TEST_CASE("scalar.floor")
{
  CER_CHECK(floor(-0.7) == approx(-1.0));
  CER_CHECK(floor(-0.5) == approx(-1.0));
  CER_CHECK(floor(-0.4) == approx(-1.0));
  CER_CHECK(floor(0.0) == approx(0.0));
  CER_CHECK(floor(0.4) == approx(0.0));
  CER_CHECK(floor(0.6) == approx(0.0));
  CER_CHECK(floor(1.0) == approx(1.0));
}

TEST_CASE("scalar.ceil")
{
  CER_CHECK(ceil(-1.1) == approx(-1.0));
  CER_CHECK(ceil(-0.7) == approx(0.0));
  CER_CHECK(ceil(-0.5) == approx(0.0));
  CER_CHECK(ceil(-0.4) == approx(0.0));
  CER_CHECK(ceil(0.0) == approx(0.0));
  CER_CHECK(ceil(0.4) == approx(1.0));
  CER_CHECK(ceil(0.6) == approx(1.0));
  CER_CHECK(ceil(1.0) == approx(1.0));
  CER_CHECK(ceil(1.1) == approx(2.0));
}

TEST_CASE("scalar.trunc")
{
  CER_CHECK(trunc(-1.1) == approx(-1.0));
  CER_CHECK(trunc(-0.7) == approx(0.0));
  CER_CHECK(trunc(-0.5) == approx(0.0));
  CER_CHECK(trunc(-0.4) == approx(0.0));
  CER_CHECK(trunc(0.0) == approx(0.0));
  CER_CHECK(trunc(0.4) == approx(0.0));
  CER_CHECK(trunc(0.6) == approx(0.0));
  CER_CHECK(trunc(1.0) == approx(+1.0));
  CER_CHECK(trunc(1.1) == approx(+1.0));
}

TEST_CASE("scalar.mix")
{
  CER_CHECK(mix(1.0, 2.0, 0.0) == approx(1.0));
  CER_CHECK(mix(1.0, 2.0, 1.0) == approx(2.0));
  CER_CHECK(mix(1.0, 2.0, 0.5) == approx(1.5));

  CER_CHECK(mix(-1.0, 2.0, 0.0) == approx(-1.0));
  CER_CHECK(mix(-1.0, 2.0, 1.0) == approx(+2.0));
  CER_CHECK(mix(-1.0, 2.0, 0.5) == approx(+0.5));

  CER_CHECK(mix(-1.0, -2.0, 0.0) == approx(-1.0));
  CER_CHECK(mix(-1.0, -2.0, 1.0) == approx(-2.0));
  CER_CHECK(mix(-1.0, -2.0, 0.5) == approx(-1.5));
}

TEST_CASE("scalar.fract")
{
  CER_CHECK(fract(-1.2) == approx(-0.2));
  CER_CHECK(fract(-1.0) == approx(0.0));
  CER_CHECK(fract(-0.7) == approx(-0.7));
  CER_CHECK(fract(0.0) == approx(0.0));
  CER_CHECK(fract(+0.7) == approx(+0.7));
  CER_CHECK(fract(+1.0) == approx(0.0));
  CER_CHECK(fract(+1.2) == approx(0.2));
}

TEST_CASE("scalar.round")
{
  CER_CHECK(round(-1.1) == approx(-1.0));
  CER_CHECK(round(-0.7) == approx(-1.0));
  CER_CHECK(round(-0.5) == approx(-1.0));
  CER_CHECK(round(-0.4) == approx(0.0));
  CER_CHECK(round(0.0) == approx(0.0));
  CER_CHECK(round(0.4) == approx(0.0));
  CER_CHECK(round(0.6) == approx(+1.0));
  CER_CHECK(round(1.0) == approx(+1.0));
  CER_CHECK(round(1.1) == approx(+1.0));
}

TEST_CASE("scalar.round_up")
{
  CER_CHECK(round_up(-1.1) == approx(-2.0));
  CER_CHECK(round_up(-0.7) == approx(-1.0));
  CER_CHECK(round_up(-0.5) == approx(-1.0));
  CER_CHECK(round_up(-0.4) == approx(-1.0));
  CER_CHECK(round_up(0.0) == approx(0.0));
  CER_CHECK(round_up(0.4) == approx(+1.0));
  CER_CHECK(round_up(0.6) == approx(+1.0));
  CER_CHECK(round_up(1.0) == approx(+1.0));
  CER_CHECK(round_up(1.1) == approx(+2.0));
}

TEST_CASE("scalar.round_down")
{
  CER_CHECK(round_down(-1.1) == approx(-1.0));
  CER_CHECK(round_down(-0.7) == approx(0.0));
  CER_CHECK(round_down(-0.5) == approx(0.0));
  CER_CHECK(round_down(-0.4) == approx(0.0));
  CER_CHECK(round_down(0.0) == approx(0.0));
  CER_CHECK(round_down(0.4) == approx(0.0));
  CER_CHECK(round_down(0.6) == approx(0.0));
  CER_CHECK(round_down(1.0) == approx(+1.0));
  CER_CHECK(round_down(1.1) == approx(+1.0));
}

TEST_CASE("scalar.snap")
{
  CER_CHECK(snap(0.0, 1.0) == approx(0.0));
  CER_CHECK(snap(+0.4, 1.0) == approx(0.0));
  CER_CHECK(snap(+0.5, 1.0) == approx(1.0));
  CER_CHECK(snap(+0.6, 1.0) == approx(1.0));
  CER_CHECK(snap(-0.4, 1.0) == approx(0.0));
  CER_CHECK(snap(-0.5, 1.0) == approx(-1.0));
  CER_CHECK(snap(-0.6, 1.0) == approx(-1.0));

  CER_CHECK(snap(+1.4, 1.0) == approx(1.0));
  CER_CHECK(snap(+1.5, 1.0) == approx(2.0));
  CER_CHECK(snap(+1.6, 1.0) == approx(2.0));
  CER_CHECK(snap(-1.4, 1.0) == approx(-1.0));
  CER_CHECK(snap(-1.5, 1.0) == approx(-2.0));
  CER_CHECK(snap(-1.6, 1.0) == approx(-2.0));

  CER_CHECK(snap(0.0, 2.0) == approx(0.0));
  CER_CHECK(snap(+0.4, 2.0) == approx(0.0));
  CER_CHECK(snap(+0.5, 2.0) == approx(0.0));
  CER_CHECK(snap(+0.6, 2.0) == approx(0.0));
  CER_CHECK(snap(-0.4, 2.0) == approx(0.0));
  CER_CHECK(snap(-0.5, 2.0) == approx(0.0));
  CER_CHECK(snap(-0.6, 2.0) == approx(0.0));

  CER_CHECK(snap(+1.4, 2.0) == approx(2.0));
  CER_CHECK(snap(+1.5, 2.0) == approx(2.0));
  CER_CHECK(snap(+1.6, 2.0) == approx(2.0));
  CER_CHECK(snap(-1.4, 2.0) == approx(-2.0));
  CER_CHECK(snap(-1.5, 2.0) == approx(-2.0));
  CER_CHECK(snap(-1.6, 2.0) == approx(-2.0));
}

TEST_CASE("scalar.snapUp")
{
  CER_CHECK(snapUp(0.0, 1.0) == approx(0.0));
  CER_CHECK(snapUp(+0.4, 1.0) == approx(1.0));
  CER_CHECK(snapUp(+0.5, 1.0) == approx(1.0));
  CER_CHECK(snapUp(+0.6, 1.0) == approx(1.0));
  CER_CHECK(snapUp(-0.4, 1.0) == approx(-1.0));
  CER_CHECK(snapUp(-0.5, 1.0) == approx(-1.0));
  CER_CHECK(snapUp(-0.6, 1.0) == approx(-1.0));

  CER_CHECK(snapUp(+1.4, 1.0) == approx(2.0));
  CER_CHECK(snapUp(+1.5, 1.0) == approx(2.0));
  CER_CHECK(snapUp(+1.6, 1.0) == approx(2.0));
  CER_CHECK(snapUp(-1.4, 1.0) == approx(-2.0));
  CER_CHECK(snapUp(-1.5, 1.0) == approx(-2.0));
  CER_CHECK(snapUp(-1.6, 1.0) == approx(-2.0));

  CER_CHECK(snapUp(0.0, 2.0) == approx(0.0));
  CER_CHECK(snapUp(+0.4, 2.0) == approx(2.0));
  CER_CHECK(snapUp(+0.5, 2.0) == approx(2.0));
  CER_CHECK(snapUp(+0.6, 2.0) == approx(2.0));
  CER_CHECK(snapUp(-0.4, 2.0) == approx(-2.0));
  CER_CHECK(snapUp(-0.5, 2.0) == approx(-2.0));
  CER_CHECK(snapUp(-0.6, 2.0) == approx(-2.0));

  CER_CHECK(snapUp(+1.4, 2.0) == approx(2.0));
  CER_CHECK(snapUp(+1.5, 2.0) == approx(2.0));
  CER_CHECK(snapUp(+1.6, 2.0) == approx(2.0));
  CER_CHECK(snapUp(-1.4, 2.0) == approx(-2.0));
  CER_CHECK(snapUp(-1.5, 2.0) == approx(-2.0));
  CER_CHECK(snapUp(-1.6, 2.0) == approx(-2.0));
}

TEST_CASE("scalar.snapDown")
{
  CER_CHECK(snapDown(0.0, 1.0) == approx(0.0));
  CER_CHECK(snapDown(+0.4, 1.0) == approx(0.0));
  CER_CHECK(snapDown(+0.5, 1.0) == approx(0.0));
  CER_CHECK(snapDown(+0.6, 1.0) == approx(0.0));
  CER_CHECK(snapDown(-0.4, 1.0) == approx(0.0));
  CER_CHECK(snapDown(-0.5, 1.0) == approx(0.0));
  CER_CHECK(snapDown(-0.6, 1.0) == approx(0.0));

  CER_CHECK(snapDown(+1.4, 1.0) == approx(1.0));
  CER_CHECK(snapDown(+1.5, 1.0) == approx(1.0));
  CER_CHECK(snapDown(+1.6, 1.0) == approx(1.0));
  CER_CHECK(snapDown(-1.4, 1.0) == approx(-1.0));
  CER_CHECK(snapDown(-1.5, 1.0) == approx(-1.0));
  CER_CHECK(snapDown(-1.6, 1.0) == approx(-1.0));

  CER_CHECK(snapDown(0.0, 2.0) == approx(0.0));
  CER_CHECK(snapDown(+0.4, 2.0) == approx(0.0));
  CER_CHECK(snapDown(+0.5, 2.0) == approx(0.0));
  CER_CHECK(snapDown(+0.6, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-0.4, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-0.5, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-0.6, 2.0) == approx(0.0));

  CER_CHECK(snapDown(+1.4, 2.0) == approx(0.0));
  CER_CHECK(snapDown(+1.5, 2.0) == approx(0.0));
  CER_CHECK(snapDown(+1.6, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-1.4, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-1.5, 2.0) == approx(0.0));
  CER_CHECK(snapDown(-1.6, 2.0) == approx(0.0));
}

TEST_CASE("scalar.correct")
{
  CER_CHECK(correct(+1.1) == approx(+1.1));

  CER_CHECK(correct(+1.1, 0, 0.4) == approx(+1.0));
  CER_CHECK(correct(-1.1, 0, 0.4) == approx(-1.0));
  CER_CHECK(correct(+1.3, 0, 0.4) == approx(+1.0));
  CER_CHECK(correct(+1.4, 0, 0.3) == approx(+1.4));

  CER_CHECK(correct(+1.1, 1, 0.4) == approx(+1.1));
  CER_CHECK(correct(-1.1, 1, 0.4) == approx(-1.1));
  CER_CHECK(correct(+1.3, 1, 0.4) == approx(+1.3));
  CER_CHECK(correct(+1.4, 1, 0.3) == approx(+1.4));
}

TEST_CASE("scalar.is_equal")
{
  CER_CHECK(is_equal(+1.0, +1.0, 0.0));
  CER_CHECK(is_equal(-1.0, -1.0, 0.0));
  CER_CHECK(is_equal(-1.001, -1.001, 0.0));
  CER_CHECK(is_equal(+1.0, +1.001, 0.1));
  CER_CHECK(is_equal(+1.0, +1.0999, 0.1));

  CER_CHECK_FALSE(is_equal(+1.0, +1.11, 0.1));
  CER_CHECK_FALSE(is_equal(+1.0, +1.1, 0.09));
  CER_CHECK_FALSE(is_equal(-1.0, +1.11, 0.1));
  CER_CHECK_FALSE(is_equal(+1.0, +1.1, 0.0));
}

TEST_CASE("scalar.is_zero")
{
  CER_CHECK(is_zero(0.0, 0.0));
  CER_CHECK(is_zero(0.0, 0.1));
  CER_CHECK(is_zero(0.099, 0.1));
  CER_CHECK(is_zero(-0.099, 0.1));
  CER_CHECK_FALSE(is_zero(0.099, 0.0));
  CER_CHECK_FALSE(is_zero(-1.0, 0.0));
}

TEST_CASE("scalar.contains")
{
  CER_CHECK(contains(0.0, 0.0, 1.0));
  CER_CHECK(contains(1.0, 0.0, 1.0));
  CER_CHECK(contains(0.0, 1.0, 0.0));
  CER_CHECK(contains(1.0, 1.0, 0.0));

  CER_CHECK_FALSE(contains(+1.1, 0.0, 1.0));
  CER_CHECK_FALSE(contains(+1.1, 1.0, 0.0));
  CER_CHECK_FALSE(contains(-0.1, 0.0, 1.0));
  CER_CHECK_FALSE(contains(-0.1, 1.0, 0.0));
}

TEST_CASE("scalar.to_radians")
{
  using c = constants<double>;

  CER_CHECK(to_radians(0.0) == 0.0);
  CER_CHECK(to_radians(90.0) == c::half_pi());
  CER_CHECK(to_radians(180.0) == c::pi());
  CER_CHECK(to_radians(360.0) == c::two_pi());
  CER_CHECK(to_radians(-180.0) == -c::pi());
  CER_CHECK(to_radians(-360.0) == -c::two_pi());
}

TEST_CASE("scalar.to_degrees")
{
  using c = constants<double>;

  CER_CHECK(to_degrees(0.0) == 0.0);
  CER_CHECK(to_degrees(c::half_pi()) == 90.0);
  CER_CHECK(to_degrees(c::pi()) == 180.0);
  CER_CHECK(to_degrees(c::two_pi()) == 360.0);
  CER_CHECK(to_degrees(-c::pi()) == -180.0);
  CER_CHECK(to_degrees(-c::two_pi()) == -360.0);
}

TEST_CASE("scalar.normalize_radians")
{
  using c = constants<double>;

  CER_CHECK(normalize_radians(c::two_pi()) == 0.0);
  CER_CHECK(normalize_radians(c::half_pi()) == c::half_pi());
  CER_CHECK(normalize_radians(-c::half_pi()) == c::three_half_pi());
  CER_CHECK(normalize_radians(c::half_pi() + c::two_pi()) == c::half_pi());
}

TEST_CASE("scalar.normalize_degrees")
{
  CER_CHECK(normalize_degrees(0.0) == 0.0);
  CER_CHECK(normalize_degrees(360.0) == 0.0);
  CER_CHECK(normalize_degrees(90.0) == 90.0);
  CER_CHECK(normalize_degrees(-90.0) == 270.0);
  CER_CHECK(normalize_degrees(360.0 + 90.0) == 90.0);
}

TEST_CASE("scalar.succ")
{
  CER_CHECK(succ(0u, 1u) == 0u);
  CER_CHECK(succ(0u, 2u) == 1u);
  CER_CHECK(succ(1u, 2u) == 0u);
  CER_CHECK(succ(0u, 3u, 2u) == 2u);
  CER_CHECK(succ(2u, 3u, 2u) == 1u);
}

TEST_CASE("scalar.pred")
{
  CER_CHECK(pred(0u, 1u) == 0u);
  CER_CHECK(pred(0u, 2u) == 1u);
  CER_CHECK(pred(1u, 2u) == 0u);
  CER_CHECK(pred(0u, 3u, 2u) == 1u);
  CER_CHECK(pred(2u, 3u, 2u) == 0u);
}

TEST_CASE("scalar.nextgreater")
{
  CHECK(+1.0 < nextgreater(+1.0));
  CHECK(-1.0 < nextgreater(-1.0));
}

#define CHECK_SQRT(v) CHECK(sqrt(v) == approx(std::sqrt(v)));

TEST_CASE("scalar.sqrt")
{
  for (double v = 0.0; v < 20.0; v += 0.1)
  {
    CHECK_SQRT(v)
  }

  CHECK(is_nan(sqrt(nan<double>())));
  CHECK(is_nan(sqrt(-1.0)));
  CHECK_SQRT(std::numeric_limits<double>::infinity());
}

#define CE_CHECK_SQRT(v) CHECK(sqrt_c(v) == approx(std::sqrt(v)))

TEST_CASE("scalar.sqrt_c")
{
  CE_CHECK_SQRT(0.0);
  CE_CHECK_SQRT(0.2);
  CE_CHECK_SQRT(1.0);
  CE_CHECK_SQRT(2.0);
  CE_CHECK_SQRT(4.0);
  CE_CHECK_SQRT(5.2);
  CE_CHECK_SQRT(5.2394839489348);
  CE_CHECK_SQRT(223235.2394839489348);
  CE_CHECK_SQRT(std::numeric_limits<double>::infinity());

  CE_CHECK(is_nan(sqrt_c(nan<double>())));
  CE_CHECK(is_nan(sqrt_c(-1.0)));
}

template <typename T>
static void checkSolution(
  const std::tuple<std::size_t, T, T>& expected,
  const std::tuple<std::size_t, T, T>& actual)
{
  const auto expectedNum = std::get<0>(expected);
  const auto actualNum = std::get<0>(actual);

  CHECK(actualNum == expectedNum);
  if (expectedNum > 0)
  {
    CHECK(std::get<1>(actual) == approx(std::get<1>(expected), 0.00000001));
  }
  if (expectedNum > 1)
  {
    CHECK(std::get<2>(actual) == approx(std::get<2>(expected), 0.00000001));
  }
}

template <typename T>
static void checkSolution(
  const std::tuple<std::size_t, T, T, T>& expected,
  const std::tuple<std::size_t, T, T, T>& actual)
{
  checkSolution<T>(
    {std::get<0>(expected), std::get<1>(expected), std::get<2>(expected)},
    {std::get<0>(actual), std::get<1>(actual), std::get<2>(actual)});

  if (std::get<0>(expected) > 2)
  {
    CHECK(std::get<3>(actual) == approx(std::get<3>(expected), 0.00000001));
  }
}

template <typename T>
static void checkSolution(
  const std::tuple<std::size_t, T, T, T, T>& expected,
  const std::tuple<std::size_t, T, T, T, T>& actual)
{
  checkSolution<T>(
    {std::get<0>(expected),
     std::get<1>(expected),
     std::get<2>(expected),
     std::get<3>(expected)},
    {std::get<0>(actual), std::get<1>(actual), std::get<2>(actual), std::get<3>(actual)});

  if (std::get<0>(expected) > 3)
  {
    CHECK(std::get<4>(actual) == approx(std::get<4>(expected), 0.00000001));
  }
}
TEST_CASE("scalar.solve_quadratic")
{
  using c = constants<double>;

  checkSolution({2u, 2.0, -8.0}, solve_quadratic(1.0, 6.0, -16.0, c::almost_zero()));
  checkSolution({2u, -1.0, -9.0}, solve_quadratic(1.0, 10.0, 9.0, c::almost_zero()));
  checkSolution({2u, 7.0, -4.0}, solve_quadratic(0.5, -1.5, -14.0, c::almost_zero()));
  checkSolution(
    {1u, 2.0, nan<double>()}, solve_quadratic(1.0, -4.0, 4.0, c::almost_zero()));
  checkSolution(
    {0u, nan<double>(), nan<double>()},
    solve_quadratic(1.0, 12.0, 37.0, c::almost_zero()));
}

TEST_CASE("scalar.solve_cubic")
{
  using c = constants<double>;

  checkSolution(
    {1u, -2.0, nan<double>(), nan<double>()},
    solve_cubic(1.0, 0.0, -2.0, 4.0, c::almost_zero()));
  checkSolution(
    {1u, 7.0 / 9.0, nan<double>(), nan<double>()},
    solve_cubic(9.0, -43.0, 145.0, -91.0, c::almost_zero()));
  checkSolution(
    {3u, 4.464101615, 2.0, -2.464101615},
    solve_cubic(1.0, -4.0, -7.0, 22.0, c::almost_zero()));

  // casus irreducibilis
  checkSolution(
    {2u, -2.0, 1.0, nan<double>()}, solve_cubic(1.0, 0.0, -3.0, 2.0, c::almost_zero()));
  checkSolution(
    {3u, 4.0 / 3.0, 1.0 / 3.0, -10.0 / 6.0},
    solve_cubic(1.0, 0.0, -7.0 / 3.0, 20.0 / 27.0, c::almost_zero()));
}

TEST_CASE("scalar.solve_quartic")
{
  using c = constants<double>;

  checkSolution(
    {0u, nan<double>(), nan<double>(), nan<double>(), nan<double>()},
    solve_quartic(1.0, 1.0, 1.0, 1.0, 1.0, c::almost_zero()));
  checkSolution(
    {0u, nan<double>(), nan<double>(), nan<double>(), nan<double>()},
    solve_quartic(1.0, -1.0, 1.0, -1.0, 1.0, c::almost_zero()));
  checkSolution(
    {4u,
     -0.203258341626567109,
     -4.91984728399109344,
     2.76090563295441601,
     0.362199992663244539},
    solve_quartic(1.0, 2.0, -14.0, 2.0, 1.0, c::almost_zero()));
  checkSolution(
    {2u, 1.5986745079, -1.0, nan<double>(), nan<double>()},
    solve_quartic(1.0, 3.0, 0.0, -8.0, -6.0, c::almost_zero()));
  checkSolution(
    {2u, -1.0, -1.0, nan<double>(), nan<double>()},
    solve_quartic(1.0, 4.0, 6.0, 4.0, 1.0, c::almost_zero()));
  checkSolution(
    {2u, -3.0, 2.0, nan<double>(), nan<double>()},
    solve_quartic(1.0, 2.0, -11.0, -12.0, 36.0, c::almost_zero()));
  checkSolution(
    {4u, -1.0 - sqrt(6.0), -1.0 - sqrt(11.0), sqrt(11.0) - 1.0, sqrt(6.0) - 1.0},
    solve_quartic(1.0, 4.0, -11.0, -30.0, 50.0, c::almost_zero()));
}
} // namespace vm
