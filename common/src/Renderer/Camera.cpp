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
        
        const Camera::Viewport& Camera::unzoomedViewport() const {
            return m_unzoomedViewport;
        }
        
        const Camera::Viewport& Camera::zoomedViewport() const {
            return m_zoomedViewport;
        }

        float Camera::zoom() const {
            return m_zoom;
        }
        
        void Camera::zoom(const float factor) {
            assert(factor > 0.0f);
            if (factor == 1.0f)
                return;
            setZoom(m_zoom * factor);
        }
        
        void Camera::setZoom(const float zoom) {
            assert(zoom > 0.0f);
            if (zoom == m_zoom)
                return;
            m_zoom = zoom;
            updateZoomedViewport();
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        const vec3f& Camera::position() const {
            return m_position;
        }
        
        const vec3f& Camera::direction() const {
            return m_direction;
        }
        
        const vec3f& Camera::up() const {
            return m_up;
        }
        
        const vec3f& Camera::right() const {
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

        const Mat4x4f Camera::orthogonalBillboardMatrix() const {
            vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbUp = m_up;
            bbRight = cross(bbUp, bbLook);
            
            return Mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                           bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                           bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                           0.0f,          0.0f,       0.0f,       1.0f);
        }
        
        const Mat4x4f Camera::verticalBillboardMatrix() const {
            vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbLook[2] = 0.0f;
            if (isZero(bbLook)) {
                bbLook = -m_up;
                bbLook[2] = 0.0f;
            }
            bbLook = normalize(bbLook);
            bbUp = vec3f::pos_z;
            bbRight = cross(bbUp, bbLook);
            
            return Mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                           bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                           bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                           0.0f,          0.0f,       0.0f,       1.0f);
        }

        void Camera::frustumPlanes(Plane3f& top, Plane3f& right, Plane3f& bottom, Plane3f& left) const {
            doComputeFrustumPlanes(top, right, bottom, left);
        }

        Ray3f Camera::viewRay() const {
            return Ray3f(m_position, m_direction);
        }

        Ray3f Camera::pickRay(const int x, const int y) const {
            return doGetPickRay(unproject(static_cast<float>(x), static_cast<float>(y), 0.5f));
        }

        Ray3f Camera::pickRay(const vec3f& point) const {
            return doGetPickRay(point);
        }

        float Camera::distanceTo(const vec3f& point) const {
            return distance(point, m_position);
        }
        
        float Camera::squaredDistanceTo(const vec3f& point) const {
            return squaredDistance(point, m_position);
        }

        float Camera::perpendicularDistanceTo(const vec3f& point) const {
            return dot(point - m_position, m_direction);
        }

        vec3f Camera::defaultPoint(const float distance) const {
            return m_position + distance * direction();
        }
        
        vec3f Camera::defaultPoint(const int x, const int y) const {
            const Ray3f ray = pickRay(x, y);
            return defaultPoint(ray);
        }

        float Camera::perspectiveScalingFactor(const vec3f& position) const {
            return doGetPerspectiveScalingFactor(position);
        }

        vec3f Camera::project(const vec3f& point) const {
            if (!m_valid)
                validateMatrices();
            
            vec3f win = m_matrix * point;
            win[0] = m_unzoomedViewport.x + m_unzoomedViewport.width *(win.x() + 1.0f)/2.0f;
            win[1] = m_unzoomedViewport.y + m_unzoomedViewport.height*(win.y() + 1.0f)/2.0f;
            win[2] = (win.z() + 1.0f)/2.0f;
            return win;
        }

        vec3f Camera::unproject(const vec3f& point) const {
            return unproject(point.x(), point.y(), point.z());
        }

        vec3f Camera::unproject(const float x, const float y, const float depth) const {
            if (!m_valid)
                validateMatrices();
            
            vec3f normalized;
            normalized[0] = 2.0f*(x - m_unzoomedViewport.x)/m_unzoomedViewport.width - 1.0f;
            normalized[1] = 2.0f*(m_unzoomedViewport.height - y - m_unzoomedViewport.y)/m_unzoomedViewport.height - 1.0f;
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
            if (viewport == m_unzoomedViewport) {
                return false;
            }
            m_unzoomedViewport = viewport;
            updateZoomedViewport();
            m_valid = false;
            return true;
        }

        void Camera::moveTo(const vec3f& position) {
            if (position == m_position)
                return;
            m_position = position;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::moveBy(const vec3f& delta) {
            if (isZero(delta))
                return;
            m_position += delta;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::lookAt(const vec3f& point, const vec3f& up) {
            setDirection(normalize(point - m_position), up);
        }
        
        void Camera::setDirection(const vec3f& direction, const vec3f& up) {
            if (direction == m_direction && up == m_up)
                return;
            m_direction = direction;
            
            const vec3f rightUnnormalized = cross(m_direction, up);
            if (isZero(rightUnnormalized)) {
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
            if (yaw == 0.0f && pitch == 0.0f)
                return;
            
            const Quatf rotation = Quatf(vec3f::pos_z, yaw) * Quatf(m_right, pitch);
            vec3f newDirection = rotation * m_direction;
            vec3f newUp = rotation * m_up;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;
                
                newUp = normalize(newUp);
                newDirection = normalize(newDirection);
            }
            
            setDirection(newDirection, newUp);
        }
        
        void Camera::orbit(const vec3f& center, const float horizontal, const float vertical) {
            if (horizontal == 0.0f && vertical == 0.0f)
                return;
            
            Quatf rotation = Quatf(vec3f::pos_z, horizontal) * Quatf(m_right, vertical);
            vec3f newDirection = rotation * m_direction;
            vec3f newUp = rotation * m_up;
            vec3f offset = m_position - center;
            
            if (newUp[2] < 0.0f) {
                newUp[2] = 0.0f;
                newDirection[0] = 0.0f;
                newDirection[1] = 0.0f;

                newUp = normalize(newUp);
                newDirection = normalize(newDirection);

                // correct rounding errors
                const float cos = Math::clamp(dot(m_direction, newDirection), -1.0f, 1.0f);
                const float angle = acosf(cos);
                if (!Math::zero(angle)) {
                    const vec3f axis = normalize(cross(m_direction, newDirection));
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

        void Camera::renderFrustum(RenderContext& renderContext, Vbo& vbo, const float size, const Color& color) const {
            doRenderFrustum(renderContext, vbo, size, color);
        }

        float Camera::pickFrustum(const float size, const Ray3f& ray) const {
            return doPickFrustum(size, ray);
        }

        FloatType Camera::pickPointHandle(const Ray3& pickRay, const vec3& handlePosition, const FloatType handleRadius) const {
            const FloatType scaling = static_cast<FloatType>(perspectiveScalingFactor(vec3f(handlePosition)));
            return pickRay.intersectWithSphere(handlePosition, 2.0 * handleRadius * scaling);
        }

        FloatType Camera::pickLineSegmentHandle(const Ray3& pickRay, const Edge3& handlePosition, const FloatType handleRadius) const {
            const Ray3::LineDistance distance = pickRay.distanceToSegment(handlePosition.start(), handlePosition.end());
            if (distance.parallel)
                return Math::nan<FloatType>();
            
            const vec3 pointHandlePosition = handlePosition.pointAtDistance(distance.lineDistance);
            return pickPointHandle(pickRay, pointHandlePosition, handleRadius);
        }

        Camera::Camera() :
        m_nearPlane(1.0f),
        m_farPlane(8000.0f),
        m_unzoomedViewport(Viewport(0, 0, 1024, 768)),
        m_zoom(1.0f),
        m_position(vec3f::zero),
        m_valid(false) {
            setDirection(vec3f::pos_x, vec3f::pos_z);
            updateZoomedViewport();
        }
        
        Camera::Camera(const float nearPlane, const float farPlane, const Viewport& viewport, const vec3f& position, const vec3f& direction, const vec3f& up) :
        m_nearPlane(nearPlane),
        m_farPlane(farPlane),
        m_unzoomedViewport(viewport),
        m_zoom(1.0f),
        m_position(position),
        m_valid(false) {
            assert(m_nearPlane >= 0.0f);
            assert(m_farPlane > m_nearPlane);
            assert(isUnit(direction));
            assert(isUnit(up));
            setDirection(direction, up);
            updateZoomedViewport();
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

        void Camera::updateZoomedViewport() {
            m_zoomedViewport = Viewport(m_unzoomedViewport.x, m_unzoomedViewport.y,
                                        static_cast<int>(Math::round(m_unzoomedViewport.width / zoom())),
                                        static_cast<int>(Math::round(m_unzoomedViewport.height / zoom())));
        }
    }
}
