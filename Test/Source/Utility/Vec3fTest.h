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

#ifndef TrenchBroom_Vec3fTest_h
#define TrenchBroom_Vec3fTest_h

#include "TestSuite.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <functional>

namespace TrenchBroom {
    namespace VecMath {
        class Vec3fTest : public TestSuite<Vec3fTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&Vec3fTest::testAddition);
            }
        public:
            void testAddition() {
                assert(Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 2.0f, 1.0f) == Vec3f(4.0f, 4.0f, 4.0f));
            }
        };
    }
}

#endif
