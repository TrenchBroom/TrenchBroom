/*
 Copyright (C) 2010-2012 Kristian Duske

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

#ifndef TrenchBroom_Camera_h
#define TrenchBroom_Camera_h

#include <GL/glew.h>
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class Camera {
        public:
            class Viewport {
            public:
                int x, y, width, height;
            public:
                Viewport() : x(0), y(0), width(0), height(0) {}
                Viewport(int i_x, int i_y, int i_width, int i_height) : x(i_x), y(i_y), width(i_width), height(i_height) {}
            };
        protected:
            bool m_ortho;
            float m_fieldOfVision;
            float m_nearPlane;
            float m_farPlane;

            Vec3f m_position;
            Vec3f m_direction;
            Vec3f m_up;
            Vec3f m_right;

            Viewport m_viewport;
            
            mutable Mat4f m_matrix;
            mutable Mat4f m_invertedMatrix;
            mutable bool m_valid;
            
            void validate() const;
        public:
            Camera(float fieldOfVision, float nearPlane, float farPlane, const Vec3f& position, const Vec3f& direction);
            
            inline bool ortho() const {
                return m_ortho;
            }
            
            inline void setOrtho(bool ortho) {
                if (m_ortho == ortho)
                    return;
                m_ortho = ortho;
                m_valid = false;
            }
            
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline const Vec3f& direction() const {
                return m_direction;
            }
            
            inline const Vec3f& up() const {
                return m_up;
            }
            
            inline const Vec3f& right() const {
                return m_right;
            }
            
            inline float fieldOfVision() const {
                return m_fieldOfVision;
            }
            
            inline void setFieldOfVision(float fieldOfVision) {
                if (fieldOfVision == m_fieldOfVision)
                    return;
                m_fieldOfVision = fieldOfVision;
                m_valid = false;
            }
            
            inline float nearPlane() const {
                return m_nearPlane;
            }
            
            inline void setNearPlane(float nearPlane) {
                if (nearPlane == m_nearPlane)
                    return;
                m_nearPlane = nearPlane;
                m_valid = false;
            }
            
            inline float farPlane() const {
                return m_farPlane;
            }

            inline void setFarPlane(float farPlane) {
                if (farPlane == m_farPlane)
                    return;
                m_farPlane = farPlane;
                m_valid = false;
            }

            inline void setViewport(int x, int y, int width, int height) {
                if (x != m_viewport.x ||
                    y != m_viewport.y ||
                    width != m_viewport.width ||
                    height != m_viewport.height) {
                    m_viewport.x = x;
                    m_viewport.y = y;
                    m_viewport.width = width;
                    m_viewport.height = height;
                    m_valid = false;
                }
            }
            
            void update(int x, int y, int width, int height);
            
            const Vec3f defaultPoint() const;
            const Vec3f defaultPoint(const Vec3f& direction) const;
            const Vec3f defaultPoint(float x, float y) const;
            const Vec3f project(const Vec3f& point) const;
            const Vec3f unproject(float x, float y, float depth) const;
            const Ray pickRay(float x, float y) const;
            
            const Mat4f& matrix() const;
            const Mat4f billboardMatrix(bool fixUp = false) const;
            void frustumPlanes(Plane& top, Plane& right, Plane& bottom, Plane& left) const;

            float distanceTo(const Vec3f& point) const;
            float squaredDistanceTo(const Vec3f& point) const;

            void moveTo(Vec3f position);
            void moveBy(float forward, float right, float up);
            void lookAt(Vec3f point, Vec3f up);
            void setDirection(Vec3f direction, Vec3f up);

            void rotate(float yawAngle, float pitchAngle);
            void orbit(Vec3f center, float hAngle, float vAngle);

        };
    }
}

#endif
