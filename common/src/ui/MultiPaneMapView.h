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

#include "ui/CameraLinkHelper.h"
#include "ui/MapViewContainer.h"

#include <vector>

class QWidget;
class QWidget;

namespace tb::ui
{
class MultiPaneMapView : public MapViewContainer
{
private:
  using MapViewList = std::vector<MapView*>;
  MapViewList m_mapViews;
  MapView* m_maximizedView = nullptr;

protected:
  CameraLinkHelper m_linkHelper;

  explicit MultiPaneMapView(QWidget* parent = nullptr);

public:
  ~MultiPaneMapView() override;

protected:
  void addMapView(MapView* mapView);

public: // implement ViewEffectsService interface
  void flashSelection() override;

public: // implement MapView interface
  void installActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool isCurrent() const override;
  MapViewBase* firstMapViewBase() override;
  bool canSelectTall() override;
  void selectTall() override;
  void reset2dCameras(const render::Camera& masterCamera, bool animate) override;
  void focusCameraOnSelection(bool animate) override;
  void moveCameraToPosition(const vm::vec3f& position, bool animate) override;
  void moveCameraToCurrentTracePoint() override;
  bool cancelMouseDrag() override;
  void refreshViews() override;

public: // implement MapViewContainer interface
  bool canMaximizeCurrentView() const override;
  bool currentViewMaximized() const override;
  void toggleMaximizeCurrentView() override;

protected:
  MapView* currentMapView() const override;

public:
  void cycleChildMapView(MapView* after) override;

private: // subclassing interface
  virtual void maximizeView(MapView* view) = 0;
  virtual void restoreViews() = 0;
};

} // namespace tb::ui
