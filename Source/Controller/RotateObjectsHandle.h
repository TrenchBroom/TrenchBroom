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

#ifndef __TrenchBroom__RotateObjectsHandle__
#define __TrenchBroom__RotateObjectsHandle__

#include "Controller/ObjectsHandle.h"
#include "Model/Picker.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type RotateObjectsHandleHit    = 1 << 4;
        }
        
        class RotateObjectsHandleHit : public Hit {
        public:
            typedef enum {
                HAXAxis,
                HAYAxis,
                HAZAxis
            } HitArea;
        private:
            HitArea m_hitArea;
        public:
            RotateObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea);
            bool pickable(Filter& filter) const;
            
            inline HitArea hitArea() const {
                return m_hitArea;
            }
        };
    }

    namespace Controller {
        class RotateObjectsHandle : public ObjectsHandle<Model::RotateObjectsHandleHit> {
        protected:
            float m_handleRadius;
            float m_handleThickness;
            bool m_hit;
            Model::RotateObjectsHandleHit::HitArea m_hitArea;
            float m_angle;
            
            Model::RotateObjectsHandleHit* pickAxis(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateObjectsHandleHit::HitArea hitArea);
        public:
            RotateObjectsHandle(float handleRadius, float handleThickness);

            Model::RotateObjectsHandleHit* pick(const Ray& ray);
            
            inline float handleRadius() const {
                return m_handleRadius;
            }
            
            inline float handleThickness() const {
                return m_handleThickness;
            }

            inline bool hit() const {
                return m_hit;
            }
            
            inline Model::RotateObjectsHandleHit::HitArea hitArea() const {
                return m_hitArea;
            }
            
            inline float angle() const {
                return m_angle;
            }
            
            inline void setAngle(float angle) {
                m_angle = angle;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsHandle__) */
