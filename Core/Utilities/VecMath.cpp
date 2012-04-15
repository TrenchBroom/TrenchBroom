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
#include <cassert>
#include <cstdlib>

#pragma mark Vec2f

Vec2f& Vec2f::operator= (const Vec2f& right) {
    if (this != &right) {
        x = right.x;
        y = right.y;
    }
    return *this;
}

const Vec2f Vec2f::operator+ (const Vec2f& right) const {
    return Vec2f(x + right.x, 
                 y + right.y);
}

const Vec2f Vec2f::operator- (const Vec2f& right) const {
    return Vec2f(x - right.x, 
                 y - right.y);
}

const Vec2f Vec2f::operator* (const float right) const {
    return Vec2f(x * right, 
                 y * right);
}

const Vec2f Vec2f::operator/ (const float right) const {
    return Vec2f(x / right, 
                 y / right);
}

const float Vec2f::operator| (const Vec2f& right) const {
    return x * right.x + y * right.y;
}

Vec2f& Vec2f::operator+= (const Vec2f& right) {
    x += right.x;
    y += right.y;
    return *this;
}

Vec2f& Vec2f::operator-= (const Vec2f& right) {
    x -= right.x;
    y -= right.y;
    return *this;
}

Vec2f& Vec2f::operator*= (const float right) {
    x *= right;
    y *= right;
    return *this;
}

Vec2f& Vec2f::operator/= (const float right) {
    x /= right;
    y /= right;
    return *this;
}

float& Vec2f::operator[] (const int index) {
    assert(index >= 0 && index < 2);
    if (index == 0) return x;
    return y;
}

Vec2f::Vec2f() : x(0), y(0) {}
Vec2f::Vec2f(float x, float y) : x(x), y(y) {}

float Vec2f::length() const {
    return sqrt(lengthSquared());
}

float Vec2f::lengthSquared() const {
    return *this | *this;
}

const Vec2f Vec2f::normalize() const {
    float l = length();
    return Vec2f(x / l, 
                 y / l);
}

bool Vec2f::equals(const Vec2f& other) const {
    return equals(other, AlmostZero);
}

bool Vec2f::equals(const Vec2f& other, float delta) const {
    Vec2f diff = other - *this;
    return diff.lengthSquared() <= delta * delta;
}

#pragma mark Vec3f
Vec3f& Vec3f::operator= (const Vec3f& right) {
    if (this != &right) {
        x = right.x;
        y = right.y;
        z = right.z;
    }
    return *this;
}

const Vec3f Vec3f::operator+ (const Vec3f& right) const {
    return Vec3f(x + right.x, 
                 y + right.y, 
                 z + right.z);
}

const Vec3f Vec3f::operator- (const Vec3f& right) const {
    return Vec3f(x - right.x, 
                 y - right.y, 
                 z - right.z);
}

const Vec3f Vec3f::operator* (const float right) const {
    return Vec3f(x * right, 
                 y * right, 
                 z * right);
}

const Vec3f Vec3f::operator/ (const float right) const {
    return Vec3f(x / right, 
                 y / right, 
                 z / right);
}

const float Vec3f::operator| (const Vec3f& right) const {
    return x * right.x + y * right.y + z * right.z;
}

