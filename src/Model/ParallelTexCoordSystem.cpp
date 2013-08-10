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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ParallelTexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis, const Vec3& normal, const float rotation) :
        m_xAxis(xAxis),
        m_yAxis(yAxis) {
            const FloatType angle = static_cast<FloatType>(Math<float>::radians(rotation));
            const Quat3 rot(normal, -angle);
            m_initialXAxis = rot * m_xAxis;
            m_initialYAxis = rot * m_yAxis;
        }
        
        void ParallelTexCoordSystem::update(const Vec3& normal, const float rotation) {
            const FloatType angle = static_cast<FloatType>(Math<float>::radians(rotation));
            const Quat3 rot(normal, angle);
            m_xAxis = rot * m_initialXAxis;
            m_yAxis = rot * m_initialYAxis;
        }
        
        const Vec3& ParallelTexCoordSystem::xAxis() const {
            return m_xAxis;
        }
        
        const Vec3& ParallelTexCoordSystem::yAxis() const {
            return m_yAxis;
        }
    }
}
