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
#include <gmock/gmock.h>

#include "TestUtils.h"
#include "View/Grid.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Assets/Texture.h"
#include "Model/MapFormat.h"
#include "Model/World.h"

#include <cmath>

namespace TrenchBroom {
    namespace View {
        static const BBox3 worldBounds(8192.0);

        TEST(GridTest, size) {
            for (int i = Grid::MinSize; i < Grid::MaxSize; ++i)
                ASSERT_EQ(i, Grid(i).size());
        }

        TEST(GridTest, actualSizeInteger) {
            for (int i = 0; i < Grid::MaxSize; ++i) {
                const int actualSize = static_cast<int>(std::pow(2, i));
                ASSERT_EQ(actualSize, Grid(i).actualSize());
            }
        }

        TEST(GridTest, actualSizeSubInteger) {
            ASSERT_EQ(0.5, Grid(-1).actualSize());
            ASSERT_EQ(0.25, Grid(-2).actualSize());
            ASSERT_EQ(0.125, Grid(-3).actualSize());
        }

        TEST(GridTest, changeSize) {
            Grid g(0);
            g.incSize();
            ASSERT_EQ(1u, g.size());
            g.decSize();
            ASSERT_EQ(0u, g.size());
            g.decSize();
            ASSERT_EQ(-1, g.size());

            g.setSize(4u);
            ASSERT_EQ(4u, g.size());
        }

        TEST(GridTest, offsetScalars) {
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).offset(0.0));
            ASSERT_DOUBLE_EQ(0.3, Grid(2u).offset(0.3));
            ASSERT_DOUBLE_EQ(-0.3, Grid(2u).offset(-0.3));

            ASSERT_DOUBLE_EQ(0.0, Grid(2u).offset(4.0));
            ASSERT_DOUBLE_EQ(0.3, Grid(2u).offset(4.3));
            ASSERT_DOUBLE_EQ(-0.3, Grid(2u).offset(-4.3));

