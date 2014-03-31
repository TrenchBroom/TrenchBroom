/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Hit.h"

namespace TrenchBroom {
    TEST(HitTest, testFindFirst) {
        const Hit::HitType f1 = Hit::freeHitType();
        const Hit::HitType f2 = Hit::freeHitType();
        const Hit::HitType f3 = Hit::freeHitType();
        const Hit::HitType o = Hit::freeHitType();
        
        // create a list of hits with the following types: f1; f2,o; f2
        Hits hits;
        hits.addHit(Hit(f1, 1.0, Vec3::Null, 1u));
        hits.addHit(Hit(f2, 2.0, Vec3::Null, 2u));
        hits.addHit(Hit(o,  2.0, Vec3::Null, 3u));
        hits.addHit(Hit(f3, 3.0, Vec3::Null, 4u));
        
        ASSERT_EQ(1u, hits.findFirst(f1, true).target<unsigned>());
        ASSERT_EQ(2u, hits.findFirst(f2, true).target<unsigned>());
        ASSERT_EQ(3u, hits.findFirst(o,  true).target<unsigned>());
        ASSERT_EQ(4u, hits.findFirst(f3, true).target<unsigned>());
        ASSERT_FALSE(hits.findFirst(f2, false).isMatch());
        ASSERT_FALSE(hits.findFirst(o,  false).isMatch());
        ASSERT_FALSE(hits.findFirst(f3, false).isMatch());

        ASSERT_TRUE(hits.findFirst(f2, f1).isMatch());
        ASSERT_TRUE(hits.findFirst(o, f1).isMatch());
        ASSERT_FALSE(hits.findFirst(f2, f3).isMatch());
    }
}
