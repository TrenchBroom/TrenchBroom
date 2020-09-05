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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "View/ScaleObjectsTool.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE("ScaleObjectsToolTest.moveBBoxFace_NonProportional", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                    vm::vec3( 125, 100, 100));

            EXPECT_EQ(exp1, moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(25, 0, 0), ProportionalAxes::None(),
                                         AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-200, 0, 0), ProportionalAxes::None(), AnchorPos::Opposite).is_empty());
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-225, 0, 0), ProportionalAxes::None(), AnchorPos::Opposite).is_empty());

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-125,-100,-100),
                                    vm::vec3( 125, 100, 100));

            EXPECT_EQ(exp2, moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(25, 0, 0), ProportionalAxes::None(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-100, 0, 0), ProportionalAxes::None(), AnchorPos::Center).is_empty());
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-125, 0, 0), ProportionalAxes::None(), AnchorPos::Center).is_empty());
        }

        TEST_CASE("ScaleObjectsToolTest.moveBBoxFace_Proportional", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-112.5,-112.5),
                                    vm::vec3( 125, 112.5, 112.5));

            EXPECT_EQ(vm::vec3(225,225,225), exp1.size());
            EXPECT_EQ(exp1, moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(25, 0, 0), ProportionalAxes::All(),
                                         AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-200, 0, 0), ProportionalAxes::All(), AnchorPos::Opposite).is_empty());
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-225, 0, 0), ProportionalAxes::All(), AnchorPos::Opposite).is_empty());

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-125,-125,-125),
                                    vm::vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(25, 0, 0), ProportionalAxes::All(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-100, 0, 0), ProportionalAxes::All(), AnchorPos::Center).is_empty());
            EXPECT_TRUE(moveBBoxSide(input1, BBoxSide(vm::vec3::pos_x()), vm::vec3(-125, 0, 0), ProportionalAxes::All(), AnchorPos::Center).is_empty());
        }

        TEST_CASE("ScaleObjectsToolTest.moveBBoxCorner", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                    vm::vec3( 125, 125, 125));

            EXPECT_EQ(exp1, moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(25,25,25), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(-200,0,0), AnchorPos::Opposite).is_empty());
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(-225,0,0), AnchorPos::Opposite).is_empty());

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-125,-125,-125),
                                    vm::vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(25,25,25), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(-100,0,0), AnchorPos::Center).is_empty());
            EXPECT_TRUE(moveBBoxCorner(input1, BBoxCorner(vm::vec3(1,1,1)), vm::vec3(-125,0,0), AnchorPos::Center).is_empty());
        }

        TEST_CASE("ScaleObjectsToolTest.moveBBoxEdge_NonProportional", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                    vm::vec3( 125, 125, 100));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(25,25,0), ProportionalAxes::None(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-200,-200,0), ProportionalAxes::None(), AnchorPos::Opposite).is_empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-225,-225,0), ProportionalAxes::None(), AnchorPos::Opposite).is_empty());

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-125,-125,-100),
                                    vm::vec3( 125, 125, 100));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(25,25,0), ProportionalAxes::None(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-100,-200,0), ProportionalAxes::None(), AnchorPos::Center).is_empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-125,-225,0), ProportionalAxes::None(), AnchorPos::Center).is_empty());
        }

        TEST_CASE("ScaleObjectsToolTest.moveBBoxEdge_NonProportional_NegY", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-125,-100),
                                    vm::vec3( 100, 100, 125));

            // move the (+Z, -Y, +/-X) edge by Z=25, Y=-25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,-1,1), vm::vec3(-1,-1,1)), vm::vec3(0,-25,25), ProportionalAxes::None(), AnchorPos::Opposite));

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-100,-125,-125),
                                    vm::vec3( 100, 125, 125));

            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,-1,1), vm::vec3(-1,-1,1)), vm::vec3(0,-25,25), ProportionalAxes::None(), AnchorPos::Center));
        }


        TEST_CASE("ScaleObjectsToolTest.moveBBoxEdge_Proportional", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-100,-100,-100),
                                      vm::vec3( 100, 100, 100));

            const auto exp1 = vm::bbox3(vm::vec3(-100,-100,-112.5),
                                    vm::vec3( 125, 125, 112.5));

            // move the (+X, +Y, +/-Z) edge by X=25, Y=25
            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(25,25,0), ProportionalAxes::All(), AnchorPos::Opposite));

            // attempting to collapse the bbox returns an empty box
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-200,-200,0), ProportionalAxes::All(), AnchorPos::Opposite).is_empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-225,-225,0), ProportionalAxes::All(), AnchorPos::Opposite).is_empty());

            // test with center anchor
            const auto exp2 = vm::bbox3(vm::vec3(-125,-125,-125),
                                    vm::vec3( 125, 125, 125));

            EXPECT_EQ(exp2, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(25,25,0), ProportionalAxes::All(), AnchorPos::Center));
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-100,-100,0), ProportionalAxes::All(), AnchorPos::Center).is_empty());
            EXPECT_TRUE(moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,-1), vm::vec3(1,1,1)), vm::vec3(-125,-125,0), ProportionalAxes::All(), AnchorPos::Center).is_empty());
        }

        TEST_CASE("ScaleObjectsToolTest.moveBBoxEdge", "[ScaleObjectsToolTest]") {
            const auto input1 = vm::bbox3(vm::vec3(-64,-64,-16),
                                      vm::vec3( 64, 64, 16));

            const auto exp1 = vm::bbox3(vm::vec3(-64, -64, -16),
                                    vm::vec3( 128, 64, 48));

            // NOTE: the Y=64 part of the delta is ignored because the edge we are moving points along the Y axis
            const auto delta = vm::vec3(64, 64, 32);

            EXPECT_EQ(exp1, moveBBoxEdge(input1, BBoxEdge(vm::vec3(1,1,1), vm::vec3(1,-1,1)), delta, ProportionalAxes(true, false, true), AnchorPos::Opposite));
        }
    }
}
