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

#include "MultiMapView.h"

#include "Ensure.h"
#include "View/MapView.h"

namespace TrenchBroom
{
namespace View
{
MultiMapView::MultiMapView(QWidget* parent)
  : MapViewContainer(parent)
  , m_maximizedView(nullptr)
{
}

MultiMapView::~MultiMapView() = default;

void MultiMapView::addMapView(MapView* mapView)
{
  ensure(mapView != nullptr, "mapView is nullptr");
  m_mapViews.push_back(mapView);
  mapView->setContainer(this);
}

void MultiMapView::doFlashSelection()
{
  for (MapView* mapView : m_mapViews)
    mapView->flashSelection();
}

void MultiMapView::doInstallActivationTracker(MapViewActivationTracker& activationTracker)
{
  for (auto* mapView : m_mapViews)
  {
    mapView->installActivationTracker(activationTracker);
  }
}

bool MultiMapView::doGetIsCurrent() const
{
  for (MapView* mapView : m_mapViews)
  {
    if (mapView->isCurrent())
      return true;
  }
  return false;
}

MapViewBase* MultiMapView::doGetFirstMapViewBase()
{
  ensure(!m_mapViews.empty(), "MultiMapView empty in doGetFirstMapViewBase()");
  return m_mapViews.at(0)->firstMapViewBase();
}

bool MultiMapView::doCanSelectTall()
{
  if (currentMapView() == nullptr)
    return false;
  return currentMapView()->canSelectTall();
}

void MultiMapView::doSelectTall()
{
  if (currentMapView() != nullptr)
    currentMapView()->selectTall();
}

void MultiMapView::doFocusCameraOnSelection(const bool animate)
{
  for (MapView* mapView : m_mapViews)
    mapView->focusCameraOnSelection(animate);
}

void MultiMapView::doMoveCameraToPosition(const vm::vec3& position, const bool animate)
{
  for (MapView* mapView : m_mapViews)
    mapView->moveCameraToPosition(position, animate);
}

void MultiMapView::doMoveCameraToCurrentTracePoint()
{
  for (MapView* mapView : m_mapViews)
    mapView->moveCameraToCurrentTracePoint();
}

bool MultiMapView::doCanMaximizeCurrentView() const
{
  return m_maximizedView != nullptr || currentMapView() != nullptr;
}

bool MultiMapView::doCurrentViewMaximized() const
{
  return m_maximizedView != nullptr;
}

void MultiMapView::doToggleMaximizeCurrentView()
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

MapView* MultiMapView::doGetCurrentMapView() const
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

void MultiMapView::cycleChildMapView(MapView*)
{
  // only CyclingMapView support cycling
}

bool MultiMapView::doCancelMouseDrag()
{
  bool result = false;
  for (MapView* mapView : m_mapViews)
  {
    result |= mapView->cancelMouseDrag();
  }
  return result;
}

void MultiMapView::doRefreshViews()
{
  for (MapView* mapView : m_mapViews)
  {
    mapView->refreshViews();
  }
}
} // namespace View
} // namespace TrenchBroom
