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

#ifndef TrenchBroom_RotateObjectsToolController
#define TrenchBroom_RotateObjectsToolController

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
        class RotateObjectsTool;
        
        class RotateObjectsToolController : public ToolControllerGroup {
        protected:
            class MoveCenterBase;
            class RotateObjectsBase;
        protected:
            RotateObjectsTool* m_tool;
        protected:
            RotateObjectsToolController(RotateObjectsTool* tool);
        public:
            virtual ~RotateObjectsToolController();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
        private: // subclassing interface
            virtual Model::Hit doPick(const InputState& inputState) = 0;
            virtual void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };
        
        class RotateObjectsToolController2D : public RotateObjectsToolController {
        private:
            class MoveCenterPart;
            class RotateObjectsPart;
        public:
            RotateObjectsToolController2D(RotateObjectsTool* tool);
        private:
            Model::Hit doPick(const InputState& inputState);
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
        
        class RotateObjectsToolController3D : public RotateObjectsToolController {
        private:
            class MoveCenterPart;
            class RotateObjectsPart;
        public:
            RotateObjectsToolController3D(RotateObjectsTool* tool);
        private:
            Model::Hit doPick(const InputState& inputState);
            void doRenderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(TrenchBroom_RotateObjectsToolController) */
