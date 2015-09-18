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

#ifndef TrenchBroom_RotateObjectsHandle
#define TrenchBroom_RotateObjectsHandle

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Model/Hit.h"
#include "Renderer/PointHandleRenderer.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        
        class RotateObjectsHandle {
        public:
            static const Model::Hit::HitType HandleHit;

            typedef enum {
                HitArea_None,
                HitArea_Center,
                HitArea_XAxis,
                HitArea_YAxis,
                HitArea_ZAxis
            } HitArea;
        private:
            Vec3 m_position;
        public:
            const Vec3& position() const;
            void setPosition(const Vec3& position);
            
            Model::Hit pick2D(const Ray3& pickRay, const Renderer::Camera& camera) const;
            Model::Hit pick3D(const Ray3& pickRay, const Renderer::Camera& camera) const;

            Vec3 pointHandlePosition(const HitArea area, const Vec3& cameraPos) const;
            
            Vec3 rotationAxis(const HitArea area) const;
            Vec3 pointHandleAxis(const HitArea area, const Vec3& cameraPos) const;
        public:
//            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle);
            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea highlight);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea highlight);
        private:
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

            Model::Hit pickPointHandle(const Ray3& pickRay, const Renderer::Camera& camera, const Vec3& position, const HitArea area) const;
            Model::Hit selectHit(const Model::Hit& closest, const Model::Hit& hit) const;
            
            Vec3 getPointHandlePosition(const Vec3& axis) const;
            Color getAngleIndicatorColor(const HitArea area) const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsHandle) */
