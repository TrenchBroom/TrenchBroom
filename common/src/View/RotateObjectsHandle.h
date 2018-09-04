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
            vec3 m_position;
        public:
            const vec3& position() const;
            void setPosition(const vec3& position);
            
            Model::Hit pick2D(const ray3& pickRay, const Renderer::Camera& camera) const;
            Model::Hit pick3D(const ray3& pickRay, const Renderer::Camera& camera) const;

            vec3 pointHandlePosition(const HitArea area, const vec3& cameraPos) const;
            FloatType handleRadius() const;
            
            vec3 rotationAxis(const HitArea area) const;
            vec3 pointHandleAxis(const HitArea area, const vec3& cameraPos) const;
        public:
//            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle);
            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
            void renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
        private:
            template <typename T>
            void computeAxes(const vec<T,3>& cameraPos, vec<T,3>& xAxis, vec<T,3>& yAxis, vec<T,3>& zAxis) const {
                const auto viewDir = vec<T,3>(normalize(m_position - vec3(cameraPos)));
                if (Math::eq(std::abs(viewDir.z()), static_cast<T>(1.0))) {
                    xAxis = vec<T,3>::pos_x;
                    yAxis = vec<T,3>::pos_y;
                } else {
                    xAxis = Math::pos(viewDir.x()) ? vec<T,3>::neg_x : vec<T,3>::pos_x;
                    yAxis = Math::pos(viewDir.y()) ? vec<T,3>::neg_y : vec<T,3>::pos_y;
                }
                zAxis = Math::pos(viewDir.z()) ? vec<T,3>::neg_z : vec<T,3>::pos_z;
            }

            Model::Hit pickPointHandle(const ray3& pickRay, const Renderer::Camera& camera, const vec3& position, const HitArea area) const;
            Model::Hit selectHit(const Model::Hit& closest, const Model::Hit& hit) const;
            
            vec3 getPointHandlePosition(const vec3& axis) const;
            Color getAngleIndicatorColor(const HitArea area) const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsHandle) */
