#/*
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

#ifndef TrenchBroom_ScaleObjectsToolController
#define TrenchBroom_ScaleObjectsToolController

#include "Model/Hit.h"
#include "Renderer/EdgeRenderer.h"
#include "View/ToolController.h"

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
        class ScaleObjectsTool;
        
        class ScaleObjectsToolController : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, MouseDragPolicy, RenderPolicy, NoDropPolicy> {
        protected:
            ScaleObjectsTool* m_tool;
        protected:
            ScaleObjectsToolController(ScaleObjectsTool* tool);
        public:
            virtual ~ScaleObjectsToolController();
        private:
            Tool* doGetTool() override;
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;
            
            void doModifierKeyChange(const InputState& inputState) override;
            
            void doMouseMove(const InputState& inputState) override;
            
            bool doStartMouseDrag(const InputState& inputState) override;
            bool doMouseDrag(const InputState& inputState) override;
            void doEndMouseDrag(const InputState& inputState) override;
            void doCancelMouseDrag() override;
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
            Renderer::DirectEdgeRenderer buildEdgeRendererForSide();
            Renderer::DirectEdgeRenderer buildEdgeRendererForEdge();
            
            bool doCancel() override;
            
            bool handleInput(const InputState& inputState) const;
            
            bool updateResize(const InputState& inputState);
        private:
            virtual Model::Hit doPick(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) = 0;
        };
        
        class ScaleObjectsToolController2D : public ScaleObjectsToolController {
        public:
            ScaleObjectsToolController2D(ScaleObjectsTool* tool);
        private:
            Model::Hit doPick(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) override;
        };
        
        class ScaleObjectsToolController3D : public ScaleObjectsToolController {
        public:
            ScaleObjectsToolController3D(ScaleObjectsTool* tool);
        private:
            Model::Hit doPick(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) override;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsToolController) */

