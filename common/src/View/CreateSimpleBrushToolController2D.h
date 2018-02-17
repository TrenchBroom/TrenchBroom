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

#ifndef TrenchBroom_CreateBrushToolController2D
#define TrenchBroom_CreateBrushToolController2D

#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class CreateSimpleBrushTool;
        class Grid;
        
        class CreateSimpleBrushToolController2D : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            CreateSimpleBrushTool* m_tool;
            MapDocumentWPtr m_document;
            Vec3 m_initialPoint;
            BBox3 m_bounds;
        public:
            CreateSimpleBrushToolController2D(CreateSimpleBrushTool* tool, MapDocumentWPtr document);
        private:
            Tool* doGetTool() override;
            
            DragInfo doStartDrag(const InputState& inputState) override;
            DragResult doDrag(const InputState& inputState, const Vec3& lastHandlePosition, const Vec3& nextHandlePosition) override;
            void doEndDrag(const InputState& inputState) override;
            void doCancelDrag() override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            
            bool doCancel() override;
        private:
            bool updateBounds(const InputState& inputState, const Vec3& currentPoint);
            void snapBounds(const InputState& inputState, BBox3& bounds);
        };
    }
}

#endif /* defined(TrenchBroom_CreateBrushToolController2D) */