const Vec3f Vec3f::operator% (const Vec3f& right) const {
    return Vec3f(y * right.z - z * right.y, 
                 z * right.x - x * right.z, 
                 x * right.y - y * right.x);
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

Vec3f& Vec3f::operator/= (const float right) {
    x /= right;
    y /= right;
    z /= right;
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

float& Vec3f::operator[] (const int index) {
    assert(index >= 0 && index < 3);
    if (index == 0) return x;
    if (index == 1) return y;
    return z;
}

Vec3f::Vec3f() : x(0), y(0), z(0) {}

Vec3f::Vec3f(const string& str) {
    const char* cstr = str.c_str();
    size_t pos = 0;
    string blank = " \t\n\r";

    pos = str.find_first_not_of(blank, pos);
    x = atof(cstr + pos);
    pos = str.find_first_of(blank, pos);
    pos = str.find_first_not_of(blank, pos);
    y = atof(cstr + pos);
    pos = str.find_first_of(blank, pos);
    pos = str.find_first_not_of(blank, pos);
    z = atof(cstr + pos);
}

Vec3f::Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3f::length() const {
    return sqrt(lengthSquared());
}

float Vec3f::lengthSquared() const {
    return *this | *this;
}

const Vec3f Vec3f::normalize() const {
    float l = length();
    return Vec3f(x / l, 
                 y / l,
                 z / l);
}

bool Vec3f::equals(const Vec3f& other) const {
    return equals(other, AlmostZero);
}

bool Vec3f::equals(const Vec3f& other, float delta) const {
    Vec3f diff = other - *this;
    return diff.lengthSquared() <= delta * delta;
}

EAxis Vec3f::strongestAxis() const {
    int ax = fabsf(x);
    int ay = fabsf(y);
    int az = fabsf(z);
    if (ax >= ay && ax >= az) return TB_AX_X;
    if (ay >= ax && ay >= az) return TB_AX_Y;
    return TB_AX_Z;
}

const Vec3f Vec3f::snap() const {
    return snap(AlmostZero);
}

const Vec3f Vec3f::snap(float epsilon) const {
    float xr = roundf(x);
    float yr = roundf(y);
    float zr = roundf(z);
    return Vec3f(feq(x, xr) ? xr : x, 
                 feq(y, yr) ? yr : y, 
                 feq(z, zr) ? zr : z);
}

const Vec3f Vec3f::rotate90(EAxis axis, bool clockwise) const {
    switch (axis) {
        case TB_AX_X:
            if (clockwise) return Vec3f(x, z, -y);
            return Vec3f(x, -z, y);
        case TB_AX_Y:
            if (clockwise) return Vec3f(-z, y, x);
            return Vec3f(z, y, -x);
        case TB_AX_Z:
            if (clockwise) return Vec3f(y, -x, z);
            return Vec3f(-y, x, z);
    }
}

const Vec3f Vec3f::rotate90(EAxis axis, const Vec3f& center, bool clockwise) const {
    Vec3f result = *this - center;
    result.rotate90(axis, clockwise);
    return result += center;
}

const Vec3f Vec3f::flip(EAxis axis) const {
    switch (axis) {
        case TB_AX_X:
            return Vec3f(-x, y, z);
        case TB_AX_Y:
            return Vec3f(x, -y, z);
        case TB_AX_Z:
            return Vec3f(x, y, -z);
    }
}

const Vec3f Vec3f::flip(EAxis axis, const Vec3f& center) const {
    Vec3f result = *this - center;
    result.flip(axis);
    return result += center;
}

#pragma mark Vec4f

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
    return Vec4f(x + right.x, 
                 y + right.y, 
                 z + right.z, 
                 w + right.w);
}

const Vec4f Vec4f::operator- (const Vec4f& right) const {
    return Vec4f(x - right.x, 
                 y - right.y, 
                 z - right.z, 
                 w - right.w);
}

const Vec4f Vec4f::operator* (const float right) const {
    return Vec4f(x * right, 
                 y * right, 
                 z * right, 
                 w * right);
}

const Vec4f Vec4f::operator/ (const float right) const {
    return Vec4f(x / right, 
                 y / right, 
                 z / right, 
                 w / right);
}

const float Vec4f::operator| (const Vec4f right) const {
    return x * right.x + y * right.y + z * right.z + w * right.w;
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

Vec4f& Vec4f::operator/= (const float right) {
    x /= right;
    y /= right;
    z /= right;
    w /= right;
    return *this;
}

float& Vec4f::operator[] (const int index) {
    assert(index >= 0 && index < 4);
    if (index == 0) return x;
    if (index == 1) return y;
    if (index == 2) return z;
    return w;
}

Vec4f::Vec4f() : x(0), y(0), z(0), w(0) {}
Vec4f::Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

float Vec4f::length() const {
    return sqrt(lengthSquared());
}

float Vec4f::lengthSquared() const {
    return *this | *this;
}

const Vec4f Vec4f::normalize() const {
    float l = length();
    return Vec4f(x / l, 
                 y / l, 
                 z / l, 
                 w / l);
}

bool Vec4f::equals(const Vec4f& other) const {
    return equals(other, AlmostZero);
}

bool Vec4f::equals(const Vec4f& other, float delta) const {
    Vec4f diff = other - *this;
    return diff.lengthSquared() <= delta * delta;
}

#pragma mark Mat2f

Mat2f& Mat2f::operator= (const Mat2f& right) {
    for (int i = 0; i < 4; i++)
        v[i] = right.v[i];
    return *this;
}

const Mat2f Mat2f::operator+ (const Mat2f& right) const {
    return Mat2f(v[0] + right.v[0], v[2] + right.v[2],
                 v[1] + right.v[1], v[3] + right.v[3]);
}

const Mat2f Mat2f::operator- (const Mat2f& right) const {
    return Mat2f(v[0] - right.v[0], v[2] - right.v[2],
                 v[1] - right.v[1], v[3] - right.v[3]);
}

const Mat2f Mat2f::operator* (const float right) const {
    return Mat2f(v[0] * right, v[2] * right,
                 v[1] * right, v[3] * right);
}

const Vec2f Mat2f::operator* (const Vec2f& right) const {
    return Vec2f(v[0] * right.x + v[2] * right.y, 
                 v[1] * right.x + v[3] * right.y);
}

const Mat2f Mat2f::operator* (const Mat2f& right) const {
    return Mat2f(v[0] * right.v[0] + v[2] * right.v[1],
                 v[1] * right.v[0] + v[3] * right.v[1],
                 v[0] * right.v[2] + v[2] * right.v[3],
                 v[1] * right.v[2] + v[3] * right.v[3]);
}

const Mat2f Mat2f::operator/ (const float right) const {
    return Mat2f(v[0] / right, 
                 v[1] / right, 
                 v[2] / right, 
                 v[3] / right);
}

Mat2f& Mat2f::operator+= (const Mat2f& right) {
    for (int i = 0; i < 4; i++)
        v[i] += right.v[i];
    return *this;
}

Mat2f& Mat2f::operator-= (const Mat2f& right) {
    for (int i = 0; i < 4; i++)
        v[i] -= right.v[i];
    return *this;
}

Mat2f& Mat2f::operator*= (const float right) {
    for (int i = 0; i < 4; i++)
        v[i] *= right;
    return *this;
}

Mat2f& Mat2f::operator*= (const Mat2f& right) {
    *this = *this * right;
    return *this;
}

Mat2f& Mat2f::operator/= (const float right) {
    *this *= (1 / right);
    return *this;
}

float& Mat2f::operator[] (const int index) {
    assert(index >= 0 && index < 4);
    return v[index];
}

Mat2f::Mat2f() {
    for (int i = 0; i < 4; i++)
        v[i] = 0;
}

Mat2f::Mat2f(float v11, float v12, float v21, float v22) {
    v[0] = v11;
    v[2] = v12;
    v[1] = v21;
    v[3] = v22;
}

void Mat2f::setIdentity() {
    for (int c = 0; c < 2; c++)
        for (int r = 0; r < 2; r++)
            v[c * 2 + r] = c == r ? 1 : 0;
}

void Mat2f::setValue(int row, int col, float value) {
    assert(row >= 0 && row < 2);
    assert(col >= 0 && col < 2);
    v[2 * col + row] = value;
}

void Mat2f::setColumn(int col, const Vec2f& values) {
    assert(col >= 0 && col < 2);
    v[col + 0] = values.x;
    v[col + 1] = values.y;
}

const Mat2f Mat2f::invert(bool& invertible) const {
    float det = determinant();
    if (det == 0) {
        invertible = false;
        return Mat2f();
    }
    
    invertible = true;
    return adjugate() / det;
}

const Mat2f Mat2f::adjugate() const {
    Mat2f result;
    result[0] =  v[3];
    result[3] =  v[0];
    result[1] = -v[1];
    result[2] = -v[2];
    return result;
}

const Mat2f Mat2f::negate() const {
    return Mat2f(v[0] * -1, v[1] * -1,
                 v[2] * -1, v[3] * -1);
}

const Mat2f Mat2f::transpose() const {
    Mat2f result;
    for (int c = 0; c < 2; c++) {
        result[c * 2 + c] = v[c * 2 + c];
        for (int r = c + 1; r < 2; r++) {
            result[c * 2 + r] = v[r * 2 + c];
            result[r * 2 + c] = v[c * 2 + r];
        }
    }
    return result;
}

float Mat2f::determinant() const {
    return v[0] * v[3] - v[2] * v[1];
}

#pragma mark Mat3f

Mat3f& Mat3f::operator= (const Mat3f& right) {
    for (int i = 0; i < 9; i++)
        v[i] = right.v[i];
    return *this;
}

const Mat3f Mat3f::operator+ (const Mat3f& right) const {
    return Mat3f(v[0] + right.v[0], v[1] + right.v[1], v[2] + right.v[2],
                 v[3] + right.v[3], v[4] + right.v[4], v[5] + right.v[5],
                 v[6] + right.v[6], v[7] + right.v[7], v[8] + right.v[8]);
}

const Mat3f Mat3f::operator- (const Mat3f& right) const {
    return Mat3f(v[0] - right.v[0], v[1] - right.v[1], v[2] - right.v[2],
                 v[3] - right.v[3], v[4] - right.v[4], v[5] - right.v[5],
                 v[6] - right.v[6], v[7] - right.v[7], v[8] - right.v[8]);
}

const Mat3f Mat3f::operator* (const float right) const {
    return Mat3f(v[0] * right, v[1] * right, v[2] * right,
                 v[3] * right, v[4] * right, v[5] * right,
                 v[6] * right, v[7] * right, v[8] * right);
}

const Vec3f Mat3f::operator* (const Vec3f& right) const {
    return Vec3f(v[0] * right.x + v[3] * right.y + v[6] * right.z,
                 v[1] * right.x + v[4] * right.y + v[7] * right.z,
                 v[2] * right.x + v[5] * right.y + v[8] * right.z);
}

const Mat3f Mat3f::operator* (const Mat3f& right) const {
    Mat3f result;
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            for (int i = 0; i < 3; i++)
                result[c * 3 + r] += v[i * 3 + r] * right.v[c * 3 + i];
    return result;
}

const Mat3f Mat3f::operator/ (const float right) const {
    return Mat3f(v[0] / right, v[1] / right, v[2] / right,
                 v[3] / right, v[4] / right, v[5] / right,
                 v[6] / right, v[7] / right, v[8] / right);
}

Mat3f& Mat3f::operator+= (const Mat3f& right) {
    for (int i = 0; i < 9; i++)
        v[i] += right.v[i];
    return *this;
}

Mat3f& Mat3f::operator-= (const Mat3f& right) {
    for (int i = 0; i < 9; i++)
        v[i] -= right.v[i];
    return *this;
}

Mat3f& Mat3f::operator*= (const float right) {
    for (int i = 0; i < 9; i++)
        v[i] *= right;
    return *this;
}

Mat3f& Mat3f::operator*= (const Mat3f& right) {
    *this = *this * right;
    return *this;
}

Mat3f& Mat3f::operator/= (const float right) {
    *this *= (1 / right);
    return *this;
}

float& Mat3f::operator[] (const int index) {
    assert(index >= 0 && index < 9);
    return v[index];
}

Mat3f::Mat3f() {
    for (int i = 0; i < 9; i++)
        v[i] = 0;
}

Mat3f::Mat3f(float v11, float v12, float v13, float v21, float v22, float v23, float v31, float v32, float v33) {
    v[0] = v11; v[3] = v12; v[6] = v13;
    v[1] = v21; v[4] = v22; v[7] = v23;
    v[2] = v31; v[5] = v32; v[8] = v33;
}

void Mat3f::setIdentity() {
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            v[c * 3 + r] = c == r ? 1 : 0;
}

void Mat3f::setValue(int row, int col, float value) {
    assert(row >= 0 && row < 3);
    assert(col >= 0 && col < 3);
    v[col * 3 + row] = value;
}

void Mat3f::setColumn(int col, const Vec3f& values) {
    v[col + 0] = values.x;
    v[col + 1] = values.y;
    v[col + 2] = values.z;
}

const Mat3f Mat3f::invert(bool& invertible) const {
    float det = determinant();
    invertible = det != 0;
    if (!invertible) return Mat3f();
    return adjugate() / det;
}

const Mat3f Mat3f::adjugate() const {
    Mat3f result;
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            result[c * 3 + r] = ((c + r) % 2 == 0 ? 1 : -1) * minor(c, r).determinant();
    return result;
}

const Mat3f Mat3f::negate() const {
    return Mat3f(v[0] * - 1, v[1] * - 1, v[2] * - 1,
                 v[3] * - 1, v[4] * - 1, v[5] * - 1,
                 v[6] * - 1, v[7] * - 1, v[8] * - 1);
}

const Mat3f Mat3f::transpose() const {
    Mat3f result;
    for (int c = 0; c < 3; c++) {
        result[c * 3 + c] = v[c * 3 + c];
        for (int r = c + 1; r < 3; r++) {
            result[c * 3 + r] = v[r * 3 + c];
            result[r * 3 + c] = v[c * 3 + r];
        }
    }
    return result;
}

float Mat3f::determinant() const {
    return v[0] * v[4] * v[8]
         + v[3] * v[7] * v[2]
         + v[6] * v[1] * v[5]
         - v[2] * v[4] * v[6]
         - v[5] * v[7] * v[0]
         - v[8] * v[1] * v[3];
}

const Mat2f Mat3f::minor(int row, int col) const {
    Mat2f result;
    int i = 0;
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            if (c != col && r != row)
                result[i++] = v[c * 3 + r];
    return result;
}

#pragma mark Mat4f

Mat4f& Mat4f::operator= (const Mat4f& right) {
    for (int i = 0; i < 16; i++)
        v[i] = right.v[i];
    return *this;
}

const Mat4f Mat4f::operator+ (const Mat4f& right) const {
    return Mat4f(v[ 0] + right.v[ 0], v[ 1] + right.v[ 1], v[ 2] + right.v[ 2], v[ 3] + right.v[ 3],
                 v[ 4] + right.v[ 4], v[ 5] + right.v[ 5], v[ 6] + right.v[ 6], v[ 7] + right.v[ 7],
                 v[ 8] + right.v[ 8], v[ 9] + right.v[ 9], v[10] + right.v[10], v[11] + right.v[11],
                 v[12] + right.v[12], v[13] + right.v[13], v[14] + right.v[14], v[15] + right.v[15]);
}

const Mat4f Mat4f::operator- (const Mat4f& right) const {
    return Mat4f(v[ 0] - right.v[ 0], v[ 1] - right.v[ 1], v[ 2] - right.v[ 2], v[ 3] - right.v[ 3],
                 v[ 4] - right.v[ 4], v[ 5] - right.v[ 5], v[ 6] - right.v[ 6], v[ 7] - right.v[ 7],
                 v[ 8] - right.v[ 8], v[ 9] - right.v[ 9], v[10] - right.v[10], v[11] - right.v[11],
                 v[12] - right.v[12], v[13] - right.v[13], v[14] - right.v[14], v[15] - right.v[15]);
}

const Mat4f Mat4f::operator* (const float right) const {
    return Mat4f(v[ 0] * right, v[ 1] * right, v[ 2] * right, v[ 3] * right,
                 v[ 4] * right, v[ 5] * right, v[ 6] * right, v[ 7] * right,
                 v[ 8] * right, v[ 9] * right, v[10] * right, v[11] * right,
                 v[12] * right, v[13] * right, v[14] * right, v[15] * right);
}

const Vec3f Mat4f::operator* (const Vec3f& right) const {
    float w = v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15];
    return Vec3f((v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12]) / w,
                 (v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13]) / w,
                 (v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14]) / w);
}

