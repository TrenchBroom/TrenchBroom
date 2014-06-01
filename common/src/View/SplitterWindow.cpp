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
        m_oldSize(GetSize()),
        m_oldCursorWindow(NULL) {
            for (size_t i = 0; i < NumWindows; ++i) {
                m_windows[i] = NULL;
                m_minSizes[i] = wxSize(0, 0);
            }
            
            Bind(wxEVT_LEFT_DOWN, &SplitterWindow::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &SplitterWindow::OnMouseButton, this);
            Bind(wxEVT_MOTION, &SplitterWindow::OnMouseMotion, this);
            Bind(wxEVT_SIZE, &SplitterWindow::OnSize, this);
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
                bindMouseEvents(m_windows[i]);
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
                bindMouseEvents(m_windows[i]);
        }
        
        void SplitterWindow::setSashWindow(wxWindow* sashWindow) {
            assert(sashWindow != NULL);
            assert(sashWindow->GetParent() == this);
            assert(m_sashWindow == NULL);
            
            m_sashWindow = sashWindow;
            bindMouseEvents(m_sashWindow);
        }

        void SplitterWindow::setMinSize(wxWindow* window, const wxSize& minSize) {
            assert(m_splitMode != SplitMode_Unset);
            assert(minSize.x >= 0 && minSize.y != 0);
            
            wxSize splitterMinSize;
            for (size_t i = 0; i < NumWindows; ++i) {
                if (m_windows[i] == window)
                    m_minSizes[i] = minSize;
                
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        splitterMinSize.x = std::max(splitterMinSize.x, m_minSizes[i].x);
                        splitterMinSize.y += m_minSizes[i].y;
                        break;
                    case SplitMode_Vertical:
                        splitterMinSize.x += m_minSizes[i].x;
                        splitterMinSize.y = std::max(splitterMinSize.y, m_minSizes[i].y);
                        break;
                    case SplitMode_Unset:
                        break;
                }
            }
            
            if (m_sashWindow != NULL) {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        splitterMinSize.y += m_sashWindow->GetSize().y;
                        break;
                    case SplitMode_Vertical:
                        splitterMinSize.x += m_sashWindow->GetSize().x;
                        break;
                    case SplitMode_Unset:
                        break;
                }
            }
            
            SetMinClientSize(splitterMinSize);
        }

        void SplitterWindow::setSashGravity(const float sashGravity) {
            assert(sashGravity >= 0.0f && sashGravity <= 1.0f);
            m_sashGravity = sashGravity;
        }

        void SplitterWindow::OnMouseButton(wxMouseEvent& event) {
            if (event.LeftDown()) {
                wxWindow* window = static_cast<wxWindow*>(event.GetEventObject());
                const wxPoint screenPos = window->ClientToScreen(event.GetPosition());
                const wxPoint clientPos = ScreenToClient(screenPos);

                if (isOnSash(clientPos)) {
                    CaptureMouse();
                    switch (m_splitMode) {
                        case SplitMode_Horizontal:
                            m_dragOffset = clientPos.y - m_sashPosition;
                            break;
                        case SplitMode_Vertical:
                            m_dragOffset = clientPos.x - m_sashPosition;
                            break;
                        case SplitMode_Unset:
                            break;
                    }
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
            wxWindow* window = static_cast<wxWindow*>(event.GetEventObject());
            const wxPoint screenPos = window->ClientToScreen(event.GetPosition());
            const wxPoint clientPos = ScreenToClient(screenPos);

            if (dragging()) {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        setSashPosition(clientPos.y - m_dragOffset);
                        sizeWindows();
                        break;
                    case SplitMode_Vertical:
                        setSashPosition(clientPos.x - m_dragOffset);
                        sizeWindows();
                        break;
                    case SplitMode_Unset:
                        break;
                }
            } else {
                if (isOnSash(clientPos)) {
                    if (m_oldCursorWindow == NULL) {
                        setCursor(window);
                    } else if (m_oldCursorWindow != window) {
                        resetCursor();
                        setCursor(window);
                    }
                } else if (m_oldCursorWindow != NULL) {
                    resetCursor();
                }
                event.Skip();
            }
        }

        bool SplitterWindow::dragging() const {
            return GetCapture() == this;
        }

        bool SplitterWindow::isOnSash(const wxPoint& pos) const {
            switch (m_splitMode) {
                case SplitMode_Horizontal:
                    return (pos.y >= m_sashPosition - HalfMinSashSize &&
                            pos.y <= m_sashPosition + sashSize() + HalfMinSashSize &&
                            pos.x >= 0 && pos.x <= GetClientSize().x);
                case SplitMode_Vertical:
                    return (pos.x >= m_sashPosition - HalfMinSashSize &&
                            pos.x <= m_sashPosition + sashSize() + HalfMinSashSize &&
                            pos.y >= 0 && pos.y <= GetClientSize().y);
                case SplitMode_Unset:
                    return false;
            }
        }
        
        void SplitterWindow::setCursor(wxWindow* window) {
            m_oldCursor = window->GetCursor();
            m_oldCursorWindow = window;
            window->SetCursor(sizeCursor());
        }

        wxCursor SplitterWindow::sizeCursor() const {
            switch (m_splitMode) {
                case SplitMode_Horizontal:
                    return wxCursor(wxCURSOR_SIZENS);
                case SplitMode_Vertical:
                    return wxCursor(wxCURSOR_SIZEWE);
                case SplitMode_Unset:
                    return wxCursor();
            }
        }
        
        void SplitterWindow::resetCursor() {
            m_oldCursorWindow->SetCursor(m_oldCursor);
            m_oldCursorWindow = NULL;
        }

        void SplitterWindow::OnSize(wxSizeEvent& event) {
            updateSashPosition(m_oldSize, event.GetSize());
            sizeWindows();
            m_oldSize = event.GetSize();
            event.Skip();
        }

        void SplitterWindow::bindMouseEvents(wxWindow* window) {
            window->Bind(wxEVT_LEFT_DOWN, &SplitterWindow::OnMouseButton, this);
            window->Bind(wxEVT_LEFT_UP, &SplitterWindow::OnMouseButton, this);
            window->Bind(wxEVT_MOTION, &SplitterWindow::OnMouseMotion, this);
            const wxWindowList& children = window->GetChildren();
            
            wxWindowList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it)
                bindMouseEvents(*it);
        }

        void SplitterWindow::updateSashPosition(const wxSize& oldSize, const wxSize& newSize) {
            initSashPosition();

            const wxSize diff = newSize - oldSize;
            switch (m_splitMode) {
                case SplitMode_Horizontal: {
                    setSashPosition(m_sashPosition + static_cast<int>(m_sashGravity * diff.y));
                    break;
                }
                case SplitMode_Vertical: {
                    setSashPosition(m_sashPosition + static_cast<int>(m_sashGravity * diff.x));
                    break;
                }
                case SplitMode_Unset:
                    break;
            }
        }

        void SplitterWindow::initSashPosition() {
            if (m_sashPosition == -1) {
                switch (m_splitMode) {
                    case SplitMode_Horizontal: {
                        const int clientHeight = GetClientSize().y;
                        m_sashPosition = m_minSizes[0].y + static_cast<int>(m_sashGravity * (clientHeight - m_minSizes[0].y - m_minSizes[1].y - sashSize()));
                        break;
                    }
                    case SplitMode_Vertical: {
                        const int clientWidth = GetClientSize().x;
                        m_sashPosition = m_minSizes[0].x + static_cast<int>(m_sashGravity * (clientWidth - m_minSizes[0].x - m_minSizes[1].x - sashSize()));
                        break;
                    }
                    case SplitMode_Unset:
                        break;
                }
            }
        }

        void SplitterWindow::setSashPosition(const int position) {
            m_sashPosition = position;
            switch (m_splitMode) {
                case SplitMode_Horizontal: {
                    m_sashPosition = std::max(m_sashPosition, m_minSizes[0].y);
                    m_sashPosition = std::min(m_sashPosition, GetClientSize().y - m_minSizes[1].y - sashSize());
                    break;
                }
                case SplitMode_Vertical: {
                    m_sashPosition = std::max(m_sashPosition, m_minSizes[0].x);
                    m_sashPosition = std::min(m_sashPosition, GetClientSize().x - m_minSizes[1].x - sashSize());
                    break;
                }
                case SplitMode_Unset:
                    break;
            }
        }

        void SplitterWindow::sizeWindows() {
            initSashPosition();
            
            switch (m_splitMode) {
                case SplitMode_Horizontal: {
                    const int width = GetClientSize().x;
                    m_windows[0]->SetPosition(wxPoint(0, 0));
                    m_windows[0]->SetSize(wxSize(width, m_sashPosition));
                    m_windows[1]->SetPosition(wxPoint(0, m_sashPosition + sashSize()));
                    m_windows[1]->SetSize(wxSize(width, GetClientSize().y - m_sashPosition - sashSize()));
                    if (m_sashWindow != NULL) {
                        m_sashWindow->SetPosition(wxPoint(0, m_sashPosition));
                        m_sashWindow->SetSize(wxSize(width, m_sashWindow->GetSize().y));
                    }
                    break;
                }
                case SplitMode_Vertical: {
                    const int height = GetClientSize().y;
                    m_windows[0]->SetPosition(wxPoint(0, 0));
                    m_windows[0]->SetSize(wxSize(m_sashPosition, height));
                    m_windows[1]->SetPosition(wxPoint(m_sashPosition + sashSize(), 0));
                    m_windows[1]->SetSize(wxSize(GetClientSize().x - m_sashPosition - sashSize(), height));
                    if (m_sashWindow != NULL) {
                        m_sashWindow->SetPosition(wxPoint(m_sashPosition, 0));
                        m_sashWindow->SetSize(wxSize(m_sashWindow->GetSize().x, height));
                    }
                    break;
                }
                case SplitMode_Unset:
                    break;
            }
        }

        int SplitterWindow::sashSize() const {
            if (m_sashWindow == NULL)
                return 1;
            
            switch (m_splitMode) {
                case SplitMode_Horizontal:
                    return m_sashWindow->GetSize().y;
                case SplitMode_Vertical:
                    return m_sashWindow->GetSize().x;
                case SplitMode_Unset:
                    return 0;
            }
        }
    }
}
