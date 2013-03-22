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

#include <algorithm>
#include <array>
#include <limits>

namespace TrenchBroom {
    namespace Math {
        namespace FindIntegerPlanePoints {
            namespace CursorPoint {
                typedef unsigned int Type;
                static const Type Center        = 0;
                static const Type TopLeft       = 1;
                static const Type Top           = 2;
                static const Type TopRight      = 3;
                static const Type Left          = 4;
                static const Type Right         = 5;
                static const Type BottomLeft    = 6;
                static const Type Bottom        = 7;
                static const Type BottomRight   = 8;
                
                static const Type ExtraPoints[] = { TopLeft, TopRight, BottomLeft, BottomRight };

                static const Vec2f MoveOffsets[] = {
                    Vec2f(-1.0f,  1.0f), Vec2f( 0.0f,  1.0f), Vec2f( 1.0f,  1.0f),
                    Vec2f(-1.0f,  0.0f), Vec2f( 0.0f,  0.0f), Vec2f( 1.0f,  0.0f),
                    Vec2f(-1.0f, -1.0f), Vec2f( 0.0f, -1.0f), Vec2f( 1.0f, -1.0f)
                };
            };

            class Cursor {
            private:
                const Plane& m_plane;
                const float m_frequency;
                
                Vec2f m_position;
                float m_errors[9];
                
                inline float error(const Vec2f& point) {
                    const float z = m_plane.z(point.x, point.y);
                    return std::abs(z - Math::round(z));
                }
                
                inline void updateError(const CursorPoint::Type point) {
                    m_errors[point] = error(m_position + CursorPoint::MoveOffsets[point]);
                }
                
                inline CursorPoint::Type move(const CursorPoint::Type direction) {
                    m_errors[CursorPoint::Center] = m_errors[direction];
                    m_position += CursorPoint::MoveOffsets[direction];
                    
                    // TODO optimize here by remembering the previous values? Probably not worth the trouble.
                    CursorPoint::Type bestPoint = CursorPoint::Center;

                    for (CursorPoint::Type i = 1; i < 9; i++) {
                        updateError(i);
                        if (m_errors[i] < m_errors[bestPoint])
                            bestPoint = i;
                    }
                    
                    return bestPoint;
                }

                const void findLocalMinimum() {
                    for (CursorPoint::Type i = 0; i < 9; i++)
                        updateError(i);
                    
                    // find the initial best point
                    CursorPoint::Type bestPoint = CursorPoint::Center;
                    for (CursorPoint::Type i = 0; i < 9; i++)
                        if (m_errors[i] < m_errors[bestPoint])
                            bestPoint = i;
                    
                    while (bestPoint != CursorPoint::Center)
                        bestPoint = move(bestPoint);
                }
            public:
                Cursor(const Plane& plane, const float frequency) :
                m_plane(plane),
                m_frequency(frequency) {}
                
                const Vec3f findMinimum(const Vec3f& initialPosition) {
                    m_position.x = Math::round(initialPosition.x);
                    m_position.y = Math::round(initialPosition.y);
                    
                    findLocalMinimum();
                    const Vec2f localMinimumPosition = m_position;
                    const float localMinimumError = m_errors[CursorPoint::Center];
                    
                    Vec2f globalMinimumPosition = localMinimumPosition;
                    float globalMinimumError = localMinimumError;
                    
                    if (globalMinimumError > 0.0f) {
                        // To escape local minima, let's search some adjacent quadrants
                        // The number of extra quadrants should depend on the frequency: The higher the frequency, the
                        // more quadrants should be searched.
                        unsigned int numQuadrants = static_cast<unsigned int>(std::ceil(m_frequency * m_frequency * 10.0f));
                        bool b = false;
                        for (unsigned int i = 1; i < numQuadrants; i++) {
                            for (CursorPoint::Type q = 1; q < 9; q++) {
                                m_position = localMinimumPosition + i * 3.0f * CursorPoint::MoveOffsets[q];
                                findLocalMinimum();
                                const float newError = m_errors[CursorPoint::Center];
                                if (newError < globalMinimumError) {
                                    globalMinimumPosition = m_position;
                                    globalMinimumError = newError;
                                }
                            }
                        }
                    }
                
                    return Vec3f(globalMinimumPosition.x,
                                 globalMinimumPosition.y,
                                 Math::round(m_plane.z(globalMinimumPosition.x,
                                                       globalMinimumPosition.y)));
                }
            };
        
        
        
            inline float planeFrequency(const Plane& plane) {
                static const float c = 1.0f - std::sin(Math::Pi / 4.0f);
                
                const Vec3f& axis = plane.normal.firstAxis();
                const float d = plane.normal.dot(axis);
                assert(d != 0.0f);
                
                return (1.0f - d) / c;
            }
            
            inline void setDefaultPlanePoints(const Plane& plane, std::array<Vec3f, 3>& points) {
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
            
            inline void findPoints(const Plane& plane, std::array<Vec3f, 3>& points) {
                const float frequency = planeFrequency(plane);
                if (Math::zero(frequency, 1.0f / 7084.0f)) {
                    setDefaultPlanePoints(plane, points);
                } else {
                    const CoordinatePlane& coordPlane = CoordinatePlane::plane(plane.normal);
                    const Plane swizzledPlane(coordPlane.swizzle(plane.normal), plane.distance);
                    const Vec3f waveDirection = coordPlane.swizzle(coordPlane.project(plane.normal).normalized());
                    const Vec3f right = waveDirection.crossed(swizzledPlane.normal.firstAxis());
                    const float waveLength = 1.0f / frequency;
                    // const unsigned int stepSize = static_cast<unsigned int>(std::max(1.0f, std::floor(waveLength / 2.0f)));
                    const float pointDistance = std::max(256.0f, waveLength);
                    
                    float multiplier = 2.0f;
                    Cursor cursor(swizzledPlane, frequency);
                    points[0] = cursor.findMinimum(swizzledPlane.anchor());
                    points[1] = cursor.findMinimum(points[0] + pointDistance * waveDirection);
                    
                    Vec3f v1, v2;
                    do {
                        points[2] = cursor.findMinimum(points[0] + pointDistance * right - multiplier * pointDistance * waveDirection);
                        v1 = points[2] - points[0];
                        v2 = points[1] - points[0];
                        v1.cross(v2);
                        multiplier *= 1.5f;
                    } while (v1.null());

                    if (v1.z > 0.0f != swizzledPlane.normal.z > 0.0f)
                        std::swap(points[0], points[2]);
                    
                    for (unsigned int i = 0; i < 3; i++)
                        points[i] = coordPlane.unswizzle(points[i]);
                    
                    
                    // coordPlane.unswizzle(points.begin(), points.end());
                }
            }
        }
    }
}

#endif
