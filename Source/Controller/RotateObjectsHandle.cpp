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

#include "RotateObjectsHandle.h"

namespace TrenchBroom {
    namespace Model {
        RotateObjectsHandleHit::RotateObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::RotateObjectsHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}
        
        bool RotateObjectsHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        Model::RotateObjectsHandleHit* RotateObjectsHandle::pickAxis(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateObjectsHandleHit::HitArea hitArea) {
            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.lengthSquared();
                if (missDistance >= m_handleRadius * m_handleRadius &&
                    missDistance <= (m_handleRadius + m_handleThickness) * (m_handleRadius + m_handleThickness) &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::RotateObjectsHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }

        RotateObjectsHandle::RotateObjectsHandle(float handleRadius, float handleThickness) :
        m_handleRadius(handleRadius),
        m_handleThickness(handleThickness) {}
        
        Model::RotateObjectsHandleHit* RotateObjectsHandle::pick(const Ray& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Model::RotateObjectsHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickAxis(ray, xAxis, yAxis, zAxis, Model::RotateObjectsHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, yAxis, xAxis, zAxis, Model::RotateObjectsHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, zAxis, xAxis, yAxis, Model::RotateObjectsHandleHit::HAZAxis));

            if (!locked()) {
                if (closestHit != NULL) {
                    if (!m_hit || m_hitArea != closestHit->hitArea()) {
                        m_hit = true;
                        m_hitArea = closestHit->hitArea();
                        setUpdated();
                    }
                } else if (m_hit) {
                    m_hit = false;
                    setUpdated();
                }
            }

            return closestHit;
        }
    }
}