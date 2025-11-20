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

#include "MultiPaneMapView.h"

#include "Contracts.h"
#include "ui/MapView.h"

#include <algorithm>

namespace tb::ui
{
MultiPaneMapView::MultiPaneMapView(QWidget* parent)
  : MapViewContainer{parent}
{
}

MultiPaneMapView::~MultiPaneMapView() = default;

void MultiPaneMapView::addMapView(MapView* mapView)
{
  contract_pre(mapView != nullptr);

  m_mapViews.push_back(mapView);
  mapView->setContainer(this);
}

void MultiPaneMapView::flashSelection()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->flashSelection();
  }
}

void MultiPaneMapView::installActivationTracker(
  MapViewActivationTracker& activationTracker)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->installActivationTracker(activationTracker);
  }
}

bool MultiPaneMapView::isCurrent() const
{
  return std::ranges::any_of(
    m_mapViews, [](auto* mapView) { return mapView->isCurrent(); });
}

MapViewBase* MultiPaneMapView::firstMapViewBase()
{
  contract_pre(!m_mapViews.empty());

  return m_mapViews.front()->firstMapViewBase();
}

bool MultiPaneMapView::canSelectTall()
{
  return currentMapView() && currentMapView()->canSelectTall();
}

void MultiPaneMapView::selectTall()
{
  if (currentMapView())
  {
    currentMapView()->selectTall();
  }
}

void MultiPaneMapView::reset2dCameras(
  const render::Camera& masterCamera, const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->reset2dCameras(masterCamera, animate);
  }
}

void MultiPaneMapView::focusCameraOnSelection(const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->focusCameraOnSelection(animate);
  }
}

void MultiPaneMapView::moveCameraToPosition(const vm::vec3f& position, const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToPosition(position, animate);
  }
}

void MultiPaneMapView::moveCameraToCurrentTracePoint()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToCurrentTracePoint();
  }
}

bool MultiPaneMapView::cancelMouseDrag()
{
  auto result = false;
  for (auto* mapView : m_mapViews)
  {
    result |= mapView->cancelMouseDrag();
  }
  return result;
}

void MultiPaneMapView::refreshViews()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->refreshViews();
  }
}

bool MultiPaneMapView::canMaximizeCurrentView() const
{
  return m_maximizedView || currentMapView();
}

bool MultiPaneMapView::currentViewMaximized() const
{
  return m_maximizedView;
}

void MultiPaneMapView::toggleMaximizeCurrentView()
{
  if (m_maximizedView)
  {
    restoreViews();
    m_maximizedView = nullptr;
  }
  else
  {
    m_maximizedView = currentMapView();
    if (m_maximizedView)
    {
      maximizeView(m_maximizedView);
    }
  }
}

MapView* MultiPaneMapView::currentMapView() const
{
  for (auto* mapView : m_mapViews)
  {
    if (mapView->isCurrent())
    {
      return mapView;
    }
  }
  return nullptr;
}

void MultiPaneMapView::cycleChildMapView(MapView*)
{
  // only CyclingMapView support cycling
}

} // namespace tb::ui
