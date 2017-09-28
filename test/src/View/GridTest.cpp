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

#include <cmath>

namespace TrenchBroom {
    namespace View {
        TEST(GridTest, size) {
            for (size_t i = 0; i < Grid::MaxSize; ++i)
                ASSERT_EQ(i, Grid(i).size());
        }
        
        TEST(GridTest, actualSize) {
            for (size_t i = 0; i < Grid::MaxSize; ++i) {
                const size_t actualSize = static_cast<size_t>(std::pow(2, i));
                ASSERT_EQ(actualSize, Grid(i).actualSize());
            }
        }
        
        TEST(GridTest, changeSize) {
            Grid g(0);
            g.incSize();
            ASSERT_EQ(1u, g.size());
            g.decSize();
            ASSERT_EQ(0u, g.size());
            
            g.setSize(4u);
            ASSERT_EQ(4u, g.size());
        }
        
        TEST(GridTest, offsetScalars) {
            ASSERT_FLOAT_EQ(0.0, Grid(2u).offset(0.0));
            ASSERT_FLOAT_EQ(0.3, Grid(2u).offset(0.3));
            ASSERT_FLOAT_EQ(-0.3, Grid(2u).offset(-0.3));

            ASSERT_FLOAT_EQ(0.0, Grid(2u).offset(4.0));
            ASSERT_FLOAT_EQ(0.3, Grid(2u).offset(4.3));
            ASSERT_FLOAT_EQ(-0.3, Grid(2u).offset(-4.3));

            ASSERT_FLOAT_EQ(-1.0, Grid(2u).offset(3.0));
            ASSERT_FLOAT_EQ(1.0, Grid(2u).offset(5.0));
        }
        
        TEST(GridTest, snapScalars) {
            ASSERT_FLOAT_EQ(0.0, Grid(0u).snap(0.0));
            ASSERT_FLOAT_EQ(0.0, Grid(0u).snap(0.3));
            ASSERT_FLOAT_EQ(0.0, Grid(0u).snap(0.49));
            ASSERT_FLOAT_EQ(1.0, Grid(0u).snap(0.5));
            ASSERT_FLOAT_EQ(1.0, Grid(0u).snap(1.3));

            ASSERT_FLOAT_EQ(0.0, Grid(2u).snap(0.0));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snap(1.999));
            ASSERT_FLOAT_EQ(4.0, Grid(2u).snap(2.0));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snap(-1.999));
            ASSERT_FLOAT_EQ(-4.0, Grid(2u).snap(-2.0));

            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(0.0, false));
            ASSERT_FLOAT_EQ(4.0, Grid(2u).snapUp(1.999, false));
            ASSERT_FLOAT_EQ(4.0, Grid(2u).snapUp(2.0, false));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(-1.999, false));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(-2.0, false));
            ASSERT_FLOAT_EQ(-4.0, Grid(2u).snapUp(-4.0, false));

            ASSERT_FLOAT_EQ(4.0, Grid(2u).snapUp(0.0, true));
            ASSERT_FLOAT_EQ(4.0, Grid(2u).snapUp(1.999, true));
            ASSERT_FLOAT_EQ(4.0, Grid(2u).snapUp(2.0, true));
            ASSERT_FLOAT_EQ(8.0, Grid(2u).snapUp(4.0, true));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(-1.999, true));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(-2.0, true));
            ASSERT_FLOAT_EQ(0.0, Grid(2u).snapUp(-4.0, true));
        }
        
        TEST(GridTest, snapOnLine) {
            const Line3d X(Vec3d(5.0, 0.0, 0.0), Vec3d::PosX);
            
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3::Null, X));
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3(1.0, 1.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3d(4.0, 0.0, 0.0), Grid(2u).snap(Vec3(3.0, 1.0, 0.0), X));
            ASSERT_VEC_EQ(Vec3d(4.0, 0.0, 0.0), Grid(2u).snap(Vec3(3.0, 1.0, 2.0), X));
            
            const Line3d L(Vec3d::Null, Vec3d(1.0, 2.0, 0.0).normalized());
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3::Null, L));
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), L));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(10.0, 0.0, 0.0), L));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(7.5, 0.0, 0.0), L));
        }
        
        TEST(GridTest, snapOnEdge) {
            const Edge3d E(Vec3d::Null, Vec3d(1.0, 2.0, 0.0) * 2.0);
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3::Null, E));
            ASSERT_VEC_EQ(Vec3::Null, Grid(2u).snap(Vec3(1.0, 0.0, 0.0), E));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(10.0, 0.0, 0.0), E));
            ASSERT_VEC_EQ(Vec3d(2.0, 4.0, 0.0), Grid(2u).snap(Vec3(7.5, 0.0, 0.0), E));
            ASSERT_TRUE(Grid(2u).snap(Vec3(20.0, 0.0, 0.0), E).nan());
            ASSERT_TRUE(Grid(2u).snap(Vec3(-10.0, 0.0, 0.0), E).nan());
        }
    }
}
