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
            float missDistance = ray.squaredDistanceToSegment(m_position - m_axisLength * axis, m_position + m_axisLength * axis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return NULL;
            
            return new Model::MoveObjectsHandleHit(ray.pointAtDistance(distance), distance, hitArea);
        }
        
        Model::MoveObjectsHandleHit* MoveObjectsHandle::pickPlane(const Ray& ray, const Vec3f& normal, Model::MoveObjectsHandleHit::HitArea hitArea) {
            Plane plane(normal, m_position);
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                Vec3f hitPoint = ray.pointAtDistance(distance);
                float missDistance = (hitPoint - m_position).lengthSquared();
                if (missDistance < m_planeRadius * m_planeRadius)
                    return new Model::MoveObjectsHandleHit(hitPoint, distance, hitArea);
            }
            
            return NULL;
        }
        
        Model::MoveObjectsHandleHit* MoveObjectsHandle::selectHit(Model::MoveObjectsHandleHit* closestHit, Model::MoveObjectsHandleHit* hit) {
            if (closestHit == NULL)
                return hit;
            if (hit != NULL) {
                if (hit->distance() < closestHit->distance()) {
                    delete closestHit;
                    return hit;
                }
                
                delete hit;
            }
            
            return closestHit;
        }

        MoveObjectsHandle::MoveObjectsHandle(float axisLength, float planeRadius) :
        m_axisLength(axisLength),
        m_planeRadius(planeRadius),
        m_locked(false) {}
        
        void MoveObjectsHandle::axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis) {
            if (!m_locked) {
                Vec3f view = m_position - origin;
                view.normalize();
                
                if (eq(fabsf(view.z), 1.0f)) {
                    m_xAxis = Vec3f::PosX;
                    m_yAxis = Vec3f::PosY;
                } else {
                    m_xAxis = view.x > 0.0f ? Vec3f::NegX : Vec3f::PosX;
                    m_yAxis = view.y > 0.0f ? Vec3f::NegY : Vec3f::PosY;
                }
                
                if (view.z >= 0.0f)
                    m_zAxis = Vec3f::NegZ;
                else
                    m_zAxis = Vec3f::PosZ;
            }
            
            xAxis = m_xAxis;
            yAxis = m_yAxis;
            zAxis = m_zAxis;
        }

        Model::MoveObjectsHandleHit* MoveObjectsHandle::pick(const Ray& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Model::MoveObjectsHandleHit* closestHit = NULL;
            
            closestHit = selectHit(closestHit, pickAxis(ray, xAxis, Model::MoveObjectsHandleHit::HAXAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, yAxis, Model::MoveObjectsHandleHit::HAYAxis));
            closestHit = selectHit(closestHit, pickAxis(ray, zAxis, Model::MoveObjectsHandleHit::HAZAxis));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosX, Model::MoveObjectsHandleHit::HAYZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosY, Model::MoveObjectsHandleHit::HAXZPlane));
            closestHit = selectHit(closestHit, pickPlane(ray, Vec3f::PosZ, Model::MoveObjectsHandleHit::HAXYPlane));

            return closestHit;
        }
    }
}