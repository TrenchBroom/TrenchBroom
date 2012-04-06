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

#include "Camera.h"
#include <cmath>


namespace TrenchBroom {
    namespace Model {
        Camera::Camera(float fov, float near, float far, Vec3f position, Vec3f direction) : m_fov(fov), m_near(near), m_far(far), m_position(position), m_direction(direction) {
            if (m_direction.equals(ZAxisPos)) {
                m_right = YAxisNeg;
                m_up = XAxisNeg;
            } else if (m_direction.equals(ZAxisNeg)) {
                m_right = YAxisNeg;
                m_up = XAxisPos;
            } else {
                m_right = m_direction % ZAxisPos;
                m_up = m_right % m_direction;
            }
        }
        
        const Vec3f& Camera::position() const {
            return m_position;
        }
        
        const Vec3f& Camera::direction() const {
            return m_direction;
        }
        
        const Vec3f& Camera::up() const {
            return m_up;
        }
        
        const Vec3f& Camera::right() const {
            return m_right;
        }
        
        float Camera::fov() const {
            return m_fov;
        }
        
        float Camera::near() const {
            return m_near;
        }
        
        float Camera::far() const {
            return m_far;
        }
        
        void Camera::moveTo(Vec3f position) {
            m_position = position;
        }
        
        void Camera::moveBy(float forward, float right, float up) {
            m_position += m_direction * forward;
            m_position += m_right * right;
            m_position += m_up * up;
        }
        
        void Camera::lookAt(Vec3f point, Vec3f up) {
            setDirection((point - m_position).normalize(), up);
        }
        
        void Camera::setDirection(Vec3f direction, Vec3f up) {
            m_direction = direction;
            m_right = (m_direction % up).normalize();
            m_up = m_right % m_direction;
        }
        
        void Camera::rotate(float yawAngle, float pitchAngle) {
            if (yawAngle == 0 && pitchAngle == 0) return;
            
            Quat rotation = Quat(yawAngle, ZAxisPos) * Quat(pitchAngle, m_right);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;

            if (newUp.z < 0) {
                newUp.z = 0;
                newDirection.x = 0;
                newDirection.y = 0;
            }
            
            setDirection(newDirection, newUp);
        }
        
        void Camera::orbit(Vec3f center, float hAngle, float vAngle) {
            if (hAngle == 0 && vAngle == 0) return;
            
            Quat rotation = Quat(hAngle, ZAxisPos) * Quat(vAngle, m_right);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;
            Vec3f offset = m_position - center;
            
            if (newUp.z < 0) {
                newUp = m_up;
                newDirection.x = 0;
                newDirection.y = 0;
                newDirection = newDirection.normalize();
                
                // correct rounding errors
                float cos = fmaxf(-1, fminf(1, m_direction | newDirection));
                float angle = acosf(cos);
                if (fabsf(angle) <= AlmostZero) {
                    Vec3f axis = (m_direction % newDirection).normalize();
                    rotation = Quat(angle, axis);
                    offset = rotation * offset;
                    newUp = rotation * newUp;
                }
            } else {
                offset = rotation * offset;
            }
            
            setDirection(newDirection, newUp);
            moveTo(offset + center);
        }
        
        void Camera::setFov(float fov) {
            m_fov = fov;
        }
        
        void Camera::setNear(float near) {
            m_near = near;
        }
        
        void Camera::setFar(float far) {
            m_far = far;
        }
    }
}
