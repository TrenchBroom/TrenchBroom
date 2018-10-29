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

#include <vecmath/scalar.h>

#include <limits>

namespace vm {
    TEST(ScalarTest, identity) {
        const auto id = identity();
        ASSERT_EQ(1, id(1));
        ASSERT_EQ(-1, id(-1));
        ASSERT_DOUBLE_EQ(1.234, id(1.234));
    }

    TEST(ScalarTest, isnan) {
        ASSERT_TRUE(isnan(std::numeric_limits<double>::quiet_NaN()));
        ASSERT_TRUE(isnan(std::numeric_limits<float>::quiet_NaN()));
        ASSERT_FALSE(isnan(1.0));
        ASSERT_FALSE(isnan(1.0f));
    }

    TEST(ScalarTest, isInf) {
        ASSERT_TRUE(isInf(+std::numeric_limits<double>::infinity()));
        ASSERT_TRUE(isInf(-std::numeric_limits<double>::infinity()));
        ASSERT_TRUE(isInf(+std::numeric_limits<float>::infinity()));
        ASSERT_TRUE(isInf(-std::numeric_limits<float>::infinity()));
        ASSERT_FALSE(isInf(0.0));
        ASSERT_FALSE(isInf(0.0f));
    }

    TEST(ScalarTest, nan) {
        ASSERT_TRUE(isnan(nan<double>()));
        ASSERT_TRUE(isnan(nan<float>()));
    }

    TEST(ScalarTest, min) {
        ASSERT_EQ(+1.0, min(+1.0, +1.0));
        ASSERT_EQ(+1.0, min(+1.0, +2.0));
        ASSERT_EQ(+1.0, min(+2.0, +1.0));
        ASSERT_EQ(-1.0, min(-1.0, +2.0));
        ASSERT_EQ(-2.0, min(+1.0, -2.0));
        ASSERT_EQ(-2.0, min(-1.0, -2.0));
    }

    TEST(ScalarTest, max) {
        ASSERT_EQ(+1.0, max(+1.0, +1.0));
        ASSERT_EQ(+2.0, max(+1.0, +2.0));
        ASSERT_EQ(+2.0, max(+2.0, +1.0));
        ASSERT_EQ(+2.0, max(-1.0, +2.0));
        ASSERT_EQ(+1.0, max(+1.0, -2.0));
        ASSERT_EQ(-1.0, max(-1.0, -2.0));
    }

    TEST(ScalarTest, absMin) {
        ASSERT_EQ(+1.0, absMin(+1.0, +1.0));
        ASSERT_EQ(+1.0, absMin(+1.0, +2.0));
        ASSERT_EQ(+1.0, absMin(+2.0, +1.0));
        ASSERT_EQ(-1.0, absMin(-1.0, +2.0));
        ASSERT_EQ(+1.0, absMin(+1.0, -2.0));
        ASSERT_EQ(-1.0, absMin(-1.0, -2.0));
    }

    TEST(ScalarTest, absMax) {
        ASSERT_EQ(+1.0, absMax(+1.0, +1.0));
        ASSERT_EQ(+2.0, absMax(+1.0, +2.0));
        ASSERT_EQ(+2.0, absMax(+2.0, +1.0));
        ASSERT_EQ(+2.0, absMax(-1.0, +2.0));
        ASSERT_EQ(-2.0, absMax(+1.0, -2.0));
        ASSERT_EQ(-2.0, absMax(-1.0, -2.0));
    }

    TEST(ScalarTest, absDifference) {
        ASSERT_EQ(3, absDifference(+4, +7));
        ASSERT_EQ(3, absDifference(+7, +4));
        ASSERT_EQ(6, absDifference(+7, -1));
        ASSERT_EQ(6, absDifference(-7, +1));
        ASSERT_EQ(6, absDifference(-7, -1));
    }

