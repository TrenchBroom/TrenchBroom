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

#ifndef TrenchBroom_ClipToolController
#define TrenchBroom_ClipToolController

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/RenderContext.h"
#include "View/ClipTool.h"
#include "View/InputState.h"
#include "View/ToolController.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class PickResult;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class ClipTool;
        class Grid;
        class InputState;
        class Tool;
        
        class ClipToolController : public ToolControllerGroup {
        protected:
            class MoveClipPointBase : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
            protected:
                ClipTool* m_tool;
                MoveClipPointBase(ClipTool* tool);
            public:
                virtual ~MoveClipPointBase();
            private:
                Tool* doGetTool();
                DragInfo doStartDrag(const InputState& inputState);
                DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
                void doEndDrag(const InputState& inputState);
                void doCancelDrag();
                bool doCancel();
            private:
                virtual DragRestricter* doCreateDragRestricter(const InputState& inputState, const Vec3& initialPoint) const = 0;
                virtual DragSnapper* doCreateDragSnapper(const InputState& inputState) const = 0;
                virtual Vec3::List doGetHelpVectors(const InputState& inputState) const = 0;
            };
        protected:
            ClipTool* m_tool;
        protected:
            ClipToolController(ClipTool* tool);
            virtual ~ClipToolController();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            bool doMouseClick(const InputState& inputState);

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
            bool doCancel();
        private: // subclassing interface
            virtual bool doAddClipPoint(const InputState& inputState) = 0;
            virtual void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };
        
        class ClipToolController2D : public ClipToolController {
        private:
            class MoveClipPointPart;
        public:
            ClipToolController2D(ClipTool* tool);
        private:
            bool doAddClipPoint(const InputState& inputState);
            void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
        
        class ClipToolController3D : public ClipToolController {
        private:
            static Vec3::List selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint);
            static Model::BrushFaceList selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint);
        private:
            class MoveClipPointPart;
        public:
            ClipToolController3D(ClipTool* tool);
        private:
            bool doMouseDoubleClick(const InputState& inputState);

            bool doAddClipPoint(const InputState& inputState);
            void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
    }
}

#endif /* defined(TrenchBroom_ClipToolController) */
