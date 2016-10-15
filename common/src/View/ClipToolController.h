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
            class Callback {
            protected:
                ClipTool* m_tool;
            public:
                Callback(ClipTool* tool);
                virtual ~Callback();
                ClipTool* tool() const;
                
                bool addClipPoint(const InputState& inputState, Vec3& position);
                bool setClipFace(const InputState& inputState);
                
                virtual DragRestricter* createDragRestricter(const InputState& inputState, const Vec3& initialPoint) const = 0;
                virtual DragSnapper* createDragSnapper(const InputState& inputState) const = 0;
                virtual Vec3::List getHelpVectors(const InputState& inputState) const = 0;
                
                void renderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            private:
                virtual bool doGetNewClipPointPosition(const InputState& inputState, Vec3& position) const = 0;
            };
            
            class PartBase {
            protected:
                Callback* m_callback;
                PartBase(Callback* callback);
            public:
                virtual ~PartBase();
            };
            
            class AddClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, MousePolicy, RestrictedDragPolicy, RenderPolicy, NoDropPolicy>, protected PartBase {
            private:
                bool m_secondPointSet;
            public:
                AddClipPointPart(Callback* callback);
            private:
                Tool* doGetTool();
                bool doMouseClick(const InputState& inputState);
                bool doMouseDoubleClick(const InputState& inputState);
                DragInfo doStartDrag(const InputState& inputState);
                DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
                void doEndDrag(const InputState& inputState);
                void doCancelDrag();
                void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
                bool doCancel();
            };
            
            class MoveClipPointPart : public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy>, protected PartBase {
            public:
                MoveClipPointPart(Callback* callback);
            private:
                Tool* doGetTool();
                DragInfo doStartDrag(const InputState& inputState);
                DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint);
                void doEndDrag(const InputState& inputState);
                void doCancelDrag();
                bool doCancel();
            };
        protected:
            ClipTool* m_tool;
        protected:
            ClipToolController(ClipTool* tool);
            virtual ~ClipToolController();
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            
            bool doCancel();
        };
        
        class ClipToolController2D : public ClipToolController {
        private:
            class Callback2D;
        public:
            ClipToolController2D(ClipTool* tool);
        };
        
        class ClipToolController3D : public ClipToolController {
        private:
            static Vec3::List selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint);
            static Model::BrushFaceList selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint);
        private:
            class Callback3D;
        public:
            ClipToolController3D(ClipTool* tool);
        };
    }
}

#endif /* defined(TrenchBroom_ClipToolController) */
