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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

namespace TrenchBroom {
    class Logger;
    
    namespace Renderer {
        class MapRenderer;
        class Vbo;
    }
    
    namespace View {
        class MapViewBase;
        class MapView2D;
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
            Renderer::Vbo* m_vbo;
            MapViewBar* m_mapViewBar;
            MapView2D* m_mapView2D;
            MapView3D* m_mapView3D;
            MapViewBase* m_currentMapView;
        public:
            SwitchableMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document);
            ~SwitchableMapView();

            Vec3 pasteObjectsDelta(const BBox3& bounds) const;

            void centerCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration = 150);
        private:
            void createGui();
        private:
            void bindEvents();
            void OnIdleSetFocus(wxIdleEvent& event);
            void OnCycleMapView(wxCommandEvent& event);
        private:
            void switchToMapView(MapViewBase* mapView);
        };
    }
}

#endif /* defined(__TrenchBroom__SwitchableMapView__) */
