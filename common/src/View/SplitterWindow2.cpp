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

#include "SplitterWindow2.h"

#include "View/BorderLine.h"
#include "View/PersistentSplitterWindow2.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/control.h>
#include <wx/dcclient.h>
#include <wx/log.h>
#include <wx/wupdlock.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        SplitterWindow2::SplitterWindow2(wxWindow* parent) :
        wxPanel(parent, wxID_ANY),
        m_splitMode(SplitMode_Unset),
        m_sash(NULL),
        m_maximizedWindow(NULL),
        m_sashGravity(0.5f),
        m_initialSashPosition(-1),
        m_sashPosition(-1),
        m_sashCursorSet(0),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxDefaultSize;
            }
            
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow2::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow2::OnSize, this);
            Bind(wxEVT_IDLE, &SplitterWindow2::OnIdle, this);
        }
        
        void SplitterWindow2::splitHorizontally(wxWindow* left, wxWindow* right, const wxSize& leftMin, const wxSize& rightMin) {
            split(left, right, leftMin, rightMin, SplitMode_Horizontal);
        }
        
        void SplitterWindow2::splitVertically(wxWindow* top, wxWindow* bottom, const wxSize& topMin, const wxSize& bottomMin) {
            split(top, bottom, topMin, bottomMin, SplitMode_Vertical);
        }
        
        bool SplitterWindow2::isMaximized(wxWindow* window) const {
            assert(window == m_windows[0] || window == m_windows[1]);
            return (m_maximizedWindow == window);
        }
        
        void SplitterWindow2::maximize(wxWindow* window) {
            assert(window == m_windows[0] || window == m_windows[1]);
            m_maximizedWindow = window;
            m_maximizedWindow->Show();
            unmaximizedWindow()->Hide();
            sizeWindows();
        }
        
        void SplitterWindow2::restore() {
            if (m_maximizedWindow != NULL) {
                unmaximizedWindow()->Show();
                m_maximizedWindow = NULL;
                sizeWindows();
            }
        }

        void SplitterWindow2::split(wxWindow* window1, wxWindow* window2, const wxSize& min1, const wxSize& min2, const SplitMode splitMode) {
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
        
        void SplitterWindow2::bindMouseEvents(wxWindow* window) {
            window->Bind(wxEVT_ENTER_WINDOW, &SplitterWindow2::OnMouseEnter, this);
            window->Bind(wxEVT_LEAVE_WINDOW, &SplitterWindow2::OnMouseLeave, this);
            window->Bind(wxEVT_LEFT_DOWN, &SplitterWindow2::OnMouseButton, this);
            window->Bind(wxEVT_LEFT_UP, &SplitterWindow2::OnMouseButton, this);
            window->Bind(wxEVT_MOTION, &SplitterWindow2::OnMouseMotion, this);
        }
        
        void SplitterWindow2::setMinSize(wxWindow* window, const wxSize& minSize) {
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
        
        void SplitterWindow2::setSashGravity(const float sashGravity) {
            assert(sashGravity >= 0.0f && sashGravity <= 1.0f);
            m_sashGravity = sashGravity;
        }
        
        void SplitterWindow2::OnMouseEnter(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            setSashCursor();
        }
        
        void SplitterWindow2::OnMouseLeave(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            setSashCursor();
        }
        
        void SplitterWindow2::OnMouseButton(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            assert(m_splitMode != SplitMode_Unset);
            
            if (event.LeftDown())
                m_sash->CaptureMouse();
            else if (event.LeftUp() && dragging())
                m_sash->ReleaseMouse();
			setSashCursor();
            Refresh();
        }
        
        void SplitterWindow2::OnMouseMotion(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            assert(m_splitMode != SplitMode_Unset);
            
            const wxPoint screenPos = wxGetMousePosition();
            const wxPoint clientPos = ScreenToClient(screenPos);
            
            if (dragging()) {
                setSashPosition(h(clientPos));
                sizeWindows();
            }
            setSashCursor();
        }
        
        void SplitterWindow2::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (IsBeingDeleted()) return;

            setSashCursor();
        }
        
        bool SplitterWindow2::dragging() const {
            return GetCapture() == m_sash;
        }
        
        void SplitterWindow2::setSashCursor() {
			if (dragging() || m_sash->HitTest(m_sash->ScreenToClient(wxGetMousePosition())) != wxHT_WINDOW_OUTSIDE)
				wxSetCursor(sizeCursor());
			else
				wxSetCursor(wxCursor(wxCURSOR_ARROW));
        }
        
        wxCursor SplitterWindow2::sizeCursor() const {
            switch (m_splitMode) {
                case SplitMode_Horizontal:
                    return wxCursor(wxCURSOR_SIZENS);
                case SplitMode_Vertical:
                    return wxCursor(wxCURSOR_SIZEWE);
                case SplitMode_Unset:
                    return wxCursor();
                    switchDefault()
            }
        }
        
        void SplitterWindow2::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            if (IsShownOnScreen()) {
                Unbind(wxEVT_IDLE, &SplitterWindow2::OnIdle, this);
                
                // if the initial sash position could not be set until now, then it probably cannot be set at all
                m_initialSashPosition = -1;
            }
        }
        void SplitterWindow2::OnSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            updateSashPosition(m_oldSize, event.GetSize());
            sizeWindows();
            m_oldSize = event.GetSize();
            event.Skip();
        }
        
        void SplitterWindow2::updateSashPosition(const wxSize& oldSize, const wxSize& newSize) {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                const wxSize diff = newSize - oldSize;
                const int actualDiff = wxRound(m_sashGravity * h(diff));
                if (actualDiff != 0)
                    setSashPosition(m_sashPosition + actualDiff);
            }
        }
        
        void SplitterWindow2::initSashPosition() {
            if (m_splitMode != SplitMode_Unset && m_sashPosition == -1 && h(GetClientSize()) > 0)
                setSashPosition(h(m_minSizes[0]) + wxRound(m_sashGravity * (h(m_minSizes[1]) - h(m_minSizes[0]))) + 1);
        }
        
        bool SplitterWindow2::setSashPosition(int position) {
            if (m_initialSashPosition != -1)
                position = m_initialSashPosition;
            if (position == m_sashPosition)
                return true;
            m_sashPosition = position;
            m_sashPosition = std::max(m_sashPosition, h(m_minSizes[0]));
            m_sashPosition = std::min(m_sashPosition, h(GetClientSize()) - h(m_minSizes[1]) - sashSize());
            if (m_sashPosition < 0) {
                m_sashPosition = -1;
                return false;
            }
            return true;
        }
        
        void SplitterWindow2::sizeWindows() {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                const wxWindowUpdateLocker lockUpdates(this);
                
                if (m_maximizedWindow != NULL) {
                    m_maximizedWindow->SetSize(wxRect(GetClientAreaOrigin(), GetClientSize()));
                    m_sash->SetSize(wxRect(wxPoint(0, 0),wxPoint(0, 0)));
                } else {
                    const int origH = h(GetClientAreaOrigin());
                    const int origV = h(GetClientAreaOrigin());
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
                    setHV(sashSize, SplitterWindow2::sashSize(), sizeV);
                    m_sash->SetSize(wxRect(sashPos, sashSize));
                }
            }
        }
        
        int SplitterWindow2::sashSize() const {
            return 2;
        }
 
        wxWindow* SplitterWindow2::unmaximizedWindow() {
            assert(m_maximizedWindow != NULL);
            return m_windows[0] == m_maximizedWindow ? m_windows[1] : m_windows[0];
        }
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow2* window) {
    return new TrenchBroom::View::PersistentSplitterWindow2(window);
}
