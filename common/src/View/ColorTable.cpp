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

#include "ColorTable.h"

#include "View/ColorTableSelectedCommand.h"

#include <wx/dcclient.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace View {
        ColorTable::ColorTable(wxWindow* parent, wxWindowID winId, int cellSize, const wxPoint& pos, const wxSize& size, long style) :
        wxScrolledWindow(parent, winId, pos, size, (style & ~static_cast<long>(wxHSCROLL)) | static_cast<long>(wxVSCROLL)),
        m_cellSize(cellSize),
        m_margin(2) {
            assert(m_cellSize > 0);
            
            Bind(wxEVT_SIZE, &ColorTable::OnSize, this);
            Bind(wxEVT_PAINT, &ColorTable::OnPaint, this);
            Bind(wxEVT_LEFT_UP, &ColorTable::OnMouseUp, this);
            
            SetScrollRate(0, m_cellSize + m_margin);
        }

        void ColorTable::setColors(const ColorList& colors) {
            m_colors = colors;
            m_selectedColors.clear();
            updateVirtualSize();
        }

        void ColorTable::setSelection(const ColorList& colors) {
            m_selectedColors = colors;
            Refresh();
        }

        void ColorTable::OnSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            updateVirtualSize();
        }

        void ColorTable::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            const wxSize virtualSize = GetVirtualSize();
            const int cols = computeCols(virtualSize.x);
            const int rows = computeRows(cols);
            
            const wxPoint viewStart = GetViewStart();
            int xRate, yRate;
            GetScrollPixelsPerUnit(&xRate, &yRate);

            const int startX = -(viewStart.x * xRate) + m_margin;
            int x = startX;
            int y = -(viewStart.y * yRate) + m_margin;
            
            wxPaintDC dc(this);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.DrawRectangle(0, 0, virtualSize.x, virtualSize.y);
            
            ColorList::const_iterator it = m_colors.begin();
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    if (it != m_colors.end()) {
                        const wxColour& color = *it;
                        
                        if (std::find(m_selectedColors.begin(), m_selectedColors.end(), color) != m_selectedColors.end()) {
                            dc.SetPen(*wxRED_PEN);
                            dc.SetBrush(*wxRED_BRUSH);
                            dc.DrawRectangle(x-1, y-1, m_cellSize+2, m_cellSize+2);
                        }

                        dc.SetPen(wxPen(color));
                        dc.SetBrush(wxBrush(color));
                        dc.DrawRectangle(x, y, m_cellSize, m_cellSize);

                        ++it;
                    }
                    x += m_cellSize + m_margin;
                }
                y += m_cellSize + m_margin;
                x = startX;
            }
        }
        
        void ColorTable::OnMouseUp(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            const wxSize virtualSize = GetVirtualSize();
            const int cols = computeCols(virtualSize.x);

            const wxPoint pos = CalcScrolledPosition(event.GetPosition());
            const int col = (pos.x - m_margin) / (m_cellSize + m_margin);
            const int row = (pos.y - m_margin) / (m_cellSize + m_margin);
            
            const size_t index = static_cast<size_t>(row * cols + col);
            if (index < m_colors.size()) {
                const wxColor& color = m_colors[index];

                ColorTableSelectedCommand command;
                command.setColor(color);
                command.SetEventObject(this);
                command.SetId(GetId());
                ProcessEvent(command);
            }
        }

        void ColorTable::updateVirtualSize() {
            int width = GetClientSize().x;
            int cols = computeCols(width);
            int rows = computeRows(cols);
            int height = computeHeight(rows);
            SetVirtualSize(width, height);
            
            if (GetClientSize().x != width) {
                width = GetClientSize().x;
                cols = computeCols(width);
                rows = computeRows(cols);
                height = computeHeight(rows);
                SetVirtualSize(width, height);
                assert(width == GetClientSize().x);
            }
        }

        int ColorTable::computeCols(const int width) const {
            return (width - m_margin) / (m_cellSize + m_margin);
        }

        int ColorTable::computeRows(const int cols) const {
            if (cols == 0)
                return 0;
            return (static_cast<int>(m_colors.size()) + cols - 1) / cols;
        }

        int ColorTable::computeHeight(const int rows) const {
            return m_margin + rows * (m_cellSize + m_margin) - 1; // no idea why the -1 is necessary, but it is
        }
    }
}