    TEST(ScalarTest, clamp) {
        ASSERT_EQ( 0.0, clamp( 0.0,  0.0, +1.0));
        ASSERT_EQ(+1.0, clamp(+1.0,  0.0, +1.0));
        ASSERT_EQ( 0.0, clamp(-1.0,  0.0, +1.0));
        ASSERT_EQ(+1.0, clamp(+2.0,  0.0, +1.0));
        ASSERT_EQ(+0.5, clamp(+0.5,  0.0, +1.0));

        ASSERT_EQ( 0.0, clamp( 0.0, -1.0,  0.0));
        ASSERT_EQ(-1.0, clamp(-1.0, -1.0,  0.0));
        ASSERT_EQ( 0.0, clamp(+1.0, -1.0,  0.0));
        ASSERT_EQ(-1.0, clamp(-2.0, -1.0,  0.0));
        ASSERT_EQ(-0.5, clamp(-0.5, -1.0,  0.0));

        ASSERT_EQ( 0.0, clamp( 0.0, -1.0, +1.0));
        ASSERT_EQ(-1.0, clamp(-1.0, -1.0, +1.0));
        ASSERT_EQ(+1.0, clamp(+1.0, -1.0, +1.0));
        ASSERT_EQ(-1.0, clamp(-2.0, -1.0, +1.0));
        ASSERT_EQ(+1.0, clamp(+2.0, -1.0, +1.0));
    }

    TEST(ScalarTest, sign) {
        ASSERT_EQ(-1, sign(-2));
        ASSERT_EQ(-1, sign(-1));
        ASSERT_EQ( 0, sign( 0));
        ASSERT_EQ(+1, sign( 1));
        ASSERT_EQ(+1, sign( 2));
    }

    TEST(ScalarTest, step) {
        ASSERT_EQ(0, step(1, -1));
        ASSERT_EQ(0, step(1,  0));
        ASSERT_EQ(1, step(1,  1));
        ASSERT_EQ(1, step(1,  2));
    }

    TEST(ScalarTest, smoothstep) {
        ASSERT_DOUBLE_EQ(0.0,     smoothstep(0.0, 1.0, -1.0));
        ASSERT_DOUBLE_EQ(0.0,     smoothstep(0.0, 1.0,  0.0));
        ASSERT_DOUBLE_EQ(0.15625, smoothstep(0.0, 1.0,  0.25));
        ASSERT_DOUBLE_EQ(0.5,     smoothstep(0.0, 1.0,  0.5));
        ASSERT_DOUBLE_EQ(0.84375, smoothstep(0.0, 1.0,  0.75));
        ASSERT_DOUBLE_EQ(1.0,     smoothstep(0.0, 1.0,  1.0));
        ASSERT_DOUBLE_EQ(1.0,     smoothstep(0.0, 1.0,  2.0));
    }

    TEST(ScalarTest, mod) {
        ASSERT_DOUBLE_EQ( 0.0, mod(+4.0, +2.0));
        ASSERT_DOUBLE_EQ(+1.0, mod(+5.0, +2.0));
        ASSERT_DOUBLE_EQ(-1.0, mod(-5.0, +2.0));
        ASSERT_DOUBLE_EQ(+1.0, mod(+5.0, -2.0));
        ASSERT_DOUBLE_EQ(-1.0, mod(-5.0, -2.0));
        ASSERT_DOUBLE_EQ(+1.5, mod(+5.5, +2.0));
    }

