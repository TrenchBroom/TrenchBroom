/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_TwoPaneMapView
#define TrenchBroom_TwoPaneMapView

#include "View/CameraLinkHelper.h"
#include "View/MultiMapView.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    class Logger;
    
    namespace Renderer {
        class MapRenderer;
        class Vbo;
    }

    namespace View {
        class CyclingMapView;
        class GLContextManager;
        class MapViewBase;
        class MapView3D;
        class MapViewToolBox;
        class SplitterWindow2;
        
        class TwoPaneMapView : public MultiMapView {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;

            CameraLinkHelper m_linkHelper;
            SplitterWindow2* m_splitter;
            MapView3D* m_mapView3D;
            CyclingMapView* m_mapView2D;
        public:
            TwoPaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager);
        private:
            void createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager);
        private: // implement MultiMapView subclassing interface
            void doMaximizeView(MapView* view);
            void doRestoreViews();
        };
    }
}

#endif /* defined(TrenchBroom_TwoPaneMapView) */
