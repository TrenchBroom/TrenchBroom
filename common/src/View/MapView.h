/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include "FloatType.h"
#include "View/ViewEffectsService.h"

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace View {
        class MapViewActivationTracker;
        class MapViewBase;
        class MapViewContainer;

        class MapView : public ViewEffectsService {
        private:
            MapViewContainer* m_container;
        public:
            MapView();
            ~MapView() override;

            void setContainer(MapViewContainer* container);
            void installActivationTracker(MapViewActivationTracker& activationTracker);

            bool isCurrent() const;
            MapViewBase* firstMapViewBase();

            bool canSelectTall();
            void selectTall();

            vm::vec3 pasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const;

            void focusCameraOnSelection(bool animate);
            void moveCameraToPosition(const vm::vec3& position, bool animate);

            void moveCameraToCurrentTracePoint();

            bool cancelMouseDrag();

            /**
             * If the parent of this view is a CyclingMapView, cycle to the
             * next child, otherwise do nothing.
             */
            void cycleMapView();

            /**
             * Requests repaint of the managed map views. Note, this must be used instead of QWidget::update()
             */
            void refreshViews();
        private:
            virtual void doInstallActivationTracker(MapViewActivationTracker& activationTracker) = 0;

            virtual bool doGetIsCurrent() const = 0;
            virtual MapViewBase* doGetFirstMapViewBase() = 0;

            virtual bool doCanSelectTall() = 0;
            virtual void doSelectTall() = 0;

            virtual vm::vec3 doGetPasteObjectsDelta(const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const = 0;

            virtual void doFocusCameraOnSelection(bool animate) = 0;
            virtual void doMoveCameraToPosition(const vm::vec3& position, bool animate) = 0;

            virtual void doMoveCameraToCurrentTracePoint() = 0;

            virtual bool doCancelMouseDrag() = 0;

            virtual void doRefreshViews() = 0;
        };
    }
}

