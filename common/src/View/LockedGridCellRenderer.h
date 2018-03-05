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

#ifndef TRENCHBROOM_LOCKEDGRIDCELLRENDERER_H
#define TRENCHBROOM_LOCKEDGRIDCELLRENDERER_H

#include <wx/grid.h>

class wxBitmap;

namespace TrenchBroom {
    namespace View {
        class LockedGridCellRenderer : public wxGridCellStringRenderer {
        private:
            wxBitmap m_image;
        public:
            LockedGridCellRenderer();
        public:
            void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) override;
            wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col) override;
        };
    }
}

#endif //TRENCHBROOM_LOCKEDGRIDCELLRENDERER_H
