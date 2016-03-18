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

#ifndef TrenchBroom_CreateComplexBrushToolController3D
#define TrenchBroom_CreateComplexBrushToolController3D

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Polyhedron.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class CreateComplexBrushTool;
        class Grid;

        class CreateComplexBrushToolController3D : public ToolControllerGroup {
        private:
            class Part;
            class DrawFacePart;
            class DuplicateFacePart;
            
            CreateComplexBrushTool* m_tool;
        public:
            CreateComplexBrushToolController3D(CreateComplexBrushTool* tool);
        private:
            Tool* doGetTool();

            bool doMouseClick(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            
            bool doShouldHandleMouseDrag(const InputState& inputState) const;

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_CreateComplexBrushToolController3D) */
