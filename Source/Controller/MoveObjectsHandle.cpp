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

#include "MoveObjectsHandle.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        MoveObjectsHandleHit::MoveObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea) :
        Hit(HitType::MoveObjectsHandleHit, hitPoint, distance),
        m_hitArea(hitArea) {}
        
        bool MoveObjectsHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        Model::MoveObjectsHandleHit* MoveObjectsHandle::pickAxis(const Ray& ray, Vec3f& axis, Model::MoveObjectsHandleHit::HitArea hitArea) {
            float distance;
            float missDistance = ray.squaredDistanceToSegment(position() - m_axisLength * axis, position() + m_axisLength * axis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return NULL;
            
            return new Model::MoveObjectsHandleHit(ray.pointAtDistance(distance), distance, hitArea);
        }
        
        Model::MoveObjectsHandleHit* MoveObjectsHandle::pickPlane(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::MoveObjectsHandleHit::HitArea hitArea) {
            Plane plane(normal, position());
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                Vec3f hitVector = hitPoint - position();
                float missDistance = hitVector.lengthSquared();
                if (missDistance <= m_planeRadius * m_planeRadius &&
                    hitVector.dot(axis1) >= 0.0f && hitVector.dot(axis2) >= 0.0f)
                    return new Model::MoveObjectsHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }
        

        MoveObjectsHandle::MoveObjectsHandle(float axisLength, float planeRadius) :
        ObjectsHandle(),
        m_axisLength(axisLength),
        m_planeRadius(planeRadius) {}
        
        Model::MoveObjectsHandleHit* MoveObjectsHandle::pick(const Ray& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Model::MoveObjectsHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickAxis(ray, xAxis, Model::MoveObjectsHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, yAxis, Model::MoveObjectsHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, zAxis, Model::MoveObjectsHandleHit::HAZAxis));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosX, yAxis, zAxis, Model::MoveObjectsHandleHit::HAYZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosY, xAxis, zAxis, Model::MoveObjectsHandleHit::HAXZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosZ, xAxis, yAxis, Model::MoveObjectsHandleHit::HAXYPlane));

            if (!locked()) {
                m_hit = closestHit != NULL;
                if (m_hit)
                    m_hitArea = closestHit->hitArea();
            }
            
            return closestHit;
        }
    }
}