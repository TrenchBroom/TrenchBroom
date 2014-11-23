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
#include "Renderer/PointHandleRenderer.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class RotateObjectsHandle {
        public:
            typedef enum {
                HitArea_None,
                HitArea_Center,
                HitArea_XAxis,
                HitArea_YAxis,
                HitArea_ZAxis
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
            Vec3 m_position;
        public:
            const Vec3& position() const;
            void setPosition(const Vec3& position);
            
            Hit pick(const Ray3& pickRay, const Renderer::Camera& camera) const;
        
            Vec3 getPointHandlePosition(const HitArea area, const Vec3& cameraPos) const;
            Vec3 getPointHandleAxis(const HitArea area, const Vec3& cameraPos) const;
            Vec3 getRotationAxis(const HitArea area, const Vec3& cameraPos) const;
        public:
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea highlight);
//            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle);
        private:
            void render2DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea highlight);
            void render3DHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea highlight);
            
            template <typename T>
            void computeAxes(const Vec<T,3>& cameraPos, Vec<T,3>& xAxis, Vec<T,3>& yAxis, Vec<T,3>& zAxis) const {
                const Vec<T,3> viewDir = (m_position - cameraPos).normalized();
                if (Math::eq(std::abs(viewDir.z()), static_cast<T>(1.0))) {
                    xAxis = Vec<T,3>::PosX;
                    yAxis = Vec<T,3>::PosY;
                } else {
                    xAxis = Math::pos(viewDir.x()) ? Vec<T,3>::NegX : Vec<T,3>::PosX;
                    yAxis = Math::pos(viewDir.y()) ? Vec<T,3>::NegY : Vec<T,3>::PosY;
                }
                zAxis = Math::pos(viewDir.z()) ? Vec<T,3>::NegZ : Vec<T,3>::PosZ;
            }

            Hit pickPointHandle(const Ray3& pickRay, const Renderer::Camera& camera, const Vec3& position, const HitArea area) const;
            Hit selectHit(const Hit& closest, const Hit& hit) const;
            
            Vec3 getPointHandlePosition(const Vec3& axis) const;
            Color getAngleIndicatorColor(const HitArea area) const;
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsHandle__) */
