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

#include "DragAndDrop.h"

#include "MacScreenDC.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        FeedbackFrame::FeedbackFrame(wxImage& image) :
        wxFrame(NULL, wxID_ANY, wxT("TrenchBroom DnD Feedback Frame"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
        m_image(image) {
            Bind(wxEVT_PAINT, &FeedbackFrame::OnPaint, this);
            Bind(wxEVT_ERASE_BACKGROUND, &FeedbackFrame::OnEraseBackground, this);
            
            int width = image.GetWidth() + 2;
            int height = image.GetHeight() + 2;
            SetClientSize(width, height);
            Show();
        }
        
        void FeedbackFrame::OnEraseBackground(wxEraseEvent& event) {
        }
        
        void FeedbackFrame::OnPaint(wxPaintEvent& event) {
            wxPaintDC dc(this);
            dc.SetPen(*wxRED_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);
            dc.DrawBitmap(m_image, 1, 1);
        }

        DropSource::DropSource(wxWindow* window, const wxImage& image, const wxPoint& imageOffset) :
        wxDropSource(window),
        m_screenDC(NULL),
        m_feedbackImage(image),
        m_imageOffset(imageOffset),
        m_showFeedback(true) {
            CurrentDropSource = this;
        }
        
        DropSource::~DropSource() {
            if (m_screenDC != NULL) {
                wxDELETE(m_screenDC);
                m_screenDC = NULL;
            }
            CurrentDropSource = NULL;
        }
        
        bool DropSource::GiveFeedback(wxDragResult effect) {
            if (m_screenDC != NULL)
                m_screenDC->Clear();
            
            if (!m_showFeedback)
                return false;
            
            wxMouseState mouseState = ::wxGetMouseState();
            int x = mouseState.GetX() - m_imageOffset.x;
            int y = mouseState.GetY() - m_imageOffset.y;
            
            if (m_screenDC == NULL) {
                m_screenDC = new MacScreenDC();
                assert(m_screenDC->IsOk());
            }
            
            m_screenDC->SetPen(*wxRED_PEN);
            m_screenDC->DrawCircle(x, y, 10);
            m_screenDC->DrawBitmap(m_feedbackImage, x, y);
            m_screenDC->Flush();
            
            return true;
        }
        
        void DropSource::setShowFeedback(bool showFeedback) {
            m_showFeedback = showFeedback;
        }

        DropSource* CurrentDropSource = NULL;
    }
}