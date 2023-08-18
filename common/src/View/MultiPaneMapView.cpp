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

#include "Ensure.h"
#include "MultiPaneMapView.h"
#include "View/MapView.h"

namespace TrenchBroom
{
namespace View
{
MultiPaneMapView::MultiPaneMapView(QWidget* parent)
  : MapViewContainer(parent)
  , m_maximizedView(nullptr)
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
  for (MapView* mapView : m_mapViews)
    mapView->flashSelection();
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
  for (MapView* mapView : m_mapViews)
  {
    if (mapView->isCurrent())
      return true;
  }
  return false;
}

MapViewBase* MultiPaneMapView::doGetFirstMapViewBase()
{
  ensure(!m_mapViews.empty(), "MultiPaneMapView empty in doGetFirstMapViewBase()");
  return m_mapViews.at(0)->firstMapViewBase();
}

bool MultiPaneMapView::doCanSelectTall()
{
  if (currentMapView() == nullptr)
    return false;
  return currentMapView()->canSelectTall();
}

void MultiPaneMapView::doSelectTall()
{
  if (currentMapView() != nullptr)
    currentMapView()->selectTall();
}

void MultiPaneMapView::doFocusCameraOnSelection(const bool animate)
{
  for (MapView* mapView : m_mapViews)
    mapView->focusCameraOnSelection(animate);
}

void MultiPaneMapView::doMoveCameraToPosition(
  const vm::vec3& position, const bool animate)
{
  for (MapView* mapView : m_mapViews)
    mapView->moveCameraToPosition(position, animate);
}

void MultiPaneMapView::doMoveCameraToCurrentTracePoint()
{
  for (MapView* mapView : m_mapViews)
    mapView->moveCameraToCurrentTracePoint();
}

bool MultiPaneMapView::doCanMaximizeCurrentView() const
{
  return m_maximizedView != nullptr || currentMapView() != nullptr;
}

bool MultiPaneMapView::doCurrentViewMaximized() const
{
  return m_maximizedView != nullptr;
}

void MultiPaneMapView::doToggleMaximizeCurrentView()
{
  if (m_maximizedView != nullptr)
  {
    doRestoreViews();
    m_maximizedView = nullptr;
  }
  else
  {
    m_maximizedView = currentMapView();
    if (m_maximizedView != nullptr)
    {
      doMaximizeView(m_maximizedView);
    }
  }
}

MapView* MultiPaneMapView::doGetCurrentMapView() const
{
  for (MapView* mapView : m_mapViews)
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
  bool result = false;
  for (MapView* mapView : m_mapViews)
  {
    result |= mapView->cancelMouseDrag();
  }
  return result;
}

void MultiPaneMapView::doRefreshViews()
{
  for (MapView* mapView : m_mapViews)
  {
    mapView->refreshViews();
  }
}
} // namespace View
} // namespace TrenchBroom