const Vec4f Mat4f::operator* (const Vec4f& right) const {
    return Vec4f(v[ 0] * right.x + v[ 4] * right.y + v[ 8] * right.z + v[12] * right.w,
                 v[ 1] * right.x + v[ 5] * right.y + v[ 9] * right.z + v[13] * right.w,
                 v[ 2] * right.x + v[ 6] * right.y + v[10] * right.z + v[14] * right.w,
                 v[ 3] * right.x + v[ 7] * right.y + v[11] * right.z + v[15] * right.w);
}

const Mat4f Mat4f::operator* (const Mat4f& right) const {
    Mat4f result;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            result[c * 4 + r] = 0;
            for (int i = 0; i < 4; i++)
                result[c * 4 + r] += v[i * 4 + r] * right.v[c * 4 + i];
        }
    }
    return result;
}

const Mat4f Mat4f::operator/ (const float right) const {
    Mat4f result = *this;
    return result /= right;
}

Mat4f& Mat4f::operator+= (const Mat4f& right) {
    for (int i = 0; i < 16; i++)
        v[i] += right.v[i];
    return *this;
}

Mat4f& Mat4f::operator-= (const Mat4f& right) {
    for (int i = 0; i < 16; i++)
        v[i] -= right.v[i];
    return *this;
}

