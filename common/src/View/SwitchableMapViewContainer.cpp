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

#include "SwitchableMapViewContainer.h"

#include <QGridLayout>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/FourPaneMapView.h"
#include "View/GLContextManager.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapViewActivationTracker.h"
#include "View/MapViewBar.h"
#include "View/MapViewContainer.h"
#include "View/MapViewLayout.h"
#include "View/MapViewToolBox.h"
#include "View/OnePaneMapView.h"
#include "View/QtUtils.h"
#include "View/ThreePaneMapView.h"
#include "View/TwoPaneMapView.h"
#include "mdl/PointTrace.h"
#include "render/MapRenderer.h"

#include "kdl/memory_utils.h"

namespace tb::View
{
SwitchableMapViewContainer::SwitchableMapViewContainer(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
  , m_contextManager{contextManager}
  , m_mapViewBar{new MapViewBar(m_document)}
  , m_toolBox{std::make_unique<MapViewToolBox>(m_document, m_mapViewBar->toolBook())}
  , m_mapRenderer{std::make_unique<render::MapRenderer>(m_document)}
  , m_activationTracker{std::make_unique<MapViewActivationTracker>()}
{
  setObjectName("SwitchableMapViewContainer");
  switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
  connectObservers();
}

SwitchableMapViewContainer::~SwitchableMapViewContainer()
{
  // we must destroy our children before we destroy our resources because they might still
  // use them in their destructors
  m_activationTracker->clear();
  delete m_mapView;
}

void SwitchableMapViewContainer::connectTopWidgets(Inspector* inspector)
{
  inspector->connectTopWidgets(m_mapViewBar);
}

void SwitchableMapViewContainer::windowActivationStateChanged(const bool active)
{
  m_activationTracker->windowActivationChanged(active);
}

bool SwitchableMapViewContainer::active() const
{
  return m_activationTracker->active();
}

void SwitchableMapViewContainer::switchToMapView(const MapViewLayout viewId)
{
  m_activationTracker->clear();

  // NOTE: not all widgets are deleted so we can't use deleteChildWidgetsAndLayout()
  delete m_mapView;
  m_mapView = nullptr;

  delete layout();

  switch (viewId)
  {
  case MapViewLayout::OnePane:
    m_mapView =
      new OnePaneMapView{m_document, *m_toolBox, *m_mapRenderer, m_contextManager};
    break;
  case MapViewLayout::TwoPanes:
    m_mapView =
      new TwoPaneMapView{m_document, *m_toolBox, *m_mapRenderer, m_contextManager};
    break;
  case MapViewLayout::ThreePanes:
    m_mapView =
      new ThreePaneMapView{m_document, *m_toolBox, *m_mapRenderer, m_contextManager};
    break;
  case MapViewLayout::FourPanes:
    m_mapView =
      new FourPaneMapView{m_document, *m_toolBox, *m_mapRenderer, m_contextManager};
    break;
    switchDefault();
  }

  installActivationTracker(*m_activationTracker);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(m_mapViewBar);
  layout->addWidget(m_mapView, 1);
  setLayout(layout);

  m_mapView->setFocus();
}

bool SwitchableMapViewContainer::anyToolActive() const
{
  return assembleBrushToolActive() || clipToolActive() || rotateObjectsToolActive()
         || scaleObjectsToolActive() || shearObjectsToolActive() || anyVertexToolActive();
}

void SwitchableMapViewContainer::deactivateTool()
{
  m_toolBox->deactivateAllTools();
}

bool SwitchableMapViewContainer::assembleBrushToolActive() const
{
  return m_toolBox->assembleBrushToolActive();
}

bool SwitchableMapViewContainer::canToggleAssembleBrushTool() const
{
  return true;
}

void SwitchableMapViewContainer::toggleAssembleBrushTool()
{
  assert(canToggleAssembleBrushTool());
  m_toolBox->toggleAssembleBrushTool();
}

bool SwitchableMapViewContainer::clipToolActive() const
{
  return m_toolBox->clipToolActive();
}

bool SwitchableMapViewContainer::canToggleClipTool() const
{
  return clipToolActive() || kdl::mem_lock(m_document)->selectedNodes().hasOnlyBrushes();
}

void SwitchableMapViewContainer::toggleClipTool()
{
  assert(canToggleClipTool());
  m_toolBox->toggleClipTool();
}

ClipTool& SwitchableMapViewContainer::clipTool()
{
  return m_toolBox->clipTool();
}

bool SwitchableMapViewContainer::rotateObjectsToolActive() const
{
  return m_toolBox->rotateObjectsToolActive();
}

bool SwitchableMapViewContainer::canToggleRotateObjectsTool() const
{
  return rotateObjectsToolActive() || kdl::mem_lock(m_document)->hasSelectedNodes();
}

void SwitchableMapViewContainer::toggleRotateObjectsTool()
{
  assert(canToggleRotateObjectsTool());
  m_toolBox->toggleRotateObjectsTool();
}

bool SwitchableMapViewContainer::scaleObjectsToolActive() const
{
  return m_toolBox->scaleObjectsToolActive();
}

bool SwitchableMapViewContainer::shearObjectsToolActive() const
{
  return m_toolBox->shearObjectsToolActive();
}

bool SwitchableMapViewContainer::canToggleScaleObjectsTool() const
{
  return scaleObjectsToolActive() || kdl::mem_lock(m_document)->hasSelectedNodes();
}

void SwitchableMapViewContainer::toggleScaleObjectsTool()
{
  assert(canToggleScaleObjectsTool());
  m_toolBox->toggleScaleObjectsTool();
}

bool SwitchableMapViewContainer::canToggleShearObjectsTool() const
{
  return shearObjectsToolActive() || kdl::mem_lock(m_document)->hasSelectedNodes();
}

void SwitchableMapViewContainer::toggleShearObjectsTool()
{
  assert(canToggleShearObjectsTool());
  m_toolBox->toggleShearObjectsTool();
}

bool SwitchableMapViewContainer::canToggleVertexTools() const
{
  return vertexToolActive() || edgeToolActive() || faceToolActive()
         || kdl::mem_lock(m_document)->selectedNodes().hasOnlyBrushes();
}

bool SwitchableMapViewContainer::anyVertexToolActive() const
{
  return vertexToolActive() || edgeToolActive() || faceToolActive();
}

bool SwitchableMapViewContainer::vertexToolActive() const
{
  return m_toolBox->vertexToolActive();
}

bool SwitchableMapViewContainer::edgeToolActive() const
{
  return m_toolBox->edgeToolActive();
}

bool SwitchableMapViewContainer::faceToolActive() const
{
  return m_toolBox->faceToolActive();
}

void SwitchableMapViewContainer::toggleVertexTool()
{
  assert(canToggleVertexTools());
  m_toolBox->toggleVertexTool();
}

void SwitchableMapViewContainer::toggleEdgeTool()
{
  assert(canToggleVertexTools());
  m_toolBox->toggleEdgeTool();
}

void SwitchableMapViewContainer::toggleFaceTool()
{
  assert(canToggleVertexTools());
  m_toolBox->toggleFaceTool();
}

VertexTool& SwitchableMapViewContainer::vertexTool()
{
  return m_toolBox->vertexTool();
}

EdgeTool& SwitchableMapViewContainer::edgeTool()
{
  return m_toolBox->edgeTool();
}

FaceTool& SwitchableMapViewContainer::faceTool()
{
  return m_toolBox->faceTool();
}

MapViewToolBox& SwitchableMapViewContainer::mapViewToolBox()
{
  return *m_toolBox;
}

bool SwitchableMapViewContainer::canMoveCameraToNextTracePoint() const
{
  auto document = kdl::mem_lock(m_document);
  if (const auto* pointFile = document->pointFile())
  {
    return pointFile->hasNextPoint();
  }
  return false;
}

bool SwitchableMapViewContainer::canMoveCameraToPreviousTracePoint() const
{
  auto document = kdl::mem_lock(m_document);
  if (const auto* pointFile = document->pointFile())
  {
    return pointFile->hasPreviousPoint();
  }
  return false;
}

void SwitchableMapViewContainer::moveCameraToNextTracePoint()
{
  auto document = kdl::mem_lock(m_document);
  assert(document->isPointFileLoaded());

  if (auto* pointFile = document->pointFile())
  {
    pointFile->advance();
    m_mapView->moveCameraToCurrentTracePoint();
  }
}

void SwitchableMapViewContainer::moveCameraToPreviousTracePoint()
{
  auto document = kdl::mem_lock(m_document);
  assert(document->isPointFileLoaded());

  if (auto* pointFile = document->pointFile())
  {
    pointFile->retreat();
    m_mapView->moveCameraToCurrentTracePoint();
  }
}

bool SwitchableMapViewContainer::canMaximizeCurrentView() const
{
  return m_mapView->canMaximizeCurrentView();
}

bool SwitchableMapViewContainer::currentViewMaximized() const
{
  return m_mapView->currentViewMaximized();
}

void SwitchableMapViewContainer::toggleMaximizeCurrentView()
{
  m_mapView->toggleMaximizeCurrentView();
}

void SwitchableMapViewContainer::connectObservers()
{
  m_notifierConnection +=
    m_toolBox->refreshViewsNotifier.connect([&](Tool& tool) { refreshViews(tool); });
}

void SwitchableMapViewContainer::refreshViews(Tool&)
{
  // NOTE: it doesn't work to call QWidget::update() here. The actual OpenGL view is a
  // QWindow embedded in the widget hierarchy with QWidget::createWindowContainer(), and
  // we need to call QWindow::requestUpdate().
  m_mapView->refreshViews();
}

void SwitchableMapViewContainer::installActivationTracker(
  MapViewActivationTracker& activationTracker)
{
  m_mapView->installActivationTracker(activationTracker);
}

bool SwitchableMapViewContainer::isCurrent() const
{
  return m_mapView->isCurrent();
}

MapViewBase* SwitchableMapViewContainer::firstMapViewBase()
{
  return m_mapView->firstMapViewBase();
}

bool SwitchableMapViewContainer::canSelectTall()
{
  return m_mapView->canSelectTall();
}

void SwitchableMapViewContainer::selectTall()
{
  m_mapView->selectTall();
}

vm::vec3d SwitchableMapViewContainer::pasteObjectsDelta(
  const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const
{
  return m_mapView->pasteObjectsDelta(bounds, referenceBounds);
}

void SwitchableMapViewContainer::reset2dCameras(
  const render::Camera& masterCamera, const bool animate)
{
  m_mapView->reset2dCameras(masterCamera, animate);
}

void SwitchableMapViewContainer::focusCameraOnSelection(const bool animate)
{
  m_mapView->focusCameraOnSelection(animate);
}

void SwitchableMapViewContainer::moveCameraToPosition(
  const vm::vec3f& position, const bool animate)
{
  m_mapView->moveCameraToPosition(position, animate);
}

void SwitchableMapViewContainer::moveCameraToCurrentTracePoint()
{
  m_mapView->moveCameraToCurrentTracePoint();
}

void SwitchableMapViewContainer::flashSelection()
{
  m_mapView->flashSelection();
}

bool SwitchableMapViewContainer::cancelMouseDrag()
{
  return m_mapView->cancelMouseDrag();
}

void SwitchableMapViewContainer::refreshViews()
{
  m_mapView->refreshViews();
}

} // namespace tb::View
