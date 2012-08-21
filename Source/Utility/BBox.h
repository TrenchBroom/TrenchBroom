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
            
            const BBox MaxBounds() const {
                Vec3f c = Center();
                Vec3f diff = max - c;
                diff.x = diff.y = diff.z = std::max(diff.x, std::max(diff.y, diff.z));
                return BBox(c - diff, c + diff);
            }
            
            const Vec3f Center() const {
                return Vec3f((max.x + min.x) / 2.0f,
                             (max.y + min.y) / 2.0f,
                             (max.z + min.z) / 2.0f);
            }
            
            const Vec3f Size() const {
                return Vec3f(max.x - min.x,
                             max.y - min.y,
                             max.z - min.z);
            }
            
            void TranslateToOrigin() {
                Vec3f c = Center();
                min - c;
                max - c;
            }
            
            const BBox TranslatedToOrigin() const {
                Vec3f c = Center();
                return BBox(min - c, max - c);
            }
            
            void Repair() {
                for (int i = 0; i < 3; i++)
                    if (min[i] > max[i])
                        std::swap(min[i], max[i]);
            }
            
            const BBox Repaired() const {
                BBox result(min, max);
                result.Repair();
                return result;
            }
            
            const Vec3f Vertex(bool x, bool y, bool z) const {
                Vec3f vertex;
                vertex.x = x ? min.x : max.x;
                vertex.y = y ? min.y : max.y;
                vertex.z = z ? min.z : max.z;
                return vertex;
            }
            
            bool Contains(const Vec3f& point) const {
                return point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z;
            }
            
            bool Contains(const BBox& bounds) const {
                return bounds.min.x >= min.x && bounds.max.x <= max.x &&
                bounds.min.y >= min.y && bounds.max.y <= max.y &&
                bounds.min.z >= min.z && bounds.max.z <= max.z;
            }
            
            bool Intersects(const BBox& bounds) const {
                return ((bounds.min.x >= min.x && bounds.min.x <= max.x) || (bounds.max.x >= min.x && bounds.max.x <= max.x)) || (bounds.min.x <= min.x && bounds.max.x >= max.x) ||
                ((bounds.min.y >= min.y && bounds.min.y <= max.y) || (bounds.max.y >= min.y && bounds.max.y <= max.y)) || (bounds.min.y <= min.y && bounds.max.y >= max.y) ||
                ((bounds.min.z >= min.z && bounds.min.z <= max.z) || (bounds.max.z >= min.z && bounds.max.z <= max.z)) || (bounds.min.z <= min.z && bounds.max.z >= max.z);
                
            }
            
            float IntersectWithRay(const Ray& ray, Vec3f* sideNormal) const {
                if (ray.direction.x < 0) {
                    Plane plane(Vec3f::PosX, max);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosX;
                            return distance;
                        }
                    }
                } else if (ray.direction.x > 0) {
                    Plane plane(Vec3f::NegX, min);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegX;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.y < 0) {
                    Plane plane(Vec3f::PosY, max);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosY;
                            return distance;
                        }
                    }
                } else if (ray.direction.y > 0) {
                    Plane plane(Vec3f::NegY, min);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegY;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.z < 0) {
                    Plane plane(Vec3f::PosZ, max);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::PosZ;
                            return distance;
                        }
                    }
                } else if (ray.direction.z > 0) {
                    Plane plane(Vec3f::NegZ, min);
                    float distance = plane.IntersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.PointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = Vec3f::NegZ;
                            return distance;
                        }
                    }
                }
                
                return std::numeric_limits<float>::quiet_NaN();
            }
            
            float IntersectWithRay(const Ray& ray) const {
                return IntersectWithRay(ray, NULL);
            }
            
            const BBox Translate(const Vec3f& delta) const {
                return BBox(min.x + delta.x,
                            min.y + delta.y,
                            min.z + delta.z,
                            max.x + delta.x,
                            max.y + delta.y,
                            max.z + delta.z);
            }

            void Rotate90(Axis axis, bool clockwise) {
                min.Rotate90(axis, clockwise);
                max.Rotate90(axis, clockwise);
                Repair();
            }
            
            const BBox Rotated90(Axis axis, bool clockwise) const {
                BBox result = *this;
                result.Rotate90(axis, clockwise);
                return result;
            }
            
            void Rotate90(Axis axis, const Vec3f& center, bool clockwise) {
                min.Rotate90(axis, center, clockwise);
                max.Rotate90(axis, center, clockwise);
                Repair();
            }
            
            const BBox Rotated90(Axis axis, const Vec3f& center, bool clockwise) const {
                BBox result = *this;
                result.Rotate90(axis, center, clockwise);
                return result;
            }
            
            void Rotate(const Quat& rotation) {
                min = rotation * min;
                max = rotation * max;
                Repair();
            }
            
            const BBox Rotated(const Quat& rotation) const {
                BBox result = *this;
                result.Rotate(rotation);
                return result;
            }
            
            void Rotate(const Quat& rotation, const Vec3f& center) {
                min = rotation * (min - center) + center;
                max = rotation * (max - center) + center;
                Repair();
            }
            
            const BBox Rotated(const Quat& rotation, const Vec3f& center) const {
                BBox result = *this;
                result.Rotate(rotation, center);
                return result;
            }
            
            const BBox BoundsAfterRotation(const Quat& rotation) const {
                BBox result;
                Vec3f c = Center();
                result += (rotation * (Vertex(false, false, false) - c) + c);
                result += (rotation * (Vertex(false, false, true ) - c) + c);
                result += (rotation * (Vertex(false, true , false) - c) + c);
                result += (rotation * (Vertex(false, true , true ) - c) + c);
                result += (rotation * (Vertex(true , false, false) - c) + c);
                result += (rotation * (Vertex(true , false, true ) - c) + c);
                result += (rotation * (Vertex(true , true , false) - c) + c);
                result += (rotation * (Vertex(true , true , true ) - c) + c);
                return result;
            }
            
            void Flip(Axis axis) {
                min.Flip(axis);
                max.Flip(axis);
                Repair();
            }
            
            const BBox Flipped(Axis axis) const {
                BBox result = *this;
                result.Flip(axis);
                return result;
            }
            
            void Flip(Axis axis, const Vec3f& center) {
                min.Flip(axis, center);
                max.Flip(axis, center);
                Repair();
            }
            
            const BBox Flipped(Axis axis, const Vec3f& center) const {
                BBox result = *this;
                result.Flip(axis, center);
                return result;
            }
            
            void Expand(float f) {
                for (unsigned int i = 0; i < 3; i++) {
                    min[i] -= f;
                    max[i] += f;
                }
            }
            
            const BBox Expanded(float f) {
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
