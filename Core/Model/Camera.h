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

#ifndef TrenchBroom_Camera_h
#define TrenchBroom_Camera_h

#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        
        class Camera {
        protected:
            Vec3f m_position;
            Vec3f m_direction;
            Vec3f m_up;
            Vec3f m_right;
            float m_fov;
            float m_near;
            float m_far;
        public:
            Camera(float fov, float near, float far, Vec3f position, Vec3f direction);
            const Vec3f& position() const;
            const Vec3f& direction() const;
            const Vec3f& up() const;
            const Vec3f& right() const;
            float fov() const;
            float near() const;
            float far() const;
            
            void moveTo(Vec3f position);
            void moveBy(float forward, float right, float up);
            void lookAt(Vec3f point, Vec3f up);
            void setDirection(Vec3f direction, Vec3f up);
            
            void rotate(float yawAngle, float pitchAngle);
            void orbit(Vec3f center, float hAngle, float vAngle);
            
            void setFov(float fov);
            void setNear(float near);
            void setFar(float far);
        };
    }
}

#endif