Mat4f& Mat4f::operator*= (const float right) {
    for (int i = 0; i < 16; i++)
        v[i] *= right;
    return *this;
}

Mat4f& Mat4f::operator*= (const Mat4f& right) {
    *this = *this * right;
    return *this;
}

Mat4f& Mat4f::operator/= (const float right) {
    *this *= (1 / right);
    return *this;
}

float& Mat4f::operator[] (const int index) {
    assert(index >= 0 && index < 16);
    return v[index];
}

Mat4f::Mat4f() {
    for (int i = 0; i < 16; i++)
        v[i] = 0;
}

Mat4f::Mat4f(float v11, float v12, float v13, float v14, float v21, float v22, float v23, float v24, float v31, float v32, float v33, float v34, float v41, float v42, float v43, float v44) {
    v[ 0] = v11; v[ 4] = v12; v[ 8] = v13; v[12] = v14;
    v[ 1] = v21; v[ 5] = v22; v[ 9] = v23; v[13] = v24;
    v[ 2] = v31; v[ 6] = v32; v[10] = v33; v[14] = v34;
    v[ 3] = v41; v[ 7] = v42; v[11] = v43; v[15] = v44;
}

void Mat4f::setIdentity() {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            v[c * 4 + r] = r == c ? 1 : 0;
}

