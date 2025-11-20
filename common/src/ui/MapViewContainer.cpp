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

#include "MapViewContainer.h"

#include "Contracts.h"

namespace tb::ui
{
MapViewContainer::MapViewContainer(QWidget* parent)
  : QWidget{parent}
{
}

MapViewContainer::~MapViewContainer() = default;

vm::vec3d MapViewContainer::pasteObjectsDelta(
  const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const
{
  auto* current = currentMapView();
  contract_assert(current != nullptr);

  return current->pasteObjectsDelta(bounds, referenceBounds);
}

} // namespace tb::ui
