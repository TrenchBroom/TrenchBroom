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

#include "Utility/Vec.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Quat {
        public:
            T s;
            Vec<T,3> v;

            Quat() : s(0.0), v(Vec<T,3>::Null) {}
            
            /**
             * Creates a new quaternion that represent a clounter-clockwise rotation by the given angle (in radians) about
             * the given axis.
             */
            Quat(const T angle, const Vec<T,3>& axis) {
                setRotation(angle, axis);
            }
            
            inline const Quat<T> operator- () const {
                return Quat(-s, v);
            }

            inline const Quat<T> operator* (const T right) const {
                return Quat(s * right, v);
            }
            
            inline Quat<T>& operator*= (const T right) {
                s *= right;
                return *this;
            }
            
            inline const Quat<T> operator* (const Quat<T>& right) const {
                Quat<T> result = *this;
                return result *= right;
            }
            
            inline const Vec<T,3> operator* (const Vec<T,3>& right) const {
                Quat<T> p;
                p.s = 0.0;
                p.v = right;
                p = *this * p * conjugated();
                return p.v;
            }
            
            inline Quat<T>& operator*= (const Quat<T>& right) {
                const T& t = right.s;
                const Vec<T,3>& w = right.v;
                
                const T nx = s * w.x + t * v.x + v.y * w.z - v.z * w.y;
                const T ny = s * w.y + t * v.y + v.z * w.x - v.x * w.z;
                const T nz = s * w.z + t * v.z + v.x * w.y - v.y * w.x;
                
                s = s * t - (v.dot(w));
                v.x = nx;
                v.y = ny;
                v.z = nz;
                return *this;
            }
            
            inline Quat<T>& setRotation(const T angle, const Vec3f axis) {
                s = std::cos(angle / static_cast<T>(2.0));
                v = axis * std::sin(angle / static_cast<T>(2.0));
                return *this;
            }

            inline Vec<T,3> axis() const {
                return v.normalized();
            }
            
            inline Quat<T>& conjugate() {
                v = -v;
                return *this;
            }

            inline const Quat conjugated() const {
                Quat<T> result;
                result.s = s;
                result.v = -v;
                return result;
            }
        };

        typedef Quat<float> Quatf;
        
        template <typename T>
        inline Quat<T> operator*(const T left, const Quat<T>& right) {
            return Quat<T>(left * right.s, right.v);
        }
    }
}

#endif