void Mat4f::setValue(int row, int col, float value) {
    assert(row >= 0 && row < 4);
    assert(col >= 0 && col < 4);
    v[col * 4 + row] = value;
}

void Mat4f::setColumn(int col, const Vec3f& values) {
    assert(col >= 0 && col < 4);
    v[col * 4 + 0] = values.x;
    v[col * 4 + 1] = values.y;
    v[col * 4 + 2] = values.z;
    v[col * 4 + 3] = 0;
}

void Mat4f::setColumn(int col, const Vec4f& values) {
    assert(col >= 0 && col < 4);
    v[col * 4 + 0] = values.x;
    v[col * 4 + 1] = values.y;
    v[col * 4 + 2] = values.z;
    v[col * 4 + 3] = values.w;
}

void Mat4f::setSubMatrix(int index, const Mat2f& values) {
    switch (index) {
        case 0:
            v[ 0] = values.v[0];
            v[ 1] = values.v[1];
            v[ 4] = values.v[2];
            v[ 5] = values.v[3];
            break;
        case 1:
            v[ 2] = values.v[0];
            v[ 3] = values.v[1];
            v[ 6] = values.v[2];
            v[ 7] = values.v[3];
            break;
        case 2:
            v[ 8] = values.v[0];
            v[ 9] = values.v[1];
            v[12] = values.v[2];
            v[13] = values.v[3];
            break;
        case 3:
            v[10] = values.v[0];
            v[11] = values.v[1];
            v[14] = values.v[2];
            v[15] = values.v[3];
            break;
        default:
            break;
    }
}

const Mat2f Mat4f::subMatrix(int index) const {
    Mat2f result;
    switch (index) {
        case 0:
            result[0] = v[0];
            result[1] = v[1];
            result[2] = v[4];
            result[3] = v[5];
            break;
        case 1:
            result[0] = v[2];
            result[1] = v[3];
            result[2] = v[6];
            result[3] = v[7];
            break;
        case 2:
            result[0] = v[8];
            result[1] = v[9];
            result[2] = v[12];
            result[3] = v[13];
            break;
        case 3:
            result[0] = v[10];
            result[1] = v[11];
            result[2] = v[14];
            result[3] = v[15];
            break;
        default:
            break;
    }
    return result;
}

const Mat4f Mat4f::invert(bool& invertible) const {
    float det = determinant();
    invertible = det != 0;
    if (!invertible) return Mat4f();
    
    Mat2f A, Ai;
    bool invertibleA;

    A = subMatrix(0);
    Ai = A.invert(invertibleA);
    if (invertibleA) { // use quick method
        Mat4f result;
        Mat2f B, C, D, CAi, CAiB, AiB;
        bool invertibleD;
        
        B = subMatrix(2);
        C = subMatrix(1);
        D = subMatrix(3);

        CAi = C * Ai;
        CAiB = CAi * B;
        AiB = Ai * B;
        
        // calculate D
        D = (D - CAiB).invert(invertibleD);
        
        // calculate -C and -B
        C = (D * CAi).negate();
        B = AiB * D;
        A = (B * CAi * Ai).negate();

        result.setSubMatrix(0, A);
        result.setSubMatrix(1, C);
        result.setSubMatrix(2, B);
        result.setSubMatrix(3, D);
        return result;
    }
    
    return adjugate() / det;
}

const Mat4f Mat4f::adjugate() const {
    Mat4f result;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            result[c * 4 + r] = ((c + r) % 2 == 0 ? 1 : -1) * minor(c, r).determinant();
    return result.transpose();
}

const Mat4f Mat4f::negate() const {
    return Mat4f(v[ 0] * -1, v[ 1] * -1, v[ 2] * -1, v[ 3] * -1,
                 v[ 4] * -1, v[ 5] * -1, v[ 6] * -1, v[ 7] * -1,
                 v[ 8] * -1, v[ 9] * -1, v[10] * -1, v[11] * -1,
                 v[12] * -1, v[13] * -1, v[14] * -1, v[15] * -1);
}

const Mat4f Mat4f::transpose() const {
    Mat4f result;
    for (int c = 0; c < 4; c++) {
        result[c * 4 + c] = v[c * 4 + c];
        for (int r = c + 1; r < 4; r++) {
            result[c * 4 + r] = v[r * 4 + c];
            result[r * 4 + c] = v[c * 4 + r];
        }
    }
    return result;
}

float Mat4f::determinant() const {
    // Laplace after first col
    float det = 0;
    for (int r = 0; r < 4; r++)
        det += (r % 2 == 0 ? 1 : -1) *v[r] * minor(0, r).determinant();
    return det;
}

const Mat3f Mat4f::minor(int row, int col) const {
    Mat3f result;
    int i = 0;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            if (c != col && r != row)
                result[i++] = v[c * 4 + r];
    return result;
}

