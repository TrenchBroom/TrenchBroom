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

#ifndef __TrenchBroom__SplitterWindow4__
#define __TrenchBroom__SplitterWindow4__

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
            wxSize m_minSizes[NumWindows];
            
            int m_initialSashPosition[2];
            int m_sashPosition[2];
            
            bool m_sashCursorSet;
            
            wxSize m_oldSize;
            
            friend class PersistentSplitterWindow4;
        public:
            SplitterWindow4(wxWindow* parent);
            
            void split(wxWindow* topLeft, wxWindow* topRight, wxWindow* bottomRight, wxWindow* bottomLeft,
                       const wxSize& topLeftMin     = wxDefaultSize,
                       const wxSize& topRightMin    = wxDefaultSize,
                       const wxSize& bottomRightMin = wxDefaultSize,
                       const wxSize& bottomLeftMin  = wxDefaultSize);

            void setMinSize(wxWindow* window, const wxSize& minSize);
        private:
            bool containsWindow(wxWindow* window) const;
        private:
            void OnMouseEnter(wxMouseEvent& event);
            void OnMouseLeave(wxMouseEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
        private:
            bool dragging() const;
            void setSashCursor();
            wxCursor sizeCursor() const;
            bool isOnHSash(const wxPoint& point) const;
            bool isOnVSash(const wxPoint& point) const;
        public:
            void OnIdle(wxIdleEvent& event);
            void OnSize(wxSizeEvent& event);
        private:
            void updateSashPosition(const wxSize& oldSize, const wxSize& newSize);
            void initSashPosition();
            bool setSashPosition(int position);
            void sizeWindows();
            int sashSize() const;
        };
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow4* window);

#endif /* defined(__TrenchBroom__SplitterWindow4__) */
