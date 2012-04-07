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

#include <string>
#include <cmath>

using namespace std;

static const float AlmostZero = 0.001f;
static const float PointStatusEpsilon = 0.01f;

typedef enum {
    A_X,
    A_Y,
    A_Z
} EAxis;

typedef enum {
    PS_ABOVE, // point is above the plane
    PS_BELOW, // point is below the plane
    PS_INSIDE // point is contained inside the plane
} EPointStatus;

class Vec2f {
public:
    float x,y;
    Vec2f& operator= (const Vec2f& right);
    const Vec2f operator+ (const Vec2f& right) const;
    const Vec2f operator- (const Vec2f& right) const;
    const Vec2f operator* (const float right) const;
    const Vec2f operator/ (const float right) const;
    const float operator| (const Vec2f& right) const; // dot product
    Vec2f& operator+= (const Vec2f& right);
    Vec2f& operator-= (const Vec2f& right);
    Vec2f& operator*= (const float right);
    Vec2f& operator/= (const float right);
    float& operator[] (const int index);
    Vec2f();
    Vec2f(float x, float y);
    
    float length() const;
    float lengthSquared() const;
    const Vec2f normalize() const;
    bool equals(Vec2f other) const;
    bool equals(Vec2f other, float delta) const;
};

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
    float& operator[] (const int index);
    Vec3f();
    Vec3f(const string& str);
    Vec3f(float x, float y, float z);
    
    float length() const;
    float lengthSquared() const;
    const Vec3f normalize() const;
    bool equals(Vec3f other) const;
    bool equals(Vec3f other, float delta) const;
    
    const Vec3f snap() const;
    const Vec3f snap(float epsilon) const;
    const Vec3f rotate90(EAxis axis, bool clockwise) const;
    const Vec3f rotate90(EAxis axis, const Vec3f& center, bool clockwise) const;
    const Vec3f flip(EAxis axis) const;
    const Vec3f flip(EAxis axis, const Vec3f& center) const;
};

static const Vec3f XAxisPos( 1,  0,  0);
static const Vec3f XAxisNeg(-1,  0,  0);
static const Vec3f YAxisPos( 0,  1,  0);
static const Vec3f YAxisNeg( 0, -1,  0);
static const Vec3f ZAxisPos( 0,  0,  1);
static const Vec3f ZAxisNeg( 0,  0, -1);
static const Vec3f Null3f(0, 0, 0);
static const Vec3f Nan3f(NAN, NAN, NAN);

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
    float& operator[] (const int index);
    Vec4f();
    Vec4f(float x, float y, float z, float w);
    
    float length() const;
    float lengthSquared() const;
    const Vec4f normalize() const;
    bool equals(Vec4f other) const;
    bool equals(Vec4f other, float delta) const;
};

class Mat2f {
public:
    float v[4];
    Mat2f& operator= (const Mat2f& right);
    const Mat2f operator+ (const Mat2f& right) const;
    const Mat2f operator- (const Mat2f& right) const;
    const Mat2f operator* (const float right) const;
    const Vec2f operator* (const Vec2f& right) const;
    const Mat2f operator* (const Mat2f& right) const;
    const Mat2f operator/ (const float right) const;
    Mat2f& operator+= (const Mat2f& right);
    Mat2f& operator-= (const Mat2f& right);
    Mat2f& operator*= (const float right);
    Mat2f& operator*= (const Mat2f& right);
    Mat2f& operator/= (const float right);
    float& operator[] (const int index);
    Mat2f();
    Mat2f(float v11, float v12, float v21, float v22);
    void setIdentity();
    void setValue(int row, int col, float value);
    void setColumn(int col, const Vec2f& values);
    const Mat2f invert(bool& invertible) const;
    const Mat2f adjugate() const;
    const Mat2f negate() const;
    const Mat2f transpose() const;
    float determinant() const;
};

static const Mat2f IdentityM2f(1, 0, 0, 1);

