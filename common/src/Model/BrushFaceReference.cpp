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

#include "Exceptions.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"

#include <kdl/vector_utils.h>

#include <cassert>

namespace TrenchBroom {
namespace Model {
BrushFaceReference::BrushFaceReference(BrushNode* node, const BrushFace& face)
  : m_node(node)
  , m_facePlane(face.boundary()) {
  assert(m_node != nullptr);
}

BrushFaceHandle BrushFaceReference::resolve() const {
  if (const auto faceIndex = m_node->brush().findFace(m_facePlane)) {
    return BrushFaceHandle(m_node, *faceIndex);
  } else {
    throw BrushFaceReferenceException();
  }
}

std::vector<BrushFaceReference> createRefs(const std::vector<BrushFaceHandle>& handles) {
  return kdl::vec_transform(handles, [](const auto& handle) {
    return BrushFaceReference(handle.node(), handle.face());
  });
}

std::vector<BrushFaceHandle> resolveAllRefs(const std::vector<BrushFaceReference>& faceRefs) {
  return kdl::vec_transform(faceRefs, [](const auto& faceRef) {
    return faceRef.resolve();
  });
}
} // namespace Model
} // namespace TrenchBroom
