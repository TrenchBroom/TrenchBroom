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

#ifndef TrenchBroom_MultiMapView
#define TrenchBroom_MultiMapView

#include "View/MapViewContainer.h"

#include <vector>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class MapView;
        
        class MultiMapView : public MapViewContainer {
        private:
            typedef std::vector<MapView*> MapViewList;
            MapViewList m_mapViews;
            MapView* m_maximizedView;
        protected:
            MultiMapView(wxWindow* parent);
        public:
            virtual ~MultiMapView();
        protected:
            void addMapView(MapView* mapView);
        private: // implement ViewEffectsService interface
            void doFlashSelection();
        private: // implement MapView interface
            bool doGetIsCurrent() const;
            void doSetToolBoxDropTarget();
            void doClearDropTarget();
            bool doCanSelectTall();
            void doSelectTall();
            void doFocusCameraOnSelection();
            void doMoveCameraToPosition(const Vec3& position);
            void doMoveCameraToCurrentTracePoint();
        private: // implement MapViewContainer interface
            bool doCanMaximizeCurrentView() const;
            bool doCurrentViewMaximized() const;
            void doToggleMaximizeCurrentView();
            MapView* doGetCurrentMapView() const;
        private: // subclassing interface
            virtual void doMaximizeView(MapView* view) = 0;
            virtual void doRestoreViews() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MultiMapView) */
