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

#ifndef __TrenchBroom__MultiMapView__
#define __TrenchBroom__MultiMapView__

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
        protected:
            MultiMapView(wxWindow* parent);
        public:
            virtual ~MultiMapView();
        protected:
            void addMapView(MapView* mapView);
        private: // implement MapView interface
            bool doGetIsCurrent() const;

            void doSetToolBoxDropTarget();
            void doClearDropTarget();
        
            Vec3 doGetPasteObjectsDelta(const BBox3& bounds) const;
            
            void doCenterCameraOnSelection();
            void doMoveCameraToPosition(const Vec3& position);
            
            void doMoveCameraToCurrentTracePoint();
        private:
            MapView* currentMapView() const;
        };
    }
}

#endif /* defined(__TrenchBroom__MultiMapView__) */
