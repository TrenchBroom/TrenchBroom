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
        class VecTest : public TestSuite<VecTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&VecTest::testConstructFromValidString);
                registerTestCase(&VecTest::testConstructFromShortString);
                registerTestCase(&VecTest::testConstructFromLongString);
                registerTestCase(&VecTest::testConstructFromInvalidString);
                registerTestCase(&VecTest::testConstructFrom1Float);
                registerTestCase(&VecTest::testConstructFrom2Floats);
                registerTestCase(&VecTest::testConstructVec3From3Floats);
                registerTestCase(&VecTest::testConstructVec3From4Floats);
                registerTestCase(&VecTest::testConstructVec4From3Floats);
                registerTestCase(&VecTest::testConstructVec4From4Floats);
                registerTestCase(&VecTest::testConstructVec2FromVec2);
                registerTestCase(&VecTest::testConstructVec2FromVec3);
                registerTestCase(&VecTest::testConstructVec2FromVec4);
                registerTestCase(&VecTest::testConstructVec3FromVec2);
                registerTestCase(&VecTest::testConstructVec4FromVec2);
                registerTestCase(&VecTest::testConstructVec4FromVec2WithLast1);
                registerTestCase(&VecTest::testConstructVec4FromVec2WithLast2);
                registerTestCase(&VecTest::testConstructVec3FromVec3WithLast1);
                registerTestCase(&VecTest::testConstructVec3FromVec3WithLast2);
                registerTestCase(&VecTest::testAssignVec2ToVec3);
                registerTestCase(&VecTest::testAssignVec3ToVec3);
                registerTestCase(&VecTest::testAssignVec4ToVec3);
                registerTestCase(&VecTest::testInvertVec3);
                registerTestCase(&VecTest::testAddVec3);
                registerTestCase(&VecTest::testSubtractVec3);
                registerTestCase(&VecTest::testMultiplyVec3WithScalar);
                registerTestCase(&VecTest::testDivideVec3ByScalar);
                registerTestCase(&VecTest::testAddAndAssignVec3);
                registerTestCase(&VecTest::testSubtractAndAssignVec3);
                registerTestCase(&VecTest::testMultiplyAndAssignVec3WithScalar);
                registerTestCase(&VecTest::testDivideAndAssignVec3ByScalar);
            }
        public:
            void testConstructFromValidString() {
                assert(Vec3f("1.0 3 3.5") == Vec3f(1.0f, 3.0f, 3.5f));
            }
            
            void testConstructFromShortString() {
                assert(Vec3f("1.0 3") == Vec3f(1.0f, 3.0f, 0.0f));
            }
            
            void testConstructFromLongString() {
                assert(Vec3f("1.0 3 2.0f 1.0f 2.0f") == Vec3f(1.0f, 3.0f, 2.0f));
            }
            
            void testConstructFromInvalidString() {
                assert(Vec3f("asdf") == Vec3f::Null);
            }
            
            void testConstructFrom1Float() {
                assert(Vec3f(1.0f) == Vec3f(1.0f, 0.0f, 0.0f));
            }

            void testConstructFrom2Floats() {
                assert(Vec3f(1.0f, 2.0f) == Vec3f(1.0f, 2.0f, 0.0f));
            }
            
            void testConstructVec3From3Floats() {
                assert(Vec3f(1.0f, 2.0f, 3.0f) == Vec3f(1.0f, 2.0f, 3.0f));
            }
            
            void testConstructVec3From4Floats() {
                assert(Vec3f(1.0f, 2.0f, 3.0f, 4.0f) == Vec3f(1.0f, 2.0f, 3.0f));
            }
            
            void testConstructVec4From3Floats() {
                assert(Vec4f(1.0f, 2.0f, 3.0f) == Vec4f(1.0f, 2.0f, 3.0f, 0.0f));
            }
            
            void testConstructVec4From4Floats() {
                assert(Vec4f(1.0f, 2.0f, 3.0f, 4.0f) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
            }

            void testConstructVec2FromVec2() {
                const Vec2f v(3.0f, 4.0f);
                assert(Vec2f(v) == v);
            }
            
            void testConstructVec2FromVec3() {
                const Vec3f v(3.0f, 5.0f, 2.0f);
                assert(Vec2f(v) == Vec2f(v[0], v[1]));
            }
            
            void testConstructVec2FromVec4() {
                const Vec4f v(3.0f, 5.0f, 2.0f, 7.0f);
                assert(Vec2f(v) == Vec2f(v[0], v[1]));
            }
            
            void testConstructVec3FromVec2() {
                const Vec2f v(3.0f, 5.0f);
                assert(Vec3f(v) == Vec2f(v[0], v[1], 0.0f));
            }
            
            void testConstructVec4FromVec2() {
                const Vec2f v(3.0f, 5.0f);
                assert(Vec4f(v) == Vec2f(v[0], v[1], 0.0f, 0.0f));
            }
            
            void testConstructVec4FromVec2WithLast1() {
                const Vec2f v(3.0f, 5.0f);
                assert(Vec4f(v, 2.0f) == Vec4f(v[0], v[1], 0.0f, 2.0f));
            }
            
            void testConstructVec4FromVec2WithLast2() {
                const Vec2f v(3.0f, 5.0f);
                assert(Vec4f(v, 2.0f, 4.0f) == Vec4f(v[0], v[1], 2.0f, 4.0f));
            }
            
            void testConstructVec3FromVec3WithLast1() {
                const Vec3f v(3.0f, 5.0f, 8.0f);
                assert(Vec3f(v, 2.0f) == Vec3f(v[0], v[1], 2.0f));
            }

            void testConstructVec3FromVec3WithLast2() {
                const Vec3f v(3.0f, 5.0f, 8.0f);
                assert(Vec3f(v, 2.0f, 4.0f) == Vec3f(v[0], 2.0f, 4.0f));
            }
            
            void testAssignVec2ToVec3() {
                const Vec2f t(2.0f, 3.0f);
                Vec3f v;
                assert((v = t) == Vec3f(t));
            }
            
            void testAssignVec3ToVec3() {
                const Vec3f t(2.0f, 3.0f, 5.0f);
                Vec3f v;
                assert((v = t) == Vec3f(t));
            }

            void testAssignVec4ToVec3() {
                const Vec4f t(2.0f, 3.0f, 5.0f, 6.0f);
                Vec3f v;
                assert((v = t) == Vec3f(t));
            }
            
            void testInvertVec3() {
                assert(-Vec3f( 1.0f,  2.0f,  3.0f) ==
                        Vec3f(-1.0f, -2.0f, -3.0f));
            }
            
            void testAddVec3() {
                assert(Vec3f(1.0f, 2.0f, 3.0f) +
                       Vec3f(3.0f, 2.0f, 1.0f) ==
                       Vec3f(4.0f, 4.0f, 4.0f));
            }
            
            void testSubtractVec3() {
                assert(Vec3f(2.0f, 3.0f, 1.0f) -
                       Vec3f(1.0f, 2.0f, 2.0f) ==
                       Vec3f(1.0f, 1.0f, -1.0f));
            }

            void testMultiplyVec3WithScalar() {
                assert(Vec3f(2.0f, 3.0f, 1.0f) * 3.0f ==
                       Vec3f(6.0f, 9.0f, 3.0f));
            }

            void testDivideVec3ByScalar() {
                assert(Vec3f(2.0f, 36.0f, 4.0f) / 2.0f ==
                       Vec3f(1.0f, 18.0f, 2.0f));
            }
            
            void testAddAndAssignVec3() {
                Vec3f v(1.0f, 2.0f, 3.0f);
                assert((v += Vec3f(3.0f, 2.0f, 1.0f)) == Vec3f(4.0f, 4.0f, 4.0f));
            }
            
            void testSubtractAndAssignVec3() {
                Vec3f v(2.0f, 3.0f, 1.0f);
                assert((v -= Vec3f(1.0f, 2.0f, 2.0f)) == Vec3f(1.0f, 1.0f, -1.0f));
            }
            
            void testMultiplyAndAssignVec3WithScalar() {
                Vec3f v(2.0f, 3.0f, 1.0f);
                assert((v *= 3.0f) == Vec3f(6.0f, 9.0f, 3.0f));
            }
            
            void testDivideAndAssignVec3ByScalar() {
                Vec3f v(2.0f, 36.0f, 4.0f);
                assert((v /= 2.0f) == Vec3f(1.0f, 18.0f, 2.0f));
            }
        };
    }
}

#endif
