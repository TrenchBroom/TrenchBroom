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
                HitArea_None    = 0,
                HitArea_Center  = 1,
                HitArea_XAxis   = 2,
                HitArea_YAxis   = 4,
                HitArea_ZAxis   = 8
            } HitArea;
        private:
            vm::vec3 m_position;
        public:
            const vm::vec3& position() const;
            void setPosition(const vm::vec3& position);
            
            Model::Hit pick2D(const ray3& pickRay, const Renderer::Camera& camera) const;
            Model::Hit pick3D(const ray3& pickRay, const Renderer::Camera& camera) const;

            vm::vec3 pointHandlePosition(const HitArea area, const vm::vec3& cameraPos) const;
            FloatType handleRadius() const;
            
            vm::vec3 rotationAxis(const HitArea area) const;
            vm::vec3 pointHandleAxis(const HitArea area, const vm::vec3& cameraPos) const;
        public:
//            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle);
            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
            void renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
        private:
            template <typename T>
            void computeAxes(const vm::vec<T,3>& cameraPos, vm::vec<T,3>& xAxis, vm::vec<T,3>& yAxis, vm::vec<T,3>& zAxis) const {
                const auto viewDir = vm::vec<T,3>(vm::normalize(m_position - vm::vec3(cameraPos)));
                if (Math::eq(std::abs(viewDir.z()), static_cast<T>(1.0))) {
                    xAxis = vm::vec<T,3>::pos_x;
                    yAxis = vm::vec<T,3>::pos_y;
                } else {
                    xAxis = Math::pos(viewDir.x()) ? vm::vec<T,3>::neg_x : vm::vec<T,3>::pos_x;
                    yAxis = Math::pos(viewDir.y()) ? vm::vec<T,3>::neg_y : vm::vec<T,3>::pos_y;
                }
                zAxis = Math::pos(viewDir.z()) ? vm::vec<T,3>::neg_z : vm::vec<T,3>::pos_z;
            }

            Model::Hit pickPointHandle(const ray3& pickRay, const Renderer::Camera& camera, const vm::vec3& position, const HitArea area) const;
            Model::Hit selectHit(const Model::Hit& closest, const Model::Hit& hit) const;
            
            vm::vec3 getPointHandlePosition(const vm::vec3& axis) const;
            Color getAngleIndicatorColor(const HitArea area) const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsHandle) */
