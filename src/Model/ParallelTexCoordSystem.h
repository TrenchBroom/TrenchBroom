/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ParallelTexCoordSystem__
#define __TrenchBroom__ParallelTexCoordSystem__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class ParallelTexCoordSystem {
        private:
            Vec3 m_initialXAxis;
            Vec3 m_initialYAxis;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis, const Vec3& normal, const float rotation);
            ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2);
            void update(const Vec3& normal, const float rotation);
            const Vec3& xAxis() const;
            const Vec3& yAxis() const;
        };
    }
}

#endif /* defined(__TrenchBroom__ParallelTexCoordSystem__) */
