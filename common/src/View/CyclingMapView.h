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

#ifndef TrenchBroom_CyclingMapView
#define TrenchBroom_CyclingMapView

#include "TrenchBroom.h"
#include "View/CameraLinkHelper.h"
#include "View/MapViewContainer.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace Renderer {
        class MapRenderer;
    }
    
    namespace View {
        class GLContextManager;
        class MapViewBase;
        class MapViewToolBox;
        
        class CyclingMapView : public MapViewContainer, public CameraLinkableView {
        public:
            typedef enum {
                View_3D  = 1,
                View_XY  = 2,
                View_XZ  = 4,
                View_YZ  = 8,
                View_ZZ  = View_XZ | View_YZ,
                View_2D  = View_XY | View_ZZ,
                View_ALL = View_3D | View_2D
            } View;
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            
            using MapViewList = std::vector<MapViewBase*>;
            MapViewList m_mapViews;
            MapViewBase* m_currentMapView;
        public:
            CyclingMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, View views);
        private:
            void createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, View views);
        private:
            void bindEvents();
            void OnCycleMapView(wxCommandEvent& event);
        private:
            void switchToMapView(MapViewBase* mapView);
        private: // implement ViewEffectsService interface
            void doFlashSelection() override;
        private: // implement MapView interface
            bool doGetIsCurrent() const override;
            void doSetToolBoxDropTarget() override;
            void doClearDropTarget() override;
            bool doCanSelectTall() override;
            void doSelectTall() override;
            void doFocusCameraOnSelection(bool animate) override;
            void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
            void doMoveCameraToCurrentTracePoint() override;
            bool doCanMaximizeCurrentView() const override;
            bool doCurrentViewMaximized() const override;
            void doToggleMaximizeCurrentView() override;

            bool doCancelMouseDrag() override;

        private: // implement MapViewContainer interface
            MapView* doGetCurrentMapView() const override;
        private: // implement CameraLinkableView interface
            void doLinkCamera(CameraLinkHelper& linkHelper) override;
        };
    }
}

#endif /* defined(TrenchBroom_CyclingMapView) */
