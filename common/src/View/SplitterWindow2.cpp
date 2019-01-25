/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include <cassert>
#include <iostream>
#include <algorithm>

namespace TrenchBroom {
    namespace View {
        SplitterWindow2::SplitterWindow2(QWidget* parent) :
        QWidget(parent, wxID_ANY),
        m_splitMode(SplitMode_Unset),
        m_sash(nullptr),
        m_maximizedWindow(nullptr),
        m_sashGravity(0.5),
        m_initialSplitRatio(-1.0),
        m_currentSplitRatio(m_initialSplitRatio),
        m_sashCursorSet(0),
        m_oldSize(GetSize()) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = nullptr;
                m_minSizes[i] = wxDefaultSize;
            }
            
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitterWindow2::OnMouseCaptureLost, this);
            Bind(wxEVT_SIZE, &SplitterWindow2::OnSize, this);
            Bind(wxEVT_IDLE, &SplitterWindow2::OnIdle, this);
        }
        
        void SplitterWindow2::splitHorizontally(QWidget* left, QWidget* right, const wxSize& leftMin, const wxSize& rightMin) {
            split(left, right, leftMin, rightMin, SplitMode_Horizontal);
        }
        
        void SplitterWindow2::splitVertically(QWidget* top, QWidget* bottom, const wxSize& topMin, const wxSize& bottomMin) {
            split(top, bottom, topMin, bottomMin, SplitMode_Vertical);
        }
        
        void SplitterWindow2::setMinSize(QWidget* window, const wxSize& minSize) {
            assert(m_splitMode != SplitMode_Unset);
            
            wxSize splitterMinSize;
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] == window) {
                    m_minSizes[i] = minSize;
                }

                setH(splitterMinSize, h(splitterMinSize) + h(m_minSizes[i]));
                setV(splitterMinSize, std::max(v(splitterMinSize), v(m_minSizes[i])));
            }
            
            SetMinClientSize(splitterMinSize);
        }
        
        void SplitterWindow2::setSashGravity(const double sashGravity) {
            m_sashGravity = std::max(std::min(sashGravity, 1.0), 0.0);
        }
        
        bool SplitterWindow2::isMaximized(QWidget* window) const {
            assert(window == m_windows[0] || window == m_windows[1]);
            return (m_maximizedWindow == window);
        }
        
        void SplitterWindow2::maximize(QWidget* window) {
            assert(window == m_windows[0] || window == m_windows[1]);
            m_maximizedWindow = window;
            m_maximizedWindow->Show();
            unmaximizedWindow()->Hide();
            sizeWindows();
        }
        
        void SplitterWindow2::restore() {
            if (m_maximizedWindow != nullptr) {
                unmaximizedWindow()->Show();
                m_maximizedWindow = nullptr;
                sizeWindows();
            }
        }

        int SplitterWindow2::currentSashPosition() const {
            return sashPosition(m_currentSplitRatio);
        }
        
        int SplitterWindow2::sashPosition(const double ratio) const {
            return sashPosition(ratio, h(GetSize()));
        }
        
        int SplitterWindow2::sashPosition(const double ratio, const wxCoord size) const {
            return static_cast<int>(ratio * size);
        }
        
        double SplitterWindow2::splitRatio(const int position) const {
            const auto l = static_cast<double>(h(GetSize()));
            if (l <= 0.0) {
                return -1.0;
            } else {
                return static_cast<double>(position) / l;
            }
        }

        void SplitterWindow2::split(QWidget* window1, QWidget* window2, const wxSize& min1, const wxSize& min2, const SplitMode splitMode) {
            ensure(window1 != nullptr, "window1 is null");
            assert(window1->GetParent() == this);
            ensure(window2 != nullptr, "window2 is null");
            assert(window2->GetParent() == this);
            assert(m_splitMode == SplitMode_Unset);
            
            m_windows[0] = window1;
            m_windows[1] = window2;
            m_splitMode = splitMode;
            
            if (m_splitMode == SplitMode_Horizontal) {
                m_sash = new BorderLine(this, BorderLine::Direction_Vertical, sashSize());
            } else {
                m_sash = new BorderLine(this, BorderLine::Direction_Horizontal, sashSize());
            }
            bindMouseEvents(m_sash);
            
            setMinSize(window1, min1);
            setMinSize(window2, min2);
        }
        
        void SplitterWindow2::bindMouseEvents(QWidget* window) {
            window->Bind(wxEVT_ENTER_WINDOW, &SplitterWindow2::OnMouseEnter, this);
            window->Bind(wxEVT_LEAVE_WINDOW, &SplitterWindow2::OnMouseLeave, this);
            window->Bind(wxEVT_LEFT_DOWN, &SplitterWindow2::OnMouseButton, this);
            window->Bind(wxEVT_LEFT_UP, &SplitterWindow2::OnMouseButton, this);
            window->Bind(wxEVT_MOTION, &SplitterWindow2::OnMouseMotion, this);
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
            
            if (event.LeftDown()) {
                m_sash->CaptureMouse();
            } else if (event.LeftUp() && dragging()) {
                m_sash->ReleaseMouse();
            }
            setSashCursor();
        }
        
        void SplitterWindow2::OnMouseMotion(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            assert(m_splitMode != SplitMode_Unset);
            
            const auto screenPos = wxGetMousePosition();
            const auto clientPos = ScreenToClient(screenPos);
            
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
            if (dragging() || m_sash->HitTest(m_sash->ScreenToClient(wxGetMousePosition())) != wxHT_WINDOW_OUTSIDE) {
                wxSetCursor(sizeCursor());
            } else {
                wxSetCursor(wxCursor(wxCURSOR_ARROW));
            }
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
                m_initialSplitRatio = -1.0;
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
                const auto diff = newSize - oldSize;
                const auto actualDiff = wxRound(m_sashGravity * h(diff));
                setSashPosition(sashPosition(m_currentSplitRatio, h(oldSize)) + actualDiff);
            }
        }
        
        void SplitterWindow2::initSashPosition() {
            if (m_splitMode != SplitMode_Unset && m_currentSplitRatio == -1.0 && h(GetClientSize()) > 0) {
                setSashPosition(h(m_minSizes[0]) + wxRound(m_sashGravity * (h(m_minSizes[1]) - h(m_minSizes[0]))));
            }
        }
        
        bool SplitterWindow2::setSashPosition(int newSashPosition) {
            if (m_initialSplitRatio != -1.0) {
                newSashPosition = sashPosition(m_initialSplitRatio);
            }
            if (newSashPosition == currentSashPosition()) {
                return true;
            }

            newSashPosition = std::min(newSashPosition, h(GetClientSize()) - h(m_minSizes[1]) - sashSize());
            newSashPosition = std::max(newSashPosition, h(m_minSizes[0]));
            if (newSashPosition >= h(m_minSizes[0]) && newSashPosition <= h(GetClientSize()) - h(m_minSizes[1])) {
                m_currentSplitRatio = splitRatio(newSashPosition);
            }

            return m_currentSplitRatio >= 0.0;
        }
        
        void SplitterWindow2::sizeWindows() {
            initSashPosition();
            
            if (m_splitMode != SplitMode_Unset) {
                if (m_maximizedWindow != nullptr) {
                    m_maximizedWindow->SetSize(wxRect(GetClientAreaOrigin(), GetClientSize()));
                    m_sash->SetSize(wxRect(wxPoint(0, 0),wxPoint(0, 0)));
                } else {
                    const auto origH = h(GetClientAreaOrigin());
                    const auto origV = h(GetClientAreaOrigin());
                    const auto sizeH = h(GetClientSize());
                    const auto sizeV = v(GetClientSize());
                    
                    wxPoint pos[2];
                    wxSize size[2];
                    
                    setHV(pos[0], origH, origV);
                    setHV(pos[1], origH + currentSashPosition() + sashSize(), origV);
                    setHV(size[0], currentSashPosition(), sizeV);
                    setHV(size[1], sizeH - currentSashPosition() - sashSize(), sizeV);
                    
                    for (size_t i = 0; i < NumWindows; ++i) {
                        m_windows[i]->SetSize(wxRect(pos[i], size[i]));
                    }

                    wxPoint sashPos;
                    wxSize sashSize;
                    
                    setHV(sashPos, origH + currentSashPosition(), origV);
                    setHV(sashSize, SplitterWindow2::sashSize(), sizeV);
                    m_sash->SetSize(wxRect(sashPos, sashSize));
                }
            }
        }
        
        int SplitterWindow2::sashSize() const {
            return 2;
        }
 
        QWidget* SplitterWindow2::unmaximizedWindow() {
            ensure(m_maximizedWindow != nullptr, "maximizedWindow is null");
            return m_windows[0] == m_maximizedWindow ? m_windows[1] : m_windows[0];
        }
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow2* window) {
    return new TrenchBroom::View::PersistentSplitterWindow2(window);
}
