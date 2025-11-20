/*
Copyright (C) 2020 Kristian Duske

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

#include "BrushFaceHandle.h"

#include "Contracts.h"
#include "mdl/Brush.h"
#include "mdl/BrushNode.h"

#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include <ranges>

namespace tb::mdl
{
BrushFaceHandle::BrushFaceHandle(BrushNode* node, const size_t faceIndex)
  : m_node{node}
  , m_faceIndex{faceIndex}
{
  contract_pre(m_node != nullptr);
  contract_pre(m_faceIndex < m_node->brush().faceCount());
}

BrushNode* BrushFaceHandle::node() const
{
  return m_node;
}

size_t BrushFaceHandle::faceIndex() const
{
  return m_faceIndex;
}

const BrushFace& BrushFaceHandle::face() const
{
  return m_node->brush().face(m_faceIndex);
}

kdl_reflect_impl(BrushFaceHandle);

std::vector<BrushNode*> toNodes(const std::vector<BrushFaceHandle>& handles)
{
  return handles | std::views::transform([](const auto& handle) { return handle.node(); })
         | kdl::ranges::to<std::vector>();
}

std::vector<BrushFaceHandle> toHandles(BrushNode* brushNode)
{
  std::vector<BrushFaceHandle> result;
  result.reserve(brushNode->brush().faceCount());
  for (size_t i = 0u; i < brushNode->brush().faceCount(); ++i)
  {
    result.emplace_back(brushNode, i);
  }
  return result;
}

} // namespace tb::mdl
