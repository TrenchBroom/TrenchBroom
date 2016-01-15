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

#ifndef TrenchBroom_VertexTool
#define TrenchBroom_VertexTool

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "View/MoveToolAdapter.h"
#include "View/Tool.h"
#include "View/UndoableCommand.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
        class SelectionResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }
    
    namespace View {
        class InputState;
        class Lasso;
        class MovementRestriction;
        class Selection;
        
        class VertexTool : public Tool {
        private:
            typedef enum {
                Mode_Move,
                Mode_Split,
                Mode_Snap
            } Mode;

            MapDocumentWPtr m_document;
            VertexHandleManager m_handleManager;
            Mode m_mode;
            size_t m_changeCount;
            bool m_ignoreChangeNotifications;
            Vec3 m_dragHandlePosition;
            bool m_dragging;
        public:
            VertexTool(MapDocumentWPtr document);
            
            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            
            bool deselectAll();
            bool mergeVertices(const Model::Hit& hit);
            bool handleDoubleClicked(const Model::Hit& hit);
            bool select(const Model::Hit::List& hits, bool addToSelection);
            void select(const Lasso& lasso, bool modifySelection);

            bool beginMove(const Model::Hit& hit);
            Vec3 snapMoveDelta(const Vec3& delta, const Model::Hit& hit, bool relative);
            MoveResult move(const Vec3& delta);
            void endMove();
            void cancelMove();
            
            void renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position);
            void renderEdgeHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition);
            void renderFaceHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handlePosition);
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position);
            
            bool cancel();
            
            bool handleBrushes(const Vec3& position, Model::BrushSet& brushes) const;
            bool handleSelected(const Vec3& position) const;
            bool hasSelectedHandles() const;
            void moveVerticesAndRebuildBrushGeometry(const Vec3& delta);
            bool canSnapVertices() const;
            void snapVertices(size_t snapTo);
        private:
            void selectVertex(const Model::Hit::List& hits, bool addToSelection);
            void selectEdge(const Model::Hit::List& hits, bool addToSelection);
            void selectFace(const Model::Hit::List& hits, bool addToSelection);
            
            String actionName() const;
            
            MoveResult moveVertices(const Vec3& delta);
            MoveResult doMoveVertices(const Vec3& delta);
            MoveResult doMoveEdges(const Vec3& delta);
            MoveResult doMoveFaces(const Vec3& delta);
            MoveResult doSplitEdges(const Vec3& delta);
            MoveResult doSplitFaces(const Vec3& delta);

            void rebuildBrushGeometry();

            bool doActivate();
            bool doDeactivate();
            
            void bindObservers();
            void unbindObservers();
            
            void commandDo(Command::Ptr command);
            void commandDone(Command::Ptr command);
            void commandDoFailed(Command::Ptr command);
            void commandUndo(UndoableCommand::Ptr command);
            void commandUndone(UndoableCommand::Ptr command);
            void commandUndoFailed(UndoableCommand::Ptr command);

            void commandDoOrUndo(Command::Ptr command);
            void commandDoneOrUndoFailed(Command::Ptr command);
            void commandDoFailedOrUndone(Command::Ptr command);
            bool isVertexCommand(const Command::Ptr command) const;
            
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
        private: // implement Tool interface
            String doGetIconName() const;
        };
    }
}

#endif /* defined(TrenchBroom_VertexTool) */
