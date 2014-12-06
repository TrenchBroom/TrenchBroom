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

#ifndef __TrenchBroom__SwitchableMapViewContainer__
#define __TrenchBroom__SwitchableMapViewContainer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MapViewLayout.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    class Logger;
    
    namespace Renderer {
        class MapRenderer;
        class Vbo;
    }

    namespace IO {
        class Path;
    }
    
    namespace View {
        class GLContextManager;
        class MapViewBar;
        class MapViewContainer;
        class MapViewToolBox;
        
        class SwitchableMapViewContainer : public wxPanel {
        private:
            Logger* m_logger;
            MapDocumentWPtr m_document;
            GLContextManager& m_contextManager;
            
            MapViewBar* m_mapViewBar;
            MapViewToolBox* m_toolBox;
            
            Renderer::MapRenderer* m_mapRenderer;
            Renderer::Vbo* m_vbo;
            
            MapViewContainer* m_mapView;
        public:
            SwitchableMapViewContainer(wxWindow* parent, Logger* logger, MapDocumentWPtr document, GLContextManager& contextManager);
            ~SwitchableMapViewContainer();
            
            void switchToMapView(MapViewLayout viewId);

            Vec3 pasteObjectsDelta(const BBox3& bounds) const;
            
            void centerCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            
            bool canMoveCameraToNextTracePoint() const;
            bool canMoveCameraToPreviousTracePoint() const;
            void moveCameraToNextTracePoint();
            void moveCameraToPreviousTracePoint();
        private:
            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__SwitchableMapViewContainer__) */