    TEST(ScalarTest, floor) {
        ASSERT_DOUBLE_EQ(-1.0, floor(-0.7));
        ASSERT_DOUBLE_EQ(-1.0, floor(-0.5));
        ASSERT_DOUBLE_EQ(-1.0, floor(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, floor( 0.0));
        ASSERT_DOUBLE_EQ( 0.0, floor( 0.4));
        ASSERT_DOUBLE_EQ( 0.0, floor( 0.6));
        ASSERT_DOUBLE_EQ( 1.0, floor( 1.0));
    }

    TEST(ScalarTest, ceil) {
        ASSERT_DOUBLE_EQ(-1.0, ceil(-1.1));
        ASSERT_DOUBLE_EQ( 0.0, ceil(-0.7));
        ASSERT_DOUBLE_EQ( 0.0, ceil(-0.5));
        ASSERT_DOUBLE_EQ( 0.0, ceil(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, ceil( 0.0));
        ASSERT_DOUBLE_EQ( 1.0, ceil( 0.4));
        ASSERT_DOUBLE_EQ( 1.0, ceil( 0.6));
        ASSERT_DOUBLE_EQ( 1.0, ceil( 1.0));
        ASSERT_DOUBLE_EQ( 2.0, ceil( 1.1));
    }

    TEST(ScalarTest, trunc) {
        ASSERT_DOUBLE_EQ(-1.0, trunc(-1.1));
        ASSERT_DOUBLE_EQ( 0.0, trunc(-0.7));
        ASSERT_DOUBLE_EQ( 0.0, trunc(-0.5));
        ASSERT_DOUBLE_EQ( 0.0, trunc(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, trunc( 0.0));
        ASSERT_DOUBLE_EQ( 0.0, trunc( 0.4));
        ASSERT_DOUBLE_EQ( 0.0, trunc( 0.6));
        ASSERT_DOUBLE_EQ(+1.0, trunc( 1.0));
        ASSERT_DOUBLE_EQ(+1.0, trunc( 1.1));
    }

    TEST(ScalarTest, mix) {
        ASSERT_DOUBLE_EQ(1.0, mix(1.0, 2.0, 0.0));
        ASSERT_DOUBLE_EQ(2.0, mix(1.0, 2.0, 1.0));
        ASSERT_DOUBLE_EQ(1.5, mix(1.0, 2.0, 0.5));

        ASSERT_DOUBLE_EQ(-1.0, mix(-1.0, 2.0, 0.0));
        ASSERT_DOUBLE_EQ(+2.0, mix(-1.0, 2.0, 1.0));
        ASSERT_DOUBLE_EQ(+0.5, mix(-1.0, 2.0, 0.5));

        ASSERT_DOUBLE_EQ(-1.0, mix(-1.0, -2.0, 0.0));
        ASSERT_DOUBLE_EQ(-2.0, mix(-1.0, -2.0, 1.0));
        ASSERT_DOUBLE_EQ(-1.5, mix(-1.0, -2.0, 0.5));
    }

    TEST(ScalarTest, fract) {
        ASSERT_DOUBLE_EQ(-0.2, fract(-1.2));
        ASSERT_DOUBLE_EQ( 0.0, fract(-1.0));
        ASSERT_DOUBLE_EQ(-0.7, fract(-0.7));
        ASSERT_DOUBLE_EQ( 0.0, fract( 0.0));
        ASSERT_DOUBLE_EQ(+0.7, fract(+0.7));
        ASSERT_DOUBLE_EQ( 0.0, fract(+1.0));
        ASSERT_DOUBLE_EQ( 0.2, fract(+1.2));
    }

    TEST(ScalarTest, round) {
        ASSERT_DOUBLE_EQ(-1.0, round(-1.1));
        ASSERT_DOUBLE_EQ(-1.0, round(-0.7));
        ASSERT_DOUBLE_EQ(-1.0, round(-0.5));
        ASSERT_DOUBLE_EQ( 0.0, round(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, round( 0.0));
        ASSERT_DOUBLE_EQ( 0.0, round( 0.4));
        ASSERT_DOUBLE_EQ(+1.0, round( 0.6));
        ASSERT_DOUBLE_EQ(+1.0, round( 1.0));
        ASSERT_DOUBLE_EQ(+1.0, round( 1.1));
    }

    TEST(ScalarTest, roundUp) {
        ASSERT_DOUBLE_EQ(-2.0, roundUp(-1.1));
        ASSERT_DOUBLE_EQ(-1.0, roundUp(-0.7));
        ASSERT_DOUBLE_EQ(-1.0, roundUp(-0.5));
        ASSERT_DOUBLE_EQ(-1.0, roundUp(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, roundUp( 0.0));
        ASSERT_DOUBLE_EQ(+1.0, roundUp( 0.4));
        ASSERT_DOUBLE_EQ(+1.0, roundUp( 0.6));
        ASSERT_DOUBLE_EQ(+1.0, roundUp( 1.0));
        ASSERT_DOUBLE_EQ(+2.0, roundUp( 1.1));
    }

    TEST(ScalarTest, roundDown) {
        ASSERT_DOUBLE_EQ(-1.0, roundDown(-1.1));
        ASSERT_DOUBLE_EQ( 0.0, roundDown(-0.7));
        ASSERT_DOUBLE_EQ( 0.0, roundDown(-0.5));
        ASSERT_DOUBLE_EQ( 0.0, roundDown(-0.4));
        ASSERT_DOUBLE_EQ( 0.0, roundDown( 0.0));
        ASSERT_DOUBLE_EQ( 0.0, roundDown( 0.4));
        ASSERT_DOUBLE_EQ( 0.0, roundDown( 0.6));
        ASSERT_DOUBLE_EQ(+1.0, roundDown( 1.0));
        ASSERT_DOUBLE_EQ(+1.0, roundDown( 1.1));
    }

    TEST(ScalarTest, snap) {
        ASSERT_DOUBLE_EQ( 0.0, snap( 0.0, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(+0.4, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snap(+0.5, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snap(+0.6, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(-0.4, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snap(-0.5, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snap(-0.6, 1.0));

        ASSERT_DOUBLE_EQ( 1.0, snap(+1.4, 1.0));
        ASSERT_DOUBLE_EQ( 2.0, snap(+1.5, 1.0));
        ASSERT_DOUBLE_EQ( 2.0, snap(+1.6, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snap(-1.4, 1.0));
        ASSERT_DOUBLE_EQ(-2.0, snap(-1.5, 1.0));
        ASSERT_DOUBLE_EQ(-2.0, snap(-1.6, 1.0));

        ASSERT_DOUBLE_EQ( 0.0, snap( 0.0, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(+0.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(+0.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(+0.6, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(-0.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(-0.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snap(-0.6, 2.0));

        ASSERT_DOUBLE_EQ( 2.0, snap(+1.4, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snap(+1.5, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snap(+1.6, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snap(-1.4, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snap(-1.5, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snap(-1.6, 2.0));
    }

    TEST(ScalarTest, snapUp) {
        ASSERT_DOUBLE_EQ( 0.0, snapUp( 0.0, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snapUp(+0.4, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snapUp(+0.5, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snapUp(+0.6, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapUp(-0.4, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapUp(-0.5, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapUp(-0.6, 1.0));

        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.4, 1.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.5, 1.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.6, 1.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.4, 1.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.5, 1.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.6, 1.0));

        ASSERT_DOUBLE_EQ( 0.0, snapUp( 0.0, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+0.4, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+0.5, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+0.6, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-0.4, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-0.5, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-0.6, 2.0));

        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.4, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.5, 2.0));
        ASSERT_DOUBLE_EQ( 2.0, snapUp(+1.6, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.4, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.5, 2.0));
        ASSERT_DOUBLE_EQ(-2.0, snapUp(-1.6, 2.0));
    }

    TEST(ScalarTest, snapDown) {
        ASSERT_DOUBLE_EQ( 0.0, snapDown( 0.0, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.4, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.5, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.6, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.4, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.5, 1.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.6, 1.0));

        ASSERT_DOUBLE_EQ( 1.0, snapDown(+1.4, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snapDown(+1.5, 1.0));
        ASSERT_DOUBLE_EQ( 1.0, snapDown(+1.6, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapDown(-1.4, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapDown(-1.5, 1.0));
        ASSERT_DOUBLE_EQ(-1.0, snapDown(-1.6, 1.0));

        ASSERT_DOUBLE_EQ( 0.0, snapDown( 0.0, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+0.6, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-0.6, 2.0));

        ASSERT_DOUBLE_EQ( 0.0, snapDown(+1.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+1.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(+1.6, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-1.4, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-1.5, 2.0));
        ASSERT_DOUBLE_EQ( 0.0, snapDown(-1.6, 2.0));
    }

    TEST(ScalarTest, correct) {
        ASSERT_DOUBLE_EQ(+1.1, correct(+1.1));

        ASSERT_DOUBLE_EQ(+1.0, correct(+1.1, 0, 0.4));
        ASSERT_DOUBLE_EQ(-1.0, correct(-1.1, 0, 0.4));
        ASSERT_DOUBLE_EQ(+1.0, correct(+1.3, 0, 0.4));
        ASSERT_DOUBLE_EQ(+1.4, correct(+1.4, 0, 0.3));

        ASSERT_DOUBLE_EQ(+1.1, correct(+1.1, 1, 0.4));
        ASSERT_DOUBLE_EQ(-1.1, correct(-1.1, 1, 0.4));
        ASSERT_DOUBLE_EQ(+1.3, correct(+1.3, 1, 0.4));
        ASSERT_DOUBLE_EQ(+1.4, correct(+1.4, 1, 0.3));
    }

    TEST(ScalarTest, isEqual) {
        ASSERT_TRUE(isEqual(+1.0, +1.0, 0.0));
        ASSERT_TRUE(isEqual(-1.0, -1.0, 0.0));
        ASSERT_TRUE(isEqual(-1.001, -1.001, 0.0));
        ASSERT_TRUE(isEqual(+1.0, +1.001, 0.1));
        ASSERT_TRUE(isEqual(+1.0, +1.0999, 0.1));

        ASSERT_FALSE(isEqual(+1.0, +1.11, 0.1));
        ASSERT_FALSE(isEqual(+1.0, +1.1, 0.09));
        ASSERT_FALSE(isEqual(-1.0, +1.11, 0.1));
        ASSERT_FALSE(isEqual(+1.0, +1.1, 0.0));
    }

    TEST(ScalarTest, isZero) {
        ASSERT_TRUE(isZero(0.0, 0.0));
        ASSERT_TRUE(isZero(0.0, 0.1));
        ASSERT_TRUE(isZero(0.099, 0.1));
        ASSERT_TRUE(isZero(-0.099, 0.1));
        ASSERT_FALSE(isZero(0.099, 0.0));
        ASSERT_FALSE(isZero(-1.0, 0.0));
    }

    TEST(ScalarTest, contains) {
        ASSERT_TRUE(contains(0.0, 0.0, 1.0));
        ASSERT_TRUE(contains(1.0, 0.0, 1.0));
        ASSERT_TRUE(contains(0.0, 1.0, 0.0));
        ASSERT_TRUE(contains(1.0, 1.0, 0.0));

        ASSERT_FALSE(contains(+1.1, 0.0, 1.0));
        ASSERT_FALSE(contains(+1.1, 1.0, 0.0));
        ASSERT_FALSE(contains(-0.1, 0.0, 1.0));
        ASSERT_FALSE(contains(-0.1, 1.0, 0.0));
    }

    TEST(ScalarTest, toRadians) {
        using c = constants<double>;
        ASSERT_EQ(0.0, toRadians(0.0));
        ASSERT_EQ(c::piOverTwo(), toRadians(90.0));
        ASSERT_EQ(c::pi(), toRadians(180.0));
        ASSERT_EQ(c::twoPi(), toRadians(360.0));
        ASSERT_EQ(-c::pi(), toRadians(-180.0));
        ASSERT_EQ(-c::twoPi(), toRadians(-360.0));
    }

    TEST(ScalarTest, toDegrees) {
        using c = constants<double>;
        ASSERT_EQ(0.0, toDegrees(0.0));
        ASSERT_EQ(90.0, toDegrees(c::piOverTwo()));
        ASSERT_EQ(180.0, toDegrees(c::pi()));
        ASSERT_EQ(360.0, toDegrees(c::twoPi()));
        ASSERT_EQ(-180.0, toDegrees(-c::pi()));
        ASSERT_EQ(-360.0, toDegrees(-c::twoPi()));
    }

    TEST(ScalarTest, normalizeRadians) {
        using c = constants<double>;
        ASSERT_EQ(0.0, normalizeRadians(0.0));
        ASSERT_EQ(0.0, normalizeRadians(c::twoPi()));
        ASSERT_EQ(c::piOverTwo(), normalizeRadians(c::piOverTwo()));
        ASSERT_EQ(c::threePiOverTwo(), normalizeRadians(-c::piOverTwo()));
        ASSERT_EQ(c::piOverTwo(), normalizeRadians(c::piOverTwo() + c::twoPi()));
    }

    TEST(ScalarTest, normalizeDegrees) {
        ASSERT_EQ(0.0, normalizeDegrees(0.0));
        ASSERT_EQ(0.0, normalizeDegrees(360.0));
        ASSERT_EQ(90.0, normalizeDegrees(90.0));
        ASSERT_EQ(270.0, normalizeDegrees(-90.0));
        ASSERT_EQ(90.0, normalizeDegrees(360.0 + 90.0));
    }

    TEST(ScalarTest, succ) {
        ASSERT_EQ(0u, succ(0u, 1u));
        ASSERT_EQ(1u, succ(0u, 2u));
        ASSERT_EQ(0u, succ(1u, 2u));
        ASSERT_EQ(2u, succ(0u, 3u, 2u));
        ASSERT_EQ(1u, succ(2u, 3u, 2u));
    }

    TEST(ScalarTest, pred) {
        ASSERT_EQ(0u, pred(0u, 1u));
        ASSERT_EQ(1u, pred(0u, 2u));
        ASSERT_EQ(0u, pred(1u, 2u));
        ASSERT_EQ(1u, pred(0u, 3u, 2u));
        ASSERT_EQ(0u, pred(2u, 3u, 2u));
    }

    TEST(ScalarTest, nextgreater) {
        ASSERT_TRUE(+1.0 < nextgreater(+1.0));
        ASSERT_TRUE(-1.0 < nextgreater(-1.0));
    }
}
