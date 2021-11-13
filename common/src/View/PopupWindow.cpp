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

#include "PopupWindow.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QWindow>

namespace TrenchBroom {
namespace View {
// PopupWindow

PopupWindow::PopupWindow(QWidget* parent)
  : QWidget(parent, Qt::Popup) {}

void PopupWindow::positionTouchingWidget(QWidget* refWidget) {
  const QRect screenGeom = QApplication::desktop()->availableGeometry(refWidget);
  const QRect refWidgetRectOnScreen =
    QRect(refWidget->mapToGlobal(QPoint(0, 0)), refWidget->size());
  const QSize ourSize = size();

  // Figure out y position on screen
  int y;
  if (refWidgetRectOnScreen.bottom() + ourSize.height() <= screenGeom.bottom()) { // fits below?
    y = refWidgetRectOnScreen.bottom();
  } else if (refWidgetRectOnScreen.top() - ourSize.height() >= 0) { // fits above?
    y = refWidgetRectOnScreen.top() - ourSize.height();
  } else { // otherwise put it as low as possible, but make sure the top is visible
    const auto bottom =
      std::min(refWidgetRectOnScreen.bottom() + ourSize.height(), screenGeom.bottom());
    const auto top = bottom - ourSize.height();
    y = std::max(top, 0);
  }

  // Figure out the x position on screen
  int x;
  if (refWidgetRectOnScreen.right() - ourSize.width() >= 0) { // fits left?
    x = refWidgetRectOnScreen.right() - ourSize.width();
  } else if (refWidgetRectOnScreen.left() + ourSize.width() <= screenGeom.right()) { // fits right?
    x = refWidgetRectOnScreen.left();
  } else { // otherwise put it as far to the left as possible, but make sure the left is visible
    x = std::max(refWidgetRectOnScreen.left() - ourSize.width(), 0);
  }

  // Now map x, y from global to our parent's coordinates
  const QPoint desiredPointInParentCoords = mapToParent(mapFromGlobal(QPoint(x, y)));
  setGeometry(QRect(desiredPointInParentCoords, ourSize));
}

void PopupWindow::closeEvent(QCloseEvent*) {
  emit visibilityChanged(false);
}

void PopupWindow::showEvent(QShowEvent*) {
  emit visibilityChanged(true);
}
} // namespace View
} // namespace TrenchBroom
