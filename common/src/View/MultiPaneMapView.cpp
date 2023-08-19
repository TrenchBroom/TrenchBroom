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

#include "MultiPaneMapView.h"

#include "Ensure.h"
#include "View/MapView.h"

#include <algorithm>

namespace TrenchBroom::View
{
MultiPaneMapView::MultiPaneMapView(QWidget* parent)
  : MapViewContainer{parent}
{
}

MultiPaneMapView::~MultiPaneMapView() = default;

void MultiPaneMapView::addMapView(MapView* mapView)
{
  ensure(mapView != nullptr, "mapView is nullptr");
  m_mapViews.push_back(mapView);
  mapView->setContainer(this);
}

void MultiPaneMapView::doFlashSelection()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->flashSelection();
  }
}

void MultiPaneMapView::doInstallActivationTracker(
  MapViewActivationTracker& activationTracker)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->installActivationTracker(activationTracker);
  }
}

bool MultiPaneMapView::doGetIsCurrent() const
{
  return std::any_of(m_mapViews.begin(), m_mapViews.end(), [](auto* mapView) {
    return mapView->isCurrent();
  });
}

MapViewBase* MultiPaneMapView::doGetFirstMapViewBase()
{
  ensure(!m_mapViews.empty(), "MultiPaneMapView empty in doGetFirstMapViewBase()");
  return m_mapViews.front()->firstMapViewBase();
}

bool MultiPaneMapView::doCanSelectTall()
{
  return currentMapView() && currentMapView()->canSelectTall();
}

void MultiPaneMapView::doSelectTall()
{
  if (currentMapView())
  {
    currentMapView()->selectTall();
  }
}

void MultiPaneMapView::doFocusCameraOnSelection(const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->focusCameraOnSelection(animate);
  }
}

void MultiPaneMapView::doMoveCameraToPosition(
  const vm::vec3f& position, const bool animate)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToPosition(position, animate);
  }
}

void MultiPaneMapView::doMoveCameraToCurrentTracePoint()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->moveCameraToCurrentTracePoint();
  }
}

bool MultiPaneMapView::doCanMaximizeCurrentView() const
{
  return m_maximizedView || currentMapView();
}

bool MultiPaneMapView::doCurrentViewMaximized() const
{
  return m_maximizedView;
}

void MultiPaneMapView::doToggleMaximizeCurrentView()
{
  if (m_maximizedView)
  {
    doRestoreViews();
    m_maximizedView = nullptr;
  }
  else
  {
    m_maximizedView = currentMapView();
    if (m_maximizedView)
    {
      doMaximizeView(m_maximizedView);
    }
  }
}

MapView* MultiPaneMapView::doGetCurrentMapView() const
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

bool MultiPaneMapView::doCancelMouseDrag()
{
  auto result = false;
  for (auto* mapView : m_mapViews)
  {
    result |= mapView->cancelMouseDrag();
  }
  return result;
}

void MultiPaneMapView::doRefreshViews()
{
  for (auto* mapView : m_mapViews)
  {
    mapView->refreshViews();
  }
}
} // namespace TrenchBroom::View
