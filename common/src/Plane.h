/*
Copyright (C) 2010-2014 Kristian Duske

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
#include "Mat.h"
#include "Ray.h"
#include "Vec.h"
#include <vector>

template <typename T, size_t S>
class Plane {
public:
    typedef std::vector<Plane> List;
    typedef std::set<Plane> Set;
    
    class WeightOrder {
    private:
        bool m_deterministic;
    public:
        WeightOrder(const bool deterministic) :
        m_deterministic(deterministic) {}
        
        bool operator()(const Plane<T,3>& lhs, const Plane<T,3>& rhs) const {
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

    int compare(const Plane<T,S>& other, const T epsilon = static_cast<T>(0.0)) const {
        if (Math::lt(distance, other.distance, epsilon))
            return -1;
        if (Math::gt(distance, other.distance, epsilon))
            return 1;
        return normal.compare(other.normal);
    }
    
    bool operator==(const Plane<T,S>& other) const {
        return compare(other) == 0;
    }
    
    bool operator!= (const Plane<T,S>& other) const {
        return compare(other) != 0;
    }
    
    bool operator<(const Plane<T,S>& other) const {
        return compare(other) < 0;
    }
    
    bool operator<= (const Plane<T,S>& other) const {
        return compare(other) <= 0;
    }
    
    bool operator>(const Plane<T,S>& other) const {
        return compare(other) > 0;
    }
    
    bool operator>= (const Plane<T,S>& other) const {
        return compare(other) >= 0;
    }

    const Vec<T,S> anchor() const {
        return normal * distance;
    }
    
    T intersectWithRay(const Ray<T,S>& ray) const {
        return ray.intersectWithPlane(normal, anchor());
    }
    
    T intersectWithLine(const Line<T,S>& line) const {
        const T f = line.direction.dot(normal);
        if (Math::zero(f))
            return Math::nan<T>();
        return ((distance * normal - line.point).dot(normal)) / f;
    }
    
    Math::PointStatus::Type pointStatus(const Vec<T,S>& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
        const T dist = pointDistance(point);
        if (dist >  epsilon)
            return Math::PointStatus::PSAbove;
        if (dist < -epsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }
    
    T pointDistance(const Vec<T,S>& point) const {
        return point.dot(normal) - distance;
    }
    
    T at(const Vec<T,S-1>& point, const Math::Axis::Type axis) const {
        if (Math::zero(normal[axis]))
            return static_cast<T>(0.0);
        
        T t = static_cast<T>(0.0);
        size_t index = 0;
        for (size_t i = 0; i < S; i++)
            if (i != axis)
                t += normal[i] * point[index++];
        return (distance - t) / normal[axis];
    }
            
    T xAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AX);
    }
    
    T yAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AY);
    }
    
    T zAt(const Vec<T,S-1>& point) const {
        return at(point, Math::Axis::AZ);
    }
    
    bool equals(const Plane<T,S>& other, const T epsilon = Math::Constants<T>::almostZero()) const {
        return Math::eq(distance, other.distance, epsilon) && normal.equals(other.normal, epsilon);
    }
    
    Plane<T,S>& flip() {
        normal = -normal;
        distance = -distance;
        return *this;
    }
    
    Plane<T,S> flipped() const {
        return Plane<T,S>(*this).flip();
    }
    
    Plane<T,S>& transform(const Mat<T,S+1,S+1>& transform) {
        const Vec<T,3> oldAnchor = anchor();
        normal = stripTranslation(transform) * normal;
        normal.normalize();
        distance = (transform * oldAnchor).dot(normal);
        return *this;
    }
            
    Plane<T,S> transformed(const Mat<T,S+1,S+1>& transform) const {
        return Plane<T,S>(*this).transform(transform);
    }
    
    /*
    Plane<T>& rotate(const Quat<T>& rotation, const Vec<T,3>& center) {
        const Vec<T,3> oldAnchor = anchor();
        normal = rotation * normal;
        distance = (rotation * (oldAnchor - center) + center).dot(normal);
        return *this;
    }
    
    const Plane<T> rotated(const Quat<T>& rotation, const Vec<T,3>& center) const {
        const Vec<T,3> oldAnchor = anchor();
        return Plane(rotation * normal, rotation * (oldAnchor - center) + center);
    }
             */
    
    Vec<T,S> project(const Vec<T,S>& point) const {
        return point - point.dot(normal) * normal + distance * normal;
    }

    Vec<T,S> project(const Vec<T,S>& point, const Vec<T,S>& direction) const {
        const T f = direction.dot(normal);
        if (Math::zero(f))
            return Vec<T,S>::NaN;
        const T d = ((distance * normal - point).dot(normal)) / f;
        return point + direction * d;
    }
    
    typename Vec<T,S>::List project(const typename Vec<T,S>::List& points) const {
        typename Vec<T,S>::List result(points.size());
        for (size_t i = 0; i < points.size(); ++i)
            result[i] = project(points[i]);
        return result;
    }

    typename Vec<T,S>::List project(const typename Vec<T,S>::List& points, const Vec<T,S>& direction) const {
        typename Vec<T,S>::List result(points.size());
        for (size_t i = 0; i < points.size(); ++i)
            result[i] = project(points[i], direction);
        return result;
    }
};

template <typename T>
bool setPlanePoints(Plane<T,3>& plane, const Vec<T,3>* points) {
    return setPlanePoints(plane, points[0], points[1], points[2]);
}
            
template <typename T>
bool setPlanePoints(Plane<T,3>& plane, const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2) {
    const Vec<T,3> normal = crossed(point0, point1, point2);
    if (normal.null())
        return false;
    plane.normal = normal.normalized();
    plane.distance = point0.dot(plane.normal);
    return true;
}

template <typename T>
Plane<T,3> horizontalDragPlane(const Vec<T,3>& position) {
    return Plane<T,3>(position, Vec<T,3>::PosZ);
}

template <typename T>
Plane<T,3> verticalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    if (direction.firstComponent() != Math::Axis::AZ)
        return Plane<T,3>(position, direction.firstAxis());
    return Plane<T,3>(position, direction.secondAxis());
}

template <typename T>
Plane<T,3> orthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    return Plane<T,3>(position, direction.normalized());
}

template <typename T>
Plane<T,3> alignedOrthogonalDragPlane(const Vec<T,3>& position, const Vec<T,3>& direction) {
    return Plane<T,3>(position, direction.firstAxis());
}

template <typename T>
Plane<T,3> containingDragPlane(const Vec<T,3>& position, const Vec<T,3>& normal, const Vec<T,3>& cameraPosition) {
    const Vec<T,3> fromCamera = (position - cameraPosition).normalized();
    const Vec<T,3> vertical = crossed(normal, fromCamera);
    return Plane<T,3>(position, crossed(normal, vertical));
}

typedef Plane<float,3> Plane3f;
typedef Plane<double,3> Plane3d;

#endif
