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

#include "Polyhedron.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(PolyhedronTest, initWith4Points) {
    const Vec3d p1( 0.0, 0.0, 8.0);
    const Vec3d p2( 8.0, 0.0, 0.0);
    const Vec3d p3(-8.0, 0.0, 0.0);
    const Vec3d p4( 0.0, 8.0, 0.0);
    
    const Polyhedron<double> polyhedron(p1, p2, p3, p4);
}
