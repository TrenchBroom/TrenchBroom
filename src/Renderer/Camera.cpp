/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Camera::Viewport::Viewport() :
        x(0),
        y(0),
        width(0),
        height(0) {}
        
        Camera::Viewport::Viewport(int i_x, int i_y, unsigned int i_width, unsigned int i_height) :
        x(i_x),
        y(i_y),
        width(i_width),
        height(i_height) {}

        Camera::Camera() :
        m_fov(90.0f),
        m_nearPlane(1.0f),
        m_farPlane(8000.0f),
        m_viewport(Viewport(0, 0, 1024, 768)),
        m_position(Vec3f::Null),
        m_valid(false) {
            setDirection(Vec3f::PosX, Vec3f::PosZ);
        }


        Camera::Camera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up) :
        m_fov(fov),
        m_nearPlane(nearPlane),
        m_farPlane(farPlane),
        m_viewport(viewport),
        m_position(position),
        m_valid(false) {
            assert(m_fov > 0.0f);
            assert(m_nearPlane >= 0.0f);
            assert(m_farPlane > m_nearPlane);
            assert(Math<float>::eq(direction.length(), 1.0f));
            assert(Math<float>::eq(up.length(), 1.0f));
            setDirection(direction, up);
        }
        
        float Camera::fov() const {
            return m_fov;
        }
        
        float Camera::nearPlane() const {
            return m_nearPlane;
        }
        
        float Camera::farPlane() const {
            return m_farPlane;
        }
        
        const Camera::Viewport& Camera::viewport() const {
            return m_viewport;
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
        
        const Mat4x4f& Camera::projectionMatrix() const {
            if (!m_valid)
                validateMatrices();
            return m_projectionMatrix;
        }
        
        const Mat4x4f& Camera::viewMatrix() const {
            if (!m_valid)
                validateMatrices();
            return m_viewMatrix;
        }

        Ray3f Camera::viewRay() const {
            return Ray3f(m_position, m_direction);
        }

        void Camera::setFov(const float fov) {
            assert(fov > 0.0f);
            m_fov = fov;
            m_valid = false;
        }
        
        void Camera::setNearPlane(const float nearPlane) {
            assert(nearPlane >= 0.0f);
            assert(nearPlane < m_farPlane);
            m_nearPlane = nearPlane;
            m_valid = false;
        }
        
        void Camera::setFarPlane(const float farPlane) {
            assert(farPlane > m_nearPlane);
            m_farPlane = farPlane;
            m_valid = false;
        }
        
        void Camera::setViewport(const Viewport& viewport) {
            m_viewport = viewport;
            m_valid = false;
        }

        void Camera::moveTo(const Vec3f& position) {
            m_position = position;
            m_valid = false;
        }
        
        void Camera::moveBy(const Vec3f& delta) {
            m_position += delta;
            m_valid = false;
        }
        
        void Camera::lookAt(const Vec3f& point, const Vec3f& up) {
            setDirection((point - m_position).normalized(), up);
        }
        
        void Camera::setDirection(const Vec3f& direction, const Vec3f& up) {
            m_direction = direction;
            m_right = crossed(m_direction, up).normalized();
            m_up = crossed(m_right, m_direction);
            m_valid = false;
        }
        
        void Camera::rotate(const float yaw, const float pitch) {
            if (yaw == 0.0f && pitch == 0.0f)
                return;
            
            const Quatf rotation = Quatf(Vec3f::PosZ, yaw) * Quatf(m_right, pitch);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;
                
                newUp.normalize();
                newDirection.normalize();
            }
            
            setDirection(newDirection, newUp);
        }
        
        void Camera::orbit(const Vec3f& center, const float horizontal, const float vertical) {
            if (horizontal == 0.0f && vertical == 0.0f)
                return;
            
            Quatf rotation = Quatf(Vec3f::PosZ, horizontal) * Quatf(m_right, vertical);
            Vec3f newDirection = rotation * m_direction;
            Vec3f newUp = rotation * m_up;
            Vec3f offset = m_position - center;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;
                
                newUp.normalize();
                newDirection.normalize();
                
                // correct rounding errors
                const float cos = (std::max)(-1.0f, (std::min)(1.0f, m_direction.dot(newDirection)));
                const float angle = acosf(cos);
                if (!Mathf::zero(angle)) {
                    const Vec3f axis = crossed(m_direction, newDirection).normalized();
                    rotation = Quatf(axis, angle);
                    offset = rotation * offset;
                    newUp = rotation * newUp;
                }
            } else {
                offset = rotation * offset;
            }
            
            setDirection(newDirection, newUp);
            moveTo(offset + center);
        }

        void Camera::validateMatrices() const {
            m_projectionMatrix = perspectiveMatrix(m_fov, m_nearPlane, m_farPlane, m_viewport.width, m_viewport.height);
            m_viewMatrix = ::viewMatrix(m_direction, m_up) * translationMatrix(-m_position);
            m_matrix = m_projectionMatrix * m_viewMatrix;
            
            bool invertible = false;
            m_invertedMatrix = invertedMatrix(m_matrix, invertible);
            assert(invertible);
            m_valid = true;
        }
    }
}
