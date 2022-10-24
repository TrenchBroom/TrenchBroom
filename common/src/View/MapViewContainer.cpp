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

#include "MapViewContainer.h"

#include "Ensure.h"
#include "View/MapViewBase.h"

namespace TrenchBroom
{
namespace View
{
MapViewContainer::MapViewContainer(QWidget* parent)
  : QWidget(parent)
  , MapView()
{
}

MapViewContainer::~MapViewContainer() {}

bool MapViewContainer::canMaximizeCurrentView() const
{
  return doCanMaximizeCurrentView();
}

bool MapViewContainer::currentViewMaximized() const
{
  return doCurrentViewMaximized();
}

void MapViewContainer::toggleMaximizeCurrentView()
{
  doToggleMaximizeCurrentView();
}

MapView* MapViewContainer::currentMapView() const
{
  return doGetCurrentMapView();
}

vm::vec3 MapViewContainer::doGetPasteObjectsDelta(
  const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const
{
  MapView* current = currentMapView();
  ensure(current != nullptr, "current is nullptr");
  return current->pasteObjectsDelta(bounds, referenceBounds);
}
} // namespace View
} // namespace TrenchBroom
