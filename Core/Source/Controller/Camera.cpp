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
    namespace Controller {
        Camera::Camera(float fieldOfVision, float nearPlane, float farPlane, Vec3f position, Vec3f direction) : m_fieldOfVision(fieldOfVision), m_nearPlane(nearPlane), m_farPlane(farPlane), m_position(position), m_direction(direction) {
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
        
        float Camera::fieldOfVision() const {
            return m_fieldOfVision;
        }
        
        float Camera::nearPlane() const {
            return m_nearPlane;
        }
        
        float Camera::farPlane() const {
            return m_farPlane;
        }
        
        const Vec3f Camera::defaultPoint() {
            return m_position + m_direction * 256;
        }
        
        const Vec3f Camera::unproject(float x, float y, float depth) const {
            GLdouble rx, ry, rz;
            gluUnProject(x, y, depth, m_modelview, m_projection, m_viewport, &rx, &ry, &rz);
            
            return Vec3f(rx, ry, rz);
        }

        const Ray Camera::pickRay(float x, float y) const {
            Vec3f direction = (unproject(x, y, 0.5f) - m_position).normalize();
            return Ray(m_position, direction);
        }

        void Camera::update(float x, float y, float width, float height) {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            
            float vfrustum = tan(m_fieldOfVision * M_PI / 360) * 0.75 * m_nearPlane;
            float hfrustum = vfrustum * width / height;
            glFrustum(-hfrustum, hfrustum, -vfrustum, vfrustum, m_nearPlane, m_farPlane);
            
            const Vec3f& pos = m_position;
            const Vec3f& at = m_position + m_direction;
            const Vec3f& up = m_up;
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glViewport(x, y, width, height);
            gluLookAt(pos.x, pos.y, pos.z, at.x, at.y, at.z, up.x, up.y, up.z);

            glGetIntegerv(GL_VIEWPORT, m_viewport);
            glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview);
            glGetDoublev(GL_PROJECTION_MATRIX, m_projection);
        }
        
        void Camera::setBillboard() {
            Vec3f bbLook = m_direction * -1;
            Vec3f bbUp = m_up;
            Vec3f bbRight = bbUp % bbLook;
            
            float matrix[] = {bbRight.x, bbRight.y, bbRight.z, 0, bbUp.x, bbUp.y, bbUp.z, 0, bbLook.x, bbLook.y, bbLook.z, 0, 0, 0, 0, 1};
            glMultMatrixf(matrix);
        }

        float Camera::distanceTo(const Vec3f& point) {
            return sqrt(squaredDistanceTo(point));
        }
        
        float Camera::squaredDistanceTo(const Vec3f& point) {
            return (point - m_position).lengthSquared();
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
                if (!Math::fzero(angle)) {
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
        
        void Camera::setFieldOfVision(float fieldOfVision) {
            m_fieldOfVision = fieldOfVision;
        }
        
        void Camera::setNearPlane(float nearPlane) {
            m_nearPlane = nearPlane;
        }
        
        void Camera::setFarPlane(float farPlane) {
            m_farPlane = farPlane;
        }
    }
}
