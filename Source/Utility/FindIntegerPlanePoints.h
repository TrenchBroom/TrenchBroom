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

#include "Utility/VecMath.h"

namespace TrenchBroom {
    namespace Math {
        namespace FindIntegerPlanePoints {
            inline float planeFrequency(const Plane& plane) {
                const Vec3f& axis = plane.normal.firstAxis();
                const float dot = plane.normal.dot(axis);
                assert(dot != 0.0f);
                return 1.0f / dot;
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
            
            inline void findPoints(const Plane& plane, Vec3f points[3]) {
                const float frequency = planeFrequency(plane);
                if (Math::eq(frequency, 1.0f)) {
                    setDefaultPlanePoints(plane, points);
                } else {
                    
                }
            }
        }
    }
}

#endif
