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

#ifndef __TrenchBroom__SplitterWindow__
#define __TrenchBroom__SplitterWindow__

#include <wx/panel.h>

namespace TrenchBroom {
    namespace View {
        class SplitterWindow : public wxPanel {
        private:
            static const size_t NumWindows = 2;
            static const int HalfMinSashSize = 1;
            
            typedef enum {
                SplitMode_Unset,
                SplitMode_Horizontal,
                SplitMode_Vertical
            } SplitMode;
            
            SplitMode m_splitMode;
            wxWindow* m_windows[NumWindows];
            wxSize m_minSizes[NumWindows];
            
            wxWindow* m_sashWindow;
            float m_sashGravity;
            int m_sashPosition;
            int m_dragOffset;
            
            wxSize m_oldSize;
            wxCursor m_oldCursor;
            wxWindow* m_oldCursorWindow;
        public:
            SplitterWindow(wxWindow* parent);
            
            void splitHorizontally(wxWindow* left, wxWindow* right);
            void splitVertically(wxWindow* top, wxWindow* bottom);
            void setSashWindow(wxWindow* sashWindow);
            
            void setMinSize(wxWindow* window, const wxSize& minSize);
            void setSashGravity(float sashGravity);
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
        private:
            bool dragging() const;
            bool isOnSash(const wxPoint& pos) const;
            void setCursor(wxWindow* window);
            wxCursor sizeCursor() const;
            void resetCursor();
        public:
            void OnSize(wxSizeEvent& event);
        private:
            void bindMouseEvents(wxWindow* window);
            
            void updateSashPosition(const wxSize& oldSize, const wxSize& newSize);
            void initSashPosition();
            void setSashPosition(int position);
            void sizeWindows();
            int sashSize() const;
        };
    }
}

#endif /* defined(__TrenchBroom__SplitterWindow__) */
