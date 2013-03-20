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

#ifndef TrenchBroom_FindIntegerPlanePoints_h
#define TrenchBroom_FindIntegerPlanePoints_h

#include "Utility/CoordinatePlane.h"
#include "Utility/VecMath.h"

#include <limits>

namespace TrenchBroom {
    namespace Math {
        namespace FindIntegerPlanePoints {
            namespace CursorPoint {
                typedef unsigned int Type;
                static const Type TopLeft       = 0;
                static const Type Top           = 1;
                static const Type TopRight      = 2;
                static const Type Left          = 3;
                static const Type Center        = 4;
                static const Type Right         = 5;
                static const Type BottomLeft    = 6;
                static const Type Bottom        = 7;
                static const Type BottomRight   = 8;
            };
            
            static const Vec3f offsets[] = {
                Vec3f(-1.0f,  1.0f,  0.0f), Vec3f( 0.0f,  1.0f,  0.0f), Vec3f( 1.0f,  1.0f,  0.0f),
                Vec3f(-1.0f,  0.0f,  0.0f), Vec3f( 0.0f,  0.0f,  0.0f), Vec3f( 1.0f,  0.0f,  0.0f),
                Vec3f(-1.0f, -1.0f,  0.0f), Vec3f( 0.0f, -1.0f,  0.0f), Vec3f( 1.0f, -1.0f,  0.0f)
            };
            
            static const float distances[] = {
                0.7f, 1.0f, 0.7f,
                1.0f, 0.0f, 1.0f,
                0.7f, 1.0f, 0.7f
            };
            
            inline float planeFrequency(const Plane& plane) {
                const float c = 1.0f - std::sin(Math::Pi / 4.0f);
                
                const Vec3f& axis = plane.normal.firstAxis();
                const float d = plane.normal.dot(axis);
                assert(d != 0.0f);
                
                return (1.0f - d) / c;
            }
            
            inline void setDefaultPlanePoints(const Plane& plane, Vec3f points[3]) {
                points[0] = plane.anchor().rounded();
                const Axis::Type axis = plane.normal.firstComponent();
                switch (axis) {
                    case Axis::AX:
                        if (plane.normal.x > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosZ;
                            points[2] = points[0] + 64.0f * Vec3f::PosY;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosY;
                            points[2] = points[0] + 64.0f * Vec3f::PosZ;
                        }
                        break;
                    case Axis::AY:
                        if (plane.normal.y > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosX;
                            points[2] = points[0] + 64.0f * Vec3f::PosZ;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosZ;
                            points[2] = points[0] + 64.0f * Vec3f::PosX;
                        }
                        break;
                    default:
                        if  (plane.normal.z > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosY;
                            points[2] = points[0] + 64.0f * Vec3f::PosX;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosX;
                            points[2] = points[0] + 64.0f * Vec3f::PosY;
                        }
                        break;
                }
            }
            
            inline float error(const Plane& plane, const Vec3f& point) {
                const float z = plane.z(point.x, point.y);
                return std::abs(z - Math::round(z));
            }
            
            inline void findMinimalErrorPoint(const Plane& plane, const Vec3f& point, CursorPoint::Type& bestPoint, float& smallestError) {
                bestPoint = CursorPoint::TopLeft;
                smallestError = error(plane, point + offsets[0]);
                for (CursorPoint::Type i = 1; i < 9; i++) {
                    const float currentError = error(plane, point + offsets[i]);
                    if (currentError < smallestError) {
                        bestPoint = i;
                        smallestError = currentError;
                    }
                }
            }
            
            inline Vec3f findMinimalErrorPoint(const Plane& plane, const Vec3f& point, const unsigned int stepSize) {
                Vec3f newPoint = point;
                CursorPoint::Type bestPoint;
                float smallestError = std::numeric_limits<float>::max();
                unsigned int currentStepSize = stepSize;
                
                while (true) {
                    float currentError;
                    findMinimalErrorPoint(plane, newPoint, bestPoint, currentError);
                    if (bestPoint == CursorPoint::Center || currentError >= smallestError || currentError == 0.0f) {
                        newPoint.z = Math::round(plane.z(newPoint.x, newPoint.y));
                        return newPoint;
                    }
                    
                    newPoint += offsets[bestPoint] * std::ceil(stepSize * distances[bestPoint]);
                    smallestError = currentError;
                    if (currentStepSize > 1)
                        currentStepSize /= 2;
                }
            }
            
            inline void findPoints(const Plane& plane, Vec3f points[3]) {
                const float frequency = planeFrequency(plane);
                if (Math::zero(frequency)) {
                    setDefaultPlanePoints(plane, points);
                } else {
                    const CoordinatePlane& coordPlane = CoordinatePlane::plane(plane.normal);
                    const Plane swizzledPlane(coordPlane.swizzleTo(plane.normal), plane.distance);
                    const Vec3f waveDirection = coordPlane.swizzleTo(coordPlane.project(plane.normal).normalized());
                    const Vec3f right = waveDirection.crossed(swizzledPlane.normal.firstAxis());
                    const float waveLength = 1.0f / frequency;
                    const unsigned int stepSize = static_cast<unsigned int>(std::ceil(waveLength / 2.0f));
                    const float pointDistance = std::max(64.0f, waveLength);
                    
                    points[0] = findMinimalErrorPoint(swizzledPlane, swizzledPlane.anchor().rounded(), stepSize);
                    points[1] = findMinimalErrorPoint(swizzledPlane, points[0] + pointDistance * waveDirection, 1);
                    points[2] = findMinimalErrorPoint(swizzledPlane, points[0] + pointDistance * right, 1);
                    
                    for (unsigned int i = 0; i < 3; i++)
                        points[i] = coordPlane.swizzleFrom(points[i]);
                }
            }
        }
    }
}

#endif
