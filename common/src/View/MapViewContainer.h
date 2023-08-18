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

#include <QWidget>

#include "View/MapView.h"

namespace TrenchBroom::View
{
class MapViewActivationTracker;
class MapViewBase;

class MapViewContainer : public QWidget, public MapView
{
  Q_OBJECT
public:
  explicit MapViewContainer(QWidget* parent);
  ~MapViewContainer() override;

public:
  bool canMaximizeCurrentView() const;
  bool currentViewMaximized() const;
  void toggleMaximizeCurrentView();

protected:
  /**
   * Returns the current map view. This is the map view which had last received focus.
   *
   * @return the current map view
   */
  MapView* currentMapView() const;

private: // implement MapView interface
  vm::vec3 doGetPasteObjectsDelta(
    const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const override;

private: // subclassing interface
  virtual bool doCanMaximizeCurrentView() const = 0;
  virtual bool doCurrentViewMaximized() const = 0;
  virtual void doToggleMaximizeCurrentView() = 0;
  virtual MapView* doGetCurrentMapView() const = 0;

public:
  virtual void cycleChildMapView(MapView* after) = 0;
};
} // namespace TrenchBroom::View
