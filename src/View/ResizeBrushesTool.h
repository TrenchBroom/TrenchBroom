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

#ifndef __TrenchBroom__ResizeBrushesTool__
#define __TrenchBroom__ResizeBrushesTool__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "Model/Picker.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace Renderer {
        class EdgeRenderer;
    }
    
    namespace View {
        class ResizeBrushesTool : public Tool<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, RenderPolicy> {
        private:
            Vec3 m_totalDelta;
            Vec3 m_dragOrigin;
        public:
            static const Model::Hit::HitType ResizeHit;
            ResizeBrushesTool(BaseTool* next, MapDocumentPtr document, ControllerFacade& controller);
        private:
            void doPick(const InputState& inputState, Model::PickResult& pickResult) const;

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            bool applies(const InputState& inputState) const;
            void pickNearFaceHit(const InputState& inputState, Model::PickResult& pickResult) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace& dragFace) const;
            Renderer::EdgeRenderer buildEdgeRenderer(const Model::BrushFaceList& faces) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesTool__) */
