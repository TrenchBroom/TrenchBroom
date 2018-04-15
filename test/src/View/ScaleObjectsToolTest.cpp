/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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
#include <gmock/gmock.h>

#include "View/ScaleObjectsTool.h"

namespace TrenchBroom {
    namespace View {
        TEST(ScaleObjectsToolTest, moveBBoxFace_NonProportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-100),
                                    Vec3( 125, 100, 100));

            EXPECT_EQ(exp1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), false));

            // attempting to collapse the bbox just returns the original bbox
            EXPECT_EQ(input1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-200,0,0), false));
            EXPECT_EQ(input1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-225,0,0), false));
        }

        TEST(ScaleObjectsToolTest, moveBBoxFace_Proportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-112.5,-112.5),
                                    Vec3( 125, 112.5, 112.5));

            EXPECT_EQ(Vec3(225,225,225), exp1.size());
            EXPECT_EQ(exp1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), true));

            // attempting to collapse the bbox just returns the original bbox
            EXPECT_EQ(input1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-200,0,0), true));
            EXPECT_EQ(input1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-225,0,0), true));
        }

        TEST(ScaleObjectsToolTest, moveBBoxCorner) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-100),
                                    Vec3( 125, 125, 125));

            EXPECT_EQ(exp1, moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(25,25,25)));

            // attempting to collapse the bbox just returns the original bbox
            EXPECT_EQ(input1, moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(-200,0,0)));
        }

        TEST(ScaleObjectsToolTest, moveBBoxEdge_NonProportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-100),
                                    Vec3( 125, 125, 100));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), false));

            // attempting to collapse the bbox just returns the original bbox
            EXPECT_EQ(input1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-200,-200,0), false));
        }

        TEST(ScaleObjectsToolTest, moveBBoxEdge_Proportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-112.5),
                                    Vec3( 125, 125, 112.5));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), true));

            // attempting to collapse the bbox just returns the original bbox
            EXPECT_EQ(input1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-200,-200,0), true));
        }
    }
}
