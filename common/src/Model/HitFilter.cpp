/*
 Copyright (C) 2021 Kristian Duske

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

#include "HitFilter.h"

#include "Ensure.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"

#include <vecmath/scalar.h>

namespace TrenchBroom
{
namespace Model
{
namespace HitFilters
{
HitFilter any()
{
  return [](const Hit&) { return true; };
}

HitFilter none()
{
  return [](const Hit&) { return false; };
}

HitFilter type(const HitType::Type typeMask)
{
  return [typeMask](const Hit& hit) { return hit.hasType(typeMask); };
}

HitFilter selected()
{
  return [](const Hit& hit) {
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      return faceHandle->node()->selected() || faceHandle->face().selected();
    }
    if (const auto* node = hitToNode(hit))
    {
      return node->selected();
    }
    return false;
  };
}

HitFilter transitivelySelected()
{
  return [](const Hit& hit) {
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      return faceHandle->node()->transitivelySelected() || faceHandle->face().selected();
    }
    if (const auto* node = hitToNode(hit))
    {
      return node->transitivelySelected();
    }
    return false;
  };
}

HitFilter minDistance(const FloatType minDistance)
{
  return [minDistance](const Hit& hit) { return hit.distance() >= minDistance; };
}
} // namespace HitFilters

HitFilter operator&&(HitFilter lhs, HitFilter rhs)
{
  return [lhs = std::move(lhs), rhs = std::move(rhs)](const Hit& hit) {
    return lhs(hit) && rhs(hit);
  };
}

HitFilter operator||(HitFilter lhs, HitFilter rhs)
{
  return [lhs = std::move(lhs), rhs = std::move(rhs)](const Hit& hit) {
    return lhs(hit) || rhs(hit);
  };
}

HitFilter operator!(HitFilter filter)
{
  return [filter = std::move(filter)](const Hit& hit) { return !filter(hit); };
}
} // namespace Model
} // namespace TrenchBroom
