/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__ColorTable__
#define __TrenchBroom__ColorTable__

#include <wx/scrolwin.h>

#include <algorithm>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class ColorTable : public wxScrolledWindow {
        public:
            typedef std::vector<wxColour> ColorList;
        private:
            int m_cellSize;
            int m_margin;
            ColorList m_colors;
        public:
            ColorTable(wxWindow* parent, wxWindowID winId, int cellSize, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxBORDER_SUNKEN);
            
            template <typename Cmp>
            void setColors(const ColorList& colors, const Cmp& cmp) {
                setColors(colors);
                std::sort(m_colors.begin(), m_colors.end(), cmp);
            }
            
            void setColors(const ColorList& colors);
            
            void OnSize(wxSizeEvent& event);
            void OnPaint(wxPaintEvent& event);
            void OnMouseUp(wxMouseEvent& event);
        private:
            void updateVirtualSize();
            int computeCols(int width) const;
            int computeRows(int cols) const;
            int computeHeight(int rows) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ColorTable__) */
