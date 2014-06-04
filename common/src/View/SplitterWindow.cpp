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

#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/control.h>
#include <wx/dcclient.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        SplitterWindow::SplitterWindow(wxWindow* parent) :
        wxPanel(parent, wxID_ANY),
        m_splitMode(SplitMode_Unset),
        m_sashWindow(NULL),
        m_sashGravity(0.5f),
        m_sashPosition(-1),
        m_sashCursorSet(0),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxSize(0, 0);
            }
            
            bindMouseEvents(this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow::OnSize, this);
            Bind(wxEVT_PAINT, &SplitterWindow::OnPaint, this);
        }
        
        void SplitterWindow::splitHorizontally(wxWindow* left, wxWindow* right) {
            assert(left != NULL);
            assert(left->GetParent() == this);
            assert(right != NULL);
            assert(right->GetParent() == this);
            assert(m_splitMode == SplitMode_Unset);
            
            m_windows[0] = left;
            m_windows[1] = right;
            m_splitMode = SplitMode_Horizontal;
            
            for (size_t i = 0; i < 2; ++i)
                bindMouseEventsRecurse(m_windows[i]);
        }
        
        void SplitterWindow::splitVertically(wxWindow* top, wxWindow* bottom) {
            assert(top != NULL);
            assert(top->GetParent() == this);
            assert(bottom != NULL);
            assert(bottom->GetParent() == this);
            assert(m_splitMode == SplitMode_Unset);
            
            m_windows[0] = top;
            m_windows[1] = bottom;
            m_splitMode = SplitMode_Vertical;
            
            for (size_t i = 0; i < 2; ++i)
                bindMouseEventsRecurse(m_windows[i]);
        }
        
        void SplitterWindow::setSashWindow(wxWindow* sashWindow) {
            assert(sashWindow != NULL);
            assert(sashWindow->GetParent() == this);
            assert(m_sashWindow == NULL);
            
            m_sashWindow = sashWindow;
            bindMouseEventsRecurse(m_sashWindow);
            m_sashWindow->Fit();
        }
        
        void SplitterWindow::bindMouseEventsRecurse(wxWindow* window) {
            bindMouseEvents(window);
            const wxWindowList& children = window->GetChildren();
            
            wxWindowList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it)
                bindMouseEventsRecurse(*it);
        }
        
        void SplitterWindow::bindMouseEvents(wxWindow* window) {
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
            
            if (m_sashWindow != NULL) {
                setH(splitterMinSize, h(splitterMinSize) + h(m_sashWindow->GetSize()));
                setV(splitterMinSize, v(splitterMinSize) + v(m_sashWindow->GetSize()));
            }
            
            SetMinClientSize(splitterMinSize);
        }
        
        void SplitterWindow::setSashGravity(const float sashGravity) {
            assert(sashGravity >= 0.0f && sashGravity <= 1.0f);
            m_sashGravity = sashGravity;
        }
        
        void SplitterWindow::OnMouseButton(wxMouseEvent& event) {
            assert(m_splitMode != SplitMode_Unset);
            
            if (event.LeftDown()) {
                wxWindow* window = static_cast<wxWindow*>(event.GetEventObject());
                const wxPoint screenPos = wxGetMousePosition();
                const wxPoint clientPos = ScreenToClient(screenPos);
                
                if (isOnSash(clientPos, window)) {
                    CaptureMouse();
                    m_dragOffset = h(clientPos) - m_sashPosition;
                } else {
                    event.Skip();
                }
            } else if (event.LeftUp() && dragging()) {
                ReleaseMouse();
            } else {
                event.Skip();
            }
        }
        
        void SplitterWindow::OnMouseMotion(wxMouseEvent& event) {
            assert(m_splitMode != SplitMode_Unset);
            
            wxWindow* window = static_cast<wxWindow*>(event.GetEventObject());
            const wxPoint screenPos = wxGetMousePosition();
            const wxPoint clientPos = ScreenToClient(screenPos);
            
            if (dragging()) {
                setSashPosition(h(clientPos) - m_dragOffset);
                sizeWindows();
                setSashCursor();
            } else {
                if (isOnSash(clientPos, window))
                    setSashCursor();
                else
                    unsetSashCursor();
                event.Skip();
            }
        }

        void SplitterWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {}
        
        bool SplitterWindow::dragging() const {
            return GetCapture() == this;
        }
        
        bool SplitterWindow::isOnSash(const wxPoint& pos, const wxWindow* window) const {
            assert(m_splitMode != SplitMode_Unset);

            // make sure to stay out of other splitter windows' hot zones, too
            if (v(pos) <= HalfMinSashSize + 1 ||
                v(pos) >= v(GetClientSize()) - HalfMinSashSize - 1)
                return false;
            
            if (!window->IsKindOf(CLASSINFO(wxControl)) &&
                window->IsShownOnScreen() &&
                h(pos) >= m_sashPosition &&
                h(pos) <= m_sashPosition + sashSize())
                return true;
            
            if (sashSize() <= 2 * HalfMinSashSize &&
                h(pos) >= m_sashPosition + sashSize() / 2 - HalfMinSashSize &&
                h(pos) <= m_sashPosition + sashSize() / 2 + HalfMinSashSize)
                return true;
            
            return false;
        }
        
        void SplitterWindow::setSashCursor() {
            if (!m_sashCursorSet) {
                wxSetCursor(sizeCursor());
                m_sashCursorSet = true;
            }
        }
        
        void SplitterWindow::unsetSashCursor() {
            if (m_sashCursorSet) {
                wxSetCursor(wxNullCursor);
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
        
        void SplitterWindow::OnPaint(wxPaintEvent& event) {
            if (m_sashWindow == NULL) {
                wxPoint from, to;
                setHV(from, m_sashPosition, 0);
                setHV(to, m_sashPosition, v(GetClientSize()));
                
                wxPaintDC dc(this);
                dc.SetPen(wxPen(Colors::borderColor()));
                dc.DrawLine(from, to);
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
                setSashPosition(m_sashPosition + static_cast<int>(m_sashGravity * h(diff)));
            }
        }
        
        void SplitterWindow::initSashPosition() {
            if (m_splitMode != SplitMode_Unset && m_sashPosition == -1) {
                const int clientH = h(GetClientSize());
                m_sashPosition = h(m_minSizes[0]) + static_cast<int>(m_sashGravity * (clientH - h(m_minSizes[0]) - h(m_minSizes[1]) - sashSize()));
            }
        }
        
        void SplitterWindow::setSashPosition(const int position) {
            m_sashPosition = position;
            m_sashPosition = std::max(m_sashPosition, h(m_minSizes[0]));
            m_sashPosition = std::min(m_sashPosition, h(GetClientSize()) - h(m_minSizes[1]) - sashSize());
        }
        
        void SplitterWindow::sizeWindows() {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                const int clientV = v(GetClientSize());
                m_windows[0]->SetPosition(wxPoint(0, 0));
                
                wxPoint pos[2];
                wxSize size[2];
                
                setHV(pos[0], 0, 0);
                setHV(pos[1], m_sashPosition + sashSize(), 0);
                setHV(size[0], m_sashPosition, clientV);
                setHV(size[1], h(GetClientSize()) - m_sashPosition - sashSize(), clientV);
                
                for (size_t i = 0; i < NumWindows; ++i) {
                    m_windows[i]->SetPosition(pos[i]);
                    m_windows[i]->SetSize(size[i]);
                }
                
                if (m_sashWindow != NULL) {
                    m_sashWindow->Fit();
                    
                    wxPoint sashPos;
                    wxSize sashSize;
                    
                    setHV(sashPos, m_sashPosition, 0);
                    setHV(sashSize, this->sashSize(), v(GetClientSize()));
                    
                    m_sashWindow->SetPosition(sashPos);
                    m_sashWindow->SetSize(sashSize);
                }
            }
        }
        
        int SplitterWindow::sashSize() const {
            if (m_sashWindow == NULL)
                return 1;
            return h(m_sashWindow->GetSize());
        }
    }
}
