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

#ifndef __TrenchBroom__RotateHandle__
#define __TrenchBroom__RotateHandle__

#include "Controller/ObjectsHandle.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type RotateHandleHit    = 1 << 4;
        }
        
        class RotateHandleHit : public Hit {
        public:
            typedef enum {
                HAXAxis,
                HAYAxis,
                HAZAxis
            } HitArea;
        private:
            HitArea m_hitArea;
        public:
            RotateHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea);
            bool pickable(Filter& filter) const;
            
            inline HitArea hitArea() const {
                return m_hitArea;
            }
        };
    }
    
    namespace Renderer {
        class Vbo;
        class RenderContext;
    }

    namespace Controller {
        class RotateHandle : public ObjectsHandle<Model::RotateHandleHit> {
        protected:
            const float m_axisLength;
            const float m_ringRadius;
            const float m_ringThickness;

            Model::RotateHandleHit* pickRing(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::RotateHandleHit::HitArea hitArea);
            
            void renderAxis(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context);
            void renderRing(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& context, float angle);
        public:
            RotateHandle(float axisLength, float ringRadius, float ringThickness);
            ~RotateHandle();

            Model::RotateHandleHit* pick(const Ray& ray);
            void render(Model::RotateHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext, float angle);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateHandle__) */
