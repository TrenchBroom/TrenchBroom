/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Camera.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/ray.h>
#include <vecmath/distance.h>
#include <vecmath/intersection.h>

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        Camera::Viewport::Viewport() :
        x(0),
        y(0),
        width(0),
        height(0) {}
        
        Camera::Viewport::Viewport(const int i_x, const int i_y, const int i_width, const int i_height) :
        x(i_x),
        y(i_y),
        width(i_width),
        height(i_height) {}

        bool Camera::Viewport::operator==(const Viewport& other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }

        bool Camera::Viewport::operator!=(const Viewport& other) const {
            return !(*this == other);
        }

        const float Camera::DefaultPointDistance = 256.0f;
        
        Camera::~Camera() {}
        
        bool Camera::orthographicProjection() const {
            return projectionType() == Projection_Orthographic;
        }
        
        bool Camera::perspectiveProjection() const {
            return projectionType() == Projection_Perspective;
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
        
        float Camera::zoom() const {
            return m_zoom;
        }
        
        void Camera::zoom(const float factor) {
            assert(factor > 0.0f);
            setZoom(m_zoom * factor);
        }
        
        void Camera::setZoom(const float zoom) {
            assert(zoom > 0.0f);
            if (zoom != m_zoom) {
                m_zoom = zoom;
                doUpdateZoom();
                m_valid = false;
                cameraDidChangeNotifier(this);
            }
        }
        
        const vm::vec3f& Camera::position() const {
            return m_position;
        }
        
        const vm::vec3f& Camera::direction() const {
            return m_direction;
        }
        
        const vm::vec3f& Camera::up() const {
            return m_up;
        }
        
        const vm::vec3f& Camera::right() const {
            return m_right;
        }
        
        const vm::mat4x4f& Camera::projectionMatrix() const {
            if (!m_valid)
                validateMatrices();
            return m_projectionMatrix;
        }
        
        const vm::mat4x4f& Camera::viewMatrix() const {
            if (!m_valid)
                validateMatrices();
            return m_viewMatrix;
        }

        const vm::mat4x4f Camera::orthogonalBillboardMatrix() const {
            vm::vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbUp = m_up;
            bbRight = cross(bbUp, bbLook);
            
            return vm::mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                               bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                               bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                               0.0f,          0.0f,       0.0f,       1.0f);
        }
        
        const vm::mat4x4f Camera::verticalBillboardMatrix() const {
            vm::vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbLook[2] = 0.0f;
            if (isZero(bbLook, vm::Cf::almostZero())) {
                bbLook = -m_up;
                bbLook[2] = 0.0f;
            }
            bbLook = normalize(bbLook);
            bbUp = vm::vec3f::pos_z;
            bbRight = cross(bbUp, bbLook);
            
            return vm::mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                           bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                           bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                           0.0f,          0.0f,       0.0f,       1.0f);
        }

        void Camera::frustumPlanes(vm::plane3f& top, vm::plane3f& right, vm::plane3f& bottom, vm::plane3f& left) const {
            doComputeFrustumPlanes(top, right, bottom, left);
        }

        vm::ray3f Camera::viewRay() const {
            return vm::ray3f(m_position, m_direction);
        }

        vm::ray3f Camera::pickRay(const int x, const int y) const {
            return doGetPickRay(unproject(static_cast<float>(x), static_cast<float>(y), 0.5f));
        }

        vm::ray3f Camera::pickRay(const vm::vec3f& point) const {
            return doGetPickRay(point);
        }

        float Camera::distanceTo(const vm::vec3f& point) const {
            return distance(point, m_position);
        }
        
        float Camera::squaredDistanceTo(const vm::vec3f& point) const {
            return squaredDistance(point, m_position);
        }

        float Camera::perpendicularDistanceTo(const vm::vec3f& point) const {
            return dot(point - m_position, m_direction);
        }

        vm::vec3f Camera::defaultPoint(const float distance) const {
            return m_position + distance * direction();
        }
        
        vm::vec3f Camera::defaultPoint(const int x, const int y) const {
            const vm::ray3f ray = pickRay(x, y);
            return defaultPoint(ray);
        }

        float Camera::perspectiveScalingFactor(const vm::vec3f& position) const {
            return doGetPerspectiveScalingFactor(position);
        }

        vm::vec3f Camera::project(const vm::vec3f& point) const {
            if (!m_valid)
                validateMatrices();
            
            vm::vec3f win = m_matrix * point;
            win[0] = m_viewport.x + m_viewport.width *(win.x() + 1.0f)/2.0f;
            win[1] = m_viewport.y + m_viewport.height*(win.y() + 1.0f)/2.0f;
            win[2] = (win.z() + 1.0f)/2.0f;
            return win;
        }

        vm::vec3f Camera::unproject(const vm::vec3f& point) const {
            return unproject(point.x(), point.y(), point.z());
        }

        vm::vec3f Camera::unproject(const float x, const float y, const float depth) const {
            if (!m_valid)
                validateMatrices();
            
            vm::vec3f normalized;
            normalized[0] = 2.0f*(x - m_viewport.x)/m_viewport.width - 1.0f;
            normalized[1] = 2.0f*(m_viewport.height - y - m_viewport.y)/m_viewport.height - 1.0f;
            normalized[2] = 2.0f*depth - 1.0f;
            
            return m_inverseMatrix * normalized;
        }
        
        void Camera::setNearPlane(const float nearPlane) {
            assert(nearPlane < m_farPlane);
            if (nearPlane == m_nearPlane)
                return;
            m_nearPlane = nearPlane;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::setFarPlane(const float farPlane) {
            assert(farPlane > m_nearPlane);
            if (farPlane == m_farPlane)
                return;
            m_farPlane = farPlane;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        bool Camera::setViewport(const Viewport& viewport) {
            if (viewport == m_viewport) {
                return false;
            }
            m_viewport = viewport;
            doUpdateZoom();
            m_valid = false;
            return true;
        }

        void Camera::moveTo(const vm::vec3f& position) {
            if (position == m_position)
                return;
            m_position = position;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::moveBy(const vm::vec3f& delta) {
            if (isZero(delta, vm::Cf::almostZero()))
                return;
            m_position = m_position + delta;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::lookAt(const vm::vec3f& point, const vm::vec3f& up) {
            setDirection(normalize(point - m_position), up);
        }
        
        void Camera::setDirection(const vm::vec3f& direction, const vm::vec3f& up) {
            if (direction == m_direction && up == m_up)
                return;
            m_direction = direction;
            
            const vm::vec3f rightUnnormalized = cross(m_direction, up);
            if (isZero(rightUnnormalized, vm::Cf::almostZero())) {
                // `direction` and `up` were colinear.
                const auto axis = thirdAxis(m_direction);
                m_right = normalize(cross(m_direction, axis));
            } else {
                m_right = normalize(rightUnnormalized);
            }
            m_up = cross(m_right, m_direction);
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::rotate(const float yaw, const float pitch) {
            if (yaw == 0.0f && pitch == 0.0f) {
                return;
            }

            const auto rotation = vm::quatf(vm::vec3f::pos_z, yaw) * vm::quatf(m_right, pitch);
            auto newDirection = rotation * m_direction;
            auto newUp = rotation * m_up;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;
                
                newUp = normalize(newUp);
                newDirection = normalize(newDirection);
            }
            
            setDirection(newDirection, newUp);
        }
        
        void Camera::orbit(const vm::vec3f& center, const float horizontal, const float vertical) {
            if (horizontal == 0.0f && vertical == 0.0f) {
                return;
            }

            auto rotation = vm::quatf(vm::vec3f::pos_z, horizontal) * vm::quatf(m_right, vertical);
            auto newDirection = rotation * m_direction;
            auto newUp = rotation * m_up;
            auto offset = m_position - center;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;

                newUp = normalize(newUp);
                newDirection = normalize(newDirection);

                // correct rounding errors
                const auto cos = vm::clamp(dot(m_direction, newDirection), -1.0f, 1.0f);
                const auto angle = acosf(cos);
                if (!vm::isZero(angle, vm::Cf::almostZero())) {
                    const auto axis = normalize(cross(m_direction, newDirection));
                    rotation = vm::quatf(axis, angle);
                    offset = rotation * offset;
                    newUp = rotation * newUp;
                }
            } else {
                offset = rotation * offset;
            }
            
            setDirection(newDirection, newUp);
            moveTo(offset + center);
        }

        void Camera::renderFrustum(RenderContext& renderContext, Vbo& vbo, const float size, const Color& color) const {
            doRenderFrustum(renderContext, vbo, size, color);
        }

        float Camera::pickFrustum(const float size, const vm::ray3f& ray) const {
            return doPickFrustum(size, ray);
        }

        FloatType Camera::pickPointHandle(const vm::ray3& pickRay, const vm::vec3& handlePosition, const FloatType handleRadius) const {
            const auto scaling = static_cast<FloatType>(perspectiveScalingFactor(vm::vec3f(handlePosition)));
            return intersect(pickRay, handlePosition, 2.0 * handleRadius * scaling);
        }

        FloatType Camera::pickLineSegmentHandle(const vm::ray3& pickRay, const vm::segment3& handlePosition, const FloatType handleRadius) const {
            const auto dist = distance(pickRay, handlePosition);
            if (dist.parallel) {
                return vm::nan<FloatType>();
            }

            const auto pointHandlePosition = handlePosition.pointAtDistance(dist.position2);
            return pickPointHandle(pickRay, pointHandlePosition, handleRadius);
        }

        Camera::Camera() :
        m_nearPlane(1.0f),
        m_farPlane(8000.0f),
        m_viewport(Viewport(0, 0, 1024, 768)),
        m_zoom(1.0f),
        m_position(vm::vec3f::zero),
        m_valid(false) {
            setDirection(vm::vec3f::pos_x, vm::vec3f::pos_z);
        }
        
        Camera::Camera(const float nearPlane, const float farPlane, const Viewport& viewport, const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up) :
        m_nearPlane(nearPlane),
        m_farPlane(farPlane),
        m_viewport(viewport),
        m_zoom(1.0f),
        m_position(position),
        m_valid(false) {
            assert(m_nearPlane >= 0.0f);
            assert(m_farPlane > m_nearPlane);
            assert(isUnit(direction, vm::Cf::almostZero()));
            assert(isUnit(up, vm::Cf::almostZero()));
            setDirection(direction, up);
        }

        Camera::ProjectionType Camera::projectionType() const {
            return doGetProjectionType();
        }

        void Camera::validateMatrices() const {
            doValidateMatrices(m_projectionMatrix, m_viewMatrix);
            m_matrix = m_projectionMatrix * m_viewMatrix;

            const auto [invertible, inverse] = invert(m_matrix);
            assert(invertible); unused(invertible);
            m_inverseMatrix = inverse;
            m_valid = true;
        }
    }
}
