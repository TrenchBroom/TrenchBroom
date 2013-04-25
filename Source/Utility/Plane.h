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
#include "Utility/Vec.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Plane {
        public:
            class WeightOrder {
            private:
                bool m_deterministic;
            public:
                WeightOrder(const bool deterministic) :
                m_deterministic(deterministic) {}
                
                inline bool operator()(const Plane<T>& lhs, const Plane<T>& rhs) const {
                    int result = lhs.normal.weight() - rhs.normal.weight();
                    if (m_deterministic)
                        result += static_cast<int>(1000.0 * (lhs.distance - lhs.distance));
                    return result < 0;
                }
            };

            Vec<T,3> normal;
            T distance;
            
            Plane() : normal(Vec<T,3>::Null), distance(0.0) {}
            
            Plane(const Vec<T,3>& i_normal, const T i_distance) : normal(i_normal), distance(i_distance) {}
            
            Plane(const Vec<T,3>& i_normal, const Vec<T,3>& i_anchor) : normal(i_normal), distance(i_anchor.dot(i_normal)) {}
            
            static const Plane<T> horizontalDragPlane(const Vec<T,3>& position) {
                return Plane<T>(Vec<T,3>::PosZ, position);
            }
            
            static const Plane<T> verticalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
                if (direction.firstComponent() != Axis::AZ)
                    return Plane<T>(direction.firstAxis(), position);
                return Plane<T>(direction.secondAxis(), position);
            }
            
            static const Plane<T> orthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
                return Plane<T>(direction, position);
            }
            
            static const Plane<T> alignedOrthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
                return Plane<T>(direction.firstAxis(), position);
            }
            
            static const Plane<T> planeContainingVector(const Vec<T,3>& position, const Vec<T,3>& normalizedVector, const Vec<T,3>& viewPoint) {
                Vec<T,3> diff = viewPoint - position;
                Vec<T,3> point = position + normalizedVector * diff.dot(normalizedVector);
                Vec<T,3> normal = viewPoint - point;
                normal.normalize();
                return Plane(normal, position);
            }
            
            inline bool setPoints(const Vec<T,3>& point1, const Vec<T,3>& point2, const Vec<T,3>& point3) {
                const Vec<T,3> v1 = point3 - point1;
                const Vec<T,3> v2 = point2 - point1;
                normal = v1.crossed(v2);
                if (normal.equals(Vec<T,3>::Null, Math<T>::AlmostZero))
                    return false;
                normal.normalize();
                distance = point1.dot(normal);
                return true;
            }
            
            inline const Vec<T,3> anchor() const {
                return normal * distance;
            }
            
            inline T intersectWithRay(const Ray<T>& ray) const {
                return ray.intersectWithPlane(normal, anchor());
            }
            
            inline T intersectWithLine(const Line<T>& line) const {
                const T d = line.direction.dot(normal);
                if (Math<T>::zero(d))
                    return Math<T>::nan();
                return ((anchor() - line.point).dot(normal)) / d;
            }
            
            inline PointStatus::Type pointStatus(const Vec<T,3>& point, const T epsilon = Math<T>::PointStatusEpsilon) const {
                const T dist = pointDistance(point);
                if (dist >  epsilon)
                    return PointStatus::PSAbove;
                if (dist < -epsilon)
                    return PointStatus::PSBelow;
                return PointStatus::PSInside;
            }
            
            inline T pointDistance(const Vec<T,3>& point) const {
                return point.dot(normal) - distance;
            }
            
            inline T x(const T y, const T z) const {
                return (distance - normal.y * y - normal.z * z) / normal.x;
            }
            
            inline T y(const T x, const T z) const {
                return (distance - normal.x * x - normal.z * z) / normal.y;
            }
            
            inline T z(const T x, const T y) const {
                return (distance - normal.x * x - normal.y * y) / normal.z;
            }
                    
            inline T z(const Vec<T,2>& coords) const {
                return z(coords.x, coords.y);
            }
            
            inline bool equals(const Plane<T>& other, const T epsilon = Math<T>::AlmostZero) const {
                return normal.equals(other.normal, epsilon) && std::abs(distance - other.distance) < epsilon;
            }
            
            inline Plane<T>& translate(const Vec<T,3>& delta) {
                distance = (anchor() + delta).dot(normal);
                return *this;
            }
            
            inline const Plane<T> translated(const Vec<T,3>& delta) const {
                return Plane<T>(normal, (anchor() + delta).dot(normal));
            }
            
            inline Plane<T>& rotate90(const Axis::Type axis, const Vec<T,3>& center, const bool clockwise) {
                const Vec<T,3> oldAnchor = anchor();
                normal.rotate90(axis, clockwise);
                distance = (oldAnchor.rotated90(axis, center, clockwise)).dot(normal);
                return *this;
            }
            
            inline const Plane<T> rotated90(const Axis::Type axis, const Vec<T,3>& center, const bool clockwise) const {
                return Plane<T>(normal.rotated90(axis, clockwise), anchor().rotated90(axis, center, clockwise));
            }
            
            inline Plane<T>& rotate(const Quat<T>& rotation, const Vec<T,3>& center) {
                const Vec<T,3> oldAnchor = anchor();
                normal = rotation * normal;
                distance = (rotation * (oldAnchor - center) + center).dot(normal);
                return *this;
            }
            
            inline const Plane<T> rotated(const Quat<T>& rotation, const Vec<T,3>& center) const {
                const Vec<T,3> oldAnchor = anchor();
                return Plane(rotation * normal, rotation * (oldAnchor - center) + center);
            }
            
            inline Plane& flip(const Axis::Type axis, const Vec<T,3>& center) {
                const Vec<T,3> oldAnchor = anchor();
                normal.flip(axis);
                distance = oldAnchor.flipped(axis, center).dot(normal);
                return *this;
            }
            
            inline const Plane<T> flipped(const Axis::Type axis, const Vec<T,3>& center) const {
                const Vec<T,3> oldAnchor = anchor();
                return Plane(normal.flipped(axis), oldAnchor.flip(axis, center));
            }
            
            inline Vec<T,3> project(const Vec<T,3>& v) const {
                return v - v.dot(normal) * v.normalized();
            }
        };
                    
        typedef Plane<float> Planef;
    }
}

#endif
