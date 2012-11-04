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

#include "Utility/Vec3f.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Math {
        class Ray {
        public:
            Vec3f origin;
            Vec3f direction;

            Ray() : origin(Vec3f::Null), direction(Vec3f::Null) {}

            Ray(const Vec3f& origin, const Vec3f& direction) : origin(origin), direction(direction) {}

            inline const Vec3f pointAtDistance(float distance) const {
                return Vec3f(origin.x + direction.x * distance,
                             origin.y + direction.y * distance,
                             origin.z + direction.z * distance);
            }

            inline PointStatus::Type pointStatus(const Vec3f& point) const {
                float dot = direction.dot(point - origin);
                if (dot >  Math::PointStatusEpsilon)
                    return PointStatus::PSAbove;
                if (dot < -Math::PointStatusEpsilon)
                    return PointStatus::PSBelow;
                return PointStatus::PSInside;
            }

            float intersectWithSphere(const Vec3f& position, float radius) const {
                Vec3f diff = origin - position;
                
                float p = 2.0f * diff.dot(direction);
                float q = diff.lengthSquared() - radius * radius;

                float d = p * p - 4.0f * q;
                if (d < 0.0f)
                    return Math::nan();
                
                float s = sqrt(d);
                float t0 = (-p + s) / 2.0f;
                float t1 = (-p - s) / 2.0f;
                
                if (t0 < 0.0f && t1 < 0.0f)
                    return Math::nan();
                if (t0 > 0.0f && 1 > 0.0f)
                    return std::min(t0, t1);
                return std::max(t0, t1);
            }
            
            float squaredDistanceToSegment(const Vec3f& start, const Vec3f& end, float& distanceToClosestPoint) const {
                Vec3f u, v, w;
                u = end - start;
                v = direction;
                w = start - origin;
                
                float a = u.dot(u);
                float b = u.dot(v);
                float c = v.dot(v);
                float d = u.dot(w);
                float e = v.dot(w);
                float D = a * c - b * b;
                float sN, sD = D;
                float tN, tD = D;
                
                if (zero(D)) {
                    sN = 0.0f;
                    sD = 1.0f;
                    tN = e;
                    tD = c;
                } else {
                    sN = (b * e - c * d);
                    tN = (a * e - b * d);
                    if (sN < 0.0f) {
                        sN = 0.0f;
                        tN = e;
                        tD = c;
                    } else if (sN > sD) {
                        sN = sD;
                        tN = e + b;
                        tD = c;
                    }
                }
                
                if (tN < 0.0f)
                    return nan();
                
                float sc = zero(sN) ? 0.0f : sN / sD;
                float tc = zero(tN) ? 0.0f : tN / tD;

                distanceToClosestPoint = tc;

                u = u * sc;
                v = v * tc;
                w = w + u;
                Vec3f dP = w - v;

                return dP.lengthSquared();
            }
            
            float distanceToSegment(const Vec3f& start, const Vec3f& end, float& distanceToClosestPoint) const {
                float squaredDistance = squaredDistanceToSegment(start, end, distanceToClosestPoint);
                if (isnan(squaredDistance))
                    return squaredDistance;
                return sqrt(squaredDistance);
            }
        };
    }
}

#endif
