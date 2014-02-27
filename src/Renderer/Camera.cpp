/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Color.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <limits>

namespace TrenchBroom {
    namespace Renderer {
        Camera::Viewport::Viewport() :
        x(0),
        y(0),
        width(0),
        height(0) {}
        
        Camera::~Camera() {}
        
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

        const Mat4x4f Camera::orthogonalBillboardMatrix() const {
            Vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbUp = m_up;
            bbRight = crossed(bbUp, bbLook);
            
            return Mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                           bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                           bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                           0.0f,          0.0f,       0.0f,       1.0f);
        }
        
        const Mat4x4f Camera::verticalBillboardMatrix() const {
            Vec3f bbLook, bbUp, bbRight;
            bbLook = -m_direction;
            bbLook[2] = 0.0f;
            if (bbLook.null()) {
                bbLook = -m_up;
                bbLook[2] = 0.0f;
            }
            bbLook.normalize();
            bbUp = Vec3f::PosZ;
            bbRight = crossed(bbUp, bbLook);
            
            return Mat4x4f(bbRight.x(),   bbUp.x(),   bbLook.x(), 0.0f,
                           bbRight.y(),   bbUp.y(),   bbLook.y(), 0.0f,
                           bbRight.z(),   bbUp.z(),   bbLook.z(), 0.0f,
                           0.0f,          0.0f,       0.0f,       1.0f);
        }

        void Camera::frustumPlanes(Plane3f& top, Plane3f& right, Plane3f& bottom, Plane3f& left) const {
            computeFrustumPlanes(top, right, bottom, left);
        }

        Ray3f Camera::viewRay() const {
            return Ray3f(m_position, m_direction);
        }

        Ray3f Camera::pickRay(const int x, const int y) const {
            return doGetPickRay(x, y);
        }

        float Camera::distanceTo(const Vec3f& point) const {
            return (point - m_position).length();
        }
        
        float Camera::squaredDistanceTo(const Vec3f& point) const {
            return (point - m_position).squaredLength();
        }

        Vec3f Camera::defaultPoint() const {
            return defaultPoint(m_direction);
        }
        
        Vec3f Camera::defaultPoint(const int x, const int y) const {
            const Vec3f point = unproject(static_cast<float>(x), static_cast<float>(y), 0.5f);
            return defaultPoint((point - m_position).normalized());
        }

        Vec3f Camera::defaultPoint(const Vec3f& direction) const {
            return m_position + 256.0f*direction;
        }

        Vec3f Camera::project(const Vec3f& point) const {
            if (!m_valid)
                validateMatrices();
            
            Vec3f win = m_matrix * point;
            win[0] = m_viewport.x + m_viewport.width *(win.x() + 1.0f)/2.0f;
            win[1] = m_viewport.y + m_viewport.height*(win.y() + 1.0f)/2.0f;
            win[2] = (win.z() + 1.0f)/2.0f;
            return win;
        }

        Vec3f Camera::unproject(const float x, const float y, const float depth) const {
            if (!m_valid)
                validateMatrices();
            
            Vec3f normalized;
            normalized[0] = 2.0f*(x - m_viewport.x)/m_viewport.width - 1.0f;
            normalized[1] = 2.0f*(m_viewport.height - y - m_viewport.y)/m_viewport.height - 1.0f;
            normalized[2] = 2.0f*depth - 1.0f;
            
            return m_invertedMatrix * normalized;
        }
        
