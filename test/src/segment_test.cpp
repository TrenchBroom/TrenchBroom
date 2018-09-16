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

#include <vecmath/constants.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/segment.h>
#include <vecmath/scalar.h>
#include "TestUtils.h"

#include <iterator>
#include <vector>

namespace vm {
    TEST(SegmentTest, defaultConstructor) {
        const auto s = segment3d();
        ASSERT_EQ(vec3d::zero, s.start());
        ASSERT_EQ(vec3d::zero, s.end());
    }

    TEST(SegmentTest, constructWithPoints) {
        const auto start = vec3d(3, 0, 0);
        const auto end = vec3d(2, 0, 0);
        const auto s = segment3d(start, end);
        ASSERT_EQ(end, s.start());
        ASSERT_EQ(start, s.end());
    }

    TEST(SegmentTest, getOrigin) {
        const auto s = segment3d(vec3d(3, 0, 0), vec3d(2, 0, 0));
        ASSERT_EQ(s.start(), s.getOrigin());
    }

    TEST(SegmentTest, getDirection) {
        const auto start = vec3d(3, 0, 0);
        const auto end = vec3d(2, 0, 0);
        const auto s = segment3d(start, end);
        ASSERT_EQ(normalize(s.end() - s.start()), s.getDirection());
    }

    TEST(SegmentTest, length) {
        const auto s = segment3d(vec3d(4, 0, 0), vec3d(2, 0, 0));
        ASSERT_DOUBLE_EQ(2.0, s.length());
    }

    TEST(SegmentTest, squaredLength) {
        const auto s = segment3d(vec3d(4, 0, 0), vec3d(2, 0, 0));
        ASSERT_DOUBLE_EQ(4.0, s.squaredLength());
    }

    TEST(SegmentTest, contains1) {
        const auto z = vec3d::zero;
        const auto o = vec3d(1.0, 0.0, 0.0);
        const auto h = vec3d(0.5, 0.0, 0.0);
        const auto n = vec3d(0.5, 1.0, 0.0);
    
        ASSERT_TRUE( segment3d(z, o).contains(z, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(h, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(o, Cd::almostZero()));
        ASSERT_FALSE(segment3d(z, o).contains(n, Cd::almostZero()));
    }
    
    TEST(SegmentTest, contains2) {
        const auto z = vec3d(-64.0, -64.0, 0.0);
        const auto o = vec3d(  0.0, +64.0, 0.0);
    
        ASSERT_TRUE( segment3d(z, o).contains(z, Cd::almostZero()));
        ASSERT_TRUE( segment3d(z, o).contains(o, Cd::almostZero()));
    }

    TEST(SegmentTest, transform) {
        const auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
        const auto t = rotationMatrix(toRadians(15.0), toRadians(20.0), toRadians(-12.0)) * translationMatrix(vec3d::one);
        const auto st = s.transform(t);
        ASSERT_VEC_EQ(t * s.start(), st.start());
        ASSERT_VEC_EQ(t * s.end(), st.end());
    }

    TEST(SegmentTest, translate) {
        const auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
        const auto st = s.translate(vec3d::one);
        ASSERT_VEC_EQ(s.start() + vec3d::one, st.start());
        ASSERT_VEC_EQ(s.end() + vec3d::one, st.end());
    }

    TEST(SegmentTest, center) {
        const auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
        ASSERT_VEC_EQ(vec3d(2, 0, 0), s.center());
    }

    TEST(SegmentTest, direction) {
        const auto s = segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0));
        ASSERT_VEC_EQ(vec3d::pos_x, s.direction());
    }

    TEST(SegmentTest, getVertices) {
        const auto l = std::vector<segment3d> {
            segment3d(vec3d(0, 0, 0), vec3d(4, 0, 0)),
            segment3d(vec3d(2, 0, 0), vec3d(6, 0, 0))
        };

        auto v = std::vector<vec3d>();
        segment3d::getVertices(std::begin(l), std::end(l), std::back_inserter(v));

        const auto e = std::vector<vec3d> {
            vec3d(0, 0, 0),
            vec3d(4, 0, 0),
            vec3d(2, 0, 0),
            vec3d(6, 0, 0)
        };

        ASSERT_EQ(e, v);
    }

    TEST(SegmentTest, compare) {
        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3))
            ) < 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3))
            ) < 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
            ) > 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
            ) > 0
        );

        // with large epsilon
        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                2.0
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
                2.0
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
                2.0
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(1, 0, 0), vec3d(1, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)),
                2.0
            ) == 0
        );

        ASSERT_TRUE(
            compare(
                segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)),
                segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3))
            ) > 0
        );
    }

    TEST(SegmentTest, isEqual) {
        ASSERT_TRUE (isEqual(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)), segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)), 0.0));
        ASSERT_FALSE(isEqual(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)), segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)), 0.0));
        ASSERT_TRUE (isEqual(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)), segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)), 2.0));
    }

    TEST(SegmentTest, equal) {
        ASSERT_TRUE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) == segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) == segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }

    TEST(SegmentTest, notEqual) {
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) != segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) != segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }

    TEST(SegmentTest, lessThan) {
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)) < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)) < segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3)) < segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) < segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }

    TEST(SegmentTest, lessThanOrEqual) {
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)) <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)) <= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3)) <= segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) <= segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }

    TEST(SegmentTest, greaterThan) {
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)) > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)) > segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3)) > segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) > segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }

    TEST(SegmentTest, greaterThanOrEqual) {
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)) >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_TRUE (segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)) >= segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(3, 2, 3)) >= segment3d(vec3d(2, 0, 0), vec3d(1, 2, 3)));
        ASSERT_FALSE(segment3d(vec3d(0, 0, 0), vec3d(1, 2, 3)) >= segment3d(vec3d(0, 0, 0), vec3d(2, 2, 3)));
    }
}
