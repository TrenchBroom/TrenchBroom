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

#ifndef TrenchBroom_SplitterWindow2
#define TrenchBroom_SplitterWindow2

#include "Macros.h"

#include <wx/panel.h>

class wxPersistentObject;

namespace TrenchBroom {
    namespace View {
        class PersistentSplitterWindow2;
        
        class SplitterWindow2 : public wxPanel {
        private:
            static const size_t NumWindows = 2;
            static const int HalfMinSashSize = 2;
            
            typedef enum {
                SplitMode_Unset,
                SplitMode_Horizontal,
                SplitMode_Vertical
            } SplitMode;
            
            SplitMode m_splitMode;
            wxWindow* m_sash;
            wxWindow* m_windows[NumWindows];
            wxWindow* m_maximizedWindow;
            wxSize m_minSizes[NumWindows];
            
            double m_sashGravity;
            double m_initialSplitRatio;
            double m_currentSplitRatio;
            
            bool m_sashCursorSet;
            
            wxSize m_oldSize;
            
            friend class PersistentSplitterWindow2;
        public:
            SplitterWindow2(wxWindow* parent);
            
            void splitHorizontally(wxWindow* left, wxWindow* right, const wxSize& leftMin = wxDefaultSize, const wxSize& rightMin = wxDefaultSize);
            void splitVertically(wxWindow* top, wxWindow* bottom, const wxSize& topMin = wxDefaultSize, const wxSize& bottomMin = wxDefaultSize);
            
            void setMinSize(wxWindow* window, const wxSize& minSize);
            void setSashGravity(double sashGravity);
            
            bool isMaximized(wxWindow* window) const;
            void maximize(wxWindow* window);
            void restore();
        private:
            int currentSashPosition() const;
            int sashPosition(double ratio) const;
            int sashPosition(double ratio, wxCoord size) const;
            double splitRatio(int position) const;
            
            void split(wxWindow* window1, wxWindow* window2, const wxSize& min1, const wxSize& min2, SplitMode splitMode);
            void bindMouseEvents(wxWindow* window);
        public:
            void OnMouseEnter(wxMouseEvent& event);
            void OnMouseLeave(wxMouseEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
        private:
            bool dragging() const;
            void setSashCursor();
            wxCursor sizeCursor() const;
        public:
            void OnIdle(wxIdleEvent& event);
            void OnSize(wxSizeEvent& event);
        private:
            void updateSashPosition(const wxSize& oldSize, const wxSize& newSize);
            void initSashPosition();
            bool setSashPosition(int newSashPosition);
            void sizeWindows();
            int sashSize() const;
            
            wxWindow* unmaximizedWindow();
            
            template <typename T>
            void setHV(T& p, const int h, const int v) const {
                setH(p, h);
                setV(p, v);
            }
            
            template <typename T>
            void setH(T& p, const int h) const {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        p.y = h;
                        break;
                    case SplitMode_Vertical:
                        p.x = h;
                        break;
                    case SplitMode_Unset:
                        break;
                    switchDefault()
                }
            }
            
            template <typename T>
            void setV(T& p, const int v) const {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        p.x = v;
                        break;
                    case SplitMode_Vertical:
                        p.y = v;
                        break;
                    case SplitMode_Unset:
                        break;
                    switchDefault()
                }
            }

            template <typename T>
            int h(const T& p) const {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        return p.y;
                    case SplitMode_Vertical:
                        return p.x;
                    case SplitMode_Unset:
                        return 0;
                    switchDefault()
                }
            }
            
            template <typename T>
            int v(const T& p) const {
                switch (m_splitMode) {
                    case SplitMode_Horizontal:
                        return p.x;
                    case SplitMode_Vertical:
                        return p.y;
                    case SplitMode_Unset:
                        return 0;
                    switchDefault()
                }
            }
        };
    }
}

wxPersistentObject* wxCreatePersistentObject(TrenchBroom::View::SplitterWindow2* window);

#endif /* defined(TrenchBroom_SplitterWindow2) */
