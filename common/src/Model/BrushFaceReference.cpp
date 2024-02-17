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

#include "BrushFaceReference.h"

#include "Error.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"

#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include "vecmath/plane_io.h"

#include <cassert>

namespace TrenchBroom::Model
{
BrushFaceReference::BrushFaceReference(BrushNode* node, const BrushFace& face)
  : m_node{node}
  , m_facePlane{face.boundary()}
{
  assert(m_node != nullptr);
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
  return kdl::vec_transform(handles, [](const auto& handle) {
    return BrushFaceReference(handle.node(), handle.face());
  });
}

Result<std::vector<BrushFaceHandle>> resolveAllRefs(
  const std::vector<BrushFaceReference>& faceRefs)
{
  return kdl::fold_results(
    kdl::vec_transform(faceRefs, [](const auto& faceRef) { return faceRef.resolve(); }));
}
} // namespace TrenchBroom::Model
