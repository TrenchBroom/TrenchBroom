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

#include "VecMath.h"
#include <assert.h>

namespace TrenchBroom {
    Vec3f& Vec3f::operator= (const Vec3f& right) {
        if (this != &right) {
            x = right.x;
            y = right.y;
            z = right.z;
        }
        return *this;
    }

    const Vec3f Vec3f::operator+ (const Vec3f& right) const {
        Vec3f result = *this;
        return result += right;
    }
    
    const Vec3f Vec3f::operator- (const Vec3f& right) const {
        Vec3f result = *this;
        return result -= right;
    }
    
    const Vec3f Vec3f::operator* (const float right) const {
        Vec3f result = *this;
        return result *= right;
    }
    
    const float Vec3f::operator| (const Vec3f& right) const {
        return x * right.x + y * right.y + z * right.z;
    }
    
    const Vec3f Vec3f::operator% (const Vec3f& right) const {
        Vec3f result = *this;
        return result %= right;
    }
    
    Vec3f& Vec3f::operator+= (const Vec3f& right) {
        x += right.x;
        y += right.y;
        z += right.z;
        return *this;
    }
    
    Vec3f& Vec3f::operator-= (const Vec3f& right) {
        x -= right.x;
        y -= right.y;
        z -= right.z;
        return *this;
    }
    
    Vec3f& Vec3f::operator*= (const float right) {
        x *= right;
        y *= right;
        z *= right;
        return *this;
    }
    
    Vec3f& Vec3f::operator%= (const Vec3f& right) {
        float xt = y * right.z - z * right.y;
        float yt = z * right.x - x * right.z;
        z = x * right.y - y * right.x;
        x = xt;
        y = yt;
        return *this;
    }
    
    const float& Vec3f::operator[] (const int index) const {
        assert(index >= 0 && index < 3);
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }

    Vec3f::Vec3f() : x(0), y(0), z(0) {}
    Vec3f::Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec4f& Vec4f::operator= (const Vec4f& right) {
        if (this != &right) {
            x = right.x;
            y = right.y;
            z = right.z;
            w = right.w;
        }
        return *this;
    }
    
    const Vec4f Vec4f::operator+ (const Vec4f& right) const {
        Vec4f result = *this;
        return result += right;
    }
    
    const Vec4f Vec4f::operator- (const Vec4f& right) const {
        Vec4f result = *this;
        return result -= right;
    }
    
    const Vec4f Vec4f::operator* (const float right) const {
        Vec4f result = *this;
        return result *= right;
    }
    
    Vec4f& Vec4f::operator+= (const Vec4f& right) {
        x += right.x;
        y += right.y;
        z += right.z;
        w += right.w;
        return *this;
    }
    
    Vec4f& Vec4f::operator-= (const Vec4f& right) {
        x -= right.x;
        y -= right.y;
        z -= right.z;
        w -= right.w;
        return *this;
    }
    
    Vec4f& Vec4f::operator*= (const float right) {
        x *= right;
        y *= right;
        z *= right;
        w *= right;
        return *this;
    }
    
    const float& Vec4f::operator[] (const int index) const {
        assert(index >= 0 && index < 4);
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        return w;
    }

    Vec4f::Vec4f() : x(0), y(0), z(0), w(0) {}
    Vec4f::Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
}