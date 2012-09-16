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

#ifndef TrenchBroom_Plane_h
#define TrenchBroom_Plane_h

#include "Utility/Line.h"
#include "Utility/Quat.h"
#include "Utility/Ray.h"
#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class Plane {
        public:
            Vec3f normal;
            float distance;
            
            Plane() : normal(Vec3f::Null), distance(0) {}
            
            Plane(const Vec3f& normal, float distance) : normal(normal), distance(distance) {}
            
            Plane(const Vec3f& normal, const Vec3f& anchor) : normal(normal), distance(anchor.dot(normal)) {}
            
            inline bool setPoints(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3) {
                Vec3f v1 = point3 - point1;
                Vec3f v2 = point2 - point1;
                normal = v1.crossed(v2);
                if (normal.equals(Vec3f::Null, Math::AlmostZero))
                    return false;
                normal.normalize();
                distance = point1.dot(normal);
                return true;
            }
            
            inline const Vec3f anchor() const {
                return normal * distance;
            }
            
            inline float intersectWithRay(const Ray& ray) const {
                float d = ray.direction.dot(normal);
                if (Math::zero(d))
                    return Math::nan();

                float s = ((anchor() - ray.origin).dot(normal)) / d;
                if (Math::neg(s))
                    return Math::nan();
                return s;
            }
            
            inline float intersectWithLine(const Line& line) const {
                float d = line.direction.dot(normal);
                if (Math::zero(d))
                    return Math::nan();
                return ((anchor() - line.point).dot(normal)) / d;
            }
            
            inline PointStatus::Type pointStatus(const Vec3f& point) const {
                float dot = normal.dot(point - anchor());
                if (dot >  Math::PointStatusEpsilon)
                    return PointStatus::PSAbove;
                if (dot < -Math::PointStatusEpsilon)
                    return PointStatus::PSBelow;
                return PointStatus::PSInside;
            }
            
            inline float x(float y, float z) const {
                float l = normal.dot(anchor());
                return (l - normal.y * y - normal.z * z) / normal.x;
            }
            
            inline float y(float x, float z) const {
                float l = normal.dot(anchor());
                return (l - normal.x * x - normal.z * z) / normal.y;
            }
            
            inline float z(float x, float y) const {
                float l = normal.dot(anchor());
                return (l - normal.x * x - normal.y * y) / normal.z;
            }
            
            inline bool equals(const Plane& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            inline bool equals(const Plane& other, float epsilon) const {
                return normal.equals(other.normal, epsilon) && fabsf(distance - other.distance) <= epsilon;
            }
            
            inline void translate(const Vec3f& delta) {
                distance = (anchor() + delta).dot(normal);
            }
            
            inline const Plane translated(const Vec3f& delta) const {
                return Plane(normal, (anchor() + delta).dot(normal));
            }
            
            inline void rotate90(Axis::Type axis, bool clockwise) {
                normal.rotate90(axis, clockwise);
            }
            
            inline const Plane rotated90(Axis::Type axis, bool clockwise) const {
                return Plane(normal.rotated90(axis, clockwise), distance);
            }
            
            inline void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise) {
                normal.rotate90(axis, center, clockwise);
                distance = (anchor().rotated90(axis, center, clockwise)).dot(normal);
            }
            
            inline const Plane rotated90(Axis::Type axis, const Vec3f& center, bool clockwise) const {
                return Plane(normal.rotated90(axis, clockwise), anchor().rotated90(axis, center, clockwise));
            }
            
            inline void rotate(const Quat& rotation) {
                normal = rotation * normal;
            }
            
            inline const Plane rotated(const Quat& rotation) const {
                return Plane(rotation * normal, distance);
            }
            
            inline void rotate(const Quat& rotation, const Vec3f& center) {
                normal = rotation * normal;
                distance = (rotation * (anchor() - center) + center).dot(normal);
            }
            
            inline const Plane rotated(const Quat& rotation, const Vec3f& center) const {
                return Plane(rotation * normal, rotation * (anchor() - center) + center);
            }
            
            inline void flip(Axis::Type axis) {
                normal.flip(axis);
            }
            
            inline const Plane flipped(Axis::Type axis) const {
                return Plane(normal.flipped(axis), distance);
            }
            
            inline void flip(Axis::Type axis, const Vec3f& center) {
                normal.flip(axis);
                distance = anchor().flipped(axis, center).dot(normal);
            }
            
            inline const Plane flipped(Axis::Type axis, const Vec3f& center) const {
                return Plane(normal.flipped(axis), anchor().flipped(axis, center));
            }
        };
    }
}

#endif
