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
#include "View/PersistentSplitterWindow2.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/control.h>
#include <wx/dcclient.h>
#include <wx/log.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        SplitterWindow4::SplitterWindow4(wxWindow* parent) :
        wxPanel(parent),
        m_sashCursorSet(false),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxDefaultSize;
            }
            
            for (size_t i = 0; i < 2; ++i) {
                m_initialSashPosition[i] = -1;
                m_sashPosition[i] = -1;
            }

            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow4::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow4::OnSize, this);
            Bind(wxEVT_IDLE, &SplitterWindow4::OnIdle, this);
            Bind(wxEVT_ENTER_WINDOW, &SplitterWindow4::OnMouseEnter, this);
            Bind(wxEVT_LEAVE_WINDOW, &SplitterWindow4::OnMouseLeave, this);
            Bind(wxEVT_LEFT_DOWN, &SplitterWindow4::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &SplitterWindow4::OnMouseButton, this);
            Bind(wxEVT_MOTION, &SplitterWindow4::OnMouseMotion, this);
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
            
            for (size_t i = 0; i < NumWindows; ++i)
                setMinSize(m_windows[i], m_minSizes[i]);
        }
        
        void SplitterWindow4::setMinSize(wxWindow* window, const wxSize& minSize) {
            assert(containsWindow(window));
            assert(minSize.x >= 0 && minSize.y >= 0);
            
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] == window)
                    m_minSizes[i] = minSize;
            }

            const int leftColMinSize   = std::max(m_minSizes[Window_TopLeft].x,    m_minSizes[Window_BottomLeft].x);
            const int rightColMinSize  = std::max(m_minSizes[Window_TopRight].x,   m_minSizes[Window_BottomRight].x);
            const int topRowMinSize    = std::max(m_minSizes[Window_TopLeft].y,    m_minSizes[Window_TopRight].y);
            const int bottomRowMinSize = std::max(m_minSizes[Window_BottomLeft].y, m_minSizes[Window_BottomRight].y);
            
            const wxSize minClientSize(leftColMinSize + rightColMinSize + sashSize(),
                                       topRowMinSize + bottomRowMinSize + sashSize());
            SetMinClientSize(minClientSize);
        }

        bool SplitterWindow4::containsWindow(wxWindow* window) const {
            for (size_t i = 0; i < NumWindows; ++i)
                if (m_windows[i] == window)
                    return true;
            return false;
        }

        void SplitterWindow4::OnMouseEnter(wxMouseEvent& event) {
            setSashCursor();
        }
        
        void SplitterWindow4::OnMouseLeave(wxMouseEvent& event) {
            setSashCursor();
        }
        
        void SplitterWindow4::OnMouseButton(wxMouseEvent& event) {
        }
        
        void SplitterWindow4::OnMouseMotion(wxMouseEvent& event) {
        }
        
        void SplitterWindow4::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            setSashCursor();
        }

        bool SplitterWindow4::dragging() const {
            return GetCapture() == this;
        }
        
        void SplitterWindow4::setSashCursor() {
            const wxPoint screenPos = wxGetMousePosition();
            const wxPoint clientPos = ScreenToClient(screenPos);
            if (dragging() || m_sash->HitTest(m_sash->ScreenToClient(wxGetMousePosition())) != wxHT_WINDOW_OUTSIDE)
                wxSetCursor(sizeCursor());
            else
                wxSetCursor(wxCursor(wxCURSOR_ARROW));
        }
        
        wxCursor SplitterWindow4::sizeCursor() const {
        }
        
        bool SplitterWindow4::isOnHSash(const wxPoint& point) const {
            return point.x >= m_sashPosition[Dim_X] && point.x <= m_sashPosition[Dim_X] + sashSize();
        }
        
        bool SplitterWindow4::isOnVSash(const wxPoint& point) const {
            return point.y >= m_sashPosition[Dim_Y] && point.y <= m_sashPosition[Dim_Y] + sashSize();
        }
    }
}
