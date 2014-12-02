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

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapViewContainer::MapViewContainer(wxWindow* parent) :
        wxPanel(parent) {}
        
        MapViewContainer::~MapViewContainer() {}
        
        Vec3 MapViewContainer::pasteObjectsDelta(const BBox3& bounds) const {
            return doGetPasteObjectsDelta(bounds);
        }
        
        void MapViewContainer::centerCameraOnSelection() {
            doCenterCameraOnSelection();
        }
        
        void MapViewContainer::moveCameraToPosition(const Vec3& position) {
            doMoveCameraToPosition(position);
        }
        
        bool MapViewContainer::canMoveCameraToNextTracePoint() const {
            return doCanMoveCameraToNextTracePoint();
        }
        
        bool MapViewContainer::canMoveCameraToPreviousTracePoint() const {
            return doCanMoveCameraToPreviousTracePoint();
        }
        
        void MapViewContainer::moveCameraToNextTracePoint() {
            assert(canMoveCameraToNextTracePoint());
            doMoveCameraToNextTracePoint();
        }
        
        void MapViewContainer::moveCameraToPreviousTracePoint() {
            assert(canMoveCameraToPreviousTracePoint());
            doMoveCameraToPreviousTracePoint();
        }
    }
}
