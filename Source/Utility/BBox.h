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

#include "Utility/Mat4f.h"
#include "Utility/Plane.h"
#include "Utility/Quat.h"
#include "Utility/Ray.h"
#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class BBox {
        public:
            struct PointPosition {
                typedef enum {
                    Less,
                    Within,
                    Greater
                } Position;
                
                const Position x;
                const Position y;
                const Position z;
                
                PointPosition(Position i_x, Position i_y, Position i_z):
                x(i_x),
                y(i_y),
                z(i_z) {}

                inline const Position& operator[] (const size_t index) const {
                    assert(index >= 0 && index < 3);
                    if (index == 0) return x;
                    if (index == 1) return y;
                    return z;
                }
            };
            
            Vec3f min;
            Vec3f max;

            BBox() : min(Vec3f::Null), max(Vec3f::Null) {}

            BBox(const Vec3f& i_min, const Vec3f& i_max) : min(i_min), max(i_max) {}
            
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
            
            inline bool operator== (const BBox& right) const {
                return min == right.min && max == right.max;
            }
            
            inline const BBox mergedWith(const BBox& right) const {
                return BBox(std::min(min.x, right.min.x),
                            std::min(min.y, right.min.y),
                            std::min(min.z, right.min.z),
                            std::max(max.x, right.max.x),
                            std::max(max.y, right.max.y),
                            std::max(max.z, right.max.z));
            }

            
            inline BBox& mergeWith(const BBox& right) {
                min.x = std::min(min.x, right.min.x);
                min.y = std::min(min.y, right.min.y);
                min.z = std::min(min.z, right.min.z);
                max.x = std::max(max.x, right.max.x);
                max.y = std::max(max.y, right.max.y);
                max.z = std::max(max.z, right.max.z);
                return *this;
            }

            inline const BBox mergedWith(const Vec3f& right) const {
                return BBox(std::min(min.x, right.x),
                            std::min(min.y, right.y),
                            std::min(min.z, right.z),
                            std::max(max.x, right.x),
                            std::max(max.y, right.y),
                            std::max(max.z, right.z));
            }
            
            inline BBox& mergeWith(const Vec3f& right) {
                min.x = std::min(min.x, right.x);
                min.y = std::min(min.y, right.y);
                min.z = std::min(min.z, right.z);
                max.x = std::max(max.x, right.x);
                max.y = std::max(max.y, right.y);
                max.z = std::max(max.z, right.z);
                return *this;
            }
            
            inline const BBox maxBounds() const {
                Vec3f c = center();
                Vec3f diff = max - c;
                diff.x = diff.y = diff.z = std::max(diff.x, std::max(diff.y, diff.z));
                return BBox(c - diff, c + diff);
            }
            
            inline  const Vec3f center() const {
                return Vec3f((max.x + min.x) / 2.0f,
                             (max.y + min.y) / 2.0f,
                             (max.z + min.z) / 2.0f);
            }
            
            inline const Vec3f size() const {
                return Vec3f(max.x - min.x,
                             max.y - min.y,
                             max.z - min.z);
            }
            
            inline BBox& translateToOrigin() {
                Vec3f c = center();
                min - c;
                max - c;
                return *this;
            }
            
            inline const BBox translatedToOrigin() const {
                Vec3f c = center();
                return BBox(min - c, max - c);
            }
            
            inline BBox& repair() {
                for (unsigned int i = 0; i < 3; i++)
                    if (min[i] > max[i])
                        std::swap(min[i], max[i]);
                return *this;
            }
            
            inline const BBox repaired() const {
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

            inline const Vec3f vertex(unsigned int i) const {
                switch (i) {
                    case 0:
                        return vertex(false, false, false);
                    case 1:
                        return vertex(false, false, true);
                    case 2:
                        return vertex(false, true, false);
                    case 3:
                        return vertex(false, true, true);
                    case 4:
                        return vertex(true, false, false);
                    case 5:
                        return vertex(true, false, true);
                    case 6:
                        return vertex(true, true, false);
                    case 7:
                        return vertex(true, true, true);
                }
                return Vec3f::NaN;
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
            
            inline bool contains(const Vec3f& point) const {
                return point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z;
            }
            
            inline bool contains(const BBox& bounds) const {
                return bounds.min.x >= min.x && bounds.max.x <= max.x &&
                bounds.min.y >= min.y && bounds.max.y <= max.y &&
                bounds.min.z >= min.z && bounds.max.z <= max.z;
            }
            
            inline bool intersects(const BBox& bounds) const {
                return (((bounds.min.x >= min.x && bounds.min.x <= max.x) || (bounds.max.x >= min.x && bounds.max.x <= max.x)) || (bounds.min.x <= min.x && bounds.max.x >= max.x)) &&
                       (((bounds.min.y >= min.y && bounds.min.y <= max.y) || (bounds.max.y >= min.y && bounds.max.y <= max.y)) || (bounds.min.y <= min.y && bounds.max.y >= max.y)) &&
                       (((bounds.min.z >= min.z && bounds.min.z <= max.z) || (bounds.max.z >= min.z && bounds.max.z <= max.z)) || (bounds.min.z <= min.z && bounds.max.z >= max.z));
                
            }
            
            float intersectWithRay(const Ray& ray, Vec3f* sideNormal) const {
                bool inside = contains(ray.origin);
                
                if (ray.direction.x < 0) {
                    Plane plane(Vec3f::PosX, inside ? min : max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::NegX : Vec3f::PosX;
                            return distance;
                        }
                    }
                } else if (ray.direction.x > 0) {
                    Plane plane(Vec3f::NegX, inside ? max : min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::PosX : Vec3f::NegX;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.y < 0) {
                    Plane plane(Vec3f::PosY, inside ? min : max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::NegY : Vec3f::PosY;
                            return distance;
                        }
                    }
                } else if (ray.direction.y > 0) {
                    Plane plane(Vec3f::NegY, inside ? max : min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.z >= min.z && point.z <= max.z) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::PosY : Vec3f::NegY;
                            return distance;
                        }
                    }
                }
                
                if (ray.direction.z < 0) {
                    Plane plane(Vec3f::PosZ, inside ? min : max);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::NegZ : Vec3f::PosZ;
                            return distance;
                        }
                    }
                } else if (ray.direction.z > 0) {
                    Plane plane(Vec3f::NegZ, inside ? max : min);
                    float distance = plane.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        Vec3f point = ray.pointAtDistance(distance);
                        if (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y) {
                            if (sideNormal != NULL) *sideNormal = inside ? Vec3f::PosZ : Vec3f::NegZ;
                            return distance;
                        }
                    }
                }
                
                return std::numeric_limits<float>::quiet_NaN();
            }
            
            inline float intersectWithRay(const Ray& ray) const {
                return intersectWithRay(ray, NULL);
            }
            
            inline BBox& translate(const Vec3f& delta) {
                min += delta;
                max += delta;
                return *this;
            }
            
            inline const BBox translated(const Vec3f& delta) const {
                return BBox(min.x + delta.x,
                            min.y + delta.y,
                            min.z + delta.z,
                            max.x + delta.x,
                            max.y + delta.y,
                            max.z + delta.z);
            }

            inline BBox& rotate90(Axis::Type axis, bool clockwise) {
                min.rotate90(axis, clockwise);
                max.rotate90(axis, clockwise);
                repair();
                return *this;
            }
            
            inline const BBox rotated90(Axis::Type axis, bool clockwise) const {
                BBox result = *this;
                result.rotate90(axis, clockwise);
                return result;
            }
            
            inline BBox& rotate90(Axis::Type axis, const Vec3f& center, bool clockwise) {
                min.rotate90(axis, center, clockwise);
                max.rotate90(axis, center, clockwise);
                repair();
                return *this;
            }
            
           inline  const BBox rotated90(Axis::Type axis, const Vec3f& center, bool clockwise) const {
                BBox result = *this;
                result.rotate90(axis, center, clockwise);
                return result;
            }
            
            inline BBox& rotate(const Quat& rotation) {
                *this = rotated(rotation);
                return *this;
            }
            
            inline const BBox rotated(const Quat& rotation) const {
                BBox result;
                result.min = result.max = rotation * vertex(false, false, false);
                result.mergeWith(rotation * vertex(false, false, true ));
                result.mergeWith(rotation * vertex(false, true , false));
                result.mergeWith(rotation * vertex(false, true , true ));
                result.mergeWith(rotation * vertex(true , false, false));
                result.mergeWith(rotation * vertex(true , false, true ));
                result.mergeWith(rotation * vertex(true , true , false));
                result.mergeWith(rotation * vertex(true , true , true ));
                return result;
            }
            
            inline BBox& rotate(const Quat& rotation, const Vec3f& center) {
                *this = rotated(rotation, center);
                return *this;
            }
            
            inline const BBox rotated(const Quat& rotation, const Vec3f& center) const {
                BBox result;
                result.min = result.max = rotation * (vertex(false, false, false) - center) + center;
                result.mergeWith(rotation * (vertex(false, false, true ) - center) + center);
                result.mergeWith(rotation * (vertex(false, true , false) - center) + center);
                result.mergeWith(rotation * (vertex(false, true , true ) - center) + center);
                result.mergeWith(rotation * (vertex(true , false, false) - center) + center);
                result.mergeWith(rotation * (vertex(true , false, true ) - center) + center);
                result.mergeWith(rotation * (vertex(true , true , false) - center) + center);
                result.mergeWith(rotation * (vertex(true , true , true ) - center) + center);
                return result;
            }
            
            BBox& transform(const Mat4f& transformation) {
                min = max = transformation * vertex(false, false, false);
                mergeWith(transformation * vertex(false, false, true ));
                mergeWith(transformation * vertex(false, true , false));
                mergeWith(transformation * vertex(false, true , true ));
                mergeWith(transformation * vertex(true , false, false));
                mergeWith(transformation * vertex(true , false, true ));
                mergeWith(transformation * vertex(true , true , false));
                mergeWith(transformation * vertex(true , true , true ));
                return *this;
            }
            
            const BBox transformed(const Mat4f& transformation) const {
                BBox result = *this;
                return result.transform(transformation);
            }
            
            inline BBox& flip(Axis::Type axis) {
                min.flip(axis);
                max.flip(axis);
                repair();
                return *this;
            }
            
            inline const BBox flipped(Axis::Type axis) const {
                BBox result = *this;
                result.flip(axis);
                return result;
            }
            
            inline BBox& flip(Axis::Type axis, const Vec3f& center) {
                min.flip(axis, center);
                max.flip(axis, center);
                repair();
                return *this;
            }
            
            inline const BBox flipped(Axis::Type axis, const Vec3f& center) const {
                BBox result = *this;
                result.flip(axis, center);
                return result;
            }
            
            inline BBox& expand(float f) {
                for (unsigned int i = 0; i < 3; i++) {
                    min[i] -= f;
                    max[i] += f;
                }
                return *this;
            }
            
            inline const BBox expanded(float f) const {
                return BBox(min.x - f,
                            min.y - f,
                            min.z - f,
                            max.x + f,
                            max.y + f,
                            max.z + f);
            }
            
            inline const PointPosition pointPosition(const Vec3f& point) const {
                PointPosition::Position p[3];
                for (unsigned int i = 0; i < 3; i++) {
                    if (point[i] < min[i])
                        p[i] = PointPosition::Less;
                    else if (point[i] > max[i])
                        p[i] = PointPosition::Greater;
                    else
                        p[i] = PointPosition::Within;
                }

                return PointPosition(p[0], p[1], p[2]);
            }
        };
    }
}

#endif
