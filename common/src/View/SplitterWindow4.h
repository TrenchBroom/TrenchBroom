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

#ifndef TrenchBroom_SplitterWindow4
#define TrenchBroom_SplitterWindow4

#include "Macros.h"

#include <wx/panel.h>

class wxPersistentObject;

namespace TrenchBroom {
    namespace View {
        class PersistentSplitterWindow4;
        
        class SplitterWindow4 : public wxPanel {
        private:
            static const size_t NumWindows = 4;
            static const int HalfMinSashSize = 2;
            
            typedef enum {
                Window_TopLeft,
                Window_TopRight,
                Window_BottomRight,
                Window_BottomLeft
            } Window;
            
            typedef enum {
                Dim_X,
                Dim_Y
            } Dim;
            
            wxWindow* m_windows[NumWindows];
            wxWindow* m_maximizedWindow;
            wxSize m_minSizes[NumWindows];
            
            wxPoint m_initialSashPosition;
            wxPoint m_sashPosition;
            bool m_dragging[2];
            
            wxSize m_oldSize;
            
            friend class PersistentSplitterWindow4;
        public:
            SplitterWindow4(wxWindow* parent);
            
            void split(wxWindow* topLeft, wxWindow* topRight, wxWindow* bottomRight, wxWindow* bottomLeft,
                       const wxSize& topLeftMin     = wxSize(0,0),
                       const wxSize& topRightMin    = wxSize(0,0),
                       const wxSize& bottomRightMin = wxSize(0,0),
                       const wxSize& bottomLeftMin  = wxSize(0,0));

            void setMinSize(wxWindow* window, const wxSize& minSize);

            void maximize(wxWindow* window);
            void restore();
        private:
            int leftColMinSize() const;
            int rightColMinSize() const;
            int topRowMinSize() const;
            int bottomRowMinSize() const;
            
            bool hasWindows() const;
            bool containsWindow(wxWindow* window) const;
        private:
            void bindMouseEvents(wxWindow* window);
            
            void OnMouseEnter(wxMouseEvent& event);
            void OnMouseLeave(wxMouseEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnIdle(wxIdleEvent& event);
            void OnSize(wxSizeEvent& event);
        private:
            void updateSashCursor();
            bool sashHitTest(const wxPoint& point, Dim dim) const;

            void updateSashPosition(const wxSize& oldSize, const wxSize& newSize);
            bool initSashPosition();
            bool setSashPosition(wxPoint sashPosition);
            void sizeWindows();
            int sashSize() const;
            
            template <typename T>
            int get(const T& t, const Dim dim) const {
                switch (dim) {
                    case Dim_X:
                        return t.x;
                    case Dim_Y:
                        return t.y;
                    switchDefault()
                }
            }
        };
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow4* window);

#endif /* defined(TrenchBroom_SplitterWindow4) */