            ASSERT_DOUBLE_EQ(-1.0, Grid(2u).offset(3.0));
            ASSERT_DOUBLE_EQ(1.0, Grid(2u).offset(5.0));
        }

        TEST(GridTest, snapScalars) {
            ASSERT_DOUBLE_EQ(0.0, Grid(-1).snap(0.0));
            ASSERT_DOUBLE_EQ(0.0, Grid(-1).snap(0.1));
            ASSERT_DOUBLE_EQ(0.0, Grid(-1).snap(0.24));
            ASSERT_DOUBLE_EQ(0.5, Grid(-1).snap(0.25));
            ASSERT_DOUBLE_EQ(0.5, Grid(-1).snap(0.7));

            ASSERT_DOUBLE_EQ(0.0, Grid(0u).snap(0.0));
            ASSERT_DOUBLE_EQ(0.0, Grid(0u).snap(0.3));
            ASSERT_DOUBLE_EQ(0.0, Grid(0u).snap(0.49));
            ASSERT_DOUBLE_EQ(1.0, Grid(0u).snap(0.5));
            ASSERT_DOUBLE_EQ(1.0, Grid(0u).snap(1.3));

            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snap(0.0));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snap(1.999));
            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snap(2.0));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snap(-1.999));
            ASSERT_DOUBLE_EQ(-4.0, Grid(2u).snap(-2.0));

            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(0.0, false));
            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snapUp(1.999, false));
            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snapUp(2.0, false));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(-1.999, false));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(-2.0, false));
            ASSERT_DOUBLE_EQ(-4.0, Grid(2u).snapUp(-4.0, false));

            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snapUp(0.0, true));
            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snapUp(1.999, true));
            ASSERT_DOUBLE_EQ(4.0, Grid(2u).snapUp(2.0, true));
            ASSERT_DOUBLE_EQ(8.0, Grid(2u).snapUp(4.0, true));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(-1.999, true));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(-2.0, true));
            ASSERT_DOUBLE_EQ(0.0, Grid(2u).snapUp(-4.0, true));
        }

        TEST(GridTest, snapOnLine) {
            const Line3d X(Vec3d(5.0, 0.0, 0.0), Vec3d::PosX);

            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d::Null, X));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3(1.0, 1.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3d(4.0, 0.0, 0.0), Grid(2u).snap(Vec3(3.0, 1.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3d(4.0, 0.0, 0.0), Grid(2u).snap(Vec3(3.0, 1.0, 2.0), X));

            const Line3d L(Vec3d::Null, Vec3d(1.0, 2.0, 0.0).normalized());
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d::Null, L));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), L));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(10.0, 0.0, 0.0), L));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(7.5, 0.0, 0.0), L));
        }

        TEST(GridTest, snapOnEdge) {
            const Edge3d E(Vec3d::Null, Vec3d(1.0, 2.0, 0.0) * 2.0);
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d::Null, E));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), E));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(10.0, 0.0, 0.0), E));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(7.5, 0.0, 0.0), E));
            ASSERT_TRUE(Grid(2u).snap(Vec3(20.0, 0.0, 0.0), E).nan());
            ASSERT_TRUE(Grid(2u).snap(Vec3(-10.0, 0.0, 0.0), E).nan());
        }

        TEST(GridTest, snapOnQuad) {
            const Polygon3d quad {
                    Vec3d(-9.0, -9.0, 0.0),
                    Vec3d(+9.0, -9.0, 0.0),
                    Vec3d(+9.0, +9.0, 0.0),
                    Vec3d(-9.0, +9.0, 0.0)
            };

            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d(0.0, 0.0, 0.0), quad, Vec3d::PosZ));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d(1.0, 1.0, 0.0), quad, Vec3d::PosZ));
            ASSERT_VEC_EQ(Vec3d::Null, Grid(2u).snap(Vec3d(1.0, 1.0, 1.0), quad, Vec3d::PosZ));

            ASSERT_VEC_EQ(Vec3d(9.0, 4.0, 0.0), Grid(2u).snap(Vec3d(10.0, 3.0, 1.0), quad, Vec3d::PosZ));
            ASSERT_VEC_EQ(Vec3d(9.0, -4.0, 0.0), Grid(2u).snap(Vec3d(10.0, -2.0, 1.0), quad, Vec3d::PosZ));
        }

        TEST(GridTest, moveDeltaForPoint) {
            const auto grid16 = Grid(4);

            const auto pointOffGrid = Vec3d(17, 17, 17);
            const auto inputDelta = Vec3d(1, 1, 7); // moves point to (18, 18, 24)
            const auto pointOnGrid = Vec3d(17, 17, 32);

            ASSERT_EQ(pointOnGrid, pointOffGrid + grid16.moveDeltaForPoint(pointOffGrid, worldBounds, inputDelta));
        }

        TEST(GridTest, moveDeltaForPoint_SubInteger) {
            const auto grid05 = Grid(-1);

            const auto pointOffGrid = Vec3d(0.51, 0.51, 0.51);
            const auto inputDelta = Vec3d(0.01, 0.01, 0.30); // moves point to (0.52, 0.52, 0.81)
            const auto pointOnGrid = Vec3d(0.51, 0.51, 1.0);

            ASSERT_EQ(pointOnGrid, pointOffGrid + grid05.moveDeltaForPoint(pointOffGrid, worldBounds, inputDelta));
        }

        TEST(GridTest, moveDeltaForPoint_SubInteger2) {
            const auto grid05 = Grid(-1);

            const auto pointOffGrid = Vec3d(0.51, 0.51, 0.51);
            const auto inputDelta = Vec3d(0.01, 0.01, 1.30); // moves point to (0.52, 0.52, 1.81)
            const auto pointOnGrid = Vec3d(0.51, 0.51, 2.0);

            ASSERT_EQ(pointOnGrid, pointOffGrid + grid05.moveDeltaForPoint(pointOffGrid, worldBounds, inputDelta));
        }

        static Model::Brush* makeCube128() {
            Assets::Texture texture("testTexture", 64, 64);
            Model::World world(Model::MapFormat::Standard, nullptr, worldBounds);
            Model::BrushBuilder builder(&world, worldBounds);
            Model::Brush* cube = builder.createCube(128.0, "");
            return cube;
        }

        TEST(GridTest, moveDeltaForFace) {
            const auto grid16 = Grid(4);

            Model::Brush* cube = makeCube128();
            Model::BrushFace* topFace = cube->findFace(Vec3::PosZ);

            ASSERT_DOUBLE_EQ(64.0, topFace->boundsCenter().z());

            // try to move almost 4 grid increments up -> snaps to 3
            ASSERT_EQ(Vec3(0,0,48), grid16.moveDelta(topFace, Vec3(0, 0, 63)));
            ASSERT_EQ(Vec3(0,0,64), grid16.moveDelta(topFace, Vec3(0, 0, 64)));
            ASSERT_EQ(Vec3(0,0,64), grid16.moveDelta(topFace, Vec3(0, 0, 65)));

            delete cube;
        }

        TEST(GridTest, moveDeltaForFace_SubInteger) {
            const auto grid05 = Grid(-1);

            Model::Brush* cube = makeCube128();
            Model::BrushFace* topFace = cube->findFace(Vec3::PosZ);

            ASSERT_DOUBLE_EQ(64.0, topFace->boundsCenter().z());

            // try to move almost 4 grid increments up -> snaps to 3
            ASSERT_EQ(Vec3(0,0,1.5), grid05.moveDelta(topFace, Vec3(0, 0, 1.9)));
            ASSERT_EQ(Vec3(0,0,2), grid05.moveDelta(topFace, Vec3(0, 0, 2)));
            ASSERT_EQ(Vec3(0,0,2), grid05.moveDelta(topFace, Vec3(0, 0, 2.1)));

            delete cube;
        }
    }
}
