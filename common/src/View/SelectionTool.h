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

#ifndef TrenchBroom_SelectionTool
#define TrenchBroom_SelectionTool

#include "Model/Hit.h"
#include "View/Tool.h"
#include "View/ToolAdapter.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        
        class SelectionTool : public ToolAdapterBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy>, public Tool {
        private:
            MapDocumentWPtr m_document;
        public:
            SelectionTool(MapDocumentWPtr document);
        private:
            Tool* doGetTool();
            
            bool doMouseClick(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            
            bool handleClick(const InputState& inputState) const;
            bool isFaceClick(const InputState& inputState) const;
            bool isMultiClick(const InputState& inputState) const;
            
            const Model::Hit& firstHit(const InputState& inputState, Model::Hit::HitType type) const;
            
            void doMouseScroll(const InputState& inputState);
            void adjustGrid(const InputState& inputState);
            void drillSelection(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_SelectionTool) */
