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

#ifndef __TrenchBroom__CyclingMapView__
#define __TrenchBroom__CyclingMapView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/GLContextHolder.h"
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
        
        class CyclingMapView : public wxPanel {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            
            MapViewToolBox* m_toolBox;

            Renderer::MapRenderer* m_mapRenderer;
            Renderer::Vbo* m_vbo;
            MapViewBar* m_mapViewBar;
            MapViewBase* m_mapViews[4];
            MapViewBase* m_currentMapView;
        public:
            CyclingMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document);
            ~CyclingMapView();

            Vec3 pasteObjectsDelta(const BBox3& bounds) const;

            void centerCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            
            bool canMoveCameraToNextTracePoint() const;
            bool canMoveCameraToPreviousTracePoint() const;
            void moveCameraToNextTracePoint();
            void moveCameraToPreviousTracePoint();
            
            GLContextHolder::Ptr glContext() const;
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

#endif /* defined(__TrenchBroom__CyclingMapView__) */
