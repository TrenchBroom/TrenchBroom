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

#ifndef TrenchBroom_MapView
#define TrenchBroom_MapView

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ViewEffectsService.h"

namespace TrenchBroom {
    namespace View {
        class CameraLinkHelper;
        
        class MapView : public ViewEffectsService {
        public:
            virtual ~MapView();

            bool isCurrent() const;
            void setToolBoxDropTarget();
            void clearDropTarget();

            bool canSelectTall();
            void selectTall();

            bool canFlipObjects() const;
            void flipObjects(Math::Direction direction);
            
            Vec3 pasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const;
            
            void focusCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            
            void moveCameraToCurrentTracePoint();
        private:
            virtual bool doGetIsCurrent() const = 0;
            
            virtual void doSetToolBoxDropTarget() = 0;
            virtual void doClearDropTarget() = 0;

            virtual bool doCanSelectTall() = 0;
            virtual void doSelectTall() = 0;

            virtual bool doCanFlipObjects() const = 0;
            virtual void doFlipObjects(Math::Direction direction) = 0;
            
            virtual Vec3 doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const = 0;

            virtual void doFocusCameraOnSelection() = 0;
            virtual void doMoveCameraToPosition(const Vec3& position) = 0;
            
            virtual void doMoveCameraToCurrentTracePoint() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapView) */
