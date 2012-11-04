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

#ifndef __TrenchBroom__ClipHandle__
#define __TrenchBroom__ClipHandle__

#include "Model/Picker.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static Type ClipHandleHit    = 1 << 5;
        }
        
        class ClipHandleHit : public Hit {
        private:
            unsigned int m_pointIndex;
        public:
            ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int pointIndex);
            bool pickable(Filter& filter) const;
            
            inline unsigned int pointIndex() const {
                return m_pointIndex;
            }
        };
    }
    
    namespace Controller {
        class ClipHandle {
        private:
            float m_handleRadius;
            Vec3f m_points[3];
            unsigned int m_numPoints;
        public:
            ClipHandle(float handleRadius);
            
            Model::ClipHandleHit* pick(const Ray& ray);
            
            inline const Vec3f& point(unsigned int index) const {
                assert(index < m_numPoints);
                return m_points[index];
            }
            
            inline unsigned int numPoints() const {
                return m_numPoints;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__ClipHandle__) */
