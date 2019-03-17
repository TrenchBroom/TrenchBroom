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

            Model::Hit pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera) const;
            Model::Hit pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera) const;

            vm::vec3 pointHandlePosition(HitArea area, const vm::vec3& cameraPos) const;
            FloatType handleRadius() const;

            vm::vec3 rotationAxis(HitArea area) const;
            vm::vec3 pointHandleAxis(HitArea area, const vm::vec3& cameraPos) const;
        public:
//            void renderAngle(Renderer::RenderContext& renderContext, const HitArea handle, const FloatType angle);
            void renderHandle2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHandle3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight2D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
            void renderHighlight3D(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, HitArea area);
        private:
            Model::Hit pickPointHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const vm::vec3& position, HitArea area) const;
            Model::Hit selectHit(const Model::Hit& closest, const Model::Hit& hit) const;

            vm::vec3 getPointHandlePosition(const vm::vec3& axis) const;
            Color getAngleIndicatorColor(HitArea area) const;
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsHandle) */
