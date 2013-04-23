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
#include "Utility/Vec3.h"

#include <algorithm>

namespace TrenchBroom {
    namespace VecMath {
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
            static const CoordinatePlane& plane(Axis::Type axis) {
                switch (axis) {
                    case Axis::AX:
                        return plane(YZ);
                    case Axis::AY:
                        return plane(XZ);
                    default:
                        return plane(XY);
                }
            }

            static const CoordinatePlane& plane(const Vec3f& normal) {
                return plane(normal.firstComponent());
            }
            
            inline Vec3f project(const Vec3f& point) const {
                switch (m_plane) {
                    case XY:
                        return Vec3f(point.x, point.y, 0.0f);
                    case YZ:
                        return Vec3f(0.0f, point.y, point.z);
                    default:
                        return Vec3f(point.x, 0.0f, point.z);
                }
            }
            
            inline Vec3f swizzle(const Vec3f& point) const {
                switch (m_plane) {
                    case XY:
                        return point;
                    case YZ:
                        return Vec3f(point.y, point.z, point.x);
                    default:
                        return Vec3f(point.z, point.x, point.y);
                }
            }

            
            inline Vec3f unswizzle(const Vec3f& point) const {
                switch (m_plane) {
                    case XY:
                        return point;
                    case YZ:
                        return Vec3f(point.z, point.x, point.y);
                    default:
                        return Vec3f(point.y, point.z, point.x);
                }
            }
            
            template <typename Iterator>
            inline void swizzle(Iterator first, Iterator last) const {
                switch (m_plane) {
                    case YZ:
                        std::reverse(first, last);
                        break;
                    default:
                        break;
                }
            }
            
            template <typename Iterator>
            inline void unswizzle(Iterator first, Iterator last) const {
                swizzle(first, last);
            }
        };
    }
}

#endif
