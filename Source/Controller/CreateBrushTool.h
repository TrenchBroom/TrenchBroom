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

#ifndef __TrenchBroom__CreateBrushTool__
#define __TrenchBroom__CreateBrushTool__

#include "Controller/Tool.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    
    namespace Renderer {
        class BrushFigure;
    }
    
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class CreateBrushTool : public PlaneDragTool {
        protected:
            Model::VisibleFilter m_filter;
            Vec3f m_initialPoint;
            BBox m_bounds;
            Vec3f m_normal;
            int m_thickness;
            bool m_boundsChanged;
            Model::Brush* m_brush;
            Renderer::BrushFigure* m_brushFigure;

            void updateBoundsThickness();
            void updateBounds(const Vec3f& currentPoint);
            
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);

            void handleScroll(InputState& inputState);

            bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint);
            bool handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint);
            void handleEndPlaneDrag(InputState& inputState);
        public:
            CreateBrushTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);
        };
    }
}

#endif /* defined(__TrenchBroom__CreateBrushTool__) */
