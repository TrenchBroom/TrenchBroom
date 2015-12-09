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

#ifndef TrenchBroom_ResizeBrushesToolAdapter
#define TrenchBroom_ResizeBrushesToolAdapter

#include "Model/Hit.h"
#include "Renderer/EdgeRenderer.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class ResizeBrushesTool;
        
        class ResizeBrushesToolAdapter : public ToolAdapterBase<PickingPolicy, KeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ResizeBrushesTool* m_tool;
        protected:
            ResizeBrushesToolAdapter(ResizeBrushesTool* tool);
        public:
            virtual ~ResizeBrushesToolAdapter();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            void doModifierKeyChange(const InputState& inputState);
            
            void doMouseMove(const InputState& inputState);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            Renderer::DirectEdgeRenderer buildEdgeRenderer();
            
            bool doCancel();

            void updateDragFaces(const InputState& inputState);
            bool handleInput(const InputState& inputState) const;
        private:
            virtual Model::Hit doPick(const Ray3& pickRay, const Model::PickResult& pickResult) = 0;
        };
        
        class ResizeBrushesToolAdapter2D : public ResizeBrushesToolAdapter {
        public:
            ResizeBrushesToolAdapter2D(ResizeBrushesTool* tool);
        private:
            Model::Hit doPick(const Ray3& pickRay, const Model::PickResult& pickResult);
        };
        
        class ResizeBrushesToolAdapter3D : public ResizeBrushesToolAdapter {
        public:
            ResizeBrushesToolAdapter3D(ResizeBrushesTool* tool);
        private:
            Model::Hit doPick(const Ray3& pickRay, const Model::PickResult& pickResult);
        };
    }
}

#endif /* defined(TrenchBroom_ResizeBrushesToolAdapter) */
