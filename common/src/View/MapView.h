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

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ViewEffectsService.h"

namespace TrenchBroom {
    namespace View {
        class CameraLinkHelper;
        
        class MapView {
        public:
            virtual ~MapView();

            bool isCurrent() const;
            void setToolBoxDropTarget();
            void clearDropTarget();

            Vec3 pasteObjectsDelta(const BBox3& bounds) const;
            
            void centerCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            
            void moveCameraToCurrentTracePoint();
        private:
            virtual bool doGetIsCurrent() const = 0;
            
            virtual void doSetToolBoxDropTarget() = 0;
            virtual void doClearDropTarget() = 0;

            virtual Vec3 doGetPasteObjectsDelta(const BBox3& bounds) const = 0;

            virtual void doCenterCameraOnSelection() = 0;
            virtual void doMoveCameraToPosition(const Vec3& position) = 0;
            
            virtual void doMoveCameraToCurrentTracePoint() = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
