/*
Copyright (C) 2010-2017 Kristian Duske

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

#include "vec_decl.h"
#include "vec_impl.h"
#include "Line.h"
#include "MathUtils.h"
#include "mat_forward.h"
#include "Ray.h"

#include <set>
#include <vector>

template <typename T, size_t S>
class Plane {
public:
    using List = std::vector<Plane>;
    using Set = std::set<Plane>;

    T distance;
    vec<T,S> normal;
    
    Plane() :
    distance(static_cast<T>(0.0)),
    normal(vec<T,S>::zero) {}
    
    // Copy and move constructors
    Plane(const Plane<T,S>& other) = default;
    Plane(Plane<T,S>&& other) = default;
    
    // Assignment operators
    Plane<T,S>& operator=(const Plane<T,S>& other) = default;
    Plane<T,S>& operator=(Plane<T,S>&& other) = default;
    
    // Conversion constructor
    template <typename U>
    Plane(const Plane<U,S>& other) :
    distance(static_cast<T>(other.distance)),
    normal(other.normal) {}

    Plane(const T i_distance, const vec<T,S>& i_normal) :
    distance(i_distance),
    normal(i_normal) {}
    
    Plane(const vec<T,S>& i_anchor, const vec<T,S>& i_normal) :
    distance(dot(i_anchor, i_normal)),
    normal(i_normal) {}
            
    static const Plane<T,S> planeContainingVector(const vec<T,S>& position, const vec<T,S>& normalizedVector, const vec<T,S>& viewPoint) {
        const vec<T,S> diff = viewPoint - position;
        const vec<T,S> point = position + normalizedVector * diff.dot(normalizedVector);
        const vec<T,S> normal = normalize(viewPoint - point);
        return Plane(normal, position);
    }

    int compare(const Plane<T,S>& other, const T epsilon = static_cast<T>(0.0)) const {
        if (Math::lt(distance, other.distance, epsilon))
            return -1;
        if (Math::gt(distance, other.distance, epsilon))
            return 1;
        return ::compare(normal, other.normal, epsilon);
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

    const vec<T,S> anchor() const {
        return normal * distance;
    }
    
    T intersectWithRay(const Ray<T,S>& ray) const {
        return ray.intersectWithPlane(normal, anchor());
    }
    
    T intersectWithLine(const Line<T,S>& line) const {
        const T f = dot(line.direction, normal);
        if (Math::zero(f))
            return Math::nan<T>();
        return dot(distance * normal - line.point, normal) / f;
    }
    
    Line<T,S> intersectWithPlane(const Plane<T,S>& other) const {
        const vec<T,S> lineDirection = normalize(cross(normal, other.normal));
        
        if (isNaN(lineDirection)) {
            // the planes are parallel
            return Line<T,S>();
        }
        
        // Now we need to find a point that is on both planes.
        
        // From: http://geomalgorithms.com/a05-_intersect-1.html
        // Project the other plane's normal onto this plane.
        // This will give us a line direction from this plane's anchor that
        // intersects the other plane.
        
        const Line<T,S> lineToOtherPlane{anchor(), normalize(projectVector(other.normal))};
        const T dist = other.intersectWithLine(lineToOtherPlane);
        const vec<T,S> point = lineToOtherPlane.pointAtDistance(dist);
        
        if (isNaN(point)) {
            return Line<T,S>();
        }
        return Line<T,S>(point, lineDirection);
    }
    
    Math::PointStatus::Type pointStatus(const vec<T,S>& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
        const T dist = pointDistance(point);
        if (dist >  epsilon)
            return Math::PointStatus::PSAbove;
        if (dist < -epsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }
    
    T pointDistance(const vec<T,S>& point) const {
        return dot(point, normal) - distance;
    }
    
    T at(const vec<T,S-1>& point, const Math::Axis::Type axis) const {
        if (Math::zero(normal[axis]))
            return static_cast<T>(0.0);
        
        T t = static_cast<T>(0.0);
        size_t index = 0;
        for (size_t i = 0; i < S; i++) {
            if (i != axis) {
                t += normal[i] * point[index++];
            }
        }
        return (distance - t) / normal[axis];
    }
            
    T xAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AX);
    }
    
    T yAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AY);
    }
    
    T zAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AZ);
    }
    
    bool equals(const Plane<T,S>& other, const T epsilon = Math::Constants<T>::almostZero()) const {
        return Math::eq(distance, other.distance, epsilon) && equal(normal, other.normal, epsilon);
    }
    
    Plane<T,S>& flip() {
        normal = -normal;
        distance = -distance;
        return *this;
    }
    
    Plane<T,S> flipped() const {
        return Plane<T,S>(*this).flip();
    }
    
    Plane<T,S>& transform(const mat<T,S+1,S+1>& transform) {
        const vec<T,3> oldAnchor = anchor();
        normal = normalize(stripTranslation(transform) * normal);
        distance = dot(transform * oldAnchor, normal);
        return *this;
    }
            
    Plane<T,S> transformed(const mat<T,S+1,S+1>& transform) const {
        return Plane<T,S>(*this).transform(transform);
    }
    
    /*
    Plane<T>& rotate(const Quat<T>& rotation, const vec<T,3>& center) {
        const vec<T,3> oldAnchor = anchor();
        normal = rotation * normal;
        distance = (rotation * (oldAnchor - center) + center).dot(normal);
        return *this;
    }
    
    const Plane<T> rotated(const Quat<T>& rotation, const vec<T,3>& center) const {
        const vec<T,3> oldAnchor = anchor();
        return Plane(rotation * normal, rotation * (oldAnchor - center) + center);
    }
             */
    
    vec<T,S> projectPoint(const vec<T,S>& point) const {
        return point - dot(point, normal) * normal + distance * normal;
    }

    vec<T,S> projectPoint(const vec<T,S>& point, const vec<T,S>& direction) const {
        const T cos = dot(direction, normal);
        if (Math::zero(cos))
            return vec<T,S>::NaN;
        const T d = dot(distance * normal - point, normal) / cos;
        return point + direction * d;
    }
    
    typename vec<T,S>::List projectPoints(const typename vec<T,S>::List& points) const {
        typename vec<T,S>::List result(points.size());
        for (size_t i = 0; i < points.size(); ++i)
            result[i] = project(points[i]);
        return result;
    }

    typename vec<T,S>::List projectPoints(const typename vec<T,S>::List& points, const vec<T,S>& direction) const {
        typename vec<T,S>::List result(points.size());
        for (size_t i = 0; i < points.size(); ++i)
            result[i] = project(points[i], direction);
        return result;
    }
    
    vec<T,S> projectVector(const vec<T,S>& vector) const {
        return projectPoint(anchor() + vector) - anchor();
    }
    
    vec<T,S> projectVector(const vec<T,S>& vector, const vec<T,S>& direction) const {
        return projectPoint(anchor() + vector) - anchor();
    }
    
    typename vec<T,S>::List projectVectors(const typename vec<T,S>::List& vectors) const {
        typename vec<T,S>::List result(vectors.size());
        for (size_t i = 0; i < vectors.size(); ++i)
            result[i] = projectVector(vectors[i]);
        return result;
    }
    
    typename vec<T,S>::List projectVectors(const typename vec<T,S>::List& vectors, const vec<T,S>& direction) const {
        typename vec<T,S>::List result(vectors.size());
        for (size_t i = 0; i < vectors.size(); ++i)
            result[i] = projectVector(vectors[i], direction);
        return result;
    }
};

