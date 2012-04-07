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
#include <cmath>

#pragma mark Vec2f
Vec2f& Vec2f::operator= (const Vec2f& right) {
    if (this != &right) {
        x = right.x;
        y = right.y;
    }
    return *this;
}

const Vec2f Vec2f::operator+ (const Vec2f& right) const {
    Vec2f result = *this;
    return result += right;
}

const Vec2f Vec2f::operator- (const Vec2f& right) const {
    Vec2f result = *this;
    return result -= right;
}

const Vec2f Vec2f::operator* (const float right) const {
    Vec2f result = *this;
    return result *= right;
}

const Vec2f Vec2f::operator/ (const float right) const {
    return *this * (1 / right);
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
    return *this *= (1 / right);
}

const float& Vec2f::operator[] (const int index) const {
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
    Vec2f result = *this;
    result /= result.length();
    return result;
}

bool Vec2f::equals(Vec2f other) const {
    return equals(other, AlmostZero);
}

bool Vec2f::equals(Vec2f other, float delta) const {
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

const Vec3f Vec3f::operator/ (const float right) const {
    return *this * (1 / right);
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

Vec3f& Vec3f::operator/= (const float right) {
    return *this *= (1 / right);
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

float Vec3f::length() const {
    return sqrt(lengthSquared());
}

float Vec3f::lengthSquared() const {
    return *this | *this;
}

const Vec3f Vec3f::normalize() const {
    Vec3f result = *this;
    result /= result.length();
    return result;
}

bool Vec3f::equals(Vec3f other) const {
    return equals(other, AlmostZero);
}

bool Vec3f::equals(Vec3f other, float delta) const {
    Vec3f diff = other - *this;
    return diff.lengthSquared() <= delta * delta;
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

const Vec4f Vec4f::operator/ (const float right) const {
    return *this * (1 / right);
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
    return *this *= (1 / right);
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

float Vec4f::length() const {
    return sqrt(lengthSquared());
}

float Vec4f::lengthSquared() const {
    return *this | *this;
}

const Vec4f Vec4f::normalize() const {
    Vec4f result = *this;
    result /= result.length();
    return result;
}

bool Vec4f::equals(Vec4f other) const {
    return equals(other, AlmostZero);
}

bool Vec4f::equals(Vec4f other, float delta) const {
    Vec4f diff = other - *this;
    return diff.lengthSquared() <= delta * delta;
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

Quat::Quat(float angle, Vec3f axis) {
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

const BBox BBox::operator+ (const BBox& right) const {
    BBox result = *this;
    return result += right;
}

const BBox BBox::operator+ (const Vec3f& right) const {
    BBox result = *this;
    return result += right;
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
BBox::BBox(Vec3f min, Vec3f max) : min(min), max(max) {}
