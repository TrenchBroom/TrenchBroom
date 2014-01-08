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

#ifndef __TrenchBroom__RotateObjectsHandle__
#define __TrenchBroom__RotateObjectsHandle__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class RotateObjectsHandle {
        public:
            typedef enum {
                HACenter,
                HAXAxis,
                HAYAxis,
                HAZAxis,
                HANone
            } HitArea;
            
            class Hit {
            private:
                HitArea m_area;
                FloatType m_distance;
                Vec3 m_point;
            public:
                Hit();
                Hit(const HitArea area, const FloatType distance, const Vec3& point);
                
                bool matches() const;
                HitArea area() const;
                FloatType distance() const;
                const Vec3& point() const;
            };
        private:
            mutable Renderer::Vbo m_vbo;
            
            bool m_locked;
            Vec3 m_position;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            Vec3 m_zAxis;
        public:
            RotateObjectsHandle();
            
            void setLocked(const bool locked);
            void setPosition(const Vec3& position);
            void updateAxes(const Vec3& viewPos);
            
            Hit pick(const Ray3& pickRay) const;
            Vec3 getPointHandlePosition(const HitArea area) const;
            Vec3 getPointHandleAxis(const HitArea area) const;
            Vec3 getRotationAxis(const HitArea area) const;
            void renderHandle(Renderer::RenderContext& renderContext, const HitArea highlight) const;
            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle) const;
        private:
            Hit pickPointHandle(const Ray3& pickRay, const Vec3& position, const HitArea area) const;
            Hit selectHit(const Hit& closest, const Hit& hit) const;
            
            void renderAxes(Renderer::RenderContext& renderContext) const;
            void renderRings(Renderer::RenderContext& renderContext) const;
            void renderRingIndicators(Renderer::RenderContext& renderContext) const;
            void renderPointHandles(Renderer::RenderContext& renderContext) const;
            void renderPointHandle(Renderer::RenderContext& renderContext, const Vec3& position, const Color& color) const;
            void renderPointHandleHighlight(Renderer::RenderContext& renderContext, const HitArea highlight) const;
            void renderPointHandleHighlight(Renderer::RenderContext& renderContext, const Vec3& position) const;
            
            Vec3 getPointHandlePosition(const Vec3& axis) const;
            Color getAngleIndicatorColor(const HitArea area) const;
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsHandle__) */
