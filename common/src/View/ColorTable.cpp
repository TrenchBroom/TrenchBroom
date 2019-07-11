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

#include "ColorTable.h"

#include <QPainter>
#include <QColor>
#include <QMouseEvent>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace View {
        ColorTable::ColorTable(int cellSize, QWidget* parent) :
        QWidget(parent),
        m_cellSize(cellSize),
        m_margin(2) {
            assert(m_cellSize > 0);

            //SetScrollRate(0, m_cellSize + m_margin);
        }

        void ColorTable::setColors(const ColorList& colors) {
            m_colors = colors;
            m_selectedColors.clear();

            // FIXME: Double check that this is working
            updateGeometry();
        }

        void ColorTable::setSelection(const ColorList& colors) {
            m_selectedColors = colors;
            update();
        }

        void ColorTable::paintEvent(QPaintEvent* event) {
            const QSize virtualSize = size();
            const int cols = computeCols(virtualSize.width());
            const int rows = computeRows(cols);

            const int startX = m_margin;
            int x = startX;
            int y = m_margin;

            QPainter dc(this);
            // FIXME: what about this?
//            dc.setPen(QColor(Qt::transparent));
//            dc.setBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
//            dc.drawRect(0, 0, virtualSize.x, virtualSize.y);

            auto it = std::begin(m_colors);
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    if (it != std::end(m_colors)) {
                        const QColor& color = *it;

                        if (std::find(std::begin(m_selectedColors), std::end(m_selectedColors), color) != std::end(m_selectedColors)) {
                            dc.setPen(QColor(Qt::red));
                            dc.setBrush(QColor(Qt::red));
                            dc.drawRect(x-1, y-1, m_cellSize+2, m_cellSize+2);
                        }

                        dc.setPen(color);
                        dc.setBrush(color);
                        dc.drawRect(x, y, m_cellSize, m_cellSize);

                        ++it;
                    }
                    x += m_cellSize + m_margin;
                }
                y += m_cellSize + m_margin;
                x = startX;
            }
        }

        void ColorTable::mouseReleaseEvent(QMouseEvent* event) {
            const QSize virtualSize = size();
            const int cols = computeCols(virtualSize.width());

            const QPoint pos = event->pos();
            const int col = (pos.x() - m_margin) / (m_cellSize + m_margin);
            const int row = (pos.y() - m_margin) / (m_cellSize + m_margin);

            const size_t index = static_cast<size_t>(row * cols + col);
            if (index < m_colors.size()) {
                const QColor& color = m_colors[index];

                emit colorTableSelected(color);
            }
        }

        bool ColorTable::hasHeightForWidth() const {
            return true;
        }

        int ColorTable::heightForWidth(int w) const {
            int cols = computeCols(w);
            int rows = computeRows(cols);
            int height = computeHeight(rows);
            return height;
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
