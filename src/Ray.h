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

#ifndef TrenchBroom_Ray_h
#define TrenchBroom_Ray_h

#include "MathUtils.h"
#include "Vec.h"

#include <algorithm>

template <typename T, size_t S>
class Ray {
public:
    Vec<T,S> origin;
    Vec<T,S> direction;

    Ray() :
    origin(Vec<T,S>::Null),
    direction(Vec<T,S>::Null) {}

    Ray(const Vec<T,S>& i_origin, const Vec<T,S>& i_direction) :
    origin(i_origin),
    direction(i_direction) {}

    template <typename U>
    Ray(const Ray<U,S>& other) :
    origin(other.origin),
    direction(other.direction) {}
    
    inline const Vec<T,S> pointAtDistance(const T distance) const {
        return origin + direction * distance;
    }

    inline Math::PointStatus::Type pointStatus(const Vec<T,S>& point) const {
        const T dot = direction.dot(point - origin);
        if (dot >  Math::Constants<T>::PointStatusEpsilon)
            return Math::PointStatus::PSAbove;
        if (dot < -Math::Constants<T>::PointStatusEpsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }

    inline const T intersectWithPlane(const Vec<T,S>& normal, const Vec<T,S>& anchor) const {
        const T d = direction.dot(normal);
        if (Math::zero(d))
            return Math::nan<T>();
        
        const T s = ((anchor - origin).dot(normal)) / d;
        if (Math::neg(s))
            return Math::nan<T>();
        return s;
    }

    const T intersectWithSphere(const Vec<T,S>& position, const T radius) const {
        const Vec<T,S> diff = origin - position;
        
        const T p = static_cast<T>(2.0) * diff.dot(direction);
        const T q = diff.squaredLength() - radius * radius;

        const T d = p * p - static_cast<T>(4.0) * q;
        if (d < static_cast<T>(0.0))
            return Math::nan<T>();
        
        const T s = std::sqrt(d);
        const T t0 = (-p + s) / static_cast<T>(2.0);
        const T t1 = (-p - s) / static_cast<T>(2.0);
        
        if (t0 < static_cast<T>(0.0) && t1 < static_cast<T>(0.0))
            return Math::nan<T>();
        if (t0 > static_cast<T>(0.0) && t1 > static_cast<T>(0.0))
            return std::min(t0, t1);
        return std::max(t0, t1);
    }
    
    const T intersectWithSphere(const Vec<T,S>& position, const T radius, const T scalingFactor, const T maxDistance) const {
        const T distanceToCenter = (position - origin).length();
        if (distanceToCenter > maxDistance)
            return Math::nan<T>();
            
        const T scaledRadius = radius * scalingFactor * distanceToCenter;
        return intersectWithSphere(position, scaledRadius);
    }
    
    struct PointDistance {
        T rayDistance;
        T distance;
    };
    
    const PointDistance distanceToPointSquared(const Vec<T,S>& point) const {
        const Vec3f originToPoint = point - origin;
        PointDistance result;
        result.rayDistance = std::max(originToPoint.dot(direction), static_cast<T>(0.0));
        if (result.rayDistance == static_cast<T>(0.0))
            result.distance = originToPoint.squaredLength();
        else
            result.distance = (pointAtDistance(result.rayDistance) - point).squaredLength();
        return result;
    }
    
    inline const PointDistance distanceToPoint(const Vec<T,S>& point) const {
        PointDistance distance2 = distanceToPointSquared(point);
        distance2.distance = std::sqrt(distance2.distance);
        return distance2;
    }
    
    struct LineDistance {
        bool parallel;
        T rayDistance;
        T distance;
        Vec<T,S> point;
        
        inline static const LineDistance Parallel(const T distance) {
            LineDistance result;
            result.parallel = true;
            result.rayDistance = Math::nan<T>();
            result.distance = distance;
            result.point = Vec<T,S>::Null;
            return result;
        }

        inline static const LineDistance NonParallel(const T rayDistance, const T distance, const Vec<T,S>& point) {
            LineDistance result;
            result.parallel = false;
            result.rayDistance = rayDistance;
            result.distance = distance;
            result.point = point;
            return result;
        }
    };

    const LineDistance distanceToSegmentSquared(const Vec<T,S>& start, const Vec<T,3>& end) const {
        Vec<T,S> u, v, w;
        u = end - start;
        v = direction;
        w = start - origin;
        
        const T a = u.dot(u);
        const T b = u.dot(v);
        const T c = v.dot(v);
        const T d = u.dot(w);
        const T e = v.dot(w);
        const T D = a * c - b * b;
        T sN, sD = D;
        T tN, tD = D;
        
        if (Math::zero(D))
            return LineDistance::Parallel(w.squaredLength());

        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < static_cast<T>(0.0)) {
            sN = static_cast<T>(0.0);
            tN = e;
            tD = c;
        } else if (sN > sD) {
            sN = sD;
            tN = e + b;
            tD = c;
        }
        
        const T sc = Math::zero(sN) ? static_cast<T>(0.0) : sN / sD;
        const T tc = std::max(Math::zero(tN) ? static_cast<T>(0.0) : tN / tD, static_cast<T>(0.0));

        u = u * sc; // vector from segment start to the closest point on the segment
        v = v * tc; // vector from ray origin to closest point on the ray
        w = w + u;
        const Vec<T,S> dP = w - v;

        return LineDistance::NonParallel(tc, dP.squaredLength(), start + u);
    }
    
    inline const LineDistance distanceToSegment(const Vec<T,S>& start, const Vec<T,S>& end, Vec<T,S>& pointOnSegment, T& distanceToClosestPoint) const {
        LineDistance distance2 = squaredDistanceToSegment(start, end);
        distance2.distance = std::sqrt(distance2.distance);
        return distance2;
    }
    
    const LineDistance distanceToLineSquared(const Vec<T,S>& lineAnchor, const Vec<T,S>& lineDir) const {
        const Vec<T,S> w0 = origin - lineAnchor;
        const T a = direction.dot(direction);
        const T b = direction.dot(lineDir);
        const T c = lineDir.dot(lineDir);
        const T d = direction.dot(w0);
        const T e = lineDir.dot(w0);
        
        const T f = a * c - b * b;
        if (Math::zero(f))
            return LineDistance::Parallel(w0.squaredLength());
        
        const T sc = std::max((b * e - c * d) / f, static_cast<T>(0.0));
        const T tc = (a * e - b * d) / f;
        
        const Vec<T,S> rp = origin + sc * direction;
        const Vec<T,S> lp = lineAnchor + tc * lineDir;
        return LineDistance::NonParallel(sc, (rp - lp).squaredLength(), lp);
    }
    
    inline const LineDistance distanceToLine(const Vec<T,S>& lineAnchor, const Vec<T,S>& lineDir) const {
        LineDistance distance2 = distanceToLineSquared(lineAnchor, lineDir);
        distance2.distance = std::sqrt(distance2.distance);
        return distance2;
    }
};

typedef Ray<float,3> Ray3f;
typedef Ray<double,3> Ray3d;

#endif
