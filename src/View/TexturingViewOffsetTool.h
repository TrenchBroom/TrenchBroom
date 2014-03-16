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

#ifndef __TrenchBroom__TexturingViewOffsetTool__
#define __TrenchBroom__TexturingViewOffsetTool__

#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class OrthographicCamera;
    }
    
    namespace View {
        class TexturingViewOffsetTool : public ToolImpl<NoActivationPolicy, NoPickingPolicy, NoMousePolicy, PlaneDragPolicy, NoDropPolicy, NoRenderPolicy> {
        private:
            Model::BrushFace* m_face;
        public:
            TexturingViewOffsetTool(MapDocumentWPtr document, ControllerWPtr controller);
        private:
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag(const InputState& inputState);
        };
    }
}

#endif /* defined(__TrenchBroom__TexturingViewOffsetTool__) */
