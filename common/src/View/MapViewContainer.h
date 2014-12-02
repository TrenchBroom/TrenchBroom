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

#ifndef __TrenchBroom__MapViewContainer__
#define __TrenchBroom__MapViewContainer__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/GLContextHolder.h"

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class MapViewContainer : public wxPanel {
        public:
            MapViewContainer(wxWindow* parent);
            virtual ~MapViewContainer();

            Vec3 pasteObjectsDelta(const BBox3& bounds) const;
            
            void centerCameraOnSelection();
            void moveCameraToPosition(const Vec3& position);
            
            bool canMoveCameraToNextTracePoint() const;
            bool canMoveCameraToPreviousTracePoint() const;
            void moveCameraToNextTracePoint();
            void moveCameraToPreviousTracePoint();
            
            GLContextHolder::Ptr glContext() const;
        private:
            virtual Vec3 doGetPasteObjectsDelta(const BBox3& bounds) const = 0;

            virtual void doCenterCameraOnSelection() = 0;
            virtual void doMoveCameraToPosition(const Vec3& position) = 0;
            
            virtual bool doCanMoveCameraToNextTracePoint() const = 0;
            virtual bool doCanMoveCameraToPreviousTracePoint() const = 0;
            virtual void doMoveCameraToNextTracePoint() = 0;
            virtual void doMoveCameraToPreviousTracePoint() = 0;
            
            virtual GLContextHolder::Ptr doGetGLContext() const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapViewContainer__) */
