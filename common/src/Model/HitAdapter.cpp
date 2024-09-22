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

#include "HitAdapter.h"

#include "Hit.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/PatchNode.h"

namespace TrenchBroom::Model
{

Node* hitToNode(const Hit& hit)
{
  if (hit.type() == EntityNode::EntityHitType)
  {
    return hit.target<EntityNode*>();
  }
  if (hit.type() == PatchNode::PatchHitType)
  {
    return hit.target<PatchNode*>();
  }
  if (hit.type() == BrushNode::BrushHitType)
  {
    return hit.target<BrushFaceHandle>().node();
  }
  return nullptr;
}

std::optional<BrushFaceHandle> hitToFaceHandle(const Hit& hit)
{
  if (hit.type() == BrushNode::BrushHitType)
  {
    return hit.target<BrushFaceHandle>();
  }
  return std::nullopt;
}

} // namespace TrenchBroom::Model
