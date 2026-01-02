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

#include "Macros.h"
#include "NotifierConnection.h"
#include "ui/MapView.h"

namespace tb
{
namespace gl
{
class ContextManager;
}

namespace ui
{
class AppController;
class ClipTool;
class EdgeTool;
class FaceTool;
class Inspector;
class MapDocument;
class MapViewBar;
enum class MapViewLayout;
class MapViewToolBox;
class MultiPaneMapView;
class Tool;
class VertexTool;

class SwitchableMapViewContainer : public QWidget, public MapView
{
  Q_OBJECT
private:
  AppController& m_appController;
  MapDocument& m_document;
  gl::ContextManager& m_contextManager;

  MapViewBar* m_mapViewBar = nullptr;
  std::unique_ptr<MapViewToolBox> m_toolBox;

  MultiPaneMapView* m_mapView = nullptr;
  std::unique_ptr<MapViewActivationTracker> m_activationTracker;

  NotifierConnection m_notifierConnection;

public:
  SwitchableMapViewContainer(
    AppController& appController,
    MapDocument& document,
    gl::ContextManager& contextManager,
    QWidget* parent = nullptr);
  ~SwitchableMapViewContainer() override;

  void connectTopWidgets(Inspector* inspector);

  void windowActivationStateChanged(bool active);

  bool active() const;
  void switchToMapView(MapViewLayout viewId);

  bool anyModalToolActive() const;
  void deactivateCurrentTool();

  bool assembleBrushToolActive() const;
  bool canToggleAssembleBrushTool() const;
  void toggleAssembleBrushTool();

  bool clipToolActive() const;
  bool canToggleClipTool() const;
  void toggleClipTool();
  ClipTool& clipTool();

  bool rotateToolActive() const;
  bool canToggleRotateTool() const;
  void toggleRotateTool();

  bool scaleToolActive() const;
  bool canToggleScaleTool() const;
  void toggleScaleTool();

  bool shearToolActive() const;
  bool canToggleShearTool() const;
  void toggleShearTool();

  bool canToggleVertexTools() const;
  bool anyVertexToolActive() const;
  bool vertexToolActive() const;
  bool edgeToolActive() const;
  bool faceToolActive() const;
  void toggleVertexTool();
  void toggleEdgeTool();
  void toggleFaceTool();
  VertexTool& vertexTool();
  EdgeTool& edgeTool();
  FaceTool& faceTool();
  MapViewToolBox& mapViewToolBox();

  bool canMoveCameraToNextTracePoint() const;
  bool canMoveCameraToPreviousTracePoint() const;
  void moveCameraToNextTracePoint();
  void moveCameraToPreviousTracePoint();

  bool canMaximizeCurrentView() const;
  bool currentViewMaximized() const;
  void toggleMaximizeCurrentView();

private:
  void connectObservers();
  void refreshViews(Tool& tool);

public: // implement MapView interface
  void installActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool isCurrent() const override;
  MapViewBase* firstMapViewBase() override;
  bool canSelectTall() override;
  void selectTall() override;
  vm::vec3d pasteObjectsDelta(
    const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const override;
  void reset2dCameras(const gl::Camera& masterCamera, bool animate) override;
  void focusCameraOnSelection(bool animate) override;
  void moveCameraToPosition(const vm::vec3f& position, bool animate) override;
  void moveCameraToCurrentTracePoint() override;
  bool cancelMouseDrag() override;
  void refreshViews() override;

public: // implement ViewEffectsService interface
  void flashSelection() override;

  deleteCopyAndMove(SwitchableMapViewContainer);
};

} // namespace ui
} // namespace tb
