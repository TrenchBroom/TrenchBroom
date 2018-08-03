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

            EXPECT_EQ(exp1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), ProportionalAxes::None(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-200,0,0), ProportionalAxes::None(), AnchorPos::Opposite).empty());
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-225,0,0), ProportionalAxes::None(), AnchorPos::Opposite).empty());

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-125,-100,-100),
                                    Vec3( 125, 100, 100));

            EXPECT_EQ(exp2, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), ProportionalAxes::None(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-100,0,0), ProportionalAxes::None(), AnchorPos::Center).empty());
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-125,0,0), ProportionalAxes::None(), AnchorPos::Center).empty());
        }

        TEST(ScaleObjectsToolTest, moveBBoxFace_Proportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-112.5,-112.5),
                                    Vec3( 125, 112.5, 112.5));

            EXPECT_EQ(Vec3(225,225,225), exp1.size());
            EXPECT_EQ(exp1, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), ProportionalAxes::All(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-200,0,0), ProportionalAxes::All(), AnchorPos::Opposite).empty());
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-225,0,0), ProportionalAxes::All(), AnchorPos::Opposite).empty());

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-125,-125,-125),
                                    Vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(25,0,0), ProportionalAxes::All(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-100,0,0), ProportionalAxes::All(), AnchorPos::Center).empty());
            EXPECT_TRUE(moveBBoxFace(input1, BBoxSide(Vec3::PosX), Vec3(-125,0,0), ProportionalAxes::All(), AnchorPos::Center).empty());
        }

        TEST(ScaleObjectsToolTest, moveBBoxCorner) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-100),
                                    Vec3( 125, 125, 125));

            EXPECT_EQ(exp1, moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(25,25,25), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(-200,0,0), AnchorPos::Opposite).empty());
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(-225,0,0), AnchorPos::Opposite).empty());

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-125,-125,-125),
                                    Vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(25,25,25), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(-100,0,0), AnchorPos::Center).empty());
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(Vec3(1,1,1)), Vec3(-125,0,0), AnchorPos::Center).empty());
        }

        TEST(ScaleObjectsToolTest, moveBBoxEdge_NonProportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-100),
                                    Vec3( 125, 125, 100));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), ProportionalAxes::None(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-200,-200,0), ProportionalAxes::None(), AnchorPos::Opposite).empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-225,-225,0), ProportionalAxes::None(), AnchorPos::Opposite).empty());

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-125,-125,-100),
                                    Vec3( 125, 125, 100));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), ProportionalAxes::None(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-100,-200,0), ProportionalAxes::None(), AnchorPos::Center).empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-125,-225,0), ProportionalAxes::None(), AnchorPos::Center).empty());
        }

        TEST(ScaleObjectsToolTest, moveBBoxEdge_NonProportional_NegY) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-125,-100),
                                    Vec3( 100, 100, 125));

            // move the (+Z, -Y, +/-X) edge by Z=25, Y=-25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,-1,1), Vec3(-1,-1,1)), Vec3(0,-25,25), ProportionalAxes::None(), AnchorPos::Opposite));

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-100,-125,-125),
                                    Vec3( 100, 125, 125));

            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(Vec3(1,-1,1), Vec3(-1,-1,1)), Vec3(0,-25,25), ProportionalAxes::None(), AnchorPos::Center));
        }


        TEST(ScaleObjectsToolTest, moveBBoxEdge_Proportional) {
            const auto input1 = BBox3(Vec3(-100,-100,-100),
                                      Vec3( 100, 100, 100));

            const auto exp1 = BBox3(Vec3(-100,-100,-112.5),
                                    Vec3( 125, 125, 112.5));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), ProportionalAxes::All(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-200,-200,0), ProportionalAxes::All(), AnchorPos::Opposite).empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-225,-225,0), ProportionalAxes::All(), AnchorPos::Opposite).empty());

            // test with center anchor
            const auto exp2 = BBox3(Vec3(-125,-125,-125),
                                    Vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(25,25,0), ProportionalAxes::All(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-100,-100,0), ProportionalAxes::All(), AnchorPos::Center).empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(Vec3(1,1,-1), Vec3(1,1,1)), Vec3(-125,-125,0), ProportionalAxes::All(), AnchorPos::Center).empty());
        }
    }
}
