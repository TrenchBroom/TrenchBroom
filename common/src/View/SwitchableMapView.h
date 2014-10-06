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

#ifndef __TrenchBroom__SwitchableMapView__
#define __TrenchBroom__SwitchableMapView__

#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    class Logger;
    
    namespace Renderer {
        class MapRenderer;
    }
    
    namespace View {
        class MapView3D;
        class MapViewBar;
        class MapViewToolBox;
        class MovementRestriction;
        
        class SwitchableMapView : public wxPanel {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            
            MapViewToolBox* m_toolBox;

            Renderer::MapRenderer* m_mapRenderer;
            MapViewBar* m_mapViewBar;
            MapView3D* m_mapView;
        public:
            SwitchableMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document);
            ~SwitchableMapView();
        private:
            void createGui();
        private:
            void bindEvents();
            void OnIdleSetFocus(wxIdleEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__SwitchableMapView__) */
