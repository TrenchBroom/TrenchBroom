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

#ifndef __TrenchBroom__VertexToolAdapter__
#define __TrenchBroom__VertexToolAdapter__

#include "View/MoveToolAdapter.h"
#include "View/ToolAdapter.h"

namespace TrenchBroom {
    class Hits;
    
    namespace View {
        class InputState;
        class MovementRestriction;
        class VertexTool;
        
        class VertexToolAdapter : public MoveToolAdapter<PickingPolicy, MousePolicy, RenderPolicy> {
        private:
            static const FloatType MaxVertexDistance;
        protected:
            VertexTool* m_tool;
        protected:
            VertexToolAdapter(VertexTool* tool, MoveToolHelper* helper);
        public:
            virtual ~VertexToolAdapter();
        private:
            bool doMouseDown(const InputState& inputState);
            bool doMouseUp(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            bool dismissClick(const InputState& inputState) const;

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
            
            const Hit& firstHit(const Hits& hits) const;
            Hits::List firstHits(const Hits& hits) const;
        };
        
        class VertexToolAdapter2D : public VertexToolAdapter {
        public:
            VertexToolAdapter2D(VertexTool* tool);
        private:
            void doPick(const InputState& inputState, Hits& hits);
        };
        
        class VertexToolAdapter3D : public VertexToolAdapter {
        public:
            VertexToolAdapter3D(VertexTool* tool, MovementRestriction& movementRestriction);
        private:
            void doPick(const InputState& inputState, Hits& hits);
        };
    }
}

#endif /* defined(__TrenchBroom__VertexToolAdapter__) */
