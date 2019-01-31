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

#include <QWindow>
#include <QScreen>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

namespace TrenchBroom {
    namespace View {
        // PopupWindow

        PopupWindow::PopupWindow(QWidget* parent) :
                QWidget(parent, Qt::Popup) {}

        void PopupWindow::positionTouchingWidget(QWidget* refWidget) {
            const QRect screenGeom = QApplication::desktop()->availableGeometry(refWidget);
            const QRect refWidgetRectOnScreen = QRect(refWidget->mapToGlobal(QPoint(0, 0)), refWidget->size());
            const QSize ourSize = size();

            //qDebug() << "screenGeom " << screenGeom << " refWidgetRectOnScreen " << refWidgetRectOnScreen << " our size: " << size();

            int y = 0;
            if (refWidgetRectOnScreen.bottom() + ourSize.height() <= screenGeom.height()) { // fits below?
                y = refWidgetRectOnScreen.bottom();
            } else if (refWidgetRectOnScreen.top() - ourSize.height() >= 0) { // fits above
                y = refWidgetRectOnScreen.top() - ourSize.height();
            }

            int x = refWidgetRectOnScreen.x();
            // FIXME: fit x on screen
            
            // now map x, y to the widget's coordinates
            const QPoint desiredPointInLocalCoords = mapFromGlobal(QPoint(x, y));
            const QPoint desiredPointInParentCoords = mapToParent(desiredPointInLocalCoords);
            setGeometry(QRect(desiredPointInParentCoords, ourSize));
        }

        void PopupWindow::closeEvent(QCloseEvent* event) {
            emit visibilityChanged(false);
        }
        void PopupWindow::showEvent(QShowEvent* event) {
            emit visibilityChanged(true);
        }
    }
}
