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

#include "MapViewContainer.h"

#include "View/MapViewBase.h"

namespace TrenchBroom {
    namespace View {
        MapViewContainer::MapViewContainer(wxWindow* parent) :
        wxPanel(parent),
        MapView() {}
        
        MapViewContainer::~MapViewContainer() {}

        bool MapViewContainer::canMaximizeCurrentView() const {
            return doCanMaximizeCurrentView();
        }
        
        bool MapViewContainer::currentViewMaximized() const {
            return doCurrentViewMaximized();
        }
        
        void MapViewContainer::toggleMaximizeCurrentView() {
            doToggleMaximizeCurrentView();
        }

        bool MapViewContainer::doCanFlipObjects() const {
            MapView* current = currentMapView();
            if (current == NULL)
                return false;
            return current->canFlipObjects();
        }
        
        void MapViewContainer::doFlipObjects(const Math::Direction direction) {
            MapView* current = currentMapView();
            assert(current != NULL);
            current->flipObjects(direction);
        }

        Vec3 MapViewContainer::doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const {
            MapView* current = currentMapView();
            assert(current != NULL);
            return current->pasteObjectsDelta(bounds, referenceBounds);
        }

        MapView* MapViewContainer::currentMapView() const {
            return doGetCurrentMapView();
        }
    }
}
