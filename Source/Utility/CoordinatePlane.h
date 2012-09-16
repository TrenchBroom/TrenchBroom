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

#ifndef TrenchBroom_CoordinatePlane_h
#define TrenchBroom_CoordinatePlane_h

#include "Utility/Math.h"
#include "Utility/Vec3f.h"

namespace TrenchBroom {
    namespace Math {
        class CoordinatePlane {
        private:
            enum Which {
                XY,
                XZ,
                YZ
            };

            const Which m_plane;
            CoordinatePlane(Which plane) : m_plane(plane) {}
            static const CoordinatePlane& plane(Which plane) {
                static CoordinatePlane xy(XY);
                static CoordinatePlane xz(XZ);
                static CoordinatePlane yz(YZ);
                switch (plane) {
                    case XY:
                        return xy;
                    case XZ:
                        return xz;
                    default:
                        return yz;
                }
            }

        public:
            static const CoordinatePlane& plane(const Vec3f& normal) {
                switch (normal.firstComponent()) {
                    case Axis::AX:
                        return plane(YZ);
                    case Axis::AY:
                        return plane(XZ);
                    default:
                        return plane(XY);
                }
            }
            
            inline void project(const Vec3f& point, Vec3f& result) const {
                switch (m_plane) {
                    case XY:
                        result.x = point.x;
                        result.y = point.y;
                        result.z = point.z;
                        break;
                    case YZ:
                        result.x = point.y;
                        result.y = point.z;
                        result.z = point.x;
                        break;
                    default:
                        result.x = point.x;
                        result.y = point.z;
                        result.z = point.y;
                        break;
                }
            }
        };
    }
}

#endif
