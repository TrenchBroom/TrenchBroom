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

#include "MapViewActivationTracker.h"

#include "Ensure.h"
#include "CollectionUtils.h"
#include "View/MapViewBase.h"

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QWindow>

namespace TrenchBroom {
    namespace View {
        MapViewActivationTracker::MapViewActivationTracker() :
        m_active(false) {}

        void MapViewActivationTracker::addWindow(MapViewBase* mapView) {
            ensure(mapView != nullptr, "map view is null");

            mapView->installEventFilter(this);
            m_mapViews.push_back(mapView);

            if (m_active) {
                clearFocusCursor(mapView);
            } else {
                setFocusCursor(mapView);
            }
        }

        void MapViewActivationTracker::clear() {
            for (auto* mapView : m_mapViews) {
                mapView->removeEventFilter(this);
            }
            m_mapViews.clear();

            m_active = false;
        }

        void MapViewActivationTracker::windowActivationChanged(const bool active) {
            if (!active) {
                qDebug() << "window deactivated";
                // window has lost activation, deactivate the group
                deactivate();
            } else {
                qDebug() << "window activated";
                auto* focusWindow = QGuiApplication::focusWindow();
                qDebug() << "focus window is " << focusWindow;
                if (VectorUtils::contains(m_mapViews, focusWindow)) {
                    qDebug() << "Activated with a map view as focus";
                }
            }
        }

        bool MapViewActivationTracker::eventFilter(QObject* object, QEvent* event) {
            auto* window = dynamic_cast<QWindow*>(object);
            ensure(window != nullptr, "expected a QWindow");

            switch (event->type()) {
                case QEvent::FocusIn:
                    setFocusEvent(static_cast<QFocusEvent*>(event));
                    break;
                case QEvent::FocusOut:
                    killFocusEvent(static_cast<QFocusEvent*>(event));
                    break;
                case QEvent::MouseButtonPress:
                    if (mouseDownEvent(static_cast<QMouseEvent*>(event), window)) {
                        return true;
                    }
                    break;
                case QEvent::MouseButtonRelease:
                    if (mouseUpEvent(static_cast<QMouseEvent*>(event), window)) {
                        return true;
                    }
                    break;
                case QEvent::MouseMove:
                    mouseMoveEvent(static_cast<QMouseEvent*>(event), window);
                    break;
                default:
                    break;
            }

            // NOTE: In all cases, we don't consume the event but let Qt continue processing it
            return QObject::eventFilter(object, event);
        }

        void MapViewActivationTracker::setFocusEvent(QFocusEvent* event) {}

        void MapViewActivationTracker::killFocusEvent(QFocusEvent* event) {
            const auto* focusedWindow = QGuiApplication::focusWindow();
            if (!VectorUtils::contains(m_mapViews, focusedWindow)) {
                deactivate();
            }
        }

        bool MapViewActivationTracker::mouseDownEvent(QMouseEvent* event, QWindow* window) {
            if (m_active) {
                // process the event normally
                return false;
            }

            auto* focusWindow = QGuiApplication::focusWindow();
            qDebug() << "focus window on mouse down is " << focusWindow;
            qDebug() << "event window on mouse down is " << window;

            window->requestActivate();
            if (event->button() != Qt::LeftButton) {
                activate();
                return false;
            }

            // discard the event (it's a left click), we'll activate on mouse up
            return true;
        }

        bool MapViewActivationTracker::mouseUpEvent(QMouseEvent* event, QWindow* window) {
            if (m_active) {
                // process the event normally
                return false;
            }

            auto* focusWindow = QGuiApplication::focusWindow();
            qDebug() << "focus window on mouse up is " << focusWindow;
            qDebug() << "event window on mouse up is " << window;

            window->requestActivate();
            activate();

            // at this point, it must be a left button event, otherwise we would have been active already
            assert(event->button() == Qt::LeftButton);

            // so we discard the event
            return true;
        }

        void MapViewActivationTracker::mouseMoveEvent(QMouseEvent* event, QWindow* window) {
            if (m_active) {
                auto* newFocus = window;
                auto* currentFocus = QGuiApplication::focusWindow();

                // If this was QWidget we could use https://doc.qt.io/qt-5/qwidget.html#enterEvent to get notified when the mouse enters
                // a widget. There's no equivalent for QWindow so we need to do it ourselves by listening to every mouse move event.
                if (currentFocus == newFocus) {
                    return;
                }

                if (VectorUtils::contains(m_mapViews, currentFocus)) {
                    newFocus->requestActivate();
                }
            }
        }

        void MapViewActivationTracker::activate() {
            if (!m_active) {
                qDebug() << "Map view group activated";
                m_active = true;
                clearFocusCursor();
            }
        }

        void MapViewActivationTracker::deactivate() {
            if (m_active) {
                qDebug() << "Map view group deactivated";
                setFocusCursor();
                m_active = false;
            }
        }

        void MapViewActivationTracker::setFocusCursor() {
            for (auto* mapView : m_mapViews) {
                setFocusCursor(mapView);
            }
        }

        void MapViewActivationTracker::setFocusCursor(MapViewBase* mapView) {
            mapView->setCursor(Qt::PointingHandCursor);
        }

        void MapViewActivationTracker::clearFocusCursor() {
            for (auto* mapView : m_mapViews) {
                clearFocusCursor(mapView);
            }
        }

        void MapViewActivationTracker::clearFocusCursor(MapViewBase* mapView) {
            mapView->setCursor(Qt::ArrowCursor);
        }
    }
}
