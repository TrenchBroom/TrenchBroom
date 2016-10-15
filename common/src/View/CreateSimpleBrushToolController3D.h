/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_CreateBrushToolController3D
#define TrenchBroom_CreateBrushToolController3D

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Polyhedron.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class CreateSimpleBrushTool;
        class Grid;

        class CreateSimpleBrushToolController3D : public ToolControllerBase<NoPickingPolicy, KeyPolicy, NoMousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            CreateSimpleBrushTool* m_tool;
            MapDocumentWPtr m_document;
            
            Vec3 m_initialPoint;
        public:
            CreateSimpleBrushToolController3D(CreateSimpleBrushTool* tool, MapDocumentWPtr document);
        private:
            Tool* doGetTool();

            void doModifierKeyChange(const InputState& inputState);

            DragInfo doStartDrag(const InputState& inputState);
            DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
            void doEndDrag(const InputState& inputState);
            void doCancelDrag();

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        private:
            void updateBounds(const Vec3& point, Vec3 cameraPosition);
        };
    }
}

#endif /* defined(TrenchBroom_CreateBrushToolController3D) */
