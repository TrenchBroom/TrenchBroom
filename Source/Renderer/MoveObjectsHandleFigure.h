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

#ifndef __TrenchBroom__MoveObjectsHandleFigure__
#define __TrenchBroom__MoveObjectsHandleFigure__

#include "Renderer/Figure.h"

#include "Utility/VecMath.h"

#include <cassert>
#include <limits>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class MoveObjectsHandleFigure : public Figure {
        public:
            class Hit {
            public:
                typedef enum {
                    TNone,
                    TXAxis,
                    TYAxis,
                    TZAxis,
                    TXYPlane,
                    TXZPlane,
                    TYZPlane
                } Type;
            private:
                Type m_type;
                Vec3f m_hitPoint;
                float m_distance;

                Hit(Type type, const Vec3f& hitPoint, float distance) :
                m_type(type),
                m_hitPoint(hitPoint),
                m_distance(distance) {}
            public:
                static Hit noHit() {
                    return Hit(TNone, Vec3f(), std::numeric_limits<float>::max());
                }
                
                static Hit hit(Type type, const Vec3f& hitPoint, float distance) {
                    assert(type != TNone);
                    return Hit(type, hitPoint, distance);
                }
                
                inline Type type() const {
                    return m_type;
                }
                
                inline const Vec3f& hitPoint() const {
                    return m_hitPoint;
                }
                
                inline float distance() const {
                    return m_distance;
                }
            };
        protected:
            float m_axisLength;
            float m_planeSize;
            Hit::Type m_lastHit;
            Vec3f m_position;
            
            bool m_locked;
            Vec3f m_xAxis, m_yAxis, m_zAxis;
            
            void axes(const Vec3f& origin, Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis);
            Hit pickAxis(const Ray& ray, const Vec3f& axis, Hit::Type type);
        public:
            MoveObjectsHandleFigure(float axisLength, float planeSize);

            Hit pick(const Ray& ray);
            
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline void setPosition(const Vec3f& position) {
                m_position = position;
            }
            
            inline void setHitType(Hit::Type type) {
                m_lastHit = type;
            }
            
            inline void setLocked(bool locked) {
                m_locked = locked;
            }
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsHandleFigure__) */
