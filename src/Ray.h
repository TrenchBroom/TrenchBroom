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

    inline const Vec<T,S> pointAtDistance(const T distance) const {
        return origin + direction * distance;
    }

    inline PointStatus::Type pointStatus(const Vec<T,S>& point) const {
        const T dot = direction.dot(point - origin);
        if (dot >  Math<T>::PointStatusEpsilon)
            return PointStatus::PSAbove;
        if (dot < -Math<T>::PointStatusEpsilon)
            return PointStatus::PSBelow;
        return PointStatus::PSInside;
    }

    inline T intersectWithPlane(const Vec<T,S>& normal, const Vec<T,S>& anchor) const {
        const T d = direction.dot(normal);
        if (Math<T>::zero(d))
            return Math<T>::nan();
        
        const T s = ((anchor - origin).dot(normal)) / d;
        if (Math<T>::neg(s))
            return Math<T>::nan();
        return s;
    }

    T intersectWithSphere(const Vec<T,S>& position, const T radius) const {
        const Vec<T,S> diff = origin - position;
        
        const T p = static_cast<T>(2.0) * diff.dot(direction);
        const T q = diff.lengthSquared() - radius * radius;

        const T d = p * p - static_cast<T>(4.0) * q;
        if (d < static_cast<T>(0.0))
            return Math<T>::nan();
        
        const T s = std::sqrt(d);
        const T t0 = (-p + s) / static_cast<T>(2.0);
        const T t1 = (-p - s) / static_cast<T>(2.0);
        
        if (t0 < static_cast<T>(0.0) && t1 < static_cast<T>(0.0))
            return Math<T>::nan();
        if (t0 > static_cast<T>(0.0) && t1 > static_cast<T>(0.0))
            return std::min(t0, t1);
        return std::max(t0, t1);
    }
    
    T intersectWithSphere(const Vec<T,S>& position, const T radius, const T scalingFactor, const T maxDistance) const {
        const T distanceToCenter = (position - origin).length();
        if (distanceToCenter > maxDistance)
            return Math<T>::nan();
            
        const T scaledRadius = radius * scalingFactor * distanceToCenter;
        return intersectWithSphere(position, scaledRadius);
    }
    
    T squaredDistanceToPoint(const Vec<T,S>& point, T& distanceToClosestPoint) const {
        distanceToClosestPoint = (point - origin).dot(direction);
        if (distanceToClosestPoint <= 0.0)
            return Math<T>::nan();
        return (pointAtDistance(distanceToClosestPoint) - point).lengthSquared();
    }
    
    T distanceToPoint(const Vec<T,S>& point, T& distanceToClosestPoint) const {
        const T squaredDistance = squaredDistanceToPoint(point, distanceToClosestPoint);
        if (Math<T>::isnan(squaredDistance))
            return squaredDistance;
        return std::sqrt(squaredDistance);
    }
    
    T squaredDistanceToSegment(const Vec<T,S>& start, const Vec<T,3>& end, Vec<T,3>& pointOnSegment, T& distanceToClosestPoint) const {
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
        
        if (Math<T>::zero(D)) {
            sN = 0.0;
            sD = 1.0;
            tN = e;
            tD = c;
        } else {
            sN = (b * e - c * d);
            tN = (a * e - b * d);
            if (sN < 0.0) {
                sN = 0.0;
                tN = e;
                tD = c;
            } else if (sN > sD) {
                sN = sD;
                tN = e + b;
                tD = c;
            }
        }
        
        if (tN < 0.0)
            return Math<T>::nan();
        
        const T sc = Math<T>::zero(sN) ? static_cast<T>(0.0) : sN / sD;
        const T tc = Math<T>::zero(tN) ? static_cast<T>(0.0) : tN / tD;

        distanceToClosestPoint = tc;

        u = u * sc;
        v = v * tc;
        w = w + u;
        const Vec<T,S> dP = w - v;

        pointOnSegment = start + u;
        return dP.lengthSquared();
    }
    
    T distanceToSegment(const Vec<T,S>& start, const Vec<T,S>& end, Vec<T,S>& pointOnSegment, T& distanceToClosestPoint) const {
        const T squaredDistance = squaredDistanceToSegment(start, end, pointOnSegment, distanceToClosestPoint);
        if (Math<T>::isnan(squaredDistance))
            return squaredDistance;
        return std::sqrt(squaredDistance);
    }
    
    T squaredDistanceToLine(const Vec<T,S>& lineAnchor, const Vec<T,S>& lineDir, Vec<T,S>& pointOnLine, T& distanceToClosestPoint) const {
        const Vec<T,S> w0 = origin - lineAnchor;
        const T a = direction.dot(direction);
        const T b = direction.dot(lineDir);
        const T c = lineDir.dot(lineDir);
        const T d = direction.dot(w0);
        const T e = lineDir.dot(w0);
        
        const T f = a * c - b * b;
        if (Math<T>::zero(f)) {
            distanceToClosestPoint = Math<T>::nan();
            return Math<T>::nan();
        }
        
        T sc = (b * e - c * d) / f;
        const T tc = (a * e - b * d) / f;
        
        if (sc < 0.0)
            sc = 0.0;
        
        const Vec<T,3> pointOnRay = origin + sc * direction;
        pointOnLine = lineAnchor + tc * lineDir;
        distanceToClosestPoint = sc;
        return (pointOnLine - pointOnRay).lengthSquared();
    }
    
    T distanceToLine(const Vec<T,S>& lineAnchor, const Vec<T,S>& lineDir, Vec<T,S>& pointOnLine, T& distanceToClosestPoint) const {
        const T squaredDistance = squaredDistanceToLine(lineAnchor, lineDir, pointOnLine, distanceToClosestPoint);
        if (Math<T>::isnan(squaredDistance))
            return squaredDistance;
        return std::sqrt(squaredDistance);
    }
};

typedef Ray<float,3> Ray3f;
typedef Ray<double,3> Ray3d;

#endif
