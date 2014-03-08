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
        class ResizeBrushesTool : public ToolImpl<NoActivationPolicy, PickingPolicy, MousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        private:
            Model::BrushFaceList m_dragFaces;
            Vec3 m_totalDelta;
            Vec3 m_dragOrigin;
            bool m_splitBrushes;
        public:
            static const Hit::HitType ResizeHit;
            ResizeBrushesTool(MapDocumentWPtr document, ControllerWPtr controller);
        private:
            void doPick(const InputState& inputState, Hits& hits);

            void doModifierKeyChange(const InputState& inputState);
            void doMouseMove(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            bool applies(const InputState& inputState) const;
            bool splitBrushes(const InputState& inputState) const;
            
            void pickNearFaceHit(const InputState& inputState, Hits& hits) const;
            
            void updateDragFaces(const InputState& inputState);
            Model::BrushFaceList collectDragFaces(Model::BrushFace& dragFace) const;
            Renderer::EdgeRenderer buildEdgeRenderer(const Model::BrushFaceList& faces) const;
            
            bool splitBrushes(const Vec3& delta);
            Model::BrushFaceList findMatchingFaces(const Model::BrushList& brushes, const Model::BrushFaceList& faces) const;
            Model::BrushFace* findMatchingFace(const Model::Brush& brush, const Model::BrushFace& face) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesTool__) */
