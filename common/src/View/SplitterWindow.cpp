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

#include "SplitterWindow.h"

#include "View/BorderLine.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/control.h>
#include <wx/dcclient.h>
#include <wx/log.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        SplitterWindow::SplitterWindow(wxWindow* parent) :
        wxPanel(parent, wxID_ANY),
        m_splitMode(SplitMode_Unset),
        m_sash(NULL),
        m_sashGravity(0.5f),
        m_sashPosition(-1),
        m_sashCursorSet(0),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxSize(0, 0);
            }
            
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow::OnSize, this);
        }
        
        void SplitterWindow::splitHorizontally(wxWindow* left, wxWindow* right, const wxSize& leftMin, const wxSize& rightMin) {
            split(left, right, leftMin, rightMin, SplitMode_Horizontal);
        }
        
        void SplitterWindow::splitVertically(wxWindow* top, wxWindow* bottom, const wxSize& topMin, const wxSize& bottomMin) {
            split(top, bottom, topMin, bottomMin, SplitMode_Vertical);
        }
        
        void SplitterWindow::split(wxWindow* window1, wxWindow* window2, const wxSize& min1, const wxSize& min2, const SplitMode splitMode) {
            assert(window1 != NULL);
            assert(window1->GetParent() == this);
            assert(window2 != NULL);
            assert(window2->GetParent() == this);
            assert(m_splitMode == SplitMode_Unset);
            
            m_windows[0] = window1;
            m_windows[1] = window2;
            m_splitMode = splitMode;

            if (m_splitMode == SplitMode_Horizontal)
                m_sash = new BorderLine(this, BorderLine::Direction_Vertical, sashSize());
            else
                m_sash = new BorderLine(this, BorderLine::Direction_Horizontal, sashSize());
            bindMouseEvents(m_sash);
            
            setMinSize(window1, min1);
            setMinSize(window2, min2);
        }

        void SplitterWindow::bindMouseEvents(wxWindow* window) {
            window->Bind(wxEVT_ENTER_WINDOW, &SplitterWindow::OnMouseEnter, this);
            window->Bind(wxEVT_LEAVE_WINDOW, &SplitterWindow::OnMouseLeave, this);
            window->Bind(wxEVT_LEFT_DOWN, &SplitterWindow::OnMouseButton, this);
            window->Bind(wxEVT_LEFT_UP, &SplitterWindow::OnMouseButton, this);
            window->Bind(wxEVT_MOTION, &SplitterWindow::OnMouseMotion, this);
        }

        void SplitterWindow::setMinSize(wxWindow* window, const wxSize& minSize) {
            assert(m_splitMode != SplitMode_Unset);
            assert(minSize.x >= 0 && minSize.y != 0);
            
            wxSize splitterMinSize;
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] == window)
                    m_minSizes[i] = minSize;
                
                setH(splitterMinSize, h(splitterMinSize) + h(m_minSizes[i]));
                setV(splitterMinSize, std::max(v(splitterMinSize), v(m_minSizes[i])));
            }
            
            SetMinClientSize(splitterMinSize);
        }
        
        void SplitterWindow::setSashGravity(const float sashGravity) {
            assert(sashGravity >= 0.0f && sashGravity <= 1.0f);
            m_sashGravity = sashGravity;
        }
        
        void SplitterWindow::OnMouseEnter(wxMouseEvent& event) {
            setSashCursor();
        }
        
        void SplitterWindow::OnMouseLeave(wxMouseEvent& event) {
            if (!dragging())
                unsetSashCursor();
        }

        void SplitterWindow::OnMouseButton(wxMouseEvent& event) {
            assert(m_splitMode != SplitMode_Unset);
            
            if (event.LeftDown()) {
                m_sash->CaptureMouse();
                setSashCursor();
            } else if (event.LeftUp() && dragging()) {
                m_sash->ReleaseMouse();
                unsetSashCursor();
            }
            Refresh();
        }
        
        void SplitterWindow::OnMouseMotion(wxMouseEvent& event) {
            assert(m_splitMode != SplitMode_Unset);
            
            const wxPoint screenPos = wxGetMousePosition();
            const wxPoint clientPos = ScreenToClient(screenPos);
            
            if (dragging()) {
                setSashPosition(h(clientPos));
                sizeWindows();
                setSashCursor();
            }
        }

        void SplitterWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            unsetSashCursor();
        }
        
        bool SplitterWindow::dragging() const {
            return GetCapture() == m_sash;
        }
        
        void SplitterWindow::setSashCursor() {
            if (!m_sashCursorSet) {
                wxSetCursor(sizeCursor());
                m_sashCursorSet = true;
            }
        }
        
        void SplitterWindow::unsetSashCursor() {
            if (m_sashCursorSet && m_sash->HitTest(m_sash->ScreenToClient(wxGetMousePosition())) == wxHT_WINDOW_OUTSIDE) {
                wxSetCursor(wxCursor(wxCURSOR_ARROW));
                m_sashCursorSet = false;
            }
        }

        wxCursor SplitterWindow::sizeCursor() const {
            switch (m_splitMode) {
                case SplitMode_Horizontal:
                    return wxCursor(wxCURSOR_SIZENS);
                case SplitMode_Vertical:
                    return wxCursor(wxCURSOR_SIZEWE);
                case SplitMode_Unset:
                    return wxCursor();
                DEFAULT_SWITCH()
            }
        }
        
        void SplitterWindow::OnSize(wxSizeEvent& event) {
            updateSashPosition(m_oldSize, event.GetSize());
            sizeWindows();
            m_oldSize = event.GetSize();
            event.Skip();
        }
        
        void SplitterWindow::updateSashPosition(const wxSize& oldSize, const wxSize& newSize) {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                const wxSize diff = newSize - oldSize;
                const int actualDiff = wxRound(m_sashGravity * h(diff));
                if (actualDiff != 0)
                    setSashPosition(m_sashPosition + actualDiff);
            }
        }
        
        void SplitterWindow::initSashPosition() {
            if (m_splitMode != SplitMode_Unset && m_sashPosition == -1 && h(GetClientSize()) > 0)
                setSashPosition(h(m_minSizes[0]) + wxRound(m_sashGravity * (h(m_minSizes[1]) - h(m_minSizes[0]))) + 1);
        }
        
        void SplitterWindow::setSashPosition(const int position) {
            if (position == m_sashPosition)
                return;
            m_sashPosition = position;
            m_sashPosition = std::max(m_sashPosition, h(m_minSizes[0]));
            m_sashPosition = std::min(m_sashPosition, h(GetClientSize()) - h(m_minSizes[1]) - sashSize());
            if (m_sashPosition < 0)
                m_sashPosition = h(GetClientSize()) / 2;
        }
        
        void SplitterWindow::sizeWindows() {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                const int origH = h(GetClientAreaOrigin());
                const int origV = h(wxWindowBase::GetClientAreaOrigin());
                const int sizeH = h(GetClientSize());
                const int sizeV = v(GetClientSize());
                
                wxPoint pos[2];
                wxSize size[2];
                
                setHV(pos[0], origH, origV);
                setHV(pos[1], origH + m_sashPosition + sashSize(), origV);
                setHV(size[0], m_sashPosition, sizeV);
                setHV(size[1], sizeH - m_sashPosition - sashSize(), sizeV);
                
                for (size_t i = 0; i < NumWindows; ++i)
                    m_windows[i]->SetSize(wxRect(pos[i], size[i]));

                wxPoint sashPos;
                wxSize sashSize;
                
                setHV(sashPos, origH + m_sashPosition, origV);
                setHV(sashSize, SplitterWindow::sashSize(), sizeV);
                m_sash->SetSize(wxRect(sashPos, sashSize));
                
                Refresh();
            }
        }
        
        int SplitterWindow::sashSize() const {
            return 2;
        }
    }
}