const Mat4f Mat4f::rotate(float angle, const Vec3f& axis) const {
    float s = sinf(angle);
    float c = cosf(angle);
    float i = 1 - c;
    
    float ix  = i  * axis.x;
    float ix2 = ix * axis.x;
    float ixy = ix * axis.y;
    float ixz = ix * axis.z;
    
    float iy  = i  * axis.y;
    float iy2 = iy * axis.y;
    float iyz = iy * axis.z;
    
    float iz2 = i  * axis.z * axis.z;
    
    float sx = s * axis.x;
    float sy = s * axis.y;
    float sz = s * axis.z;
    
    Mat4f temp;
    temp[ 0] = ix2 + c;
    temp[ 1] = ixy - sz;
    temp[ 2] = ixz + sy;
    temp[ 3] = 0;
    
    temp[ 4] = ixy + sz;
    temp[ 5] = iy2 + c;
    temp[ 6] = iyz - sx;
    temp[ 7] = 0;
    
    temp[ 8] = ixz - sy;
    temp[ 9] = iyz + sx;
    temp[10] = iz2 + c;
    temp[11] = 0;
    
    temp[12] = 0;
    temp[13] = 0;
    temp[14] = 0;
    temp[15] = 1;
    
    return *this * temp;
}

const Mat4f Mat4f::rotate(const Quat& rotation) const {
    float a = rotation.s;
    float b = rotation.v.x;
    float c = rotation.v.y;
    float d = rotation.v.z;
    
    float a2 = a * a;
    float b2 = b * b;
    float c2 = c * c;
    float d2 = d * d;
    
    Mat4f temp;
    temp[ 0] = a2 + b2 - c2 - d2;
    temp[ 1] = 2 * b * c + 2 * a * d;
    temp[ 2] = 2 * b * d - 2 * a * c;
    temp[ 3] = 0;
    
    temp[ 4] = 2 * b * c - 2 * a * d;
    temp[ 5] = a2 - b2 + c2 - d2;
    temp[ 6] = 2 * c * d + 2 * a * b;
    temp[ 7] = 0;
    
    temp[ 8] = 2 * b * d + 2 * a * c;
    temp[ 9] = 2 * c * d - 2 * a * b;
    temp[10] = a2 - b2 - c2 + d2;
    temp[11] = 0;
    
    temp[12] = 0;
    temp[13] = 0;
    temp[14] = 0;
    temp[15] = 1;
    
    return *this * temp;
}

const Mat4f Mat4f::translate(const Vec3f& delta) const {
    Mat4f temp;
    temp.setIdentity();
    temp[12] += delta.x;
    temp[13] += delta.y;
    temp[14] += delta.z;
    return *this * temp;
}

const Mat4f Mat4f::scale(const Vec3f& factors) const {
    return Mat4f(v[ 0] * factors.x, v[ 1] * factors.y, v[ 2] * factors.z, v[ 3],
                 v[ 4] * factors.x, v[ 5] * factors.y, v[ 6] * factors.z, v[ 7],
                 v[ 8] * factors.x, v[ 9] * factors.y, v[10] * factors.z, v[11],
                 v[12] * factors.x, v[13] * factors.y, v[14] * factors.z, v[15]);
}

#pragma mark Quat

const Quat Quat::operator* (const Quat& right) const {
    Quat result = *this;
    return result *= right;
}

const Vec3f Quat::operator* (const Vec3f& right) const {
    Quat p;
    p.s = 0;
    p.v = right;
    p = *this * p * conjugate();
    return p.v;
}

Quat& Quat::operator*= (const Quat& right) {
    const float& t = right.s;
    const Vec3f& w = right.v;

    float nx = s * w.x + t * v.x + v.y * w.z - v.z * w.y;
    float ny = s * w.y + t * v.y + v.z * w.x - v.x * w.z;
    float nz = s * w.z + t * v.z + v.x * w.y - v.y * w.x;
    
    s = s * t - (v | w);
    v.x = nx;
    v.y = ny;
    v.z = nz;
    return *this;
}

Quat::Quat() : s(0), v(Null3f) {}

Quat::Quat(float angle, const Vec3f& axis) {
    setRotation(angle, axis);
}

void Quat::setRotation(float angle, const Vec3f axis) {
    s = cosf(angle / 2);
    v = axis * sinf(angle / 2);
}

const Quat Quat::conjugate() const {
    Quat result;
    result.s = s;
    result.v = v * -1;
    return result;
}

#pragma mark Ray

Ray::Ray() : origin(Null3f), direction(Null3f) {}

Ray::Ray(const Vec3f& origin, const Vec3f& direction) : origin(origin), direction(direction) {}

const Vec3f Ray::pointAtDistance(float distance) const {
    return Vec3f(origin.x + direction.x * distance,
                 origin.y + direction.y * distance,
                 origin.z + direction.z * distance);
}

EPointStatus Ray::pointStatus(const Vec3f& point) const {
    float dot = direction | (point - origin);
    if (dot >  PointStatusEpsilon) return TB_PS_ABOVE;
    if (dot < -PointStatusEpsilon) return TB_PS_BELOW;
    return TB_PS_INSIDE;
}

#pragma mark Line

Line::Line() : point(Null3f), direction(Null3f) {}

Line::Line(const Vec3f& point, const Vec3f& direction) : point(point), direction(direction) {}

const Vec3f Line::pointAtDistance(float distance) const {
    return Vec3f(point.x + direction.x * distance,
                 point.y + direction.y * distance,
                 point.z + direction.z * distance);
}

#pragma mark BBox