class Mat3f {
public:
    float v[9];
    Mat3f& operator= (const Mat3f& right);
    const Mat3f operator+ (const Mat3f& right) const;
    const Mat3f operator- (const Mat3f& right) const;
    const Mat3f operator* (const float right) const;
    const Vec3f operator* (const Vec3f& right) const;
    const Mat3f operator* (const Mat3f& right) const;
    const Mat3f operator/ (const float right) const;
    Mat3f& operator+= (const Mat3f& right);
    Mat3f& operator-= (const Mat3f& right);
    Mat3f& operator*= (const float right);
    Mat3f& operator*= (const Mat3f& right);
    Mat3f& operator/= (const float right);
    float& operator[] (const int index);
    Mat3f();
    Mat3f(float v11, float v12, float v13, float v21, float v22, float v23, float v31, float v32, float v33);
    void setIdentity();
    void setValue(int row, int col, float value);
    void setColumn(int col, const Vec3f& values);
    const Mat3f invert(bool& invertible) const;
    const Mat3f adjugate() const;
    const Mat3f negate() const;
    const Mat3f transpose() const;
    float determinant() const;
    const Mat2f minor(int row, int col) const;
};

static const Mat3f IdentityM3f(1, 0, 0, 0, 1, 0, 0, 0, 1);

class Quat;
class Mat4f {
public:
    float v[16];
    Mat4f& operator= (const Mat4f& right);
    const Mat4f operator+ (const Mat4f& right) const;
    const Mat4f operator- (const Mat4f& right) const;
    const Mat4f operator* (const float right) const;
    const Vec3f operator* (const Vec3f& right) const;
    const Vec4f operator* (const Vec4f& right) const;
    const Mat4f operator* (const Mat4f& right) const;
    const Mat4f operator/ (const float right) const;
    Mat4f& operator+= (const Mat4f& right);
    Mat4f& operator-= (const Mat4f& right);
    Mat4f& operator*= (const float right);
    Mat4f& operator*= (const Mat4f& right);
    Mat4f& operator/= (const float right);
    float& operator[] (const int index);
    Mat4f();
    Mat4f(float v11, float v12, float v13, float v14, float v21, float v22, float v23, float v24, float v31, float v32, float v33, float v34, float v41, float v42, float v43, float v44);
    void setIdentity();
    void setValue(int row, int col, float value);
    void setColumn(int col, const Vec3f& values);
    void setColumn(int col, const Vec4f& values);
    void setSubMatrix(int index, const Mat2f& values);
    const Mat2f subMatrix(int index) const;
    const Mat4f invert(bool& invertible) const;
    const Mat4f adjugate() const;
    const Mat4f negate() const;
    const Mat4f transpose() const;
    float determinant() const;
    const Mat3f minor(int row, int col) const;
    
    const Mat4f rotate(float angle, const Vec3f& axis) const;
    const Mat4f rotate(const Quat& rotation) const;
    const Mat4f translate(const Vec3f& delta) const;
    const Mat4f scale(const Vec3f& factors) const;
};

static const Mat4f IdentityM4f(1, 0, 0, 0, 
                               0, 1, 0, 0, 
                               0, 0, 1, 0, 
                               0, 0, 0, 1);
static const Mat4f RotX90CWM4f( 1,  0,  0,  0, 
                                0,  0, -1,  0, 
                                0,  1,  0,  0, 
                                0,  0,  0,  1);
static const Mat4f RotY90CWM4f( 0,  0,  1,  0, 
                                0,  1,  0,  0, 
                               -1,  0,  0,  0, 
                                0,  0,  0,  1);
static const Mat4f RotZ90CWM4f( 0, -1,  0,  0, 
                                1,  0,  0,  0, 
                                0,  0,  1,  0, 
                                0,  0,  0,  1);
static const Mat4f RotX90CCWM4f( 1,  0,  0,  0, 
                                 0,  0,  1,  0, 
                                 0, -1,  0,  0, 
                                 0,  0,  0,  1);
