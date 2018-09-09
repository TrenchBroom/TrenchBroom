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

#ifndef TrenchBroom_Camera
#define TrenchBroom_Camera

#include <vecmath/VecMath.h>
#include "Color.h"
#include "TrenchBroom.h"
#include "Notifier.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        
        class Camera {
        public:
            struct Viewport {
                int x, y;
                int width, height;

                Viewport();
                Viewport(int i_x, int i_y, int i_width, int i_height);
                
                bool operator==(const Viewport& other) const;
                bool operator!=(const Viewport& other) const;

                template <typename T>
                bool contains(const T i_x, const T i_y, const T i_w, const T i_h) const {
                    return (i_x + i_w >= static_cast<T>(0) && i_x <= static_cast<T>(width) &&
                            i_y + i_h >= static_cast<T>(0) && i_y <= static_cast<T>(height));
                }
                
                template <typename T>
                bool contains(const T i_x, const T i_y) const {
                    return (i_x >= static_cast<T>(0) && i_x <= static_cast<T>(width) &&
                            i_y >= static_cast<T>(0) && i_y <= static_cast<T>(height));
                }
                
                int minDimension() const {
                    return width < height ? width : height;
                }
            };
        public:
            static const float DefaultPointDistance;
        private:
            float m_nearPlane;
            float m_farPlane;
            Viewport m_unzoomedViewport;
            Viewport m_zoomedViewport;
            float m_zoom;
            vm::vec3f m_position;
            vm::vec3f m_direction;
            vm::vec3f m_up;
            vm::vec3f m_right;

            mutable vm::mat4x4f m_projectionMatrix;
            mutable vm::mat4x4f m_viewMatrix;
            mutable vm::mat4x4f m_matrix;
            mutable vm::mat4x4f m_inverseMatrix;
        protected:
            typedef enum {
                Projection_Orthographic,
                Projection_Perspective
            } ProjectionType;
            mutable bool m_valid;
        public:
            Notifier1<const Camera*> cameraDidChangeNotifier;

            virtual ~Camera();
            
            bool orthographicProjection() const;
            bool perspectiveProjection() const;
            
            float nearPlane() const;
            float farPlane() const;
            const Viewport& unzoomedViewport() const;
            const Viewport& zoomedViewport() const;
            float zoom() const;
            void zoom(float factor);
            void setZoom(float zoom);
            const vm::vec3f& direction() const;
            const vm::vec3f& position() const;
            const vm::vec3f& up() const;
            const vm::vec3f& right() const;
            const vm::mat4x4f& projectionMatrix() const;
            const vm::mat4x4f& viewMatrix() const;
            const vm::mat4x4f orthogonalBillboardMatrix() const;
            const vm::mat4x4f verticalBillboardMatrix() const;
            void frustumPlanes(vm::plane3f& topPlane, vm::plane3f& rightPlane, vm::plane3f& bottomPlane, vm::plane3f& leftPlane) const;
            
            vm::ray3f viewRay() const;
            vm::ray3f pickRay(int x, int y) const;
            vm::ray3f pickRay(const vm::vec3f& point) const;
            float distanceTo(const vm::vec3f& point) const;
            float squaredDistanceTo(const vm::vec3f& point) const;
            float perpendicularDistanceTo(const vm::vec3f& point) const;
            vm::vec3f defaultPoint(const float distance = DefaultPointDistance) const;
            vm::vec3f defaultPoint(int x, int y) const;
            
            template <typename T>
            static vm::vec<T,3> defaultPoint(const vm::ray<T,3>& ray, const T distance = T(DefaultPointDistance)) {
                return ray.pointAtDistance(float(distance));
            }

            float perspectiveScalingFactor(const vm::vec3f& position) const;
            vm::vec3f project(const vm::vec3f& point) const;
            vm::vec3f unproject(const vm::vec3f& point) const;
            vm::vec3f unproject(float x, float y, float depth) const;
            
            void setNearPlane(float nearPlane);
            void setFarPlane(float farPlane);
            bool setViewport(const Viewport& viewport);
            void moveTo(const vm::vec3f& position);
            void moveBy(const vm::vec3f& delta);
            void lookAt(const vm::vec3f& point, const vm::vec3f& up);
            void setDirection(const vm::vec3f& direction, const vm::vec3f& up);
            void rotate(float yaw, float pitch);
            void orbit(const vm::vec3f& center, float horizontal, float vertical);
            
            void renderFrustum(RenderContext& renderContext, Vbo& vbo, float size, const Color& color) const;
            float pickFrustum(float size, const vm::ray3f& ray) const;
            
            FloatType pickPointHandle(const vm::ray3& pickRay, const vm::vec3& handlePosition, FloatType handleRadius) const;
            FloatType pickLineSegmentHandle(const vm::ray3& pickRay, const vm::segment3& handlePosition, FloatType handleRadius) const;
        protected:
            Camera();
            Camera(float nearPlane, float farPlane, const Viewport& viewport, const vm::vec3f& position, const vm::vec3f& direction, const vm::vec3f& up);
        private:
            ProjectionType projectionType() const;
            
            void validateMatrices() const;
            void updateZoomedViewport();
        private:
            virtual ProjectionType doGetProjectionType() const = 0;
            
            virtual void doValidateMatrices(vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const = 0;
            virtual vm::ray3f doGetPickRay(const vm::vec3f& point) const = 0;
            virtual void doComputeFrustumPlanes(vm::plane3f& topPlane, vm::plane3f& rightPlane, vm::plane3f& bottomPlane, vm::plane3f& leftPlane) const = 0;
            
            virtual void doRenderFrustum(RenderContext& renderContext, Vbo& vbo, float size, const Color& color) const = 0;
            virtual float doPickFrustum(float size, const vm::ray3f& ray) const = 0;
            virtual float doGetPerspectiveScalingFactor(const vm::vec3f& position) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Camera) */
