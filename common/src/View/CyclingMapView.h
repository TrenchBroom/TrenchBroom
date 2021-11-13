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

#include "FloatType.h"
#include "View/CameraLinkHelper.h"
#include "View/MapViewContainer.h"

#include <memory>
#include <vector>

class QStackedLayout;

namespace TrenchBroom {
class Logger;

namespace Renderer {
class MapRenderer;
}

namespace View {
class GLContextManager;
class MapDocument;
class MapViewToolBox;

class CyclingMapView : public MapViewContainer, public CameraLinkableView {
  Q_OBJECT
public:
  typedef enum
  {
    View_3D = 1,
    View_XY = 2,
    View_XZ = 4,
    View_YZ = 8,
    View_ZZ = View_XZ | View_YZ,
    View_2D = View_XY | View_ZZ,
    View_ALL = View_3D | View_2D
  } View;

private:
  Logger* m_logger;
  std::weak_ptr<MapDocument> m_document;

  using MapViewList = std::vector<MapViewBase*>;
  MapViewList m_mapViews;
  MapViewBase* m_currentMapView;

  QStackedLayout* m_layout;

public:
  CyclingMapView(
    std::weak_ptr<MapDocument> document, MapViewToolBox& toolBox,
    Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager, View views,
    Logger* logger, QWidget* parent = nullptr);

private:
  void createGui(
    MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager,
    View views);
  void addMapView(MapViewBase* mapView);

private:
  void switchToMapView(MapViewBase* mapView);

private: // implement ViewEffectsService interface
  void doFlashSelection() override;

private: // implement MapView interface
  bool doGetIsCurrent() const override;
  bool doCanSelectTall() override;
  void doSelectTall() override;
  void doFocusCameraOnSelection(bool animate) override;
  void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
  void doMoveCameraToCurrentTracePoint() override;
  bool doCanMaximizeCurrentView() const override;
  bool doCurrentViewMaximized() const override;
  void doToggleMaximizeCurrentView() override;

  bool doCancelMouseDrag() override;
  void doRefreshViews() override;

private: // implement MapViewContainer interface
  void doInstallActivationTracker(MapViewActivationTracker& activationTracker) override;
  MapView* doGetCurrentMapView() const override;
  MapViewBase* doGetFirstMapViewBase() override;

public:
  void cycleChildMapView(MapView* after) override;

private: // implement CameraLinkableView interface
  void doLinkCamera(CameraLinkHelper& linkHelper) override;
};
} // namespace View
} // namespace TrenchBroom
