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

#ifndef TrenchBroom_ScaleObjectsToolController
#define TrenchBroom_ScaleObjectsToolController

#include "Model/Hit.h"
#include "View/RotateObjectsHandle.h"
#include "View/RotateObjectsHandle.h"
#include "View/ToolController.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace View {
        class MovementRestriction;
        class ScaleObjectsTool;
        
        class ScaleObjectsToolController : public ToolControllerGroup {
        protected:
            class MoveCenterBase;
            class RotateObjectsBase;
        protected:
            ScaleObjectsTool* m_tool;
        protected:
            ScaleObjectsToolController(ScaleObjectsTool* tool);
        public:
            virtual ~ScaleObjectsToolController();
        private:
            Tool* doGetTool() override;
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doCancel() override;
        private: // subclassing interface
            virtual Model::Hit doPick(const InputState& inputState) = 0;
            virtual void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };
        
        class ScaleObjectsToolController2D : public ScaleObjectsToolController {
        private:
            class MoveCenterPart;
            class RotateObjectsPart;
        public:
            ScaleObjectsToolController2D(ScaleObjectsTool* tool);
        private:
            Model::Hit doPick(const InputState& inputState) override;
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
        };
        
        class ScaleObjectsToolController3D : public ScaleObjectsToolController {
        private:
            class MoveCenterPart;
            class RotateObjectsPart;
        public:
            ScaleObjectsToolController3D(ScaleObjectsTool* tool);
        private:
            Model::Hit doPick(const InputState& inputState) override;
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsToolController) */