static const Mat4f RotY90CCWM4f( 0,  0, -1,  0, 
                                 0,  1,  0,  0, 
                                 1,  0,  0,  0, 
                                 0,  0,  0,  1);
static const Mat4f RotZ90CCWM4f( 0,  1,  0,  0, 
                                -1,  0,  0,  0, 
                                 0,  0,  1,  0, 
                                 0,  0,  0,  1);
static const Mat4f MirXM4f(-1,  0,  0,  0, 
                            0,  1,  0,  0, 
                            0,  0,  1,  0, 
                            0,  0,  0,  1);
static const Mat4f MirYM4f( 1,  0,  0,  0, 
                            0, -1,  0,  0, 
                            0,  0,  1,  0, 
                            0,  0,  0,  1);
static const Mat4f MirZM4f( 1,  0,  0,  0, 
                            0,  1,  0,  0, 
                            0,  0, -1,  0, 
                            0,  0,  0,  1);

class Quat {
public:
    float s;
    Vec3f v;
    const Quat operator* (const Quat& right) const;
    const Vec3f operator* (const Vec3f& right) const;
    Quat& operator*= (const Quat& right);
    Quat();
    Quat(float angle, const Vec3f& axis);
    void setRotation(float angle, const Vec3f axis);
    const Quat conjugate() const ;
};

class Ray {
public:
    Vec3f origin;
    Vec3f direction;
    Ray();
    Ray(const Vec3f& origin, const Vec3f& direction);
    const Vec3f pointAtDistance(float distance) const;
    EPointStatus pointStatus(const Vec3f& point) const;
};

class Line {
public:
    Vec3f point;
    Vec3f direction;
    Line();
    Line(const Vec3f& point, const Vec3f& direction);
    const Vec3f pointAtDistance(float distance) const;
};

class BBox {
private:
    void repair();
public:
    Vec3f min;
    Vec3f max;
    const BBox operator+ (const BBox& right) const;
    const BBox operator+ (const Vec3f& right) const;
    BBox& operator+= (const BBox& right);
    BBox& operator+= (const Vec3f& right);
    BBox();
    BBox(const Vec3f& min, const Vec3f& max);
    const BBox maxBounds() const;
    const Vec3f center() const;
    bool contains(const Vec3f& point) const;
    bool contains(const BBox& bounds) const;
    bool intersects(const BBox& bounds) const;
    float intersectWithRay(const Ray& ray, Vec3f* sideNormal) const;
    float intersectWithRay(const Ray& ray) const;
    
    const BBox translate(const Vec3f& delta) const;
    const BBox rotate90(EAxis axis, bool clockwise) const;
    const BBox rotate90(EAxis axis, const Vec3f& center, bool clockwise) const;
    const BBox rotate(Quat rotation) const;
    const BBox rotate(Quat rotation, const Vec3f& center) const;
    const BBox flip(EAxis axis) const;
    const BBox flip(EAxis axis, const Vec3f& center) const;
    const BBox expand(float f);
};

class Plane {
public:
    Vec3f normal;
    float distance;
    Plane();
    Plane(const Vec3f& normal, float distance);
    Plane(const Vec3f& normal, const Vec3f& anchor);
    bool setPoints(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3);
    const Vec3f anchor() const;
    float intersectWithRay(const Ray& ray) const;
    float intersectWithLine(const Line& line) const;
    EPointStatus pointStatus(const Vec3f& point) const;
    float x(float y, float z) const;
    float y(float x, float z) const;
    float z(float x, float y) const;
    bool equals(const Plane& other) const;
    bool equals(const Plane& other, float epsilon) const;
    
    const Plane translate(const Vec3f& delta) const;
    const Plane rotate90(EAxis axis, bool clockwise) const;
    const Plane rotate90(EAxis axis, const Vec3f& center, bool clockwise) const;
    const Plane rotate(Quat rotation) const;
    const Plane rotate(Quat rotation, const Vec3f& center) const;
    const Plane flip(EAxis axis) const;
    const Plane flip(EAxis axis, const Vec3f& center) const;
};

#endif
