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

#include "BorderPanel.h"

#include <QPainter>
#include <QPalette>

#include "View/ViewConstants.h"

namespace TrenchBroom
{
namespace View
{
BorderPanel::BorderPanel(const Sides borders, const int thickness, QWidget* parent)
  : QWidget(parent)
  , m_borders(borders)
  , m_thickness(thickness)
{
  setForegroundRole(QPalette::Mid);
}

void BorderPanel::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);

  const QRectF r = QRectF(rect());
  const qreal thickness = static_cast<qreal>(m_thickness);

  painter.setRenderHint(QPainter::Antialiasing, false);

  painter.setPen(Qt::NoPen);
  painter.setBrush(palette().color(backgroundRole()));
  painter.drawRect(r);

  painter.setPen(Qt::NoPen);
  painter.setBrush(palette().color(foregroundRole()));
  if ((m_borders & LeftSide) != 0)
  {
    painter.drawRect(QRectF(r.topLeft(), QSizeF(thickness, r.height())));
  }
  if ((m_borders & TopSide) != 0)
  {
    painter.drawRect(QRectF(r.topLeft(), QSizeF(r.width(), thickness)));
  }
  if ((m_borders & RightSide) != 0)
  {
    painter.drawRect(
      QRectF(r.topRight() - QPointF(thickness, 0.0), QSizeF(thickness, r.height())));
  }
  if ((m_borders & BottomSide) != 0)
  {
    painter.drawRect(
      QRectF(r.bottomLeft() - QPointF(0.0, thickness), QSizeF(r.width(), thickness)));
  }
}
} // namespace View
} // namespace TrenchBroom
