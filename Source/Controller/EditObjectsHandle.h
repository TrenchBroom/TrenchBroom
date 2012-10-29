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

#ifndef __TrenchBroom__EditObjectsHandle__
#define __TrenchBroom__EditObjectsHandle__

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class EditObjectsHandle {
        public:
            class Hit {
            public:
                typedef enum {
                    TNone,
                    TXAxis,
                    TYAxis,
                    TZAxis,
                    TXYPlane,
                    TXZPlane,
                    TYZPlane,
                    TXRotation,
                    TYRotation,
                    TZRotation
                } Type;
            private:
                Ray m_ray;
                Type m_type;
                Vec3f m_hitPoint;
                float m_distance;
                
                Hit(const Ray& ray, Type type, const Vec3f& hitPoint, float distance) :
                m_ray(ray),
                m_type(type),
                m_hitPoint(hitPoint),
                m_distance(distance) {}
            public:
                static Hit noHit(const Ray& ray) {
                    return Hit(ray, TNone, Vec3f(), std::numeric_limits<float>::max());
                }
                
                static Hit hit(const Ray& ray, Type type, const Vec3f& hitPoint, float distance) {
                    assert(type != TNone);
                    return Hit(ray, type, hitPoint, distance);
                }
                
                inline const Ray& ray() const {
                    return m_ray;
                }
                
                inline Type type() const {
                    return m_type;
                }
                
                inline const Vec3f& hitPoint() const {
                    return m_hitPoint;
                }
                
                inline float distance() const {
                    return m_distance;
                }
            };
        protected:
            float m_axisLength;
            Vec3f m_position;

            void axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis);
            Hit pickAxis(const Ray& ray, Vec3f& axis, Hit::Type type);
            Hit pickPlaneOrRing(const Ray& ray, const Vec3f& normal, Hit::Type planeType, Hit::Type ringType);
        public:
            EditObjectsHandle(float axisLength);
            
            Hit pick(const Ray& ray);
            
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline void setPosition(const Vec3f& position) {
                m_position = position;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EditObjectsHandle__) */