template <typename T>
bool setPlanePoints(Plane<T,3>& plane, const vec<T,3>* points) {
    return setPlanePoints(plane, points[0], points[1], points[2]);
}
            
/*
 * The normal will be pointing towards the reader when the points are oriented like this:
 *
 * 1
 * |
 * v2
 * |
 * |
 * 0------v1----2
 */
template <typename T>
bool setPlanePoints(Plane<T,3>& plane, const vec<T,3>& point0, const vec<T,3>& point1, const vec<T,3>& point2) {
    bool result;
    std::tie(result, plane.normal) = planeNormal(point0, point1, point2);
    plane.distance = dot(point0, plane.normal); // becomes 0 if plane points failed
    return result;
}

/**
 * Computes the normal of a plane in three point form.
 *
 * The normal will be pointing towards the reader when the points are oriented like this:
 *
 * 1
 * |
 * v2
 * |
 * |
 * 0------v1----2
 *
 * The function returns a pair of a boolean and vector. The boolean indicates whether the normal could be computed, i.e.,
 * whether or not the given points are a valid three point representation of a plane. The returned vector is the normal
 * if the points are indeed valid, and 0 otherwise.
 *
 * @tparam T the component type
 * @param point0 the first plane point
 * @param point1 the second plane point
 * @param point2 the third plane point
 * @param epsilon an epsilon value used to determine whether the given three points do not define a plane
 * @return a pair of a boolean and a vector
 */
