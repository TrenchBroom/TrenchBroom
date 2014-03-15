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

#include "OrthographicCamera.h"

namespace TrenchBroom {
    namespace Renderer {
        OrthographicCamera::OrthographicCamera() :
        Camera(),
        m_zoom(1.0f, 1.0f) {}
        
        OrthographicCamera::OrthographicCamera(const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up) :
        Camera(nearPlane, farPlane, viewport, position, direction, up),
        m_zoom(1.0f, 1.0f) {}
        
        const Vec2f& OrthographicCamera::zoom() const {
            return m_zoom;
        }
        
        void OrthographicCamera::setZoom(const float zoom) {
            setZoom(Vec2f(zoom, zoom));
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
        
        Ray3f OrthographicCamera::doGetPickRay(const int x, const int y) const {
            const Vec3f p = unproject(static_cast<float>(x), static_cast<float>(y), 0.5f);
            const Vec3f v = p - position();
            const float d = v.dot(direction());
            const Vec3f o = p - d * direction();
            return Ray3f(o, direction());
        }
        
        void OrthographicCamera::doComputeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const {
            const float w2 = static_cast<float>(viewport().width) / m_zoom.x() / 2.0f;
            const float h2 = static_cast<float>(viewport().height) / m_zoom.y() / 2.0f;
            
            const Vec3f& center = position();
            topPlane    = Plane3f(center + h2 * up(), up());
            rightPlane  = Plane3f(center + w2 * right(), right());
            bottomPlane = Plane3f(center - h2 * up(), -up());
            leftPlane   = Plane3f(center - w2 * right(), -right());
        }
        
        void OrthographicCamera::doRenderFrustum(RenderContext& renderContext, Vbo& vbo, const float size, const Color& color) const {
        }
        
        float OrthographicCamera::doPickFrustum(const float size, const Ray3f& ray) const {
            return Math::nan<float>();
        }
    }
}
