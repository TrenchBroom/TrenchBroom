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

#ifndef __TrenchBroom__ObjectsHandle__
#define __TrenchBroom__ObjectsHandle__

#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Hit;
    }
    
    namespace Controller {
        template <class HitClass>
        class ObjectsHandle {
        private:
            Vec3f m_position;
            bool m_positionValid;
            bool m_locked;
            Vec3f m_xAxis, m_yAxis, m_zAxis;
            
        protected:
            inline HitClass* selectHit(HitClass* closestHit, HitClass* hit) {
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
        public:
            ObjectsHandle() :
            m_positionValid(false),
            m_locked(false) {}
            virtual ~ObjectsHandle() {}
            
            inline void axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis) {
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
            
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline void setPosition(const Vec3f& position) {
                m_positionValid = m_position.equals(position);
                m_position = position;
            }

            inline bool positionValid() {
                bool valid = m_positionValid;
                m_positionValid = true;
                return valid;
            }
            
            inline bool locked() const {
                return m_locked;
            }
            
            inline void lock() {
                m_locked = true;
            }
            
            inline void unlock() {
                m_locked = false;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__ObjectsHandle__) */
