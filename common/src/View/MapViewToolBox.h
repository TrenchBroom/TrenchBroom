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

#ifndef TrenchBroom_MapViewToolBox
#define TrenchBroom_MapViewToolBox

#include "FloatType.h"
#include "View/ToolBox.h"

#include <memory>

class QStackedLayout;

namespace TrenchBroom {
    namespace View {
        class ClipTool;
        class CreateComplexBrushTool;
        class CreateEntityTool;
        class CreateSimpleBrushTool;
        class MoveObjectsTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class ScaleObjectsTool;
        class ShearObjectsTool;
        class VertexTool;
        class EdgeTool;
        class FaceTool;
        class MapDocument;

        class MapViewToolBox : public ToolBox {
        private:
            std::weak_ptr<MapDocument> m_document;

            std::unique_ptr<ClipTool> m_clipTool;
            std::unique_ptr<CreateComplexBrushTool> m_createComplexBrushTool;
            std::unique_ptr<CreateEntityTool> m_createEntityTool;
            std::unique_ptr<CreateSimpleBrushTool> m_createSimpleBrushTool;
            std::unique_ptr<MoveObjectsTool> m_moveObjectsTool;
            std::unique_ptr<ResizeBrushesTool> m_resizeBrushesTool;
            std::unique_ptr<RotateObjectsTool> m_rotateObjectsTool;
            std::unique_ptr<ScaleObjectsTool> m_scaleObjectsTool;
            std::unique_ptr<ShearObjectsTool> m_shearObjectsTool;
            std::unique_ptr<VertexTool> m_vertexTool;
            std::unique_ptr<EdgeTool> m_edgeTool;
            std::unique_ptr<FaceTool> m_faceTool;
        public:
            MapViewToolBox(std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl);
            ~MapViewToolBox();
        public: // tools
            ClipTool* clipTool() const;
            CreateComplexBrushTool* createComplexBrushTool() const;
            CreateEntityTool* createEntityTool() const;
            CreateSimpleBrushTool* createSimpleBrushTool() const;
            MoveObjectsTool* moveObjectsTool() const;
            ResizeBrushesTool* resizeBrushesTool() const;
            RotateObjectsTool* rotateObjectsTool() const;
            ScaleObjectsTool* scaleObjectsTool() const;
            ShearObjectsTool* shearObjectsTool() const;
            VertexTool* vertexTool() const;
            EdgeTool* edgeTool() const;
            FaceTool* faceTool() const;

            void toggleCreateComplexBrushTool();
            bool createComplexBrushToolActive() const;
            void performCreateComplexBrush();

            void toggleClipTool();
            bool clipToolActive() const;
            void toggleClipSide();
            void performClip();
            void removeLastClipPoint();

            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            double rotateToolAngle() const;
            vm::vec3 rotateToolCenter() const;
            void moveRotationCenter(const vm::vec3& delta);

            void toggleScaleObjectsTool();
            bool scaleObjectsToolActive() const;

            void toggleShearObjectsTool();
            bool shearObjectsToolActive() const;

            bool anyVertexToolActive() const;

            void toggleVertexTool();
            bool vertexToolActive() const;

            void toggleEdgeTool();
            bool edgeToolActive() const;

            void toggleFaceTool();
            bool faceToolActive() const;

            void moveVertices(const vm::vec3& delta);
        private: // Tool related methods
            void createTools(std::weak_ptr<MapDocument> document, QStackedLayout* bookCtrl);
        private: // notification
            void registerTool(Tool* tool, QStackedLayout* bookCtrl);
            void bindObservers();
            void unbindObservers();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
            void updateEditorContext();
            void documentWasNewedOrLoaded(MapDocument* document);
        };
    }
}

#endif /* defined(TrenchBroom_MapViewToolBox) */
