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

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        void Camera::validate() const {
            if (m_ortho)
                m_matrix.setOrtho(m_nearPlane, m_farPlane,
                                  static_cast<float>(m_viewport.x - m_viewport.width / 2),
                                  static_cast<float>(m_viewport.y + m_viewport.height / 2),
                                  static_cast<float>(m_viewport.x + m_viewport.width / 2),
                                  static_cast<float>(m_viewport.y - m_viewport.height / 2));
            else
                m_matrix.setPerspective(m_fieldOfVision, m_nearPlane, m_farPlane, m_viewport.width, m_viewport.height);
            
            Mat4f view;
            view.setView(m_direction, m_up);
            view.translate(-1.0f * m_position);
            m_matrix *= view;
            
            bool invertible;
            m_invertedMatrix = m_matrix.inverted(invertible);
            assert(invertible);
        }
        
        Camera::Camera(float fieldOfVision, float nearPlane, float farPlane, const Vec3f& position, const Vec3f& direction) :
        m_ortho(false),
        m_fieldOfVision(fieldOfVision),
        m_nearPlane(nearPlane),
        m_farPlane(farPlane),
        m_position(position),
        m_direction(direction),
        m_valid(false) {
            if (m_direction.equals(Vec3f::PosZ)) {
                m_right = Vec3f::NegY;
                m_up = Vec3f::NegX;
            } else if (m_direction.equals(Vec3f::NegZ)) {
                m_right = Vec3f::NegY;
                m_up = Vec3f::PosX;
            } else {
                m_right = m_direction.crossed(Vec3f::PosZ);
                m_right.normalize();
                m_up = m_right.crossed(m_direction);
                m_up.normalize();
            }
        }
        
        void Camera::update(int x, int y, int width, int height) {
            setViewport(x, y, width, height);
            
            if (!m_valid)
                validate();
            
            glViewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(m_matrix.v);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        const Vec3f Camera::defaultPoint() const {
            return defaultPoint(m_direction);
        }
        
        const Vec3f Camera::defaultPoint(const Vec3f& direction) const {
            return m_position + direction * 256.0f;
        }

        const Vec3f Camera::defaultPoint(float x, float y) const {
            const Vec3f point = unproject(x, y, 0.5f);
            return defaultPoint((point - m_position).normalized());
        }

        const Vec3f Camera::project(const Vec3f& point) const {
            if (!m_valid)
                validate();
            
            Vec3f win = m_matrix * point;
            win.x = m_viewport.x + m_viewport.width  * (win.x + 1.0f) / 2.0f;
            win.y = m_viewport.y + m_viewport.height * (win.y + 1.0f) / 2.0f;
            win.z = (win.z + 1.0f) / 2.0f;
            return win;
        }

        const Vec3f Camera::unproject(float x, float y, float depth) const {
            if (!m_valid)
                validate();
            
            Vec3f normalized;
            normalized.x = 2.0f * (x - m_viewport.x) / m_viewport.width  - 1.0f;
            normalized.y = 2.0f * (m_viewport.height - y - m_viewport.y) / m_viewport.height - 1.0f;
            normalized.z = 2.0f * depth - 1.0f;
            
            return m_invertedMatrix * normalized;
        }

        const Vec3f Camera::toCameraCoordinateSystem(const Vec3f& point) const {
            if (!m_valid)
                validate();
            
            Vec3f result = m_matrix * point;
            result.x = m_viewport.width  * result.x / 2.0f;
            result.y = m_viewport.height * result.y / 2.0f;
            result.z = m_farPlane * (1.0f - result.z);
            
            return result;
        }

        const Ray Camera::pickRay(float x, float y) const {
            Vec3f direction = (unproject(x, y, 0.5f) - m_position).normalized();
            return Ray(m_position, direction);
        }

        const Mat4f& Camera::matrix() const {
            return m_matrix;
        }

        const Mat4f Camera::billboardMatrix(bool fixUp) const {
            Vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            if (fixUp) {
                bbLook.z = 0.0f;
                if (bbLook.null()) {
                    bbLook = -m_up;
                    bbLook.z = 0.0f;
                }
                bbLook.normalize();
                bbUp = Vec3f::PosZ;
            } else {
                bbUp = m_up;
            }
            bbRight = bbUp.crossed(bbLook);

            return Mat4f(bbRight.x,  bbUp.x,     bbLook.x,   0.0f,
                         bbRight.y,  bbUp.y,     bbLook.y,   0.0f,
                         bbRight.z,  bbUp.z,     bbLook.z,   0.0f,
                         0.0f,       0.0f,       0.0f,       1.0f);
        }

        void Camera::frustumPlanes(Plane& top, Plane& right, Plane& bottom, Plane& left) const {
            float vFrustum = std::tan(Math::radians(m_fieldOfVision) / 2.0f) * 0.75f * m_nearPlane;
            float hFrustum = vFrustum * static_cast<float>(m_viewport.width) / static_cast<float>(m_viewport.height);
            const Vec3f center = m_position + m_direction * m_nearPlane;

            Vec3f d = center + m_up * vFrustum - m_position;
            d.normalize();
            top = Plane(m_right.crossed(d), m_position);
            
            d = center + m_right * hFrustum - m_position;
            d.normalize();
            right = Plane(d.crossed(m_up), m_position);
            
            d = center - m_up * vFrustum - m_position;
            d.normalize();
            bottom = Plane(d.crossed(m_right), m_position);
            
            d = center - m_right * hFrustum - m_position;
            d.normalize();
            left = Plane(m_up.crossed(d), m_position);
        }

        float Camera::distanceTo(const Vec3f& point) const {
            return std::sqrt(squaredDistanceTo(point));
        }
        
        float Camera::squaredDistanceTo(const Vec3f& point) const {
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
            setDirection((point - m_position).normalized(), up);
        }
        
        void Camera::setDirection(Vec3f direction, Vec3f up) {
            m_direction = direction;
            m_right = (m_direction.crossed(up)).normalized();
            m_up = m_right.crossed(m_direction);
        }
        
        void Camera::rotate(float yawAngle, float pitchAngle) {
            if (yawAngle == 0.0f && pitchAngle == 0.0f) return;
            
            Quat rotation = Quat(yawAngle, Vec3f::PosZ) * Quat(pitchAngle, m_right);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;

            if (newUp.z < 0.0f) {
                newUp.z = 0.0f;
                newDirection.x = 0.0f;
                newDirection.y = 0.0f;
                
                newUp.normalize();
                newDirection.normalize();
            }
            
            setDirection(newDirection, newUp);
        }
        
        void Camera::orbit(Vec3f center, float hAngle, float vAngle) {
            if (hAngle == 0.0f && vAngle == 0.0f) return;
            
            Quat rotation = Quat(hAngle, Vec3f::PosZ) * Quat(vAngle, m_right);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;
            Vec3f offset = m_position - center;
            
            if (newUp.z < 0.0f) {
                newUp.z = 0.0f;
                newDirection.x = 0.0f;
                newDirection.y = 0.0f;
                
                newUp.normalize();
                newDirection.normalize();
                
                // correct rounding errors
                float cos = (std::max)(-1.0f, (std::min)(1.0f, m_direction.dot(newDirection)));
                float angle = acosf(cos);
                if (!Math::zero(angle)) {
                    Vec3f axis = (m_direction.crossed(newDirection)).normalized();
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
    }
}
