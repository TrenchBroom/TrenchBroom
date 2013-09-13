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
along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TrenchBroom_Plane_h
#define TrenchBroom_Plane_h

#include "Line.h"
#include "MathUtils.h"
#include "Ray.h"
#include "Vec.h"

template <typename T, size_t S>
class Plane {
public:
    class WeightOrder {
    private:
        bool m_deterministic;
    public:
        WeightOrder(const bool deterministic) :
        m_deterministic(deterministic) {}
        
        inline bool operator()(const Plane<T,3>& lhs, const Plane<T,3>& rhs) const {
            int result = lhs.normal.weight() - rhs.normal.weight();
            if (m_deterministic)
                result += static_cast<int>(1000.0f * (lhs.distance - lhs.distance));
            return result < 0.0f;
        }
    };

    T distance;
    Vec<T,S> normal;
    
    Plane() :
    distance(static_cast<T>(0.0)),
    normal(Vec<T,S>::Null) {}
    
    Plane(const T i_distance, const Vec<T,S>& i_normal) :
    distance(i_distance),
    normal(i_normal) {}
    
    Plane(const Vec<T,S>& i_anchor, const Vec<T,S>& i_normal) :
    distance(i_anchor.dot(i_normal)),
    normal(i_normal) {}
            
    template <typename U>
    Plane(const Plane<U,S>& other) :
    distance(static_cast<T>(other.distance)),
    normal(other.normal) {}
    
    static const Plane<T,S> planeContainingVector(const Vec<T,S>& position, const Vec<T,S>& normalizedVector, const Vec<T,S>& viewPoint) {
        const Vec<T,S> diff = viewPoint - position;
        const Vec<T,S> point = position + normalizedVector * diff.dot(normalizedVector);
        const Vec<T,S> normal = (viewPoint - point).normalize();
        return Plane(normal, position);
    }
    
    inline const Vec<T,S> anchor() const {
        return normal * distance;
    }
    
    inline T intersectWithRay(const Ray<T,S>& ray) const {
        return ray.intersectWithPlane(normal, anchor());
    }
    
    inline T intersectWithLine(const Line<T,S>& line) const {
        const T d = line.direction.dot(normal);
        if (Math::zero(d))
            return Math::nan<T>();
        return ((anchor() - line.point).dot(normal)) / d;
    }
    
    inline Math::PointStatus::Type pointStatus(const Vec<T,S>& point, const T epsilon = Math::Constants<T>::PointStatusEpsilon) const {
        const T dist = pointDistance(point);
        if (dist >  epsilon)
            return Math::PointStatus::PSAbove;
        if (dist < -epsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }
    
    inline T pointDistance(const Vec<T,S>& point) const {
        return point.dot(normal) - distance;
    }
    
    inline T at(const Vec<T,S-1>& point, const Math::Axis::Type axis) const {
        if (Math::zero(normal[axis]))
            return static_cast<T>(0.0);
        
        T t = static_cast<T>(0.0);
        size_t index = 0;
        for (size_t i = 0; i < S; i++)
            if (i != axis)
                t += normal[i] * point[index++];
        return (distance - t) / normal[axis];
    }
            
    inline T xAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AX);
    }
    
    inline T yAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AY);
    }
    
    inline T zAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AZ);
    }
    
    inline bool equals(const Plane<T,S>& other, const T epsilon = Math::Constants<T>::AlmostZero) const {
        return Math::eq(distance, other.distance, epsilon) && normal.equals(other.normal, epsilon);
    }
            
            /*
    inline Plane<T,S>& transform(const Mat4f& pointTransform, const Mat4f& vectorTransform) {
        const Vec<T,3> oldAnchor = anchor();
        normal = vectorTransform * normal;
        normal.normalize();
        distance = (pointTransform * oldAnchor).dot(normal);
        return *this;
    }
            
    inline Plane<T> transformed(const Mat4f& pointTransform, const Mat4f& vectorTransform) const {
        return Plane<T>(*this).transform(pointTransform, vectorTransform);
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
             */
    
    inline Vec<T,S> project(const Vec<T,S>& v) const {
        return v - v.dot(normal) * normal;
    }
};

template <typename T>
inline bool setPlanePoints(Plane<T,3>& plane, const Vec<T,3>* points) {
    const Vec<T,3> v1 = points[2] - points[0];
    const Vec<T,3> v2 = points[1] - points[0];
    const Vec<T,3> normal = crossed(v1, v2);
    if (normal.equals(Vec<T,3>::Null, Math::Constants<T>::AlmostZero))
        return false;
    plane.normal = normal.normalized();
    plane.distance = points[0].dot(plane.normal);
    return true;
}
            
template <typename T>
inline const Plane<T,3> horizontalDragPlane(const Vec<T,3>& position) {
    return Plane<T,3>(position, Vec<T,3>::PosZ);
}

template <typename T>
inline const Plane<T,3> verticalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    if (direction.firstComponent() != Math::Axis::AZ)
        return Plane<T,3>(position, direction.firstAxis());
    return Plane<T,3>(position, direction.secondAxis());
}

template <typename T>
inline const Plane<T,3> orthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    return Plane<T,3>(position, direction.normalized());
}

template <typename T>
inline const Plane<T,3> alignedOrthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    return Plane<T,3>(position, direction.firstAxis());
}


typedef Plane<float,3> Plane3f;

#endif
