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
        public:
            enum class Which {
                XY,
                XZ,
                YZ
            };
        private:
            const Which m_plane;
            CoordinatePlane(Which plane) : m_plane(plane) {}
        public:
            static const CoordinatePlane& plane(Which plane) {
                static CoordinatePlane xy(Which::XY);
                static CoordinatePlane xz(Which::XZ);
                static CoordinatePlane yz(Which::YZ);
                switch (plane) {
                    case Which::XY:
                        return xy;
                    case Which::XZ:
                        return xz;
                    default:
                        return yz;
                }
            }
            
            static const CoordinatePlane& plane(const Vec3f& normal) {
                switch (normal.firstComponent()) {
                    case Axis::X:
                        return plane(Which::YZ);
                    case Axis::Y:
                        return plane(Which::XZ);
                    default:
                        return plane(Which::XY);
                }
            }
            
            inline void project(const Vec3f& point, Vec3f& result) const {
                switch (m_plane) {
                    case Which::XY:
                        result.x = point.x;
                        result.y = point.y;
                        result.z = point.z;
                        break;
                    case Which::YZ:
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
