/*
 Copyright (C) 2010 Kristian Duske

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
#include <QScreen>
#include <QWindow>

namespace tb::ui
{
// PopupWindow

namespace
{

auto getXPosition(
  const QRect& refWidgetRectOnScreen, const QSize& ourSize, const QRect& screenGeom)
{
  if (refWidgetRectOnScreen.right() - ourSize.width() >= 0)
  { // fits left?
    return refWidgetRectOnScreen.right() - ourSize.width();
  }
  if (refWidgetRectOnScreen.left() + ourSize.width() <= screenGeom.right())
  { // fits right?
    return refWidgetRectOnScreen.left();
  }
  // otherwise put it as far to the left as possible, but make sure the left is visible
  return std::max(refWidgetRectOnScreen.left() - ourSize.width(), 0);
}

auto getYPosition(
  const QRect& refWidgetRectOnScreen, const QSize& ourSize, const QRect& screenGeom)
{
  if (refWidgetRectOnScreen.bottom() + ourSize.height() <= screenGeom.bottom())
  { // fits below?
    return refWidgetRectOnScreen.bottom();
  }
  if (refWidgetRectOnScreen.top() - ourSize.height() >= 0)
  { // fits above?
    return refWidgetRectOnScreen.top() - ourSize.height();
  }
  // otherwise put it as low as possible, but make sure the top is visible
  const auto bottom =
    std::min(refWidgetRectOnScreen.bottom() + ourSize.height(), screenGeom.bottom());
  const auto top = bottom - ourSize.height();
  return std::max(top, 0);
}

} // namespace

PopupWindow::PopupWindow(QWidget* parent)
  : QWidget{parent, Qt::Popup}
{
}

void PopupWindow::positionTouchingWidget(QWidget* refWidget)
{
  const auto screenGeom = QGuiApplication::primaryScreen()->availableGeometry();
  const auto refWidgetRectOnScreen =
    QRect{refWidget->mapToGlobal(QPoint{0, 0}), refWidget->size()};
  const auto ourSize = size();

  // Figure out y position on screen
  const auto x = getXPosition(refWidgetRectOnScreen, ourSize, screenGeom);
  const auto y = getYPosition(refWidgetRectOnScreen, ourSize, screenGeom);

  // Now map x, y from global to our parent's coordinates
  const auto desiredPointInParentCoords = mapToParent(mapFromGlobal(QPoint{x, y}));
  setGeometry(QRect{desiredPointInParentCoords, ourSize});
}

void PopupWindow::closeEvent(QCloseEvent*)
{
  emit visibilityChanged(false);
}

void PopupWindow::showEvent(QShowEvent*)
{
  emit visibilityChanged(true);
}

} // namespace tb::ui
