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

#ifndef __TrenchBroom__MoveHandle__
#define __TrenchBroom__MoveHandle__

#include "Controller/ObjectsHandle.h"
#include "Model/Picker.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type MoveHandleHit    = 1 << 3;
        }
        
        class MoveHandleHit : public Hit {
        public:
            typedef enum {
                HAXAxis,
                HAYAxis,
                HAZAxis,
                HAXYPlane,
                HAXZPlane,
                HAYZPlane
            } HitArea;
        private:
            HitArea m_hitArea;
        public:
            MoveHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea);
            bool pickable(Filter& filter) const;
            
            inline HitArea hitArea() const {
                return m_hitArea;
            }
        };
    }

    namespace Renderer {
        class RenderContext;
        class Vbo;
    }
    
    namespace Controller {
        class MoveHandle : public ObjectsHandle<Model::MoveHandleHit> {
        public:
            typedef enum {
                RNone,
                RXAxis,
                RYAxis,
                RZAxis
            } RestrictToAxis;
        protected:
            float m_axisLength;
            float m_planeRadius;
            
            Model::MoveHandleHit* pickAxis(const Ray& ray, Vec3f& axis, Model::MoveHandleHit::HitArea hitArea);
            Model::MoveHandleHit* pickPlane(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::MoveHandleHit::HitArea hitArea);
            
            void renderAxes(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            void renderPlanes(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
        public:
            MoveHandle(float axisLength, float planeRadius);
            
            Model::MoveHandleHit* pick(const Ray& ray);
            void render(Model::MoveHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveHandle__) */
