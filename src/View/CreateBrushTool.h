/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__CreateBrushTool__
#define __TrenchBroom__CreateBrushTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/BrushRenderer.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    
    namespace Renderer {
        class BrushRenderer;
        class RenderContext;
    }
    
    namespace View {
        class CreateBrushTool : public Tool<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, PlaneDragPolicy, RenderPolicy> {
        private:
            Vec3 m_initialPoint;
            Renderer::BrushRenderer m_brushRenderer;
            Model::Brush* m_brush;
        public:
            CreateBrushTool(BaseTool* next, MapDocumentPtr document, ControllerPtr controller);
        private:
            void doModifierKeyChange(const InputState& inputState);
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag(const InputState& inputState);

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);

            BBox3 computeBounds(const Vec3& point1, const Vec3& point2) const;
            Model::Brush* createBrush(const BBox3& bounds) const;
            void updateBrushRenderer();
            void addBrushToMap(Model::Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__CreateBrushTool__) */
