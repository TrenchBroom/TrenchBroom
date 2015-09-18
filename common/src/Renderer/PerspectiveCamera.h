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

#ifndef TrenchBroom_PerspectiveCamera
#define TrenchBroom_PerspectiveCamera

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace Renderer {
        class PerspectiveCamera : public Camera {
        private:
            float m_fov;
        public:
            PerspectiveCamera();
            PerspectiveCamera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up);
            
            float fov() const;
            void setFov(float fov);
        private:
            ProjectionType doGetProjectionType() const;

            void doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const;
            Ray3f doGetPickRay(const Vec3f& point) const;
            void doComputeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const;
            
            void doRenderFrustum(RenderContext& renderContext, Vbo& vbo, float size, const Color& color) const;
            float doPickFrustum(float size, const Ray3f& ray) const;
            
            void getFrustumVertices(float size, Vec3f (&verts)[4]) const;
            Vec2f getFrustum() const;

            float doGetPerspectiveScalingFactor(const Vec3f& position) const;
            float viewportFrustumDistance() const;
        };
    }
}

#endif /* defined(TrenchBroom_PerspectiveCamera) */
