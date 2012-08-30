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

#ifndef TrenchBroom_Math_h
#define TrenchBroom_Math_h

#include <cmath>
#include <limits>

namespace TrenchBroom {
    namespace Math {
        static const float AlmostZero = 0.001f;
        static const float PointStatusEpsilon = 0.01f;
        static const float Pi = 3.141592f;
        
        inline bool isnan(float f) {
#ifdef _MSC_VER
            return _isnan(f) != 0;
#else
            return std::isnan(f);
#endif
        }
        
        inline float nan() {
            return std::numeric_limits<float>::quiet_NaN();
        }
        
        inline float radians(float d) {
            return Pi * d / 180.0f;
        }
        
        inline float degrees(float r) {
            return 180.0f * r / Pi;
        }
        
        inline float round(float f) {
            return f > 0.0f ? floor(f + 0.5f) : ceil(f - 0.5f);
        }
        
        inline float correct(float f) {
            return round(1000.0f * f) / 1000.0f;
        }
        
        inline bool zero(float f) {
            return fabsf(f) <= AlmostZero;
        }
        
        inline bool pos(float f) {
            return f > AlmostZero;
        }
        
        inline bool neg(float f) {
            return f < -AlmostZero;
        }
        
        inline bool eq(float f1, float f2) {
            return fabsf(f1 - f2) < AlmostZero;
        }
        
        inline bool gt(float f1, float f2) {
            return f1 > f2 + AlmostZero;
        }
        
        inline bool lt(float f1, float f2) {
            return f1 < f2 - AlmostZero;
        }
        
        inline bool gte(float f1, float f2) {
            return !lt(f1, f2);
        }
        
        inline bool lte(float f1, float f2) {
            return !gt(f1, f2);
        }

		namespace Axis {
			typedef unsigned int Type;
			static const Type X = 0;
			static const Type Y = 1;
			static const Type Z = 2;
		}

		namespace PointStatus {
			typedef unsigned int Type;
			static const Type Above = 0;
			static const Type Below = 1;
			static const Type Inside = 2;
		}
    }
}

#endif
