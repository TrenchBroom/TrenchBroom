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

#include "GenericDropSource.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        GenericDropSource::GenericDropSource(wxWindow* window, const wxImage* image, const wxPoint& imageOffset) :
        wxDropSource(window),
        m_window(window),
        m_dragImage(NULL),
        m_feedbackImage(wxBitmap(64, 64)),
        m_imageOffset(imageOffset),
        m_showFeedback(true) {
            if (image != NULL)
                m_feedbackImage = wxBitmap(*image);
            CurrentDropSource = this;
        }
        
        GenericDropSource::~GenericDropSource() {
            if (m_dragImage != NULL) {
                m_dragImage->EndDrag();
                delete m_dragImage;
                m_dragImage = NULL;
            }
            CurrentDropSource = NULL;
        }
        
        bool GenericDropSource::GiveFeedback(wxDragResult effect) {
            if (!m_showFeedback) {
                if (m_dragImage != NULL)
                    m_dragImage->Hide();
                return false;
            }
            
            if (m_dragImage == NULL) {
                m_dragImage = new wxDragImage(m_feedbackImage);
                m_dragImage->BeginDrag(m_imageOffset, m_window, true, NULL);
            }
            
            m_dragImage->Show();

            wxMouseState mouseState = ::wxGetMouseState();
            wxPoint position = m_window->ScreenToClient(mouseState.GetPosition());
            m_dragImage->Move(position);
            
            return true;
        }
        
        void GenericDropSource::setShowFeedback(bool showFeedback) {
            m_showFeedback = showFeedback;
        }

        GenericDropSource* CurrentDropSource = NULL;
    }
}
