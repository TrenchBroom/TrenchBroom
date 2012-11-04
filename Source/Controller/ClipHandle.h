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
            static const Type ClipHandleHit    = 1 << 5;
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
            bool m_hasCurrentHit;
            Vec3f m_currentPoint;
            bool m_updated;
        public:
            ClipHandle(float handleRadius);
            
            Model::ClipHandleHit* pick(const Ray& ray);
            
            inline float handleRadius() const {
                return m_handleRadius;
            }
            
            inline const Vec3f& point(unsigned int index) const {
                assert(index < m_numPoints);
                return m_points[index];
            }
            
            inline unsigned int numPoints() const {
                return m_numPoints;
            }

            inline void addPoint(const Vec3f& point) {
                assert(m_numPoints < 3);
                m_points[m_numPoints++] = point;
                m_updated = true;
            }
            
            inline void deleteLastPoint() {
                assert(m_numPoints > 0);
                m_numPoints--;
                m_updated = true;
            }
            
            inline void setPoint(unsigned int index, const Vec3f& point) {
                assert(index < m_numPoints);
                m_points[index] = point;
                m_updated = true;
            }
            
            inline bool hasCurrentHit() const {
                return m_hasCurrentHit;
            }
            
            inline const Vec3f& currentPoint() const {
                return m_currentPoint;
            }

            inline void setCurrentHit(bool hasHit, const Vec3f& currentPoint = Vec3f::Null) {
                if (m_hasCurrentHit == hasHit && m_currentPoint.equals(currentPoint))
                    return;
                
                m_hasCurrentHit = hasHit;
                m_currentPoint = currentPoint;
                m_updated = true;
            }
            
            inline bool updated() {
                bool updated = m_updated;
                m_updated = false;
                return updated;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__ClipHandle__) */
