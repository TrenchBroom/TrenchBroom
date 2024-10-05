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
  virtual bool canMaximizeCurrentView() const = 0;
  virtual bool currentViewMaximized() const = 0;
  virtual void toggleMaximizeCurrentView() = 0;

protected:
  /**
   * Returns the current map view. This is the map view which had last received focus.
   *
   * @return the current map view
   */
  virtual MapView* currentMapView() const = 0;

public: // implement MapView interface
  vm::vec3d pasteObjectsDelta(
    const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const override;

public:
  virtual void cycleChildMapView(MapView* after) = 0;
};
} // namespace TrenchBroom::View
