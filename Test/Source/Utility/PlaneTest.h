/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_PlaneTest_h
#define TrenchBroom_PlaneTest_h

#include "TestSuite.h"
#include "Utility/VecMath.h"

namespace TrenchBroom {
    namespace VecMath {
        class PlaneTest : public TestSuite<PlaneTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&PlaneTest::testZ);
            }
        public:
            void testZ() {
                Planef plane;
                
                plane = Planef(Vec3f(1.0, 0.0f, 1.0f).normalized(), 0.0f);
                assert(Math<float>::eq(plane.z(1.0f, 0.0f), -1.0f));

                plane = Planef(Vec3f(0.0, 0.0f, 1.0f).normalized(), 1.0f);
                assert(Math<float>::eq(plane.z(0.0f, 0.0f), 1.0f));
            }
        };
    }
}

#endif
