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

#ifndef TrenchBroom_MapViewToolBox
#define TrenchBroom_MapViewToolBox

#include "View/ToolBox.h"
#include "View/ViewTypes.h"

class wxBookCtrlBase;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class ClipTool;
        class CreateComplexBrushTool;
        class CreateEntityTool;
        class CreateSimpleBrushTool;
        class MoveObjectsTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class SelectionTool;
        class VertexTool;
        class Selection;

        class MapViewToolBox : public ToolBox {
        private:
            MapDocumentWPtr m_document;
            
            ClipTool* m_clipTool;
            CreateComplexBrushTool* m_createComplexBrushTool;
            CreateEntityTool* m_createEntityTool;
            CreateSimpleBrushTool* m_createSimpleBrushTool;
            MoveObjectsTool* m_moveObjectsTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
            VertexTool* m_vertexTool;
        public:
            MapViewToolBox(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl);
            ~MapViewToolBox();
        public: // tools
            ClipTool* clipTool();
            CreateComplexBrushTool* createComplexBrushTool();
            CreateEntityTool* createEntityTool();
            CreateSimpleBrushTool* createSimpleBrushTool();
            MoveObjectsTool* moveObjectsTool();
            ResizeBrushesTool* resizeBrushesTool();
            RotateObjectsTool* rotateObjectsTool();
            SelectionTool* selectionTool();
            VertexTool* vertexTool();
            
            void toggleCreateComplexBrushTool();
            bool createComplexBrushToolActive() const;
            
            void toggleClipTool();
            bool clipToolActive() const;
            void toggleClipSide();
            void performClip();
            void removeLastClipPoint();
            
            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            double rotateToolAngle() const;
            const Vec3 rotateToolCenter() const;
            void moveRotationCenter(const Vec3& delta);
            
            void toggleVertexTool();
            bool vertexToolActive() const;
            void moveVertices(const Vec3& delta);
        private: // Tool related methods
            void createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl);
            void destroyTools();
        private: // notification
            void registerTool(Tool* tool, wxBookCtrlBase* bookCtrl);
            void bindObservers();
            void unbindObservers();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
            void updateEditorContext();
        };
    }
}

#endif /* defined(TrenchBroom_MapViewToolBox) */
