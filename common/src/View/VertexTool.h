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
#include "Renderer/PointGuideRenderer.h"
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
        class Grid;
        class Lasso;
        class Selection;
        
        class VertexTool : public Tool {
        public:
            static const Model::Hit::HitType VertexHandleHit;
            static const Model::Hit::HitType EdgeHandleHit;
            static const Model::Hit::HitType FaceHandleHit;
            static const Model::Hit::HitType SplitHandleHit;
            static const Model::Hit::HitType AnyHandleHit;

            typedef enum {
                MR_Continue,
                MR_Deny,
                MR_Cancel
            } MoveResult;
        private:
            typedef enum {
                Mode_Move,
                Mode_Split_Edge,
                Mode_Split_Face
            } Mode;
            
            MapDocumentWPtr m_document;
            
            Mode m_mode;
            size_t m_changeCount;
            bool m_ignoreChangeNotifications;

            VertexHandleManager m_vertexHandles;
            EdgeHandleManager m_edgeHandles;
            FaceHandleManager m_faceHandles;
            Vec3 m_dragHandlePosition;
            bool m_dragging;

            mutable Renderer::PointGuideRenderer m_guideRenderer;
        public:
            VertexTool(MapDocumentWPtr document);
            
            const Grid& grid() const;
            
            Model::BrushSet findIncidentBrushes(const Vec3& handle) const;
            Model::BrushSet findIncidentBrushes(const Edge3& handle) const;
            Model::BrushSet findIncidentBrushes(const Polygon3& handle) const;
        private:
            template <typename M, typename H>
            Model::BrushSet findIncidentBrushes(const M& manager, const H& handle) const {
                const Model::BrushList& brushes = selectedBrushes();
                return manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes));
            }
        public:
            void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
        public: // Handle selection
            bool select(const Model::Hit::List& hits, bool addToSelection);
            void select(const Lasso& lasso, bool modifySelection);
            bool deselectAll();
        public: // Vertex moving
            bool startMove(const Model::Hit& hit);
            MoveResult move(const Vec3& delta);
            void endMove();
            void cancelMove();

            Vec3 getHandlePosition(const Model::Hit& hit);
        private:
            String actionName() const;
        private:
            MoveResult moveVertices(const Vec3& delta);
            Model::VertexToBrushesMap buildBrushMap(const Vec3::List& handles) const;
        public: // Rendering
            void renderHandles(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
            void renderDragHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handle) const;
            void renderHandle(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handle, const Color& color) const;
            void renderDragHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
            void renderHighlight(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& handle) const;
            void renderDragGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Vec3& position) const;
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
