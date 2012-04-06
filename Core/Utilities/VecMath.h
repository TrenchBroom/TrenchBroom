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

#ifndef TrenchBroom_VecMath_h
#define TrenchBroom_VecMath_h

static const float AlmostZero = 0.001f;

class Vec3f {
public:
    float x,y,z;
    Vec3f& operator= (const Vec3f& right);
    const Vec3f operator+ (const Vec3f& right) const;
    const Vec3f operator- (const Vec3f& right) const;
    const Vec3f operator* (const float right) const;
    const Vec3f operator/ (const float right) const;
    const float operator| (const Vec3f& right) const; // dot product
    const Vec3f operator% (const Vec3f& right) const; // cross product
    Vec3f& operator+= (const Vec3f& right);
    Vec3f& operator-= (const Vec3f& right);
    Vec3f& operator*= (const float right);
    Vec3f& operator/= (const float right);
    Vec3f& operator%= (const Vec3f& right);
    const float& operator[] (const int index) const;
    Vec3f();
    Vec3f(float x, float y, float z);
    
    float length() const;
    float lengthSquared() const;
    const Vec3f normalize() const;
    bool equals(Vec3f other) const;
    bool equals(Vec3f other, float delta) const;
};

static const Vec3f XAxisPos( 1,  0,  0);
static const Vec3f XAxisNeg(-1,  0,  0);
static const Vec3f YAxisPos( 0,  1,  0);
static const Vec3f YAxisNeg( 0, -1,  0);
static const Vec3f ZAxisPos( 0,  0,  1);
static const Vec3f ZAxisNeg( 0,  0, -1);
static const Vec3f Null3f(0, 0, 0);

class Vec4f {
public:
    float x,y,z,w;
    Vec4f& operator= (const Vec4f& right);
    const Vec4f operator+ (const Vec4f& right) const;
    const Vec4f operator- (const Vec4f& right) const;
    const Vec4f operator* (const float right) const;
    const Vec4f operator/ (const float right) const;
    const float operator| (const Vec4f right) const; // dot product
    Vec4f& operator+= (const Vec4f& right);
    Vec4f& operator-= (const Vec4f& right);
    Vec4f& operator*= (const float right);
    Vec4f& operator/= (const float right);
    const float& operator[] (const int index) const;
    Vec4f();
    Vec4f(float x, float y, float z, float w);
    
    float length() const;
    float lengthSquared() const;
    const Vec4f normalize() const;
    bool equals(Vec4f other) const;
    bool equals(Vec4f other, float delta) const;
};

class Quat {
public:
    float s;
    Vec3f v;
    const Quat operator* (const Quat& right) const;
    const Vec3f operator* (const Vec3f& right) const;
    Quat& operator*= (const Quat& right);
    Quat();
    Quat(float angle, Vec3f axis);
    void setRotation(float angle, const Vec3f axis);
    const Quat conjugate() const ;
};

#endif