void BBox::repair() {
    min.x = fminf(min.x, max.x);
    min.y = fminf(min.y, max.y);
    min.z = fminf(min.z, max.z);
    max.x = fmaxf(min.x, max.x);
    max.y = fmaxf(min.y, max.y);
    max.z = fmaxf(min.z, max.z);
}

const BBox BBox::operator+ (const BBox& right) const {
    return BBox(fminf(min.x, right.min.x),
                fminf(min.y, right.min.y),
                fminf(min.z, right.min.z),
                fmaxf(max.x, right.max.x),
                fmaxf(max.y, right.max.y),
                fmaxf(max.z, right.max.z));
}

const BBox BBox::operator+ (const Vec3f& right) const {
    return BBox(fminf(min.x, right.x),
                fminf(min.y, right.y),
                fminf(min.z, right.z),
                fmaxf(max.x, right.x),
                fmaxf(max.y, right.y),
                fmaxf(max.z, right.z));
}

BBox& BBox::operator+= (const BBox& right) {
    min.x = fminf(min.x, right.min.x);
    min.y = fminf(min.y, right.min.y);
    min.z = fminf(min.z, right.min.z);
    max.x = fmaxf(max.x, right.max.x);
    max.y = fmaxf(max.y, right.max.y);
    max.z = fmaxf(max.z, right.max.z);
    return *this;
}

BBox& BBox::operator+= (const Vec3f& right) {
    min.x = fminf(min.x, right.x);
    min.y = fminf(min.y, right.y);
    min.z = fminf(min.z, right.z);
    max.x = fmaxf(max.x, right.x);
    max.y = fmaxf(max.y, right.y);
    max.z = fmaxf(max.z, right.z);
    return *this;
}

BBox::BBox() : min(Null3f), max(Null3f) {}
BBox::BBox(const Vec3f& min, const Vec3f& max) : min(min), max(max) {}
BBox::BBox(float minx, float miny, float minz, float maxx, float maxy, float maxz) {
    min.x = minx;
    min.y = miny;
    min.z = minz;
    max.x = maxx;
    max.y = maxy;
    max.z = maxz;
}

const BBox BBox::maxBounds() const {
    Vec3f c = center();
    Vec3f diff = max - c;
    diff.x = diff.y = diff.z = fmaxf(diff.x, fmaxf(diff.y, diff.z));
    return BBox(c - diff, c + diff);
}

const Vec3f BBox::center() const {
    return Vec3f((max.x + min.x) / 2,
                 (max.y + min.y) / 2,
                 (max.z + min.z) / 2);
}

const Vec3f BBox::size() const {
    return Vec3f(max.x - min.x,
                 max.y - min.y,
                 max.z - min.z);
}

bool BBox::contains(const Vec3f& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool BBox::contains(const BBox& bounds) const {
    return bounds.min.x >= min.x && bounds.max.x <= max.x &&
           bounds.min.y >= min.y && bounds.max.y <= max.y &&
           bounds.min.z >= min.z && bounds.max.z <= max.z;
}

bool BBox::intersects(const BBox& bounds) const {
    return ((bounds.min.x >= min.x && bounds.min.x <= max.x) || (bounds.max.x >= min.x && bounds.max.x <= max.x)) || (bounds.min.x <= min.x && bounds.max.x >= max.x) || 
           ((bounds.min.y >= min.y && bounds.min.y <= max.y) || (bounds.max.y >= min.y && bounds.max.y <= max.y)) || (bounds.min.y <= min.y && bounds.max.y >= max.y) || 
           ((bounds.min.z >= min.z && bounds.min.z <= max.z) || (bounds.max.z >= min.z && bounds.max.z <= max.z)) || (bounds.min.z <= min.z && bounds.max.z >= max.z);
    
}

float BBox::intersectWithRay(const Ray& ray, Vec3f* sideNormal) const {
    if (ray.direction.x < 0) {
        Plane plane(XAxisPos, max);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                if (sideNormal != NULL) *sideNormal = XAxisPos;
                return distance;
            }
        }
    } else if (ray.direction.x > 0) {
        Plane plane(XAxisNeg, min);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                if (sideNormal != NULL) *sideNormal = XAxisNeg;
                return distance;
            }
        }
    }

    if (ray.direction.y < 0) {
        Plane plane(YAxisPos, max);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                if (sideNormal != NULL) *sideNormal = YAxisPos;
                return distance;
            }
        }
    } else if (ray.direction.y > 0) {
        Plane plane(YAxisNeg, min);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                if (sideNormal != NULL) *sideNormal = YAxisNeg;
                return distance;
            }
        }
    }

    if (ray.direction.z < 0) {
        Plane plane(ZAxisPos, max);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                if (sideNormal != NULL) *sideNormal = ZAxisPos;
                return distance;
            }
        }
    } else if (ray.direction.z > 0) {
        Plane plane(ZAxisNeg, min);
        float distance = plane.intersectWithRay(ray);
        if (!std::isnan(distance)) {
            Vec3f point = ray.pointAtDistance(distance);
            if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                if (sideNormal != NULL) *sideNormal = ZAxisNeg;
                return distance;
            }
        }
    }
    
    return NAN;
}

float BBox::intersectWithRay(const Ray& ray) const {
    return intersectWithRay(ray, NULL);
}

const BBox BBox::translate(const Vec3f& delta) const {
    return BBox(min.x + delta.x,
                min.y + delta.y,
                min.z + delta.z,
                max.x + delta.x,
                max.y + delta.y,
                max.z + delta.z);
}

const BBox BBox::rotate90(EAxis axis, bool clockwise) const {
    BBox result;
    result.min = min.rotate90(axis, clockwise);
    result.max = max.rotate90(axis, clockwise);
    result.repair();
    return result;
}

