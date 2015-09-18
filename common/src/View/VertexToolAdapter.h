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

#ifndef TrenchBroom_VertexToolAdapter
#define TrenchBroom_VertexToolAdapter

#include "Model/Hit.h"
#include "View/MoveToolAdapter.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    namespace View {
        class Lasso;
        class InputState;
        class MovementRestriction;
        class VertexTool;
        
        class VertexToolAdapter : public MoveToolAdapter<PickingPolicy, MousePolicy, RenderPolicy>, public MoveToolDelegate {
        private:
            static const FloatType MaxVertexDistance;
        protected:
            VertexTool* m_tool;
        private:
            Lasso* m_lasso;
        protected:
            VertexToolAdapter(VertexTool* tool, MoveToolHelper* helper);
        public:
            virtual ~VertexToolAdapter();
        private:
            Tool* doGetTool();
            
            bool doMouseClick(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            bool dismissClick(const InputState& inputState) const;

            // For the lasso selection, we intercept the plane drag events here
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void doEndPlaneDrag(const InputState& inputState);
            void doCancelPlaneDrag();
            
            bool startLasso(const InputState& inputState, Plane3& plane, Vec3& initialPoint);
            bool updateLasso(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint);
            void endLasso(const InputState& inputState);
            void cancelLasso();
            
            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const InputState& inputState, const Vec3& delta);
            void doEndMove(const InputState& inputState);
            void doCancelMove();

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            bool doCancel();
            
            const Model::Hit& firstHit(const Model::PickResult& pickResult) const;
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const;
        };
        
        class VertexToolAdapter2D : public VertexToolAdapter {
        public:
            VertexToolAdapter2D(VertexTool* tool);
        private:
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
        };
        
        class VertexToolAdapter3D : public VertexToolAdapter {
        public:
            VertexToolAdapter3D(VertexTool* tool, MovementRestriction& movementRestriction);
        private:
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
        };
    }
}

#endif /* defined(TrenchBroom_VertexToolAdapter) */
