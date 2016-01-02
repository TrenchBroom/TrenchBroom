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

#ifndef TrenchBroom_ClipToolAdapter
#define TrenchBroom_ClipToolAdapter

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/RenderContext.h"
#include "View/ClipTool.h"
#include "View/InputState.h"
#include "View/ToolAdapter.h"

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
        
        template <typename DragPolicy>
        class ClipToolAdapter : public ToolAdapterBase<PickingPolicy, NoKeyPolicy, MousePolicy, DragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            typedef ToolAdapterBase<PickingPolicy, NoKeyPolicy, MousePolicy, DragPolicy, RenderPolicy, NoDropPolicy> Super;
        protected:
            ClipTool* m_tool;
            const Grid& m_grid;
        protected:
            ClipToolAdapter(ClipTool* tool, const Grid& grid) :
            m_tool(tool),
            m_grid(grid) {}
            
            virtual ~ClipToolAdapter() {}
        private:
            Tool* doGetTool() {
                return m_tool;
            }
            
            bool doMouseClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                    return false;
                return doAddClipPoint(inputState);
            }

            bool doMouseDoubleClick(const InputState& inputState) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft) ||
                    !inputState.modifierKeysPressed(ModifierKeys::MKNone))
                    return false;
                return doSetClipPlane(inputState);
            }

            void doPick(const InputState& inputState, Model::PickResult& pickResult) {
                m_tool->pick(inputState.pickRay(), inputState.camera(), pickResult);
            }
            
            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
                renderContext.setHideSelection();
                renderContext.setForceHideSelectionGuide();
            }
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->render(renderContext, renderBatch, inputState.pickResult());
                doRenderFeedback(inputState, renderContext, renderBatch);
            }
            
            bool doCancel() {
                return m_tool->removeLastPoint() || m_tool->reset();
            }
        protected:
            bool startDrag(const InputState& inputState) {
                if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                    inputState.modifierKeys() != ModifierKeys::MKNone)
                    return false;
                return true;
            }
        private: // subclassing interface
            virtual bool doAddClipPoint(const InputState& inputState) = 0;
            virtual bool doSetClipPlane(const InputState& inputState) = 0;
            virtual void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;
        };
        
        class ClipToolAdapter2D : public ClipToolAdapter<PlaneDragPolicy> {
        public:
            ClipToolAdapter2D(ClipTool* tool, const Grid& grid);
        private:
            class PointSnapper;

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            
            bool doAddClipPoint(const InputState& inputState);
            bool doSetClipPlane(const InputState& inputState);
            void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };
        
        class ClipToolAdapter3D : public ClipToolAdapter<MouseDragPolicy> {
        public:
            ClipToolAdapter3D(ClipTool* tool, const Grid& grid);
        private:
            class PointSnapper;
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            bool doAddClipPoint(const InputState& inputState);
            bool doSetClipPlane(const InputState& inputState);
            void doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            Vec3::List selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint) const;
            Model::BrushFaceList selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint) const;
        };
    }
}

#endif /* defined(TrenchBroom_ClipToolAdapter) */
