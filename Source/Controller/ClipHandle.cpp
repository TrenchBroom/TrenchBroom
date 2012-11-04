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

#include "ClipHandle.h"

namespace TrenchBroom {
    namespace Model {
        ClipHandleHit::ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int pointIndex) :
        Hit(HitType::ClipHandleHit, hitPoint, distance),
        m_pointIndex(pointIndex) {}
        
        bool ClipHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        ClipHandle::ClipHandle(float handleRadius) :
        m_handleRadius(handleRadius),
        m_numPoints(0) {
            assert(m_handleRadius > 0.0f);
        }

        Model::ClipHandleHit* ClipHandle::pick(const Ray& ray) {
            Model::ClipHandleHit* closestHit = NULL;
            for (unsigned int i = 0; i < m_numPoints; i++) {
                float distance = ray.intersectWithSphere(m_points[i], m_handleRadius);
                if (!Math::isnan(distance) && (closestHit == NULL || distance < closestHit->distance())) {
                    if (closestHit != NULL)
                        delete closestHit;
                    closestHit = new Model::ClipHandleHit(ray.pointAtDistance(distance), distance, i);
                }
            }
            
            return closestHit;
        }
    }
}