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

#ifndef VertexTool_h
#define VertexTool_h

#include "VecMath.h"
#include "TrenchBroom.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/UndoableCommand.h"
#include "View/VertexHandleManager.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderContext;
        class RenderBatch;
        class RenderService;
    }
    
    namespace View {
        class Lasso;
        class Selection;
        
        class VertexTool : public Tool {
        public:
            static const Model::Hit::HitType AnyHandleHit;

            typedef enum {
                MR_Continue,
                MR_Deny,
                MR_Cancel
            } MoveResult;
        private:
            typedef enum {
                Mode_Move,
                Mode_Split
            } Mode;
            
            MapDocumentWPtr m_document;
            
            Mode m_mode;
            size_t m_changeCount;
            bool m_ignoreChangeNotifications;

            VertexHandleManager m_vertexHandles;
            Vec3 m_dragHandlePosition;
            bool m_dragging;
        public:
            VertexTool(MapDocumentWPtr document);
            
            Model::BrushSet findIncidentBrushes(const Vec3& handle) const;
            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
        public: // Handle selection
            bool select(const Model::Hit::List& hits, bool addToSelection);
            void select(const Lasso& lasso, bool modifySelection);
            bool deselectAll();
        public: // Rendering
            void renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
        private:
            void renderHandles(const Vec3::List& handles, Renderer::RenderService& renderService, const Color& color) const;
        private:
            void rebuildBrushGeometry();
        private: // Tool interface
            bool doActivate();
            bool doDeactivate();
        private:
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

            class AddToHandleManager;
            class RemoveFromHandleManager;
            
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
        private: // General helper methods
            const Model::BrushList& selectedBrushes() const;
            void resetModeAfterDeselection();
        };
    }
}

#endif /* VertexTool_h */
