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

#include "MapView.h"

#include "FloatType.h"
#include "View/MapViewContainer.h"

#include <vecmath/vec.h>

namespace TrenchBroom::View
{
MapView::~MapView() = default;

void MapView::setContainer(MapViewContainer* container)
{
  m_container = container;
}

void MapView::installActivationTracker(MapViewActivationTracker& activationTracker)
{
  doInstallActivationTracker(activationTracker);
}

bool MapView::isCurrent() const
{
  return doGetIsCurrent();
}

MapViewBase* MapView::firstMapViewBase()
{
  return doGetFirstMapViewBase();
}

bool MapView::canSelectTall()
{
  return doCanSelectTall();
}

void MapView::selectTall()
{
  doSelectTall();
}

vm::vec3 MapView::pasteObjectsDelta(
  const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const
{
  return doGetPasteObjectsDelta(bounds, referenceBounds);
}

void MapView::focusCameraOnSelection(const bool animate)
{
  doFocusCameraOnSelection(animate);
}

void MapView::moveCameraToPosition(const vm::vec3f& position, const bool animate)
{
  doMoveCameraToPosition(position, animate);
}

void MapView::moveCameraToCurrentTracePoint()
{
  doMoveCameraToCurrentTracePoint();
}

bool MapView::cancelMouseDrag()
{
  return doCancelMouseDrag();
}

void MapView::cycleMapView()
{
  if (m_container != nullptr)
  {
    m_container->cycleChildMapView(this);
  }
}

void MapView::refreshViews()
{
  doRefreshViews();
}
} // namespace TrenchBroom::View
