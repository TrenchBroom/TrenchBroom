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

#pragma once

#include <QObject>

#include <vector>

class QFocusEvent;
class QMouseEvent;
class QWindow;

namespace TrenchBroom
{
namespace View
{
class MapViewBase;

/**
 * Tracks the activation state of the group of map views in the map window. The group of
 * map views consists of multiple map views (3D or 2D), and the group has a shared
 * activation state. If the group is active, then it will receive mouse and keyboard
 * events as per usual, but if it is inactive, then it requires a click on one of the map
 * views to make the group active again.
 *
 * The activation state of the group is indicated by a changed cursor; when the group is
 * inactive, all map views have a hand icon to indicate that the user must click on one of
 * the views before it is ready for interaction.
 *
 * The map views change into the active state when the user clicks on any of the map views
 * with the left mouse button. The map views lose active state if the focus changes to any
 * widget that is not a map view, or if the map window loses its activation state.
 *
 * When the map views are in the active state, the focus will follow the mouse cursor,
 * that is, when the user moves the mouse cursor from one map view to another, then that
 * map view will receive focus.
 */
class MapViewActivationTracker : public QObject
{
  Q_OBJECT
private:
  std::vector<MapViewBase*> m_mapViews;

  bool m_active;

public:
  MapViewActivationTracker();

public:
  /**
   * Indicates whether the map views are in an active state.
   *
   * @return true if the map views are active and false otherwise
   */
  bool active() const;

public:
  /**
   * Adds the given window to the activation group.
   *
   * @param mapView the map view to add
   */
  void addWindow(MapViewBase* mapView);

  /**
   * Clears this activation tracker;
   */
  void clear();

  /**
   * Indicates that the activation state of the map window has changed.
   *
   * @param active true if the map window has become active, and false otherwise
   */
  void windowActivationChanged(bool active);

protected: // QObject overrides
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  /**
   * Called when a map view has received focus.
   *
   * @param event the focus event
   * @param widget the window that received the event
   */
  void setFocusEvent(QFocusEvent* event, QWidget* widget);

  /**
   * Called when a map view has lost focus.
   *
   * @param event the focus event
   * @param widget the window that received the event
   */
  void killFocusEvent(QFocusEvent* event, QWidget* widget);

  /**
   * Called when a map view has received a mouse down event.
   *
   * If the group is not in an active state the event will be discarded.
   * Otherwise, the event will be processed as usual
   *
   * @param event the mouse event
   * @param widget the window that received the event
   * @return true if the event should be discarded and false otherwise
   */
  bool mouseDownEvent(QMouseEvent* event, QWidget* widget);

  /**
   * Called when a map view has received a mouse up event.
   *
   * If the group is not in an active state, it will change into the active state and the
   * event will be discarded. Otherwise, the event will be processed as usual
   *
   * @param event the mouse event
   * @param widget the window that received the event
   * @return true if the event should be discarded and false otherwise
   */
  bool mouseUpEvent(QMouseEvent* event, QWidget* widget);

  /**
   * Called when the mouse enters a map view. If the group is in the active state and the
   * map view does not have focus, it will receive the focus.
   *
   * @param event the enter event
   * @param widget the window that received the event
   */
  void enterEvent(QEvent* event, QWidget* widget);
  void dragEnterEvent(QEvent* event, QWidget* widget);

  /**
   * Called when the group is activated.
   */
  void activate();

  /**
   * Called when the group is deactivated;
   */
  void deactivate();

  /**
   * Sets the cursor of all map views to a hand cursor that indicates that the user must
   * click to activate the group.
   */
  void setFocusCursor();

  /**
   * Sets the cursor of the given map view to a hand cursor.
   *
   * @param mapView the map view
   */
  void setFocusCursor(MapViewBase* mapView);

  /**
   * Sets the cursor to the regular pointer cursor.
   */
  void clearFocusCursor();

  /**
   * Sets the cursor of the given map view to a pointer cursor.
   *
   * @param mapView the map view
   */
  void clearFocusCursor(MapViewBase* mapView);
};
} // namespace View
} // namespace TrenchBroom
