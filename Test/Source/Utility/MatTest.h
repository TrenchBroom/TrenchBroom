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

#ifndef TrenchBroom_MatTest_h
#define TrenchBroom_MatTest_h

#include "TestSuite.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <functional>

namespace TrenchBroom {
    namespace VecMath {
        class MatTest : public TestSuite<MatTest> {
        protected:
            void registerTestCases() {
                registerTestCase(&MatTest::testInvert);
                registerTestCase(&MatTest::testMatrixAdd);
                registerTestCase(&MatTest::testMatrixAddAndAssign);
                registerTestCase(&MatTest::testMatrixSubtract);
                registerTestCase(&MatTest::testMatrixSubtractAndAssign);
                registerTestCase(&MatTest::testMatrixMultiply);
                registerTestCase(&MatTest::testMatrixMultiplyAndAssign);
                registerTestCase(&MatTest::testScalarMultiply);
                registerTestCase(&MatTest::testScalarMultiplyAndAssign);
                registerTestCase(&MatTest::testScalarDivide);
                registerTestCase(&MatTest::testScalarDivideAndAssign);
                registerTestCase(&MatTest::testVectorLeftMultiplyWithSameDimension);
                registerTestCase(&MatTest::testVectorLeftMultiplyWithOneLessDimension);
                registerTestCase(&MatTest::testVectorRightMultiplyWithSameDimension);
                registerTestCase(&MatTest::testVectorRightMultiplyWithOneLessDimension);
                registerTestCase(&MatTest::testSetIdentity);
                registerTestCase(&MatTest::testTransposed);
                registerTestCase(&MatTest::testMinorMatrix00);
                registerTestCase(&MatTest::testMinorMatrix12);
                registerTestCase(&MatTest::testMinorMatrix13);
                registerTestCase(&MatTest::testMinorMatrix32);
                registerTestCase(&MatTest::testDeterminant1);
                registerTestCase(&MatTest::testDeterminant2);
                registerTestCase(&MatTest::testAdjoin);
                registerTestCase(&MatTest::testAdjoint);
            }
        public:
            void testInvert() {
                const Mat4f m1(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m2(-1.0f, -1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f, -1.0f);
                assert(-m1 == m2);
            }
            
            void testMatrixAdd() {
                const Mat4f m1(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m2(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m3(2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f);
                
                assert(m1 + m2 == m3);
            }
            
            void testMatrixAddAndAssign() {
                Mat4f m1(1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m2(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m3(2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f,
                               2.0f, 2.0f, 2.0f, 2.0f);
                
                m1 += m2;
                assert(m1 == m3);
            }

            void testMatrixSubtract() {
                const Mat4f m1(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m2(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m3(0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f);
                
                assert(m1 - m2 == m3);
            }

            void testMatrixSubtractAndAssign() {
                Mat4f m1(1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m2(1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f, 1.0f);
                const Mat4f m3(0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f);
                
                m1 -= m2;
                assert(m1 == m3);
            }
            
            void testMatrixMultiply() {
                const Mat4f m1(1.0f, 2.0f, 3.0f, 4.0f,
                               3.0f, 5.0f, 1.0f, 1.0f,
                               1.0f, 5.0f, 3.0f, 3.0f,
                               2.0f, 2.0f, 3.0f, 3.0f);
                const Mat4f m2(2.0f, 3.0f, 6.0f, 2.0f,
                               5.0f, 3.0f, 4.0f, 3.0f,
                               2.0f, 5.0f, 2.0f, 1.0f,
                               6.0f, 2.0f, 3.0f, 7.0f);
                const Mat4f m3(1.0f*2.0f + 2.0f*5.0f + 3.0f*2.0f + 4.0f*6.0f, 1.0f*3.0f + 2.0f*3.0f + 3.0f*5.0f + 4.0f*2.0f, 1.0f*6.0f + 2.0f*4.0f + 3.0f*2.0f + 4.0f*3.0f, 1.0f*2.0f + 2.0f*3.0f + 3.0f*1.0f + 4.0f*7.0f,
                               3.0f*2.0f + 5.0f*5.0f + 1.0f*2.0f + 1.0f*6.0f, 3.0f*3.0f + 5.0f*3.0f + 1.0f*5.0f + 1.0f*2.0f, 3.0f*6.0f + 5.0f*4.0f + 1.0f*2.0f + 1.0f*3.0f, 3.0f*2.0f + 5.0f*3.0f + 1.0f*1.0f + 1.0f*7.0f,
                               1.0f*2.0f + 5.0f*5.0f + 3.0f*2.0f + 3.0f*6.0f, 1.0f*3.0f + 5.0f*3.0f + 3.0f*5.0f + 3.0f*2.0f, 1.0f*6.0f + 5.0f*4.0f + 3.0f*2.0f + 3.0f*3.0f, 1.0f*2.0f + 5.0f*3.0f + 3.0f*1.0f + 3.0f*7.0f,
                               2.0f*2.0f + 2.0f*5.0f + 3.0f*2.0f + 3.0f*6.0f, 2.0f*3.0f + 2.0f*3.0f + 3.0f*5.0f + 3.0f*2.0f, 2.0f*6.0f + 2.0f*4.0f + 3.0f*2.0f + 3.0f*3.0f, 2.0f*2.0f + 2.0f*3.0f + 3.0f*1.0f + 3.0f*7.0f);
                
                assert(m1 * m2 == m3);
            }
            
            void testMatrixMultiplyAndAssign() {
                Mat4f m1(1.0f, 2.0f, 3.0f, 4.0f,
                         3.0f, 5.0f, 1.0f, 1.0f,
                         1.0f, 5.0f, 3.0f, 3.0f,
                         2.0f, 2.0f, 3.0f, 3.0f);
                const Mat4f m2(2.0f, 3.0f, 6.0f, 2.0f,
                               5.0f, 3.0f, 4.0f, 3.0f,
                               2.0f, 5.0f, 2.0f, 1.0f,
                               6.0f, 2.0f, 3.0f, 7.0f);
                const Mat4f m3(1.0f*2.0f + 2.0f*5.0f + 3.0f*2.0f + 4.0f*6.0f, 1.0f*3.0f + 2.0f*3.0f + 3.0f*5.0f + 4.0f*2.0f, 1.0f*6.0f + 2.0f*4.0f + 3.0f*2.0f + 4.0f*3.0f, 1.0f*2.0f + 2.0f*3.0f + 3.0f*1.0f + 4.0f*7.0f,
                               3.0f*2.0f + 5.0f*5.0f + 1.0f*2.0f + 1.0f*6.0f, 3.0f*3.0f + 5.0f*3.0f + 1.0f*5.0f + 1.0f*2.0f, 3.0f*6.0f + 5.0f*4.0f + 1.0f*2.0f + 1.0f*3.0f, 3.0f*2.0f + 5.0f*3.0f + 1.0f*1.0f + 1.0f*7.0f,
                               1.0f*2.0f + 5.0f*5.0f + 3.0f*2.0f + 3.0f*6.0f, 1.0f*3.0f + 5.0f*3.0f + 3.0f*5.0f + 3.0f*2.0f, 1.0f*6.0f + 5.0f*4.0f + 3.0f*2.0f + 3.0f*3.0f, 1.0f*2.0f + 5.0f*3.0f + 3.0f*1.0f + 3.0f*7.0f,
                               2.0f*2.0f + 2.0f*5.0f + 3.0f*2.0f + 3.0f*6.0f, 2.0f*3.0f + 2.0f*3.0f + 3.0f*5.0f + 3.0f*2.0f, 2.0f*6.0f + 2.0f*4.0f + 3.0f*2.0f + 3.0f*3.0f, 2.0f*2.0f + 2.0f*3.0f + 3.0f*1.0f + 3.0f*7.0f);
                
                m1 *= m2;
                assert(m1 == m3);
            }

            void testScalarMultiply() {
                const Mat4f m1(1.0f, 2.0f, 3.0f, 4.0f,
                               3.0f, 5.0f, 1.0f, 1.0f,
                               1.0f, 5.0f, 3.0f, 3.0f,
                               2.0f, 2.0f, 3.0f, 3.0f);
                const Mat4f m2(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                assert(m1 * 2.0f == m2);
            }
            
            void testScalarMultiplyAndAssign() {
                Mat4f m1(1.0f, 2.0f, 3.0f, 4.0f,
                         3.0f, 5.0f, 1.0f, 1.0f,
                         1.0f, 5.0f, 3.0f, 3.0f,
                         2.0f, 2.0f, 3.0f, 3.0f);
                const Mat4f m2(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                m1 *= 2.0f;
                assert(m1 == m2);
            }
            
            void testScalarDivide() {
                const Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                const Mat4f m2(1.0f, 2.0f, 3.0f, 4.0f,
                               3.0f, 5.0f, 1.0f, 1.0f,
                               1.0f, 5.0f, 3.0f, 3.0f,
                               2.0f, 2.0f, 3.0f, 3.0f);
                assert(m1 / 2.0f == m2);
            }
            
            void testScalarDivideAndAssign() {
                Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                         6.0f, 10.0f, 2.0f, 2.0f,
                         2.0f, 10.0f, 6.0f, 6.0f,
                         4.0f,  4.0f, 6.0f, 6.0f);
                const Mat4f m2(1.0f, 2.0f, 3.0f, 4.0f,
                               3.0f, 5.0f, 1.0f, 1.0f,
                               1.0f, 5.0f, 3.0f, 3.0f,
                               2.0f, 2.0f, 3.0f, 3.0f);
                m1 /= 2.0f;
                assert(m1 == m2);
            }
            
            void testVectorLeftMultiplyWithSameDimension() {
                const Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                const Vec4f v1(3.0f, 4.0f, 2.0f, 5.0f);
                const Vec4f v2(3.0f*2.0f + 4.0f* 6.0f + 2.0f* 2.0f + 5.0f*4.0f,
                               3.0f*4.0f + 4.0f*10.0f + 2.0f*10.0f + 5.0f*4.0f,
                               3.0f*6.0f + 4.0f* 2.0f + 2.0f* 6.0f + 5.0f*6.0f,
                               3.0f*8.0f + 4.0f* 2.0f + 2.0f* 6.0f + 5.0f*6.0f);
                assert(v1 * m1 == v2);
            }
            
            void testVectorLeftMultiplyWithOneLessDimension() {
                const Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                const Vec3f v1(3.0f,
                               4.0f,
                               2.0f);
                const Vec4f v14(v1, 1.0f);
                const Vec4f v24 = v14 * m1;
                const Vec3f v3 = v1 * m1;
                assert(v3.equals(v24.overLast()));
            }

            void testVectorRightMultiplyWithSameDimension() {
                const Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                const Vec4f v1(3.0f,
                               4.0f,
                               2.0f,
                               5.0f);
                const Vec4f v2(2.0f*3.0f +  4.0f*4.0f + 6.0f*2.0f + 8.0f*5.0f,
                               6.0f*3.0f + 10.0f*4.0f + 2.0f*2.0f + 2.0f*5.0f,
                               2.0f*3.0f + 10.0f*4.0f + 6.0f*2.0f + 6.0f*5.0f,
                               4.0f*3.0f +  4.0f*4.0f + 6.0f*2.0f + 6.0f*5.0f);
                assert(m1 * v1 == v2);
            }
            
            void testVectorRightMultiplyWithOneLessDimension() {
                const Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                               6.0f, 10.0f, 2.0f, 2.0f,
                               2.0f, 10.0f, 6.0f, 6.0f,
                               4.0f,  4.0f, 6.0f, 6.0f);
                const Vec3f v1(3.0f,
                               4.0f,
                               2.0f);
                const Vec4f v14(v1, 1.0f);
                const Vec4f v24 = m1 * v14;
                const Vec3f v2(v24.x() / v24.w(),
                               v24.y() / v24.w(),
                               v24.z() / v24.w());
                assert(m1 * v1 == v2);
            }
            
            void testSetIdentity() {
                Mat4f m1(2.0f,  4.0f, 6.0f, 8.0f,
                         6.0f, 10.0f, 2.0f, 2.0f,
                         2.0f, 10.0f, 6.0f, 6.0f,
                         4.0f,  4.0f, 6.0f, 6.0f);
                m1.setIdentity();
                assert(m1 == Mat4f::Identity);
            }
            
            void testTransposed() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat4f m2(2.0f,  6.0f,  2.0f, 4.0f,
                               4.0f, 10.0f,10.0f,  4.0f,
                               6.0f,  2.0f,  6.0f, 6.0f,
                               8.0f,  2.0f,  6.0f, 6.0f);
                assert(m1.transposed() == m2);
            }
            
            void testMinorMatrix00() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat3f m2(10.0f,  2.0f, 2.0f,
                               10.0f,  6.0f, 6.0f,
                                4.0f,  6.0f, 6.0f);
                assert(minorMatrix(m1, 0, 0) == m2);
            }
            
            void testMinorMatrix12() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat3f m2(2.0f,  4.0f,  8.0f,
                               2.0f, 10.0f,  6.0f,
                               4.0f,  4.0f,  6.0f);
                assert(minorMatrix(m1, 1, 2) == m2);
            }
            
            void testMinorMatrix13() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat3f m2(2.0f,  4.0f,  6.0f,
                               2.0f, 10.0f,  6.0f,
                               4.0f,  4.0f,  6.0f);
                assert(minorMatrix(m1, 1, 3) == m2);
            }

            void testMinorMatrix32() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat3f m2(2.0f,  4.0f,  8.0f,
                               6.0f, 10.0f,  2.0f,
                               2.0f, 10.0f,  6.0f);
                assert(minorMatrix(m1, 3, 2) == m2);
            }
            
            void testDeterminant1() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                assert(matrixDeterminant(m1) == -544.0f);
            }
            
            void testDeterminant2() {
                const Mat3f m1(2.0f,  4.0f,  6.0f,
                               2.0f, 10.0f,  6.0f,
                               4.0f,  4.0f,  6.0f);
                assert(matrixDeterminant(m1) == -72);
            }
            
            void testAdjoin() {
                Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                         6.0f, 10.0f,  2.0f, 2.0f,
                         2.0f, 10.0f,  6.0f, 6.0f,
                         4.0f,  4.0f,  6.0f, 6.0f);
                const Mat4f m2(   0.0f,  -72.0f,  104.0f,  -80.0f,
                                  0.0f,  -24.0f,  -56.0f,   64.0f,
                                272.0f,  136.0f, -136.0f, -272.0f,
                               -272.0f,  -72.0f,  104.0f,  192.0f);
                adjoinMatrix(m1);
                assert(m1 == m2);
            }
            
            void testAdjoint() {
                const Mat4f m1(2.0f,  4.0f,  6.0f, 8.0f,
                               6.0f, 10.0f,  2.0f, 2.0f,
                               2.0f, 10.0f,  6.0f, 6.0f,
                               4.0f,  4.0f,  6.0f, 6.0f);
                const Mat4f m2(   0.0f,  -72.0f,  104.0f,  -80.0f,
                                  0.0f,  -24.0f,  -56.0f,   64.0f,
                                272.0f,  136.0f, -136.0f, -272.0f,
                               -272.0f,  -72.0f,  104.0f,  192.0f);
                assert(adjointMatrix(m1) == m2);
            }
        };
    }
}


#endif
