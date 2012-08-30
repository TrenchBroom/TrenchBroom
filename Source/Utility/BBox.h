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

#ifndef TrenchBroom_BBox_h
#define TrenchBroom_BBox_h

#include "Utility/Plane.h"
#include "Utility/Quat.h"
#include "Utility/Ray.h"
#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class BBox {
        public:
            Vec3f min;
            Vec3f max;

            BBox() : min(Vec3f::Null), max(Vec3f::Null) {}

            BBox(const Vec3f& min, const Vec3f& max) : min(min), max(max) {}
            
            BBox(float minx, float miny, float minz, float maxx, float maxy, float maxz) {
                min.x = minx;
                min.y = miny;
                min.z = minz;
                max.x = maxx;
                max.y = maxy;
                max.z = maxz;
            }
            
            BBox(const Vec3f& center, float size) {
                min.x = center.x - size;
                min.y = center.y - size;
                min.z = center.z - size;
                max.x = center.x + size;
                max.y = center.y + size;
                max.z = center.z + size;
            }
            
            bool operator== (const BBox& right) const {
                return min == right.min && max == right.max;
            }
            
            const BBox operator+ (const BBox& right) const {
                return BBox(std::min(min.x, right.min.x),
                            std::min(min.y, right.min.y),
                            std::min(min.z, right.min.z),
                            std::max(max.x, right.max.x),
                            std::max(max.y, right.max.y),
                            std::max(max.z, right.max.z));
            }
            
            const BBox operator+ (const Vec3f& right) const {
                return BBox(std::min(min.x, right.x),
                            std::min(min.y, right.y),
                            std::min(min.z, right.z),
                            std::max(max.x, right.x),
                            std::max(max.y, right.y),
                            std::max(max.z, right.z));
            }
            
            BBox& operator+= (const BBox& right) {
                min.x = std::min(min.x, right.min.x);
                min.y = std::min(min.y, right.min.y);
                min.z = std::min(min.z, right.min.z);
                max.x = std::max(max.x, right.max.x);
                max.y = std::max(max.y, right.max.y);
                max.z = std::max(max.z, right.max.z);
                return *this;
            }
            
            BBox& operator+= (const Vec3f& right) {
                min.x = std::min(min.x, right.x);
                min.y = std::min(min.y, right.y);
                min.z = std::min(min.z, right.z);
                max.x = std::max(max.x, right.x);
                max.y = std::max(max.y, right.y);
                max.z = std::max(max.z, right.z);
                return *this;
            }
            
            const BBox maxBounds() const {
                Vec3f c = center();
                Vec3f diff = max - c;
                diff.x = diff.y = diff.z = std::max(diff.x, std::max(diff.y, diff.z));
                return BBox(c - diff, c + diff);
            }
            
            const Vec3f center() const {
                return Vec3f((max.x + min.x) / 2.0f,
                             (max.y + min.y) / 2.0f,
                             (max.z + min.z) / 2.0f);
            }
            
            const Vec3f size() const {
                return Vec3f(max.x - min.x,
                             max.y - min.y,
                             max.z - min.z);
            }
            
            void translateToOrigin() {
                Vec3f c = center();
                min - c;
                max - c;
            }
            
            const BBox translatedToOrigin() const {
                Vec3f c = center();
                return BBox(min - c, max - c);
            }
            
            void repair() {
                for (int i = 0; i < 3; i++)
                    if (min[i] > max[i])
                        std::swap(min[i], max[i]);
            }
            
            const BBox repaired() const {
                BBox result(min, max);
                result.repair();
                return result;
            }
            
            inline const Vec3f vertex(bool x, bool y, bool z) const {
                Vec3f vertex;
                vertex.x = x ? min.x : max.x;
                vertex.y = y ? min.y : max.y;
                vertex.z = z ? min.z : max.z;
                return vertex;
            }

            inline void vertices(Vec3f::List& result) const {
                result.resize(24);
                result[ 0] = result[ 7] = result[16] = vertex(false, false, false);
                result[ 1] = result[ 2] = result[20] = vertex(true , false, false);
                result[ 3] = result[ 4] = result[22] = vertex(true , true , false);
                result[ 5] = result[ 6] = result[18] = vertex(false, true , false);
                result[ 8] = result[15] = result[17] = vertex(false, false, true );
                result[ 9] = result[10] = result[21] = vertex(true , false, true );
                result[11] = result[12] = result[23] = vertex(true , true , true );
                result[13] = result[14] = result[19] = vertex(false, true , true );
            }
            
            bool contains(const Vec3f& point) const {
                return point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z;
            }
            
            bool contains(const BBox& bounds) const {
                return bounds.min.x >= min.x && bounds.max.x <= max.x &&
                bounds.min.y >= min.y && bounds.max.y <= max.y &&
                bounds.min.z >= min.z && bounds.max.z <= max.z;
            }
            
            bool intersects(const BBox& bounds) const {
                return ((bounds.min.x >= min.x && bounds.min.x <= max.x) || (bounds.max.x >= min.x && bounds.max.x <= max.x)) || (bounds.min.x <= min.x && bounds.max.x >= max.x) ||
                ((bounds.min.y >= min.y && bounds.min.y <= max.y) || (bounds.max.y >= min.y && bounds.max.y <= max.y)) || (bounds.min.y <= min.y && bounds.max.y >= max.y) ||
                ((bounds.min.z >= min.z && bounds.min.z <= max.z) || (bounds.max.z >= min.z && bounds.max.z <= max.z)) || (bounds.min.z <= min.z && bounds.max.z >= max.z);
                
            }
            
            float intersectWithRay(const Ray& ray, Vec3f* sideNormal) const {
                if (ray.direction.x < 0) {
                    Plane plane(Vec3f::PosX, max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosX;
                            return distance;
                        }
                    }
                } else if (ray.direction.x > 0) {
                    Plane plane(Vec3f::NegX, min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegX;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.y < 0) {
                    Plane plane(Vec3f::PosY, max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosY;
                            return distance;
                        }
                    }
                } else if (ray.direction.y > 0) {
                    Plane plane(Vec3f::NegY, min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegY;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.z < 0) {
                    Plane plane(Vec3f::PosZ, max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosZ;
                            return distance;
                        }
                    }
                } else if (ray.direction.z > 0) {
                    Plane plane(Vec3f::NegZ, min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegZ;
                            return distance;
                        }
                    }
                }
                
                return std::numeric_limits<float>::quiet_NaN();
            }
            
            float intersectWithRay(const Ray& ray) const {
                return intersectWithRay(ray, NULL);
            }
            
            void translate(const Vec3f& delta) {
                min += delta;
                max += delta;
            }
            
            const BBox translated(const Vec3f& delta) const {
                return BBox(min.x + delta.x,
                            min.y + delta.y,
                            min.z + delta.z,
                            max.x + delta.x,
                            max.y + delta.y,
                            max.z + delta.z);
            }

            void rotate90(Axis::Type axis, bool clockwise) {
                min.rotate90(axis, clockwise);
                max.rotate90(axis, clockwise);
                repair();
            }
            
            const BBox rotated90(Axis::Type axis, bool clockwise) const {
                BBox result = *this;
                result.rotate90(axis, clockwise);
                return result;
            }
            
            void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise) {
                min.rotate90(axis, center, clockwise);
                max.rotate90(axis, center, clockwise);
                repair();
            }
            
            const BBox rotated90(Axis::Type axis, const Vec3f& center, bool clockwise) const {
                BBox result = *this;
                result.rotate90(axis, center, clockwise);
                return result;
            }
            
            void rotate(const Quat& rotation) {
                min = rotation * min;
                max = rotation * max;
                repair();
            }
            
            const BBox rotated(const Quat& rotation) const {
                BBox result = *this;
                result.rotate(rotation);
                return result;
            }
            
            void rotate(const Quat& rotation, const Vec3f& center) {
                min = rotation * (min - center) + center;
                max = rotation * (max - center) + center;
                repair();
            }
            
            const BBox rotated(const Quat& rotation, const Vec3f& center) const {
                BBox result = *this;
                result.rotate(rotation, center);
                return result;
            }
            
            const BBox boundsAfterRotation(const Quat& rotation) const {
                BBox result;
                Vec3f c = center();
                result += (rotation * (vertex(false, false, false) - c) + c);
                result += (rotation * (vertex(false, false, true ) - c) + c);
                result += (rotation * (vertex(false, true , false) - c) + c);
                result += (rotation * (vertex(false, true , true ) - c) + c);
                result += (rotation * (vertex(true , false, false) - c) + c);
                result += (rotation * (vertex(true , false, true ) - c) + c);
                result += (rotation * (vertex(true , true , false) - c) + c);
                result += (rotation * (vertex(true , true , true ) - c) + c);
                return result;
            }
            
            void flip(Axis::Type axis) {
                min.flip(axis);
                max.flip(axis);
                repair();
            }
            
            const BBox flipped(Axis::Type axis) const {
                BBox result = *this;
                result.flip(axis);
                return result;
            }
            
            void flip(Axis::Type axis, const Vec3f& center) {
                min.flip(axis, center);
                max.flip(axis, center);
                repair();
            }
            
            const BBox flipped(Axis::Type axis, const Vec3f& center) const {
                BBox result = *this;
                result.flip(axis, center);
                return result;
            }
            
            void expand(float f) {
                for (unsigned int i = 0; i < 3; i++) {
                    min[i] -= f;
                    max[i] += f;
                }
            }
            
            const BBox expanded(float f) {
                return BBox(min.x -= f,
                            min.y -= f,
                            min.z -= f,
                            max.x += f,
                            max.y += f,
                            max.z += f);
            }
        };
    }
}

#endif
