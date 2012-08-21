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
            
            bool SetPoints(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3) {
                Vec3f v1 = point3 - point1;
                Vec3f v2 = point2 - point1;
                normal = v1 % v2;
                if (normal.Equals(Vec3f::Null, Math::AlmostZero))
                    return false;
                normal.Normalize();
                distance = point1 | normal;
                return true;
            }
            
            const Vec3f Anchor() const {
                return normal * distance;
            }
            
            float IntersectWithRay(const Ray& ray) const {
                float d = ray.direction | normal;
                if (Math::zero(d))
                    return Math::nan();

                float s = ((Anchor() - ray.origin) | normal) / d;
                if (Math::neg(s))
                    return Math::nan();
                return s;
            }
            
            float IntersectWithLine(const Line& line) const {
                float d = line.direction | normal;
                if (Math::zero(d))
                    return Math::nan();
                return ((Anchor() - line.point) | normal) / d;
            }
            
            PointStatus pointStatus(const Vec3f& point) const {
                float dot = normal | (point - Anchor());
                if (dot >  Math::PointStatusEpsilon)
                    return PointStatus::Above;
                if (dot < -Math::PointStatusEpsilon)
                    return PointStatus::Below;
                return PointStatus::Inside;
            }
            
            float X(float y, float z) const {
                float l = normal | Anchor();
                return (l - normal.y * y - normal.z * z) / normal.x;
            }
            
            float Y(float x, float z) const {
                float l = normal | Anchor();
                return (l - normal.x * x - normal.z * z) / normal.y;
            }
            
            float Z(float x, float y) const {
                float l = normal | Anchor();
                return (l - normal.x * x - normal.y * y) / normal.z;
            }
            
            bool Equals(const Plane& other) const {
                return Equals(other, Math::AlmostZero);
            }
            
            bool Equals(const Plane& other, float epsilon) const {
                return normal.Equals(other.normal, epsilon) && fabsf(distance - other.distance) <= epsilon;
            }
            
            void Translate(const Vec3f& delta) {
                distance = (Anchor() + delta) | normal;
            }
            
            const Plane Translated(const Vec3f& delta) const {
                return Plane(normal, (Anchor() + delta) | normal);
            }
            
            void Rotate90(Axis axis, bool clockwise) {
                normal.Rotate90(axis, clockwise);
            }
            
            const Plane Rotated90(Axis axis, bool clockwise) const {
                return Plane(normal.Rotated90(axis, clockwise), distance);
            }
            
            void Rotate90(Axis axis, const Vec3f& center, bool clockwise) {
                normal.Rotate90(axis, center, clockwise);
                distance = (Anchor().Rotated90(axis, center, clockwise)) | normal;
            }
            
            const Plane Rotated90(Axis axis, const Vec3f& center, bool clockwise) const {
                return Plane(normal.Rotated90(axis, clockwise), Anchor().Rotated90(axis, center, clockwise));
            }
            
            void Rotate(const Quat& rotation) {
                normal = rotation * normal;
            }
            
            const Plane Rotated(const Quat& rotation) const {
                return Plane(rotation * normal, distance);
            }
            
            void Rotate(const Quat& rotation, const Vec3f& center) {
                normal = rotation * normal;
                distance = (rotation * (Anchor() - center) + center) | normal;
            }
            
            const Plane Rotated(const Quat& rotation, const Vec3f& center) const {
                return Plane(rotation * normal, rotation * (Anchor() - center) + center);
            }
            
            void Flip(Axis axis) {
                normal.Flip(axis);
            }
            
            const Plane Flipped(Axis axis) const {
                return Plane(normal.Flipped(axis), distance);
            }
            
            void Flip(Axis axis, const Vec3f& center) {
                normal.Flip(axis);
                distance = Anchor().Flipped(axis, center) | normal;
            }
            
            const Plane Flipped(Axis axis, const Vec3f& center) const {
                return Plane(normal.Flipped(axis), Anchor().Flipped(axis, center));
            }
        };
    }
}

#endif
