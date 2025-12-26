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

#include "mdl/BrushFaceReference.h"

#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/result_fold.h"

#include <ranges>

namespace tb::mdl
{

BrushFaceReference::BrushFaceReference(BrushNode* node, const BrushFace& face)
  : m_node{node}
  , m_facePlane{face.boundary()}
{
  contract_pre(m_node != nullptr);
}

Result<BrushFaceHandle> BrushFaceReference::resolve() const
{
  if (const auto faceIndex = m_node->brush().findFace(m_facePlane))
  {
    return BrushFaceHandle(m_node, *faceIndex);
  }
  return Error{"Cannot resolve brush face reference"};
}

std::vector<BrushFaceReference> createRefs(const std::vector<BrushFaceHandle>& handles)
{
  return handles | std::views::transform([](const auto& handle) {
           return BrushFaceReference{handle.node(), handle.face()};
         })
         | kdl::ranges::to<std::vector>();
}

Result<std::vector<BrushFaceHandle>> resolveAllRefs(
  const std::vector<BrushFaceReference>& faceRefs)
{
  return faceRefs
         | std::views::transform([](const auto& faceRef) { return faceRef.resolve(); })
         | kdl::ranges::to<std::vector>() | kdl::fold;
}
} // namespace tb::mdl
