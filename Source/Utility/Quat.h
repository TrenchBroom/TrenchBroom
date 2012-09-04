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

#ifndef TrenchBroom_Quat_h
#define TrenchBroom_Quat_h

#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class Quat {
        public:
            float s;
            Vec3f v;

            Quat() : s(0), v(Vec3f::Null) {}
            
            Quat(float angle, const Vec3f& axis) {
                setRotation(angle, axis);
            }
            
            const Quat operator* (const Quat& right) const {
                Quat result = *this;
                return result *= right;
            }
            
            const Vec3f operator* (const Vec3f& right) const {
                Quat p;
                p.s = 0;
                p.v = right;
                p = *this * p * conjugated();
                return p.v;
            }
            
            Quat& operator*= (const Quat& right) {
                const float& t = right.s;
                const Vec3f& w = right.v;
                
                float nx = s * w.x + t * v.x + v.y * w.z - v.z * w.y;
                float ny = s * w.y + t * v.y + v.z * w.x - v.x * w.z;
                float nz = s * w.z + t * v.z + v.x * w.y - v.y * w.x;
                
                s = s * t - (v.dot(w));
                v.x = nx;
                v.y = ny;
                v.z = nz;
                return *this;
            }
            
            void setRotation(float angle, const Vec3f axis) {
                s = cosf(angle / 2);
                v = axis * sinf(angle / 2);
            }
            
            void conjugate() {
                v *= -1.0f;
            }

            const Quat conjugated() const {
                Quat result;
                result.s = s;
                result.v = v * -1.0f;
                return result;
            }
        };
    }
}

#endif
