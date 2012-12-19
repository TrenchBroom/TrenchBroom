/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_DragAndDrop_h
#define TrenchBroom_DragAndDrop_h

#include "ScreenDC.h"

#include <wx/dnd.h>
#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        class DropSource : public wxDropSource {
        private:
            ScreenDC* m_screenDC;
            wxBitmap m_feedbackImage;
            wxPoint m_imageOffset;
            bool m_showFeedback;
        public:
            DropSource(wxWindow* window, const wxImage& image, const wxPoint& imageOffset);
            virtual ~DropSource();
            
            bool GiveFeedback(wxDragResult effect);
            void setShowFeedback(bool showFeedback);
        };

        extern DropSource* CurrentDropSource;
    }
}

#endif
