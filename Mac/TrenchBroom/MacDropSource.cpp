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

#include "MacDropSource.h"

#include "MacScreenDC.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MacDropSource::MacDropSource(wxWindow* window, const wxImage* image, const wxPoint& imageOffset) :
        wxDropSource(window),
        m_screenDC(NULL),
        m_feedbackImage(wxBitmap(64, 64)),
        m_imageOffset(imageOffset),
        m_showFeedback(true),
        m_dragStarted(false) {
            if (image != NULL)
                m_feedbackImage = wxBitmap(*image);
            CurrentDropSource = this;
        }
        
        MacDropSource::~MacDropSource() {
            delete m_screenDC;
            m_screenDC = NULL;
            CurrentDropSource = NULL;
        }
        
        bool MacDropSource::GiveFeedback(wxDragResult effect) {
            if (!m_showFeedback) {
                delete m_screenDC;
                m_screenDC = NULL;
                return false;
            }
            
            if (m_screenDC == NULL)
                m_screenDC = new MacScreenDC();
            assert(m_screenDC->IsOk());
            
            wxMouseState mouseState = ::wxGetMouseState();
            int x = mouseState.GetX() - m_imageOffset.x;
            int y = mouseState.GetY() - m_imageOffset.y;
            
            m_screenDC->Clear();
            m_screenDC->DrawBitmap(m_feedbackImage, x, y);
            m_screenDC->Flush();
            return true;
        }
        
        void MacDropSource::setShowFeedback(bool showFeedback) {
            m_showFeedback = showFeedback;
        }
    }
}
