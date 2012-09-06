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

#ifndef TrenchBroom_DragPlane_h
#define TrenchBroom_DragPlane_h

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class DragPlane {
        protected:
            Vec3f m_normal;
            DragPlane(const Vec3f& normal) : m_normal(normal) {}
        public:
            static DragPlane horizontal() {
                return DragPlane(Vec3f::PosZ);
            }
            
            static DragPlane vertical(const Vec3f& vector) {
                if (vector.firstComponent() != Axis::Z)
                    return DragPlane(vector.firstAxis());
                return DragPlane(vector.secondAxis());
            }
            
            static DragPlane orthogonal(const Vec3f& vector, bool aligned) {
                if (aligned)
                    return DragPlane(vector.firstAxis());
                return DragPlane(vector);
            }
            
            static DragPlane parallel(const Vec3f& vector, const Vec3f& normal) {
                Vec3f temp = normal.crossed(vector);
                temp = vector.crossed(temp);
                temp.normalize();
                return DragPlane(temp);
            }
            
            inline float intersect(const Ray& ray, const Vec3f& planePosition) const {
                Plane plane(m_normal, planePosition);
                return plane.intersectWithRay(ray);
            }
            
            inline const Vec3f& normal() const {
                return m_normal;
            }
        };
    }
}

#endif