        void Camera::setNearPlane(const float nearPlane) {
            assert(nearPlane < m_farPlane);
            m_nearPlane = nearPlane;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::setFarPlane(const float farPlane) {
            assert(farPlane > m_nearPlane);
            m_farPlane = farPlane;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::setViewport(const Viewport& viewport) {
            m_viewport = viewport;
            m_valid = false;
        }

        void Camera::moveTo(const Vec3f& position) {
            m_position = position;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::moveBy(const Vec3f& delta) {
            m_position += delta;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void Camera::lookAt(const Vec3f& point, const Vec3f& up) {
            setDirection((point - m_position).normalized(), up);
        }
        
        void Camera::setDirection(const Vec3f& direction, const Vec3f& up) {
            m_direction = direction;
            m_right = crossed(m_direction, up).normalized();
            m_up = crossed(m_right, m_direction);
            m_valid = false;
            cameraDidChangeNotifier(this);
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
                if (!Math::zero(angle)) {
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

        void Camera::renderFrustum(RenderContext& renderContext, Vbo& vbo) const {
            doRenderFrustum(renderContext, vbo);
        }

        float Camera::pickFrustum(const Ray3f& ray) const {
            return doPickFrustum(ray);
        }

        Camera::Viewport::Viewport(const int i_x, const int i_y, const unsigned int i_width, const unsigned int i_height) :
        x(i_x),
        y(i_y),
        width(i_width),
        height(i_height) {}
        
        Camera::Camera() :
        m_nearPlane(1.0f),
        m_farPlane(8000.0f),
        m_viewport(Viewport(0, 0, 1024, 768)),
        m_position(Vec3f::Null),
        m_valid(false) {
            setDirection(Vec3f::PosX, Vec3f::PosZ);
        }
        
        Camera::Camera(const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up) :
        m_nearPlane(nearPlane),
        m_farPlane(farPlane),
        m_viewport(viewport),
        m_position(position),
        m_valid(false) {
            assert(m_nearPlane >= 0.0f);
            assert(m_farPlane > m_nearPlane);
            assert(Math::eq(direction.length(), 1.0f));
            assert(Math::eq(up.length(), 1.0f));
            setDirection(direction, up);
        }
        
        void Camera::validateMatrices() const {
            doValidateMatrices(m_projectionMatrix, m_viewMatrix);
            m_matrix = m_projectionMatrix * m_viewMatrix;
            
            bool invertible = false;
            m_invertedMatrix = invertedMatrix(m_matrix, invertible);
            assert(invertible);
            m_valid = true;
        }
        
        PerspectiveCamera::PerspectiveCamera() :
        Camera(),
        m_fov(90.0) {}
        
        PerspectiveCamera::PerspectiveCamera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up)
        : Camera(nearPlane, farPlane, viewport, position, direction, up),
        m_fov(fov) {
            assert(m_fov > 0.0);
        }

        float PerspectiveCamera::fov() const {
            return m_fov;
        }
        
        void PerspectiveCamera::setFov(const float fov) {
            assert(fov > 0.0f);
            m_fov = fov;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }

        Ray3f PerspectiveCamera::doGetPickRay(int x, int y) const {
            const Vec3f direction = (unproject(static_cast<float>(x), static_cast<float>(y), 0.5f) - position()).normalized();
            return Ray3f(position(), direction);
        }

        void PerspectiveCamera::doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const {
            projectionMatrix = perspectiveMatrix(fov(), nearPlane(), farPlane(), viewport().width, viewport().height);
            viewMatrix = ::viewMatrix(direction(), up()) * translationMatrix(-position());
        }

        void PerspectiveCamera::computeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const {
            const float vFrustum = std::tan(Math::radians(fov()) / 2.0f) * 0.75f * nearPlane();
            const float hFrustum = vFrustum * static_cast<float>(viewport().width) / static_cast<float>(viewport().height);
            const Vec3f center = position() + direction() * nearPlane();
            
            Vec3f d = center + up() * vFrustum - position();
            topPlane = Plane3f(position(), crossed(right(), d).normalized());
            
            d = center + right() * hFrustum - position();
            rightPlane = Plane3f(position(), crossed(d, up()).normalized());
            
            d = center - up() * vFrustum - position();
            bottomPlane = Plane3f(position(), crossed(d, right()).normalized());
            
            d = center - right() * hFrustum - position();
            leftPlane = Plane3f(position(), crossed(up(), d).normalized());
        }

        void PerspectiveCamera::doRenderFrustum(RenderContext& renderContext, Vbo& vbo) const {
            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List triangleVertices(6);
            Vertex::List lineVertices(8 * 2);
            
            const float vFrustum = std::tan(Math::radians(fov()) / 2.0f) * 0.75f * nearPlane();
            const float hFrustum = vFrustum * static_cast<float>(viewport().width) / static_cast<float>(viewport().height);

            const Vec3f verts[] = {
                position() + (direction() * nearPlane() + vFrustum * up() - hFrustum * right()) / nearPlane() * 32.0f, // top left
                position() + (direction() * nearPlane() + vFrustum * up() + hFrustum * right()) / nearPlane() * 32.0f, // top right
                position() + (direction() * nearPlane() - vFrustum * up() + hFrustum * right()) / nearPlane() * 32.0f, // bottom right
                position() + (direction() * nearPlane() - vFrustum * up() - hFrustum * right()) / nearPlane() * 32.0f, // bottom left
            };
            
            triangleVertices[0] = Vertex(position(), Color(1.0f, 1.0f, 1.0f, 0.7f));
            for (size_t i = 0; i < 4; ++i)
                triangleVertices[i + 1] = Vertex(verts[i], Color(1.0f, 1.0f, 1.0f, 0.2f));
            triangleVertices[5] = Vertex(verts[0], Color(1.0f, 1.0f, 1.0f, 0.2f));
            
            for (size_t i = 0; i < 4; ++i) {
                lineVertices[2 * i + 0] = Vertex(position(), Color(1.0f, 1.0f, 1.0f, 1.0f));
                lineVertices[2 * i + 1] = Vertex(verts[i], Color(1.0f, 1.0f, 1.0f, 1.0f));
            }

            for (size_t i = 0; i < 4; ++i) {
                lineVertices[8 + 2 * i + 0] = Vertex(verts[i], Color(1.0f, 1.0f, 1.0f, 1.0f));
                lineVertices[8 + 2 * i + 1] = Vertex(verts[Math::succ(i, 4)], Color(1.0f, 1.0f, 1.0f, 1.0f));
            }
            
            VertexArray triangleArray = VertexArray::ref(GL_TRIANGLE_FAN, triangleVertices);
            VertexArray lineArray = VertexArray::ref(GL_LINES, lineVertices);
            
            SetVboState setVboState(vbo);
            setVboState.mapped();
            triangleArray.prepare(vbo);
            lineArray.prepare(vbo);
            setVboState.active();
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
            triangleArray.render();
            lineArray.render();
        }

        float PerspectiveCamera::doPickFrustum(const Ray3f& ray) const {
            const float vFrustum = std::tan(Math::radians(fov()) / 2.0f) * 0.75f * nearPlane();
            const float hFrustum = vFrustum * static_cast<float>(viewport().width) / static_cast<float>(viewport().height);
            
            const Vec3f verts[] = {
                position() + (direction() * nearPlane() + vFrustum * up() - hFrustum * right()) / nearPlane() * 32.0f, // top left
                position() + (direction() * nearPlane() + vFrustum * up() + hFrustum * right()) / nearPlane() * 32.0f, // top right
                position() + (direction() * nearPlane() - vFrustum * up() + hFrustum * right()) / nearPlane() * 32.0f, // bottom right
                position() + (direction() * nearPlane() - vFrustum * up() - hFrustum * right()) / nearPlane() * 32.0f, // bottom left
            };
            
            float distance = Math::nan<float>();
            for (size_t i = 0; i < 4; ++i)
                distance = Math::selectMin(distance, intersectRayWithTriangle(ray, position(), verts[i], verts[Math::succ(i, 4)]));
            return distance;
        }

        OrthographicCamera::OrthographicCamera() :
        Camera(),
        m_zoom(1.0f, 1.0f) {}
        
        OrthographicCamera::OrthographicCamera(const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up) :
        Camera(nearPlane, farPlane, viewport, position, direction, up),
        m_zoom(1.0f) {}

        const Vec2f& OrthographicCamera::zoom() const {
            return m_zoom;
        }
        
        void OrthographicCamera::setZoom(const Vec2f& zoom) {
            assert(zoom.x() > 0.0f && zoom.y() > 0.0f);
            m_zoom = zoom;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }
        
        void OrthographicCamera::zoom(const Vec2f& factors) {
            assert(factors.x() > 0.0f && factors.y() > 0.0f);
            m_zoom *= factors;
            m_valid = false;
            cameraDidChangeNotifier(this);
        }

        void OrthographicCamera::doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const {
            const float w2 = static_cast<float>(viewport().width) / m_zoom.x() / 2.0f;
            const float h2 = static_cast<float>(viewport().height) / m_zoom.y() / 2.0f;
            
            projectionMatrix = orthoMatrix(nearPlane(), farPlane(), -w2, h2, w2, -h2);
            viewMatrix = ::viewMatrix(direction(), up()) * translationMatrix(-position());
        }

        Ray3f OrthographicCamera::doGetPickRay(int x, int y) const {
            const Vec3f p = unproject(static_cast<float>(x), static_cast<float>(y), 0.5f);
            const Vec3f v = p - position();
            const float d = v.dot(direction());
            const Vec3f o = p - d * direction();
            return Ray3f(o, direction());
          }
        
        void OrthographicCamera::computeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const {
            const float w2 = static_cast<float>(viewport().width) / m_zoom.x() / 2.0f;
            const float h2 = static_cast<float>(viewport().height) / m_zoom.y() / 2.0f;

            const Vec3f& center = position();
            topPlane    = Plane3f(center + h2 * up(), up());
            rightPlane  = Plane3f(center + w2 * right(), right());
            bottomPlane = Plane3f(center - h2 * up(), -up());
            leftPlane   = Plane3f(center - w2 * right(), -right());
        }

        void OrthographicCamera::doRenderFrustum(RenderContext& renderContext, Vbo& vbo) const {
        }

        float OrthographicCamera::doPickFrustum(const Ray3f& ray) const {
            return Math::nan<float>();
        }
    }
}
