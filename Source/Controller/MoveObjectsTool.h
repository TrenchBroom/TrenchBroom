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

#ifndef __TrenchBroom__MoveObjectsTool__
#define __TrenchBroom__MoveObjectsTool__

#include "Controller/Tool.h"
#include "Controller/ObjectsHandle.h"
#include "Model/Picker.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type MoveObjectsHandleHit    = 1 << 3;
        }
        
        class MoveObjectsHandleHit : public Hit {
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
            MoveObjectsHandleHit(const Vec3f& hitPoint, float distance, HitArea hitArea);
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
        class MoveObjectsTool : public PlaneDragTool, ObjectsHandle<Model::MoveObjectsHandleHit> {
        protected:
            typedef enum {
                RNone,
                RXAxis,
                RYAxis,
                RZAxis
            } RestrictToAxis;

            float m_axisLength;
            float m_planeRadius;
            Model::MoveObjectsHandleHit* m_lastHit;
            Vec3f m_totalDelta;
            RestrictToAxis m_restrictToAxis;
            
            Model::MoveObjectsHandleHit* pickAxis(const Ray& ray, Vec3f& axis, Model::MoveObjectsHandleHit::HitArea hitArea);
            Model::MoveObjectsHandleHit* pickPlane(const Ray& ray, const Vec3f& normal, const Vec3f& axis1, const Vec3f& axis2, Model::MoveObjectsHandleHit::HitArea hitArea);
            
            void renderAxes(Model::MoveObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            void renderPlanes(Model::MoveObjectsHandleHit* hit, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);

            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            void handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint);
            void handleEndPlaneDrag(InputState& inputState);

            void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet);
        public:
            MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, float axisLength, float planeRadius);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsTool__) */