template <typename T>
std::tuple<bool, vec<T,3>> planeNormal(const vec<T,3>& point0, const vec<T,3>& point1, const vec<T,3>& point2, const T epsilon = Math::Constants<T>::angleEpsilon()) {
    const auto v1 = point2 - point0;
    const auto v2 = point1 - point0;
    const auto normal = cross(v1, v2);

    // Fail if v1 and v2 are parallel, opposite, or either is zero-length.
    // Rearranging "A cross B = ||A|| * ||B|| * sin(theta) * n" (n is a unit vector perpendicular to A and B) gives
    // sin_theta below.
    const auto sin_theta = Math::abs(length(normal) / (length(v1) * length(v2)));
    if (Math::isnan(sin_theta) ||
        Math::isinf(sin_theta) ||
        sin_theta < epsilon) {
        return std::make_tuple(false, vec<T,3>::zero);
    } else {
        return std::make_tuple(true, normalize(normal));
    }
}



template <typename T>
Plane<T,3> fromPlanePoints(const vec<T,3>& point0, const vec<T,3>& point1, const vec<T,3>& point2) {
    Plane<T,3> result;
    assert(setPlanePoints(result, point0, point1, point2));
    return result;
}

template <typename T>
Plane<T,3> horizontalDragPlane(const vec<T,3>& position) {
    return Plane<T,3>(position, vec<T,3>::pos_z);
}

template <typename T>
Plane<T,3> verticalDragPlane(const vec<T,3>& position, const vec<T,3>& direction) {
    if (firstComponent(direction) != Math::Axis::AZ) {
        return Plane<T,3>(position, firstAxis(direction));
    } else {
        return Plane<T, 3>(position, secondAxis(direction));
    }
}

template <typename T>
Plane<T,3> orthogonalDragPlane(const vec<T,3>& position, const vec<T,3>& direction) {
    return Plane<T,3>(position, normalize(direction));
}

template <typename T>
Plane<T,3> alignedOrthogonalDragPlane(const vec<T,3>& position, const vec<T,3>& direction) {
    return Plane<T,3>(position, firstAxis(direction));
}

template <typename T>
Plane<T,3> containingDragPlane(const vec<T,3>& position, const vec<T,3>& normal, const vec<T,3>& cameraPosition) {
    const vec<T,3> fromCamera = normalize(position - cameraPosition);
    const vec<T,3> vertical = cross(normal, fromCamera);
    return Plane<T,3>(position, cross(normal, vertical));
}

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const Plane<T,S>& plane) {
    stream << "{normal:" << plane.normal << " distance:" << plane.distance << "}";
    return stream;
}

typedef Plane<float,3> Plane3f;
typedef Plane<double,3> Plane3d;

#endif
