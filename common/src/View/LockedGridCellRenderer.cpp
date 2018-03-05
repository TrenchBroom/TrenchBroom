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

#include "LockedGridCellRenderer.h"

#include "IO/ResourceUtils.h"

#include <wx/dc.h>
#include <wx/settings.h>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace View {
        LockedGridCellRenderer::LockedGridCellRenderer() :
        wxGridCellStringRenderer(),
        m_image(IO::loadImageResource("Locked_small.png")) {
            assert(m_image.IsOk());
        }

        void LockedGridCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, const int row, const int col, const bool isSelected) {
            if (!attr.IsReadOnly()) {
                wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
            } else {
                const wxRect parentRect(rect.x, rect.y, rect.GetWidth() - m_image.GetWidth(), rect.GetHeight());
                wxGridCellStringRenderer::Draw(grid, attr, dc, parentRect, row, col, isSelected);

				const wxBrush oldBrush = dc.GetBackground();
				if (isSelected) {
					dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
				}
				else {
					dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
				}
				dc.DrawRectangle(wxRect(parentRect.GetRight(), parentRect.GetY(), m_image.GetWidth(), parentRect.GetHeight()));
				dc.SetBackground(oldBrush);

                const int y = parentRect.y + (parentRect.GetHeight() - m_image.GetHeight()) / 2;
                dc.DrawBitmap(m_image, parentRect.GetRight(), y);
            }
        }

        wxSize LockedGridCellRenderer::GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const int row, const int col) {
            const wxSize parentBestSize = wxGridCellStringRenderer::GetBestSize(grid, attr, dc, row, col);
            if (!attr.IsReadOnly()) {
                return parentBestSize;
            } else {
                return wxSize(parentBestSize.x + m_image.GetWidth(), std::max(parentBestSize.y, m_image.GetHeight()));
            }
        }
    }
}
