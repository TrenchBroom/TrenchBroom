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

#ifndef TrenchBroom_MapViewContainer
#define TrenchBroom_MapViewContainer

#include "View/MapView.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class MapViewBase;
        
        class MapViewContainer : public wxPanel, public MapView {
        public:
            MapViewContainer(wxWindow* parent);
            virtual ~MapViewContainer();
        public:
            bool canMaximizeCurrentView() const;
            bool currentViewMaximized() const;
            void toggleMaximizeCurrentView();
        protected:
            MapView* currentMapView() const;
        private: // implement MapView interface
            bool doCanFlipObjects() const;
            void doFlipObjects(Math::Direction direction);

            Vec3 doGetPasteObjectsDelta(const BBox3& bounds, const BBox3& referenceBounds) const;
        private: // subclassing interface
            virtual bool doCanMaximizeCurrentView() const = 0;
            virtual bool doCurrentViewMaximized() const = 0;
            virtual void doToggleMaximizeCurrentView() = 0;
            virtual MapView* doGetCurrentMapView() const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapViewContainer) */
