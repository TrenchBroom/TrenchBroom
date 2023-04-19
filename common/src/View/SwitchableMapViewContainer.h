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
#include "Macros.h"
#include "NotifierConnection.h"
#include "View/MapView.h"

#include <memory>

#include <QWidget>

namespace TrenchBroom
{
class Logger;

namespace Renderer
{
class MapRenderer;
}

namespace View
{
class ClipTool;
class EdgeTool;
class FaceTool;
class GLContextManager;
class Inspector;
class MapDocument;
class MapViewBar;
enum class MapViewLayout;
class MapViewToolBox;
class Tool;
class VertexTool;

class SwitchableMapViewContainer : public QWidget, public MapView
{
  Q_OBJECT
private:
  Logger* m_logger;
  std::weak_ptr<MapDocument> m_document;
  GLContextManager& m_contextManager;

  MapViewBar* m_mapViewBar;
  std::unique_ptr<MapViewToolBox> m_toolBox;

  std::unique_ptr<Renderer::MapRenderer> m_mapRenderer;

  MapViewContainer* m_mapView;
  std::unique_ptr<MapViewActivationTracker> m_activationTracker;

  NotifierConnection m_notifierConnection;

public:
  SwitchableMapViewContainer(
    Logger* logger,
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);
  ~SwitchableMapViewContainer() override;

  void connectTopWidgets(Inspector* inspector);

  void windowActivationStateChanged(bool active);

  bool active() const;
  void switchToMapView(MapViewLayout viewId);

  bool anyToolActive() const;
  void deactivateTool();

  bool toolAllowsObjectDeletion() const;

  bool createComplexBrushToolActive() const;
  bool canToggleCreateComplexBrushTool() const;
  void toggleCreateComplexBrushTool();

  bool createPrimitiveBrushToolActive() const;
  bool canToggleCreatePrimitiveBrushTool() const;
  void toggleCreatePrimitiveBrushTool();

  bool clipToolActive() const;
  bool canToggleClipTool() const;
  void toggleClipTool();
  ClipTool& clipTool();

  bool rotateObjectsToolActive() const;
  bool canToggleRotateObjectsTool() const;
  void toggleRotateObjectsTool();

  bool scaleObjectsToolActive() const;
  bool canToggleScaleObjectsTool() const;
  void toggleScaleObjectsTool();

  bool shearObjectsToolActive() const;
  bool canToggleShearObjectsTool() const;
  void toggleShearObjectsTool();

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

private: // implement MapView interface
  void doInstallActivationTracker(MapViewActivationTracker& activationTracker) override;
  bool doGetIsCurrent() const override;
  MapViewBase* doGetFirstMapViewBase() override;
  bool doCanSelectTall() override;
  void doSelectTall() override;
  vm::vec3 doGetPasteObjectsDelta(
    const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const override;
  void doFocusCameraOnSelection(bool animate) override;
  void doMoveCameraToPosition(const vm::vec3& position, bool animate) override;
  void doMoveCameraToCurrentTracePoint() override;
  bool doCancelMouseDrag() override;
  void doRefreshViews() override;

private: // implement ViewEffectsService interface
  void doFlashSelection() override;

  deleteCopyAndMove(SwitchableMapViewContainer);
};
} // namespace View
} // namespace TrenchBroom
