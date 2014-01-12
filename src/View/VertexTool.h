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

#ifndef __TrenchBroom__VertexTool__
#define __TrenchBroom__VertexTool__

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MoveTool.h"
#include "View/Tool.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace Renderer {
        class TextureFont;
    }
    
    namespace View {
        class InputState;
        class MovementRestriction;
        
        class VertexTool : public MoveTool<ActivationPolicy, PickingPolicy, MousePolicy, RenderPolicy> {
        private:
            static const FloatType MaxVertexDistance;
            
            typedef enum {
                VMMove,
                VMSplit,
                VMSnap
            } VertexToolMode;

            VertexHandleManager m_handleManager;
            VertexToolMode m_mode;
            size_t m_changeCount;
            bool m_ignoreObjectChangeNotifications;
            Vec3 m_dragHandlePosition;
        public:
            VertexTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, MovementRestriction& movementRestriction, Renderer::TextureFont& font);
            MoveResult moveVertices(const Vec3& delta);
        private:
            MoveResult doMoveVertices(const Vec3& delta);
            MoveResult doMoveEdges(const Vec3& delta);
            MoveResult doMoveFaces(const Vec3& delta);

            bool doHandleMove(const InputState& inputState) const;
            Vec3 doGetMoveOrigin(const InputState& inputState) const;
            String doGetActionName(const InputState& inputState) const;
            bool doStartMove(const InputState& inputState);
            Vec3 doSnapDelta(const InputState& inputState, const Vec3& delta) const;
            MoveResult doMove(const Vec3& delta);
            void doEndMove(const InputState& inputState);
            
            bool initiallyActive() const;
            bool doActivate(const InputState& inputState);
            bool doDeactivate(const InputState& inputState);
            void bindObservers();
            void unbindObservers();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);

            bool doMouseDown(const InputState& inputState);
            bool doMouseUp(const InputState& inputState);

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext);
            
            void selectionDidChange(const Model::SelectionResult& selection);
            void objectWillChange(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void commandDoOrUndo(Controller::Command::Ptr command);
            void commandDoneOrUndoFailed(Controller::Command::Ptr command);
            void commandDoFailedOrUndone(Controller::Command::Ptr command);

            bool dismissClick(const InputState& inputState) const;
            void vertexHandleClicked(const InputState& inputState, const Model::Hit::List& hits);
            void edgeHandleClicked(const InputState& inputState, const Model::Hit::List& hits);
            void faceHandleClicked(const InputState& inputState, const Model::Hit::List& hits);
            
            Model::Hit firstHit(const Model::PickResult& pickResult) const;
            Model::Hit::List firstHits(const Model::PickResult& pickResult) const;
        };
    }
}

#endif /* defined(__TrenchBroom__VertexTool__) */
