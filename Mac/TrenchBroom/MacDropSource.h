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

#ifndef __TrenchBroom__MacDragSource__
#define __TrenchBroom__MacDragSource__

#include <wx/dnd.h>
#include <wx/frame.h>

class MacScreenDC;

namespace TrenchBroom {
    namespace View {
        class MacDropSource : public wxDropSource {
        private:
            MacScreenDC* m_screenDC;
            wxBitmap m_feedbackImage;
            wxPoint m_imageOffset;
            bool m_showFeedback;
            bool m_dragStarted;
        public:
            MacDropSource(wxWindow* window, const wxImage& image, const wxPoint& imageOffset);
            virtual ~MacDropSource();
            
            bool GiveFeedback(wxDragResult effect);
            void setShowFeedback(bool showFeedback);
        };

        
        extern MacDropSource* CurrentDropSource;
    }
}

#endif /* defined(__TrenchBroom__MacDragSource__) */
