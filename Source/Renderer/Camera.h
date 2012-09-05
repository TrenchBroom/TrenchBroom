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

#include "Utility/GLee.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class Camera {
        protected:
            Vec3f m_position;
            Vec3f m_direction;
            Vec3f m_up;
            Vec3f m_right;
            float m_fieldOfVision;
            float m_nearPlane;
            float m_farPlane;
            GLint m_viewport[4];
            GLdouble m_modelview[16];
            GLdouble m_projection[16];
        public:
            Camera(float fieldOfVision, float nearPlane, float farPlane, const Vec3f& position, const Vec3f& direction);
            
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
                m_fieldOfVision = fieldOfVision;
            }
            
            inline float nearPlane() const {
                return m_fieldOfVision;
            }
            
            inline void setNearPlane(float nearPlane) {
                m_nearPlane = nearPlane;
            }
            
            inline float farPlane() const {
                return m_farPlane;
            }

            inline void setFarPlane(float farPlane) {
                m_farPlane = farPlane;
            }

            const Vec3f defaultPoint() const;
            const Vec3f defaultPoint(const Vec3f& direction) const;
            const Vec3f defaultPoint(float x, float y) const;
            const Vec3f project(const Vec3f& point) const;
            const Vec3f unproject(float x, float y, float depth) const;
            const Ray pickRay(float x, float y) const;
            void update(float x, float y, float width, float height);
            void setBillboard();

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
