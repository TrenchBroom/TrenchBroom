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

#include "EditObjectsHandle.h"

namespace TrenchBroom {
    namespace Controller {
        EditObjectsHandle::Hit EditObjectsHandle::pickAxis(const Ray& ray, Vec3f& axis, Hit::Type type) {
            float distance;
            float missDistance = ray.squaredDistanceToSegment(m_position - m_axisLength * axis, m_position + m_axisLength * axis, distance);
            if (isnan(missDistance) || missDistance > 5.0f)
                return Hit::noHit(ray);
            
            return Hit::hit(ray, type, ray.pointAtDistance(distance), distance);
        }

        EditObjectsHandle::Hit EditObjectsHandle::pickPlaneOrRing(const Ray& ray, const Vec3f& normal, Hit::Type planeType, Hit::Type ringType) {
            Plane plane(normal, m_position);
            float distance = plane.intersectWithRay(ray);
            if (!isnan(distance)) {
                float innerRadius = (m_axisLength / 2.0f - 5.0f) * (m_axisLength / 2.0f - 5.0f);
                float outerRadius = (m_axisLength / 2.0f + 5.0f) * (m_axisLength / 2.0f + 5.0f);
                Vec3f hitPoint = ray.pointAtDistance(distance);
                float missDistance = (hitPoint - m_position).lengthSquared();
                if (missDistance < innerRadius)
                    return Hit::hit(ray, planeType, hitPoint, distance);
                if (missDistance <= outerRadius)
                    return Hit::hit(ray, ringType, hitPoint, distance);
            }
            
            return Hit::noHit(ray);
        }

        void EditObjectsHandle::axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis) {
            Vec3f view = m_position - origin;
            view.normalize();
            
            if (eq(fabsf(view.z), 1.0f)) {
                xAxis = Vec3f::PosX;
                yAxis = Vec3f::PosY;
            } else {
                xAxis = view.x > 0.0f ? Vec3f::NegX : Vec3f::PosX;
                yAxis = view.y > 0.0f ? Vec3f::NegY : Vec3f::PosY;
            }
            
            if (view.z >= 0.0f)
                zAxis = Vec3f::NegZ;
            else
                zAxis = Vec3f::PosZ;
        }
        
        EditObjectsHandle::EditObjectsHandle(float axisLength) :
        m_axisLength(axisLength) {}
        
        EditObjectsHandle::Hit EditObjectsHandle::pick(const Ray& ray) {
            Vec3f xAxis, yAxis, zAxis;
            axes(ray.origin, xAxis, yAxis, zAxis);
            Hit closestHit = Hit::noHit(ray);
            Hit hit = Hit::noHit(ray);
            
            hit = pickAxis(ray, xAxis, Hit::TXAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            hit = pickAxis(ray, yAxis, Hit::TYAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            hit = pickAxis(ray, zAxis, Hit::TZAxis);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            hit = pickPlaneOrRing(ray, Vec3f::PosX, Hit::TYZPlane, Hit::TXRotation);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            hit = pickPlaneOrRing(ray, Vec3f::PosY, Hit::TXZPlane, Hit::TYRotation);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            hit = pickPlaneOrRing(ray, Vec3f::PosZ, Hit::TXYPlane, Hit::TZRotation);
            if (hit.distance() < closestHit.distance())
                closestHit = hit;
            return closestHit;
        }
    }
}