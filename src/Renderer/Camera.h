/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Camera__
#define __TrenchBroom__Camera__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera {
        public:
            struct Viewport {
                int x, y;
                unsigned int width, height;

                Viewport();
                Viewport(int i_x, int i_y, unsigned int i_width, unsigned int i_height);
            };
        private:
            float m_fov;
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
            mutable bool m_valid;
        public:
            Camera();
            Camera(const float fov, const float nearPlane, const float farPlane, const Viewport& viewport, const Vec3f& position, const Vec3f& direction, const Vec3f& up);
        
            float fov() const;
            float nearPlane() const;
            float farPlane() const;
            const Viewport& viewport() const;
            const Vec3f& position() const;
            const Vec3f& direction() const;
            const Vec3f& up() const;
            const Vec3f& right() const;
            const Mat4x4f& projectionMatrix() const;
            const Mat4x4f& viewMatrix() const;
            
            Ray3f viewRay() const;
            
            void setFov(const float fov);
            void setNearPlane(const float nearPlane);
            void setFarPlane(const float farPlane);
            void setViewport(const Viewport& viewport);
            void moveTo(const Vec3f& position);
            void moveBy(const Vec3f& delta);
            void lookAt(const Vec3f& point, const Vec3f& up);
            void setDirection(const Vec3f& direction, const Vec3f& up);
            void rotate(const float yaw, const float pitch);
            void orbit(const Vec3f& center, const float horizontal, const float vertical);
        private:
            void validateMatrices() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Camera__) */
