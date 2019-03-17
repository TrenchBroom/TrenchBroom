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

#ifndef TrenchBroom_SwitchableMapViewContainer
#define TrenchBroom_SwitchableMapViewContainer

#include "TrenchBroom.h"
#include "View/MapViewLayout.h"
#include "View/MapView.h"
#include "View/ViewTypes.h"

#include <vecmath/scalar.h>

#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    class Logger;

    namespace Renderer {
        class MapRenderer;
    }

    namespace IO {
        class Path;
    }

    namespace View {
        class ClipTool;
        class GLContextManager;
        class Inspector;
        class MapViewContainer;
        class MapViewBar;
        class MapViewToolBox;
        class Tool;
        class VertexTool;
        class EdgeTool;
        class FaceTool;

        class SwitchableMapViewContainer : public wxPanel, public MapView {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            GLContextManager& m_contextManager;

            MapViewBar* m_mapViewBar;
            MapViewToolBox* m_toolBox;

            Renderer::MapRenderer* m_mapRenderer;

            MapViewContainer* m_mapView;
        public:
            SwitchableMapViewContainer(wxWindow* parent, Logger* logger, MapDocumentWPtr document, GLContextManager& contextManager);
            ~SwitchableMapViewContainer() override;

            void connectTopWidgets(Inspector* inspector);

            bool viewportHasFocus() const;
            void switchToMapView(MapViewLayout viewId);

            bool anyToolActive() const;
            void deactivateTool();

            bool createComplexBrushToolActive() const;
            bool canToggleCreateComplexBrushTool() const;
            void toggleCreateComplexBrushTool();

            bool clipToolActive() const;
            bool canToggleClipTool() const;
            void toggleClipTool();
            ClipTool* clipTool();

            bool rotateObjectsToolActive() const;
            bool canToggleRotateObjectsTool() const;
            void toggleRotateObjectsTool();

            bool scaleObjectsToolActive() const;
            bool canToggleScaleObjectsTool() const;
            void toggleScaleObjectsTool();

            bool shearObjectsToolActive() const;
            bool canToggleShearObjectsTool() const;
            void toggleShearObjectsTool();

            bool canToggleVertexTools() const;
            bool anyVertexToolActive() const;
            bool vertexToolActive() const;
            bool edgeToolActive() const;
            bool faceToolActive() const;
            void toggleVertexTool();
            void toggleEdgeTool();
            void toggleFaceTool();
            VertexTool* vertexTool();
            EdgeTool* edgeTool();
            FaceTool* faceTool();

            bool canMoveCameraToNextTracePoint() const;
            bool canMoveCameraToPreviousTracePoint() const;
            void moveCameraToNextTracePoint();
            void moveCameraToPreviousTracePoint();

            bool canMaximizeCurrentView() const;
            bool currentViewMaximized() const;
            void toggleMaximizeCurrentView();
        private:
            void bindObservers();
            void unbindObservers();
            void refreshViews(Tool* tool);
        private: // implement MapView interface
            bool doGetIsCurrent() const override;
            void doSetToolBoxDropTarget() override;
            void doClearDropTarget() override;
            bool doCanSelectTall() override;
            void doSelectTall() override;
            bool doCanFlipObjects() const override;
            void doFlipObjects(vm::direction direction) override;
            vm::vec3 doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const override;
            void doFocusCameraOnSelection(bool animate) override;
            void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
            void doMoveCameraToCurrentTracePoint() override;
            bool doCancelMouseDrag() override;
        private: // implement ViewEffectsService interface
            void doFlashSelection() override;
        };
    }
}

#endif /* defined(TrenchBroom_SwitchableMapViewContainer) */
