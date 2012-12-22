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

            Ray(const Vec3f& i_origin, const Vec3f& i_direction) : origin(i_origin), direction(i_direction) {}

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

            inline float intersectWithPlane(const Vec3f& normal, const Vec3f& anchor) const {
                float d = direction.dot(normal);
                if (Math::zero(d))
                    return Math::nan();
                
                float s = ((anchor - origin).dot(normal)) / d;
                if (Math::neg(s))
                    return Math::nan();
                return s;
            }

            float intersectWithSphere(const Vec3f& position, float radius) const {
                Vec3f diff = origin - position;
                
                float p = 2.0f * diff.dot(direction);
                float q = diff.lengthSquared() - radius * radius;

                float d = p * p - 4.0f * q;
                if (d < 0.0f)
                    return Math::nan();
                
                float s = std::sqrt(d);
                float t0 = (-p + s) / 2.0f;
                float t1 = (-p - s) / 2.0f;
                
                if (t0 < 0.0f && t1 < 0.0f)
                    return Math::nan();
                if (t0 > 0.0f && 1 > 0.0f)
                    return std::min(t0, t1);
                return std::max(t0, t1);
            }
            
            float intersectWithSphere(const Vec3f& position, float radius, float scalingFactor, float maxDistance) const {
                float distanceToCenter = (position - origin).length();
                if (distanceToCenter > maxDistance)
                    return Math::nan();
                    
                float scaledRadius = radius * scalingFactor * distanceToCenter;
                return intersectWithSphere(position, scaledRadius);
            }
            
            float intersectWithCube(const Vec3f& position, float size) const {
                float halfSize = size / 2.0f;
                
                if (direction.x < 0.0f) {
                    float distance = intersectWithPlane(Vec3f::PosX, Vec3f(position.x + halfSize, position.y, position.z));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.x > 0.0f) {
                    float distance = intersectWithPlane(Vec3f::NegX, Vec3f(position.x - halfSize, position.y, position.z));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                }
                
                if (direction.y < 0.0f) {
                    float distance = intersectWithPlane(Vec3f::PosY, Vec3f(position.x, position.y + halfSize, position.z));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.y > 0.0f) {
                    float distance = intersectWithPlane(Vec3f::NegY, Vec3f(position.x, position.y - halfSize, position.z));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                }
                
                if (direction.z < 0.0f) {
                    float distance = intersectWithPlane(Vec3f::PosZ, Vec3f(position.x, position.y, position.z + halfSize));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.z > 0.0f) {
                    float distance = intersectWithPlane(Vec3f::NegZ, Vec3f(position.x, position.y, position.z - halfSize));
                    if (!Math::isnan(distance)) {
                        Vec3f point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize) {
                            return distance;
                        }
                    }
                }
                
                return Math::nan();
            }
            
            float squaredDistanceToPoint(const Vec3f& point, float& distanceToClosestPoint) const {
                distanceToClosestPoint = (point - origin).dot(direction);
                if (distanceToClosestPoint <= 0.0f)
                    return Math::nan();
                return (pointAtDistance(distanceToClosestPoint) - point).lengthSquared();
            }
            
            float distanceToPoint(const Vec3f& point, float& distanceToClosestPoint) const {
                float squaredDistance = squaredDistanceToPoint(point, distanceToClosestPoint);
                if (Math::isnan(squaredDistance))
                    return squaredDistance;
                return std::sqrt(squaredDistance);
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
                return std::sqrt(squaredDistance);
            }
        };
    }
}

#endif
