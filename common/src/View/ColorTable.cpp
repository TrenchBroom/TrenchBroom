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

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
namespace View {
ColorTable::ColorTable(const int cellSize, QWidget* parent)
  : QWidget(parent)
  , m_cellSize(cellSize)
  , m_cellSpacing(2) {
  assert(m_cellSize > 0);

  auto sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sizePolicy.setHeightForWidth(true);
  setSizePolicy(sizePolicy);
}

void ColorTable::setColors(const std::vector<QColor>& colors) {
  m_colors = colors;
  m_selectedColors.clear();

  updateGeometry();
}

void ColorTable::setSelection(const std::vector<QColor>& colors) {
  m_selectedColors = colors;
  update();
}

void ColorTable::paintEvent(QPaintEvent* /* event */) {
  const auto virtualSize = size();
  const auto cols = computeCols(virtualSize.width());
  const auto rows = computeRows(cols);

  const auto startX = m_cellSpacing;
  auto x = startX;
  auto y = m_cellSpacing;

  QPainter dc(this);

  auto it = std::begin(m_colors);
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      if (it != std::end(m_colors)) {
        const auto& color = *it;

        if (
          std::find(std::begin(m_selectedColors), std::end(m_selectedColors), color) !=
          std::end(m_selectedColors)) {
          dc.setPen(QColor(Qt::red));
          dc.setBrush(QColor(Qt::red));
          dc.drawRect(x - 1, y - 1, m_cellSize + 2, m_cellSize + 2);
        }

        dc.setPen(color);
        dc.setBrush(color);
        dc.drawRect(x, y, m_cellSize, m_cellSize);

        ++it;
      }
      x += m_cellSize + m_cellSpacing;
    }
    y += m_cellSize + m_cellSpacing;
    x = startX;
  }
}

void ColorTable::mouseReleaseEvent(QMouseEvent* event) {
  const auto virtualSize = size();
  const auto cols = computeCols(virtualSize.width());

  const auto pos = event->pos();
  const auto col = (pos.x() - m_cellSpacing) / (m_cellSize + m_cellSpacing);
  const auto row = (pos.y() - m_cellSpacing) / (m_cellSize + m_cellSpacing);

  const auto index = static_cast<size_t>(row * cols + col);
  if (index < m_colors.size()) {
    const auto& color = m_colors[index];
    emit colorTableSelected(color);
  }
}

bool ColorTable::hasHeightForWidth() const {
  return true;
}

int ColorTable::heightForWidth(const int w) const {
  const auto cols = computeCols(w);
  const auto rows = computeRows(cols);
  const auto height = computeHeight(rows);
  return height;
}

int ColorTable::computeCols(const int width) const {
  return (width - m_cellSpacing) / (m_cellSize + m_cellSpacing);
}

int ColorTable::computeRows(const int cols) const {
  if (cols == 0) {
    return 0;
  } else {
    return (static_cast<int>(m_colors.size()) + cols - 1) / cols;
  }
}

int ColorTable::computeHeight(const int rows) const {
  return m_cellSpacing + rows * (m_cellSize + m_cellSpacing);
}
} // namespace View
} // namespace TrenchBroom
