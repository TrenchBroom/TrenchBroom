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

#ifndef __TrenchBroom__ClipToolAdapter__
#define __TrenchBroom__ClipToolAdapter__

#include "Hit.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class ClipTool;
        class InputState;
        
        class ClipToolAdapter : public ToolAdapterBase<PickingPolicy, NoKeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ClipTool* m_tool;
        protected:
            ClipToolAdapter(ClipTool* tool);
        public:
            virtual ~ClipToolAdapter();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Hits& hits);
            
            bool doMouseClick(const InputState& inputState);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        private:
            virtual Hit doPick(const Ray3& pickRay) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__ClipToolAdapter__) */
