#/*
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

#ifndef TrenchBroom_OrthographicCamera
#define TrenchBroom_OrthographicCamera

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera : public Camera {
        public:
            OrthographicCamera();
            OrthographicCamera(const float nearPlane, const float farPlane, const Viewport& viewport, const vec3f& position, const vec3f& direction, const vec3f& up);
            
            vec3::List viewportVertices() const;
        private:
            ProjectionType doGetProjectionType() const override;

            void doValidateMatrices(mat4x4f& projectionMatrix, mat4x4f& viewMatrix) const override;
            ray3f doGetPickRay(const vec3f& point) const override;
            void doComputeFrustumPlanes(plane3f& topPlane, plane3f& rightPlane, plane3f& bottomPlane, plane3f& leftPlane) const override;
            
            void doRenderFrustum(RenderContext& renderContext, Vbo& vbo, float size, const Color& color) const override;
            float doPickFrustum(float size, const ray3f& ray) const override;
            float doGetPerspectiveScalingFactor(const vec3f& position) const override;
        };
    }
}

#endif /* defined(TrenchBroom_OrthographicCamera) */
