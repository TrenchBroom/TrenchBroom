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

class QStackedLayout;

namespace tb
{
class Logger;

namespace gl
{
class ContextManager;
}

namespace ui
{
class AppController;
class MapDocument;
class MapViewToolBox;

class CyclingMapView : public MapViewContainer, public CameraLinkableView
{
  Q_OBJECT
public:
  static constexpr auto View_3D = 1;
  static constexpr auto View_XY = 2;
  static constexpr auto View_XZ = 4;
  static constexpr auto View_YZ = 8;
  static constexpr auto View_ZZ = View_XZ | View_YZ;
  static constexpr auto View_2D = View_XY | View_ZZ;
  static constexpr auto View_ALL = View_3D | View_2D;

private:
  MapDocument& m_document;

  std::vector<MapViewBase*> m_mapViews;
  MapViewBase* m_currentMapView = nullptr;

  QStackedLayout* m_layout = nullptr;

public:
  CyclingMapView(
    AppController& appController,
    MapDocument& document,
    MapViewToolBox& toolBox,
    gl::ContextManager& contextManager,
    int views,
    QWidget* parent = nullptr);

private:
  void createGui(
    AppController& appController,
    MapViewToolBox& toolBox,
    gl::ContextManager& contextManager,
    int views);
  void addMapView(MapViewBase* mapView);

private:
  void switchToMapView(MapViewBase* mapView);

public: // implement ViewEffectsService interface
  void flashSelection() override;

public: // implement MapView interface
  void installActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool isCurrent() const override;
  MapViewBase* firstMapViewBase() override;
  bool canSelectTall() override;
  void selectTall() override;
  void reset2dCameras(const gl::Camera& masterCamera, bool animate) override;
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

public: // implement CameraLinkableView interface
  void linkCamera(CameraLinkHelper& linkHelper) override;
};

} // namespace ui
} // namespace tb
