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

#ifndef __TrenchBroom__Camera__
#define __TrenchBroom__Camera__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Notifier.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class Camera {
        public:
            struct Viewport {
                int x, y;
                unsigned int width, height;

                Viewport();
                Viewport(int i_x, int i_y, unsigned int i_width, unsigned int i_height);
            };
        private:
            float m_nearPlane;
            float m_farPlane;
            Viewport m_viewport;
            Vec3f m_position;
            Vec3f m_direction;
            Vec3f m_up;
            Vec3f m_right;

            mutable Mat4x4f m_projectionMatrix;
            mutable Mat4x4f m_viewMatrix;
            mutable Mat4x4f m_matrix;
            mutable Mat4x4f m_invertedMatrix;
        protected:
            mutable bool m_valid;
        public:
            Notifier1<const Camera*> cameraDidChangeNotifier;

            virtual ~Camera();
            
            float nearPlane() const;
            float farPlane() const;
            const Viewport& viewport() const;
            const Vec3f& direction() const;
            const Vec3f& position() const;
            const Vec3f& up() const;
            const Vec3f& right() const;
            const Mat4x4f& projectionMatrix() const;
            const Mat4x4f& viewMatrix() const;
            const Mat4x4f orthogonalBillboardMatrix() const;
            const Mat4x4f verticalBillboardMatrix() const;
            void frustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const;
            
            Ray3f viewRay() const;
            Ray3f pickRay(int x, int y) const;
            float distanceTo(const Vec3f& point) const;
            float squaredDistanceTo(const Vec3f& point) const;
            Vec3f defaultPoint() const;
            Vec3f defaultPoint(int x, int y) const;
            Vec3f defaultPoint(const Vec3f& direction) const;
            
            Vec3f project(const Vec3f& point) const;
            Vec3f unproject(float x, float y, float depth) const;
            
            void setNearPlane(float nearPlane);
            void setFarPlane(float farPlane);
            void setViewport(const Viewport& viewport);
            void moveTo(const Vec3f& position);
            void moveBy(const Vec3f& delta);
            void lookAt(const Vec3f& point, const Vec3f& up);
            void setDirection(const Vec3f& direction, const Vec3f& up);
            void rotate(float yaw, float pitch);
            void orbit(const Vec3f& center, float horizontal, float vertical);
            
            void renderFrustum(RenderContext& renderContext, Vbo& vbo) const;
            float pickFrustum(const Ray3f& ray) const;
        protected:
            Camera();
            Camera(float nearPlane, float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up);
        private:
            void validateMatrices() const;
            virtual void doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const = 0;
            virtual Ray3f doGetPickRay(int x, int y) const = 0;
            virtual void computeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const = 0;
            virtual void doRenderFrustum(RenderContext& renderContext, Vbo& vbo) const = 0;
            virtual float doPickFrustum(const Ray3f& ray) const = 0;
        };
        
        class PerspectiveCamera : public Camera {
        private:
            float m_fov;
        public:
            PerspectiveCamera();
            PerspectiveCamera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up);
            
            float fov() const;
            void setFov(float fov);
        private:
            void doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const;
            Ray3f doGetPickRay(int x, int y) const;
            void computeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const;
            void doRenderFrustum(RenderContext& renderContext, Vbo& vbo) const;
            float doPickFrustum(const Ray3f& ray) const;
        };
        
        class OrthographicCamera : public Camera {
        private:
            Vec2f m_zoom;
        public:
            OrthographicCamera();
            OrthographicCamera(const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up);

            const Vec2f& zoom() const;
            void setZoom(const Vec2f& zoom);
            void zoom(const Vec2f& factors);
        private:
            void doValidateMatrices(Mat4x4f& projectionMatrix, Mat4x4f& viewMatrix) const;
            Ray3f doGetPickRay(int x, int y) const;
            void computeFrustumPlanes(Plane3f& topPlane, Plane3f& rightPlane, Plane3f& bottomPlane, Plane3f& leftPlane) const;
            void doRenderFrustum(RenderContext& renderContext, Vbo& vbo) const;
            float doPickFrustum(const Ray3f& ray) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Camera__) */
