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
#include "View/MapViewBase.h"

#include <kdl/vector_utils.h>

#include <QDateTime>
#include <QApplication>
#include <QMouseEvent>

namespace TrenchBroom {
    namespace View {
        MapViewActivationTracker::MapViewActivationTracker() :
        m_active(false) {}

        bool MapViewActivationTracker::active() const {
            return m_active;
        }

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
                // window has lost activation, deactivate the group
                deactivate();
            }
        }

        bool MapViewActivationTracker::eventFilter(QObject* object, QEvent* event) {
            auto* widget = dynamic_cast<QWidget*>(object);
            ensure(widget != nullptr, "expected a QWidget");

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif
            switch (event->type()) {
                case QEvent::FocusIn:
                    setFocusEvent(static_cast<QFocusEvent*>(event), widget);
                    break;
                case QEvent::FocusOut:
                    killFocusEvent(static_cast<QFocusEvent*>(event), widget);
                    break;
                case QEvent::MouseButtonPress:
                    if (mouseDownEvent(static_cast<QMouseEvent*>(event), widget)) {
                        return true;
                    }
                    break;
                case QEvent::MouseButtonRelease:
                    if (mouseUpEvent(static_cast<QMouseEvent*>(event), widget)) {
                        return true;
                    }
                    break;
                case QEvent::Enter:
                    enterEvent(event, widget);
                    break;
                case QEvent::DragEnter:
                    dragEnterEvent(event, widget);
                    break;
                default:
                    break;
            }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

            // NOTE: In all cases, we don't consume the event but let Qt continue processing it
            return QObject::eventFilter(object, event);
        }

        void MapViewActivationTracker::setFocusEvent(QFocusEvent*, QWidget* widget) {
            for (auto* mapView : m_mapViews) {
                mapView->setIsCurrent(mapView == widget);
            }
        }

        void MapViewActivationTracker::killFocusEvent(QFocusEvent*, QWidget*) {
            const auto* focusedWidget = QApplication::focusWidget();
            if (!kdl::vec_contains(m_mapViews, focusedWidget)) {
                deactivate();
            }
        }

        bool MapViewActivationTracker::mouseDownEvent(QMouseEvent* event, QWidget*) {
            if (m_active) {
                // process the event normally
                return false;
            }

            if (event->button() != Qt::LeftButton) {
                activate();
                return false;
            }

            // discard the event (it's a left click), we'll activate on mouse up
            return true;
        }

        bool MapViewActivationTracker::mouseUpEvent([[maybe_unused]] QMouseEvent* event, QWidget*) {
            if (m_active) {
                // process the event normally
                return false;
            }

            activate();

            // at this point, it must be a left button event, otherwise we would have been active already
            assert(event->button() == Qt::LeftButton);

            // so we discard the event
            return true;
        }

        void MapViewActivationTracker::enterEvent(QEvent*, QWidget* widget) {
            if (m_active) {
                widget->setFocus();
            }
        }

        void MapViewActivationTracker::dragEnterEvent(QEvent*, QWidget* widget) {
            if (!m_active) {
                activate();
            }
            widget->setFocus();
        }

        void MapViewActivationTracker::activate() {
            if (!m_active) {
                m_active = true;
                clearFocusCursor();
            }
        }

        void MapViewActivationTracker::deactivate() {
            if (m_active) {
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
