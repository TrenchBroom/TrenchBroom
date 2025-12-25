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

#include "CyclingMapView.h"

#include <QShortcut>
#include <QStackedLayout>

#include "gl/Camera.h"
#include "ui/MapDocument.h"
#include "ui/MapView2D.h"
#include "ui/MapView3D.h"
#include "ui/MapViewActivationTracker.h"

#include "kd/contracts.h"

#include "vm/scalar.h"

namespace tb::ui
{

CyclingMapView::CyclingMapView(
  MapDocument& document,
  MapViewToolBox& toolBox,
  gl::ContextManager& contextManager,
  const int views,
  QWidget* parent)
  : MapViewContainer{parent}
  , m_document{document}
{
  setObjectName("CyclingMapView");
  createGui(toolBox, contextManager, views);
}

void CyclingMapView::createGui(
  MapViewToolBox& toolBox, gl::ContextManager& contextManager, const int views)
{
  if (views & View_3D)
  {
    addMapView(new MapView3D{m_document, toolBox, contextManager});
  }
  if (views & View_XY)
  {
    addMapView(
      new MapView2D{m_document, toolBox, contextManager, MapView2D::ViewPlane::XY});
  }
  if (views & View_XZ)
  {
    addMapView(
      new MapView2D{m_document, toolBox, contextManager, MapView2D::ViewPlane::XZ});
  }
  if (views & View_YZ)
  {
    addMapView(
      new MapView2D{m_document, toolBox, contextManager, MapView2D::ViewPlane::YZ});
  }

  m_layout = new QStackedLayout{};
  // NOTE: It's important to setLayout() before adding widgets, rather than after.
  // Otherwise, they get setVisible immediately (and the first render calls happen during
  // the for loop), which breaks multisampling
  setLayout(m_layout);

  for (auto* mapView : m_mapViews)
  {
    m_layout->addWidget(mapView);
  }

  contract_assert(!m_mapViews.empty());
  switchToMapView(m_mapViews[0]);
}

void CyclingMapView::addMapView(MapViewBase* mapView)
{
  m_mapViews.push_back(mapView);
  mapView->setContainer(this);
}

void CyclingMapView::switchToMapView(MapViewBase* mapView)
{
  m_currentMapView = mapView;

  m_layout->setCurrentWidget(m_currentMapView);
  m_currentMapView->setFocus();
}

void CyclingMapView::flashSelection()
{
  m_currentMapView->flashSelection();
}

void CyclingMapView::installActivationTracker(MapViewActivationTracker& activationTracker)
{
  for (auto* mapView : m_mapViews)
  {
    activationTracker.addWindow(mapView);
  }
}

bool CyclingMapView::isCurrent() const
{
  return m_currentMapView->isCurrent();
}

MapViewBase* CyclingMapView::firstMapViewBase()
{
  return m_currentMapView;
}

bool CyclingMapView::canSelectTall()
{
  return m_currentMapView->canSelectTall();
}

void CyclingMapView::selectTall()
{
  m_currentMapView->selectTall();
}

void CyclingMapView::reset2dCameras(const gl::Camera& masterCamera, bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->reset2dCameras(masterCamera, animate);
  }
}

void CyclingMapView::focusCameraOnSelection(const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->focusCameraOnSelection(animate);
  }
}

void CyclingMapView::moveCameraToPosition(const vm::vec3f& position, const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToPosition(position, animate);
  }
}

void CyclingMapView::moveCameraToCurrentTracePoint()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToCurrentTracePoint();
  }
}

bool CyclingMapView::cancelMouseDrag()
{
  auto result = false;
  for (auto* mapView : m_mapViews)
  {
    result = result || mapView->cancelMouseDrag();
  }
  return result;
}

void CyclingMapView::refreshViews()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->refreshViews();
  }
}

bool CyclingMapView::canMaximizeCurrentView() const
{
  return false;
}

bool CyclingMapView::currentViewMaximized() const
{
  return true;
}

void CyclingMapView::toggleMaximizeCurrentView() {}

MapView* CyclingMapView::currentMapView() const
{
  return m_currentMapView;
}

void CyclingMapView::linkCamera(CameraLinkHelper& helper)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->linkCamera(helper);
  }
}

void CyclingMapView::cycleChildMapView(MapView* after)
{
  for (size_t i = 0; i < m_mapViews.size(); ++i)
  {
    if (after == m_mapViews[i])
    {
      switchToMapView(m_mapViews[vm::succ(i, m_mapViews.size())]);
      focusCameraOnSelection(false);
      break;
    }
  }
}

} // namespace tb::ui
