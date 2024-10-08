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

#include "ui/ViewEffectsService.h"

#include "vm/bbox.h"
#include "vm/vec.h"

namespace tb::render
{
class Camera;
}

namespace tb::ui
{
class MapViewActivationTracker;
class MapViewBase;
class MapViewContainer;

class MapView : public ViewEffectsService
{
private:
  MapViewContainer* m_container = nullptr;

public:
  ~MapView() override;

  void setContainer(MapViewContainer* container);
  virtual void installActivationTracker(MapViewActivationTracker& activationTracker) = 0;

  virtual bool isCurrent() const = 0;
  virtual MapViewBase* firstMapViewBase() = 0;

  virtual bool canSelectTall() = 0;
  virtual void selectTall() = 0;

  virtual vm::vec3d pasteObjectsDelta(
    const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const = 0;

  virtual void reset2dCameras(const render::Camera& masterCamera, bool animate) = 0;
  virtual void focusCameraOnSelection(bool animate) = 0;
  virtual void moveCameraToPosition(const vm::vec3f& position, bool animate) = 0;

  virtual void moveCameraToCurrentTracePoint() = 0;

  virtual bool cancelMouseDrag() = 0;

  /**
   * If the parent of this view is a CyclingMapView, cycle to the
   * next child, otherwise do nothing.
   */
  void cycleMapView();

  /**
   * Requests repaint of the managed map views. Note, this must be used instead of
   * QWidget::update()
   */
  virtual void refreshViews() = 0;
};
} // namespace tb::ui
