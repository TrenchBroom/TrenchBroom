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
#include "Utility/Vec.h"

#include <algorithm>

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
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

            static const CoordinatePlane& plane(const Vec<T,3>& normal) {
                return plane(normal.firstComponent());
            }
            
            inline Vec<T,3> project(const Vec<T,3>& point) const {
                switch (m_plane) {
                    case XY:
                        return Vec<T,3>(point.x(), point.y(), static_cast<T>(0.0));
                    case YZ:
                        return Vec<T,3>(static_cast<T>(0.0), point.y(), point.z());
                    default:
                        return Vec<T,3>(point.x(), static_cast<T>(0.0), point.z());
                }
            }
            
            inline Vec<T,3> swizzle(const Vec<T,3>& point) const {
                switch (m_plane) {
                    case XY:
                        return point;
                    case YZ:
                        return Vec<T,3>(point.y(), point.z(), point.x());
                    default:
                        return Vec<T,3>(point.z(), point.x(), point.y());
                }
            }

            
            inline Vec<T,3> unswizzle(const Vec<T,3>& point) const {
                switch (m_plane) {
                    case XY:
                        return point;
                    case YZ:
                        return Vec<T,3>(point.z(), point.x(), point.y());
                    default:
                        return Vec<T,3>(point.y(), point.z(), point.x());
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
        
        typedef CoordinatePlane<float> CoordinatePlanef;
    }
}

#endif
