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

#include "Utility/Vec3.h"

#include <algorithm>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Ray {
        public:
            Vec3<T> origin;
            Vec3<T> direction;

            Ray() : origin(Vec3<T>::Null), direction(Vec3<T>::Null) {}

            Ray(const Vec3<T>& i_origin, const Vec3<T>& i_direction) : origin(i_origin), direction(i_direction) {}

            inline const Vec3<T> pointAtDistance(const T distance) const {
                return Vec3<T>(origin.x + direction.x * distance,
                               origin.y + direction.y * distance,
                               origin.z + direction.z * distance);
            }

            inline PointStatus::Type pointStatus(const Vec3<T>& point) const {
                const T dot = direction.dot(point - origin);
                if (dot >  Math<T>::PointStatusEpsilon)
                    return PointStatus::PSAbove;
                if (dot < -Math<T>::PointStatusEpsilon)
                    return PointStatus::PSBelow;
                return PointStatus::PSInside;
            }

            inline T intersectWithPlane(const Vec3<T>& normal, const Vec3<T>& anchor) const {
                const T d = direction.dot(normal);
                if (Math<T>::zero(d))
                    return Math<T>::nan();
                
                const T s = ((anchor - origin).dot(normal)) / d;
                if (Math<T>::neg(s))
                    return Math<T>::nan();
                return s;
            }

            T intersectWithSphere(const Vec3<T>& position, const T radius) const {
                const Vec3<T> diff = origin - position;
                
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
            
            T intersectWithSphere(const Vec3<T>& position, const T radius, const T scalingFactor, const T maxDistance) const {
                const T distanceToCenter = (position - origin).length();
                if (distanceToCenter > maxDistance)
                    return Math<T>::nan();
                    
                const T scaledRadius = radius * scalingFactor * distanceToCenter;
                return intersectWithSphere(position, scaledRadius);
            }
            
            T intersectWithCube(const Vec3<T>& position, const T size) const {
                const T halfSize = size / 2.0;
                
                if (direction.x < 0.0) {
                    const T distance = intersectWithPlane(Vec3<T>::PosX, Vec3<T>(position.x + halfSize, position.y, position.z));
                    if (!Math<T>::isnan(distance)) {
                        const Vec3<T> point = pointAtDistance(distance);
                        if (point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.x > 0.0f) {
                    const T distance = intersectWithPlane(Vec3<T>::NegX, Vec3<T>(position.x - halfSize, position.y, position.z));
                    if (!Math<T>::isnan(distance)) {
                        const Vec3<T> point = pointAtDistance(distance);
                        if (point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                }
                
                if (direction.y < 0.0f) {
                    const T distance = intersectWithPlane(Vec3<T>::PosY, Vec3<T>(position.x, position.y + halfSize, position.z));
                    if (!Math<T>::isnan(distance)) {
                        const Vec3<T> point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.y > 0.0f) {
                    const T distance = intersectWithPlane(Vec3<T>::NegY, Vec3<T>(position.x, position.y - halfSize, position.z));
                    if (!Math<T>::isnan(distance)) {
                        Vec3<T> point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.z >= position.z - halfSize &&
                            point.z <= position.z + halfSize) {
                            return distance;
                        }
                    }
                }
                
                if (direction.z < 0.0f) {
                    const T distance = intersectWithPlane(Vec3<T>::PosZ, Vec3<T>(position.x, position.y, position.z + halfSize));
                    if (!Math<T>::isnan(distance)) {
                        const Vec3<T> point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize) {
                            return distance;
                        }
                    }
                } else if (direction.z > 0.0f) {
                    const T distance = intersectWithPlane(Vec3<T>::NegZ, Vec3<T>(position.x, position.y, position.z - halfSize));
                    if (!Math<T>::isnan(distance)) {
                        const Vec3<T> point = pointAtDistance(distance);
                        if (point.x >= position.x - halfSize &&
                            point.x <= position.x + halfSize &&
                            point.y >= position.y - halfSize &&
                            point.y <= position.y + halfSize) {
                            return distance;
                        }
                    }
                }
                
                return Math<T>::nan();
            }
            
            T squaredDistanceToPoint(const Vec3<T>& point, T& distanceToClosestPoint) const {
                distanceToClosestPoint = (point - origin).dot(direction);
                if (distanceToClosestPoint <= 0.0)
                    return Math<T>::nan();
                return (pointAtDistance(distanceToClosestPoint) - point).lengthSquared();
            }
            
            T distanceToPoint(const Vec3<T>& point, T& distanceToClosestPoint) const {
                const T squaredDistance = squaredDistanceToPoint(point, distanceToClosestPoint);
                if (Math<T>::isnan(squaredDistance))
                    return squaredDistance;
                return std::sqrt(squaredDistance);
            }
            
            T squaredDistanceToSegment(const Vec3<T>& start, const Vec3<T>& end, Vec3<T>& pointOnSegment, T& distanceToClosestPoint) const {
                Vec3<T> u, v, w;
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
                const Vec3<T> dP = w - v;

                pointOnSegment = start + u;
                return dP.lengthSquared();
            }
            
            T distanceToSegment(const Vec3<T>& start, const Vec3<T>& end, Vec3<T>& pointOnSegment, T& distanceToClosestPoint) const {
                const T squaredDistance = squaredDistanceToSegment(start, end, pointOnSegment, distanceToClosestPoint);
                if (Math<T>::isnan(squaredDistance))
                    return squaredDistance;
                return std::sqrt(squaredDistance);
            }
            
            T squaredDistanceToLine(const Vec3<T>& lineAnchor, const Vec3<T>& lineDir, Vec3<T>& pointOnLine, T& distanceToClosestPoint) const {
                const Vec3<T> w0 = origin - lineAnchor;
                const T a = direction.dot(direction);
                const T b = direction.dot(lineDir);
                const T c = lineDir.dot(lineDir);
                const T d = direction.dot(w0);
                const T e = lineDir.dot(w0);
                
                const T f = a * c - b * b;
                if (Math<T>::zero(f))
                    return Math<T>::nan();
                
                T sc = (b * e - c * d) / f;
                const T tc = (a * e - b * d) / f;
                
                if (sc < 0.0)
                    sc = 0.0;
                
                const Vec3<T> pointOnRay = origin + sc * direction;
                pointOnLine = lineAnchor + tc * lineDir;
                distanceToClosestPoint = sc;
                return (pointOnLine - pointOnRay).lengthSquared();
            }
            
            T distanceToLine(const Vec3<T>& lineAnchor, const Vec3<T>& lineDir, Vec3<T>& pointOnLine, T& distanceToClosestPoint) const {
                const T squaredDistance = squaredDistanceToLine(lineAnchor, lineDir, pointOnLine, distanceToClosestPoint);
                if (Math<T>::isnan(squaredDistance))
                    return squaredDistance;
                return std::sqrt(squaredDistance);
            }
        };
        
        typedef Ray<float> Rayf;
    }
}

#endif
