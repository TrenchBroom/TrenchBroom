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

#include "SplitterWindow4.h"

#include "View/BorderLine.h"
#include "View/PersistentSplitterWindow4.h"
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
        SplitterWindow4::SplitterWindow4(wxWindow* parent) :
        wxPanel(parent),
        m_maximizedWindow(NULL),
        m_initialSashPosition(-1, -1),
        m_sashPosition(-1, -1),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxDefaultSize;
            }
            
            for (size_t i = 0; i < 2; ++i)
                m_dragging[i] = false;

            SetForegroundColour(Colors::borderColor());
            
            Bind(wxEVT_PAINT, &SplitterWindow4::OnPaint, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow4::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow4::OnSize, this);
            Bind(wxEVT_IDLE, &SplitterWindow4::OnIdle, this);
            Bind(wxEVT_LEFT_DOWN, &SplitterWindow4::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &SplitterWindow4::OnMouseButton, this);
            bindMouseEvents(this);
        }
        
        void SplitterWindow4::split(wxWindow* topLeft, wxWindow* topRight, wxWindow* bottomRight, wxWindow* bottomLeft,
                                    const wxSize& topLeftMin,
                                    const wxSize& topRightMin,
                                    const wxSize& bottomRightMin,
                                    const wxSize& bottomLeftMin) {
            assert(topLeft != NULL);
            assert(topLeft->GetParent() == this);
            assert(topRight != NULL);
            assert(topRight->GetParent() == this);
            assert(bottomRight != NULL);
            assert(bottomRight->GetParent() == this);
            assert(bottomLeft != NULL);
            assert(bottomLeft->GetParent() == this);
            
            m_windows[Window_TopLeft] = topLeft;
            m_windows[Window_TopRight] = topRight;
            m_windows[Window_BottomRight] = bottomRight;
            m_windows[Window_BottomLeft] = bottomLeft;
            m_minSizes[Window_TopLeft] = topLeftMin;
            m_minSizes[Window_TopRight] = topRightMin;
            m_minSizes[Window_BottomRight] = bottomRightMin;
            m_minSizes[Window_BottomLeft] = bottomLeftMin;
            
            for (size_t i = 0; i < NumWindows; ++i) {
                setMinSize(m_windows[i], m_minSizes[i]);
                bindMouseEvents(m_windows[i]);
            }
        }
        
        void SplitterWindow4::setMinSize(wxWindow* window, const wxSize& minSize) {
            assert(containsWindow(window));
            assert(minSize.x >= 0 && minSize.y >= 0);
            
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] == window)
                    m_minSizes[i] = minSize;
            }

            const wxSize minClientSize(leftColMinSize() + rightColMinSize() + sashSize(),
                                       topRowMinSize() + bottomRowMinSize() + sashSize());
            SetMinClientSize(minClientSize);
        }

        void SplitterWindow4::maximize(wxWindow* window) {
            m_maximizedWindow = window;
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] != window)
                    m_windows[i]->Hide();
            }
            m_maximizedWindow = window;
            m_maximizedWindow->Show();
            sizeWindows();
        }
        
        void SplitterWindow4::restore() {
            if (m_maximizedWindow != NULL) {
                m_maximizedWindow = NULL;
                for (size_t i = 0; i < NumWindows; ++i)
                    m_windows[i]->Show();
                sizeWindows();
            }
        }

        int SplitterWindow4::leftColMinSize() const {
            return std::max(m_minSizes[Window_TopLeft].x, m_minSizes[Window_BottomLeft].x);
        }
        
        int SplitterWindow4::rightColMinSize() const {
            return std::max(m_minSizes[Window_TopRight].x, m_minSizes[Window_BottomRight].x);
        }
        
        int SplitterWindow4::topRowMinSize() const {
            return std::max(m_minSizes[Window_TopLeft].y, m_minSizes[Window_TopRight].y);
        }
        
        int SplitterWindow4::bottomRowMinSize() const {
            return std::max(m_minSizes[Window_BottomLeft].y, m_minSizes[Window_BottomRight].y);
        }

        bool SplitterWindow4::hasWindows() const {
            return m_windows[0] != NULL;
        }

        bool SplitterWindow4::containsWindow(wxWindow* window) const {
            for (size_t i = 0; i < NumWindows; ++i)
                if (m_windows[i] == window)
                    return true;
            return false;
        }

        void SplitterWindow4::bindMouseEvents(wxWindow* window) {
            assert(window != NULL);
            window->Bind(wxEVT_ENTER_WINDOW, &SplitterWindow4::OnMouseEnter, this);
            window->Bind(wxEVT_LEAVE_WINDOW, &SplitterWindow4::OnMouseLeave, this);
            window->Bind(wxEVT_MOTION, &SplitterWindow4::OnMouseMotion, this);
        }

        void SplitterWindow4::OnMouseEnter(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            updateSashCursor();
            event.Skip();
        }
        
        void SplitterWindow4::OnMouseLeave(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            updateSashCursor();
            event.Skip();
        }
        
        void SplitterWindow4::OnMouseButton(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            if (event.LeftDown()) {
                CaptureMouse();
                m_dragging[Dim_X] = sashHitTest(event.GetPosition(), Dim_X);
                m_dragging[Dim_Y] = sashHitTest(event.GetPosition(), Dim_Y);
            } else if (event.LeftUp() && GetCapture() == this) {
                ReleaseMouse();
                m_dragging[Dim_X] = m_dragging[Dim_Y] = false;
            }
            updateSashCursor();
            Refresh();
            event.Skip();
        }
        
        void SplitterWindow4::OnMouseMotion(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            if (GetCapture() == this) {
                assert(hasWindows());
                
                wxPoint newPosition = m_sashPosition;
                if (m_dragging[Dim_X])
                    newPosition.x = event.GetPosition().x;
                if (m_dragging[Dim_Y])
                    newPosition.y = event.GetPosition().y;
                setSashPosition(newPosition);
                sizeWindows();
            } else {
                updateSashCursor();
            }
            event.Skip();
        }
        
        void SplitterWindow4::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (IsBeingDeleted()) return;

            m_dragging[Dim_X] = m_dragging[Dim_Y] = false;
            updateSashCursor();
            event.Skip();
        }

        void SplitterWindow4::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            wxPaintDC dc(this);
            dc.SetPen(wxPen(GetForegroundColour()));
            dc.SetBrush(wxBrush(GetForegroundColour()));

            const wxPoint origin = GetClientAreaOrigin();
            const wxSize size = GetClientSize();
            
            dc.DrawRectangle(m_sashPosition.x, origin.y, sashSize(), size.y);
            dc.DrawRectangle(origin.x, m_sashPosition.y, size.x, sashSize());
            event.Skip();
        }

        void SplitterWindow4::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            if (IsShownOnScreen()) {
                Unbind(wxEVT_IDLE, &SplitterWindow4::OnIdle, this);
                
                // if the initial sash position could not be set until now, then it probably cannot be set at all
                m_initialSashPosition.x = m_initialSashPosition.y = -1;
            }
        }
        
        void SplitterWindow4::OnSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            updateSashPosition(m_oldSize, event.GetSize());
            sizeWindows();
            m_oldSize = event.GetSize();
            event.Skip();
        }

        void SplitterWindow4::updateSashCursor() {
            const wxPoint screenPos = wxGetMousePosition();
            const wxPoint clientPos = ScreenToClient(screenPos);
            const bool xResize = m_dragging[Dim_X] || sashHitTest(clientPos, Dim_X);
            const bool yResize = m_dragging[Dim_Y] || sashHitTest(clientPos, Dim_Y);

            if (xResize && yResize)
                wxSetCursor(wxCursor(wxCURSOR_SIZING));
            else if (xResize)
                wxSetCursor(wxCursor(wxCURSOR_SIZEWE));
            else if (yResize)
                wxSetCursor(wxCursor(wxCURSOR_SIZENS));
            else
                wxSetCursor(wxCursor(wxCURSOR_ARROW));
        }
        
        
        bool SplitterWindow4::sashHitTest(const wxPoint& point, const Dim dim) const {
            const int v = get(point, dim);
            const int s = get(m_sashPosition, dim);
            return v >= s && v <= s + sashSize();
        }

        void SplitterWindow4::updateSashPosition(const wxSize& oldSize, const wxSize& newSize) {
            if (!initSashPosition() && hasWindows()) {
                const wxSize diff = (newSize - oldSize) / 2;
                if (diff.x != 0 || diff.y != 0)
                    setSashPosition(m_sashPosition + diff);
            }
        }
        
        bool SplitterWindow4::initSashPosition() {
            const wxSize clientSize = GetClientSize();
            if (hasWindows() && (m_sashPosition.x == -1 || m_sashPosition.y == -1) &&
                clientSize.x > 0 && clientSize.y > 0) {
                setSashPosition(wxPoint(clientSize.x / 2, clientSize.y / 2));
                return true;
            }
            return false;
        }
        
        bool SplitterWindow4::setSashPosition(wxPoint sashPosition) {
            if (m_initialSashPosition.x != -1)
                sashPosition.x = m_initialSashPosition.x;
            if (m_initialSashPosition.y != -1)
                sashPosition.y = m_initialSashPosition.y;
            if (sashPosition == m_sashPosition)
                return true;
            m_sashPosition = sashPosition;
            m_sashPosition.x = std::max(m_sashPosition.x, leftColMinSize());
            m_sashPosition.x = std::min(m_sashPosition.x, GetClientSize().x - sashSize() - rightColMinSize());
            m_sashPosition.y = std::max(m_sashPosition.y, topRowMinSize());
            m_sashPosition.y = std::min(m_sashPosition.y, GetClientSize().x - sashSize() - bottomRowMinSize());
            
            m_sashPosition.x = std::max(m_sashPosition.x, -1);
            m_sashPosition.y = std::max(m_sashPosition.y, -1);
            
            return m_sashPosition.x != -1 && m_sashPosition.y != -1;
        }
        
        void SplitterWindow4::sizeWindows() {
            initSashPosition();
            
            if (hasWindows()) {
                if (m_maximizedWindow != NULL) {
                    m_maximizedWindow->SetSize(wxRect(GetClientAreaOrigin(), GetClientSize()));
                } else {
                    const wxWindowUpdateLocker lockUpdates(this);
                    
                    const wxPoint origin = GetClientAreaOrigin();
                    const wxSize size = GetClientSize();
                    const wxPoint& sash = m_sashPosition;
                    const int leftColX = origin.x;
                    const int leftColW = sash.x;
                    const int rightColX = leftColX + leftColW + sashSize();
                    const int rightColW = size.x - rightColX;
                    const int topRowY = origin.y;
                    const int topRowH = sash.y;
                    const int bottomRowY = topRowY + topRowH + sashSize();
                    const int bottomRowH = size.y - bottomRowY;
                    
                    m_windows[Window_TopLeft]->SetPosition(wxPoint(leftColX, topRowY));
                    m_windows[Window_TopLeft]->SetSize(wxSize(leftColW, topRowH));
                    m_windows[Window_TopRight]->SetPosition(wxPoint(rightColX, topRowY));
                    m_windows[Window_TopRight]->SetSize(wxSize(rightColW, topRowH));
                    m_windows[Window_BottomRight]->SetPosition(wxPoint(rightColX, bottomRowY));
                    m_windows[Window_BottomRight]->SetSize(wxSize(rightColW, bottomRowH));
                    m_windows[Window_BottomLeft]->SetPosition(wxPoint(leftColX, bottomRowY));
                    m_windows[Window_BottomLeft]->SetSize(wxSize(leftColW, bottomRowH));
                }
            }
        }
        
        int SplitterWindow4::sashSize() const {
            return 2;
        }
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow4* window) {
    return new TrenchBroom::View::PersistentSplitterWindow4(window);
}