const BBox BBox::rotate90(EAxis axis, const Vec3f& center, bool clockwise) const {
    BBox result;
    result.min = min.rotate90(axis, center, clockwise);
    result.max = max.rotate90(axis, center, clockwise);
    result.repair();
    return result;
}

const BBox BBox::rotate(Quat rotation) const {
    BBox result;
    result.min = rotation * min;
    result.max = rotation * max;
    result.repair();
    return result;
}

const BBox BBox::rotate(Quat rotation, const Vec3f& center) const {
    BBox result;
    result.min = rotation * (min - center) + center;
    result.max = rotation * (max - center) + center;
    result.repair();
    return result;
}

const BBox BBox::flip(EAxis axis) const {
    BBox result;
    result.min = min.flip(axis);
    result.max = max.flip(axis);
    result.repair();
    return result;
}

const BBox BBox::flip(EAxis axis, const Vec3f& center) const {
    BBox result;
    result.min = min.flip(axis, center);
    result.max = max.flip(axis, center);
    result.repair();
    return result;
}

const BBox BBox::expand(float f) {
    return BBox(min.x -= f,
                min.y -= f,
                min.z -= f,
                max.x += f,
                max.y += f,
                max.z += f);
}

#pragma mark Plane

Plane::Plane() : normal(Null3f), distance(0) {}

Plane::Plane(const Vec3f& normal, float distance) : normal(normal), distance(distance) {}

Plane::Plane(const Vec3f& normal, const Vec3f& anchor) : normal(normal), distance(anchor | normal) {}

bool Plane::setPoints(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3) {
    Vec3f v1 = point3 - point1;
    Vec3f v2 = point2 - point1;
    normal = v1 % v2;
    if (normal.equals(Null3f, AlmostZero)) return false;
    normal = normal.normalize();
    distance = point1 | normal;
    return true;
}

const Vec3f Plane::anchor() const {
    return normal * distance;
}

float Plane::intersectWithRay(const Ray& ray) const {
    float d = ray.direction | normal;
    if (fzero(d)) return NAN;
    float s = ((anchor() - ray.origin) | normal) / d;
    if (fneg(s)) return NAN;
    return s;
}

float Plane::intersectWithLine(const Line& line) const {
    float d = line.direction | normal;
    if (fzero(d)) return NAN;
    return ((anchor() - line.point) | normal) / d;
}

EPointStatus Plane::pointStatus(const Vec3f& point) const {
    float dot = normal | (point - anchor());
    if (dot >  PointStatusEpsilon) return TB_PS_ABOVE;
    if (dot < -PointStatusEpsilon) return TB_PS_BELOW;
    return TB_PS_INSIDE;
}

float Plane::x(float y, float z) const {
    float l = normal | anchor();
    return (l - normal.y * y - normal.z * z) / normal.x;
}

float Plane::y(float x, float z) const {
    float l = normal | anchor();
    return (l - normal.x * x - normal.z * z) / normal.y;
}

float Plane::z(float x, float y) const {
    float l = normal | anchor();
    return (l - normal.x * x - normal.y * y) / normal.z;
}

bool Plane::equals(const Plane& other) const {
    return equals(other, AlmostZero);
}

bool Plane::equals(const Plane& other, float epsilon) const {
    return normal.equals(other.normal, epsilon) && fabsf(distance - other.distance) <= epsilon;
}

const Plane Plane::translate(const Vec3f& delta) const {
    return Plane(normal, (anchor() + delta) | normal);
}

const Plane Plane::rotate90(EAxis axis, bool clockwise) const {
    return Plane(normal.rotate90(axis, clockwise), distance);
}

const Plane Plane::rotate90(EAxis axis, const Vec3f& center, bool clockwise) const {
    return Plane(normal.rotate90(axis, clockwise), anchor().rotate90(axis, center, clockwise));
}

const Plane Plane::rotate(Quat rotation) const {
    return Plane(rotation * normal, distance);
}

const Plane Plane::rotate(Quat rotation, const Vec3f& center) const {
    return Plane(rotation * normal, rotation * (anchor() - center) + center);
}

const Plane Plane::flip(EAxis axis) const {
    return Plane(normal.flip(axis), distance);
}

const Plane Plane::flip(EAxis axis, const Vec3f& center) const {
    return Plane(normal.flip(axis), anchor().flip(axis, center));
}

const CoordinatePlane& CoordinatePlane::plane(CPlane plane) {
    static CoordinatePlane xy(TB_CP_XY);
    static CoordinatePlane xz(TB_CP_XZ);
    static CoordinatePlane yz(TB_CP_YZ);
    switch (plane) {
        case TB_CP_XY:
            return xy;
        case TB_CP_XZ:
            return xz;
        case TB_CP_YZ:
            return yz;
    }
}

const CoordinatePlane& CoordinatePlane::plane(const Vec3f& normal) {
    switch (normal.strongestAxis()) {
        case TB_AX_X:
            return plane(TB_CP_YZ);
        case TB_AX_Y:
            return plane(TB_CP_XZ);
        case TB_AX_Z:
            return plane(TB_CP_XY);
    }
}

const Vec3f CoordinatePlane::project(const Vec3f& point) {
    switch (m_plane) {
        case TB_CP_XY:
            return Vec3f(point.x, point.y, point.z);
        case TB_CP_YZ:
            return Vec3f(point.y, point.z, point.x);
        case TB_CP_XZ:
            return Vec3f(point.x, point.z, point.y);
    }
}
