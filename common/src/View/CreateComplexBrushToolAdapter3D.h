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

#ifndef TrenchBroom_CreateComplexBrushToolAdapter3D
#define TrenchBroom_CreateComplexBrushToolAdapter3D

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Polyhedron.h"
#include "View/ToolAdapter.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class CreateComplexBrushTool;
        class Grid;

        class CreateComplexBrushToolAdapter3D : public ToolAdapterBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, DelegatingMouseDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            CreateComplexBrushTool* m_tool;
            MapDocumentWPtr m_document;
            
            class DragDelegate;
            class DrawFaceDelegate;
            class DuplicateFaceDelegate;
            
            Polyhedron3 m_polyhedron;
        public:
            CreateComplexBrushToolAdapter3D(CreateComplexBrushTool* tool, MapDocumentWPtr document);
        public:
            void performCreateBrush();
        private:
            Tool* doGetTool();

            bool doMouseClick(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            
            MouseDragPolicy* doCreateDelegate(const InputState& inputState);
            void doDeleteDelegate(MouseDragPolicy* delegate);
            
            void doMouseDragStarted();
            void doMouseDragged();
            void doMouseDragCancelled();
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_CreateComplexBrushToolAdapter3D) */
