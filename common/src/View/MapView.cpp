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

#include "MapView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapView::~MapView() {}
        
        bool MapView::isCurrent() const {
            return doGetIsCurrent();
        }

        void MapView::setToolBoxDropTarget() {
            doSetToolBoxDropTarget();
        }
        
        void MapView::clearDropTarget() {
            doClearDropTarget();
        }

        bool MapView::canSelectTall() {
            return doCanSelectTall();
        }
        
        void MapView::selectTall() {
            doSelectTall();
        }

        bool MapView::canFlipObjects() const {
            return doCanFlipObjects();
        }
        
        void MapView::flipObjects(const Math::Direction direction) {
            assert(canFlipObjects());
            doFlipObjects(direction);
        }

        Vec3 MapView::pasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const {
            return doGetPasteObjectsDelta(bounds, referenceBounds);
        }
        
        void MapView::focusCameraOnSelection() {
            doFocusCameraOnSelection();
        }
        
        void MapView::moveCameraToPosition(const Vec3& position) {
            doMoveCameraToPosition(position);
        }
        
        void MapView::moveCameraToCurrentTracePoint() {
            doMoveCameraToCurrentTracePoint();
        }
    }
}
