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
            
            Plane(const Vec3f& normal, const Vec3f& anchor) : normal(normal), distance(anchor | normal) {}
            
            bool setPoints(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3) {
                Vec3f v1 = point3 - point1;
                Vec3f v2 = point2 - point1;
                normal = v1 % v2;
                if (normal.equals(Vec3f::Null, Math::AlmostZero))
                    return false;
                normal.normalize();
                distance = point1 | normal;
                return true;
            }
            
            const Vec3f anchor() const {
                return normal * distance;
            }
            
            float intersectWithRay(const Ray& ray) const {
                float d = ray.direction | normal;
                if (Math::zero(d))
                    return Math::nan();

                float s = ((anchor() - ray.origin) | normal) / d;
                if (Math::neg(s))
                    return Math::nan();
                return s;
            }
            
            float intersectWithLine(const Line& line) const {
                float d = line.direction | normal;
                if (Math::zero(d))
                    return Math::nan();
                return ((anchor() - line.point) | normal) / d;
            }
            
            PointStatus::Type pointStatus(const Vec3f& point) const {
                float dot = normal | (point - anchor());
                if (dot >  Math::PointStatusEpsilon)
                    return PointStatus::Above;
                if (dot < -Math::PointStatusEpsilon)
                    return PointStatus::Below;
                return PointStatus::Inside;
            }
            
            float x(float y, float z) const {
                float l = normal | anchor();
                return (l - normal.y * y - normal.z * z) / normal.x;
            }
            
            float y(float x, float z) const {
                float l = normal | anchor();
                return (l - normal.x * x - normal.z * z) / normal.y;
            }
            
            float z(float x, float y) const {
                float l = normal | anchor();
                return (l - normal.x * x - normal.y * y) / normal.z;
            }
            
            bool equals(const Plane& other) const {
                return equals(other, Math::AlmostZero);
            }
            
            bool equals(const Plane& other, float epsilon) const {
                return normal.equals(other.normal, epsilon) && fabsf(distance - other.distance) <= epsilon;
            }
            
            void translate(const Vec3f& delta) {
                distance = (anchor() + delta) | normal;
            }
            
            const Plane translated(const Vec3f& delta) const {
                return Plane(normal, (anchor() + delta) | normal);
            }
            
            void rotate90(Axis::Type axis, bool clockwise) {
                normal.rotate90(axis, clockwise);
            }
            
            const Plane rotated90(Axis::Type axis, bool clockwise) const {
                return Plane(normal.rotated90(axis, clockwise), distance);
            }
            
            void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise) {
                normal.rotate90(axis, center, clockwise);
                distance = (anchor().rotated90(axis, center, clockwise)) | normal;
            }
            
            const Plane rotated90(Axis::Type axis, const Vec3f& center, bool clockwise) const {
                return Plane(normal.rotated90(axis, clockwise), anchor().rotated90(axis, center, clockwise));
            }
            
            void rotate(const Quat& rotation) {
                normal = rotation * normal;
            }
            
            const Plane rotated(const Quat& rotation) const {
                return Plane(rotation * normal, distance);
            }
            
            void rotate(const Quat& rotation, const Vec3f& center) {
                normal = rotation * normal;
                distance = (rotation * (anchor() - center) + center) | normal;
            }
            
            const Plane rotated(const Quat& rotation, const Vec3f& center) const {
                return Plane(rotation * normal, rotation * (anchor() - center) + center);
            }
            
            void flip(Axis::Type axis) {
                normal.flip(axis);
            }
            
            const Plane flipped(Axis::Type axis) const {
                return Plane(normal.flipped(axis), distance);
            }
            
            void flip(Axis::Type axis, const Vec3f& center) {
                normal.flip(axis);
                distance = anchor().flipped(axis, center) | normal;
            }
            
            const Plane flipped(Axis::Type axis, const Vec3f& center) const {
                return Plane(normal.flipped(axis), anchor().flipped(axis, center));
            }
        };
    }
}

#endif
