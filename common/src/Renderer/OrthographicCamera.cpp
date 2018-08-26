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

#include "OrthographicCamera.h"

namespace TrenchBroom {
    namespace Renderer {
        OrthographicCamera::OrthographicCamera() :
        Camera() {}
        
        OrthographicCamera::OrthographicCamera(const float nearPlane, const float farPlane, const Viewport& viewport, const vec3f& position, const vec3f& direction, const vec3f& up) :
        Camera(nearPlane, farPlane, viewport, position, direction, up) {}
        
        vec3::List OrthographicCamera::viewportVertices() const {
            const float w2 = static_cast<float>(zoomedViewport().width)  / 2.0f;
            const float h2 = static_cast<float>(zoomedViewport().height) / 2.0f;
            
            vec3::List result(4);
            result[0] = vec3(position() - w2 * right() + h2 * up());
            result[1] = vec3(position() + w2 * right() + h2 * up());
            result[2] = vec3(position() + w2 * right() - h2 * up());
            result[3] = vec3(position() - w2 * right() - h2 * up());
            return result;
        }

        Camera::ProjectionType OrthographicCamera::doGetProjectionType() const {
            return Projection_Orthographic;
        }

        void OrthographicCamera::doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const {
            const float w2 = static_cast<float>(zoomedViewport().width) / 2.0f;
            const float h2 = static_cast<float>(zoomedViewport().height) / 2.0f;
            
            projectionMatrix = orthoMatrix(nearPlane(), farPlane(), -w2, h2, w2, -h2);
            viewMatrix = ::viewMatrix(direction(), up()) * translationMatrix(-position());
        }
        
        Ray3f OrthographicCamera::doGetPickRay(const vec3f& point) const {
            const vec3f v = point - position();
            const float d = dot(v, direction());
            const vec3f o = point - d * direction();
            return Ray3f(o, direction());
        }
        
        void OrthographicCamera::doComputeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const {
            const float w2 = static_cast<float>(zoomedViewport().width) / 2.0f;
            const float h2 = static_cast<float>(zoomedViewport().height) / 2.0f;
            
            const vec3f& center = position();
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

        float OrthographicCamera::doGetPerspectiveScalingFactor(const vec3f& position) const {
            return 1.0f / zoom();
        }
    }
}
