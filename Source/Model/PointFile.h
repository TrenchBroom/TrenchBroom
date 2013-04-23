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

#ifndef __TrenchBroom__PointFile__
#define __TrenchBroom__PointFile__

#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class PointFile {
        private:
            Vec3f::List m_points;
            size_t m_current;
            
            static String path(const String& mapFilePath);
            void load(const String& mapFilePath);
        public:
            PointFile(const String& mapFilePath);
            static bool exists(const String& mapFilePath);
            
            inline bool hasNextPoint() const {
                return m_current < m_points.size() - 1;
            }
            
            inline bool hasPreviousPoint() const {
                return m_current > 0;
            }
            
            inline const Vec3f::List points() const {
                return m_points;
            }
           
            inline const Vec3f& currentPoint() const {
                return m_points[m_current];
            }
            
            inline const Vec3f& nextPoint() {
                assert(hasNextPoint());
                return m_points[++m_current];
            }
            
            inline const Vec3f& previousPoint() {
                assert(hasPreviousPoint());
                return m_points[--m_current];
            }
            
            inline const Vec3f direction() const {
                if (m_points.size() <= 1)
                    return Vec3f::PosX;
                if (m_current >= m_points.size() - 1)
                    return (m_points[m_points.size() - 1] - m_points[m_points.size() - 2]).normalized();
                return (m_points[m_current + 1] - m_points[m_current]).normalized();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__PointFile__) */
