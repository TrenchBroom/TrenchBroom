/*
 Copyright (C) 2025 Kristian Duske

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

#include "NodeIndex.h"

#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/compact_trie.h"
#include "kdl/vector_utils.h"

namespace tb::mdl
{
namespace
{

template <typename F>
void withGroupNode(GroupNode& groupNode, const F& f)
{
  f(groupNode.group().name());
}

template <typename F>
void withEntityNode(EntityNodeBase& entityNode, const F& f)
{
  for (const auto& property : entityNode.entity().properties())
  {
    f(property.key());
    f(property.value());
  }
}

template <typename F>
void withBrushNode(BrushNode& brushNode, const F& f)
{
  for (const auto& face : brushNode.brush().faces())
  {
    f(face.attributes().materialName());
  }
}

template <typename F>
void withPatchNode(PatchNode& patchNode, const F& f)
{
  f(patchNode.patch().materialName());
}

} // namespace

NodeIndex::NodeIndex()
  : m_index{std::make_unique<NodeStringIndex>()}
{
}

NodeIndex::~NodeIndex() = default;

void NodeIndex::addNode(Node& node)
{
  const auto addToIndex = [&](const std::string_view key) {
    m_index->insert(key, &node);
  };

  node.accept(kdl::overload(
    [&](WorldNode* worldNode) { withEntityNode(*worldNode, addToIndex); },
    [](LayerNode*) {},
    [&](GroupNode* groupNode) { withGroupNode(*groupNode, addToIndex); },
    [&](EntityNode* entityNode) { withEntityNode(*entityNode, addToIndex); },
    [&](BrushNode* brushNode) { withBrushNode(*brushNode, addToIndex); },
    [&](PatchNode* patchNode) { withPatchNode(*patchNode, addToIndex); }));
}

void NodeIndex::removeNode(Node& node)
{
  const auto removeFromIndex = [&](const std::string_view key) {
    m_index->remove(key, &node);
  };

  node.accept(kdl::overload(
    [&](WorldNode* worldNode) { withEntityNode(*worldNode, removeFromIndex); },
    [](LayerNode*) {},
    [&](GroupNode* groupNode) { withGroupNode(*groupNode, removeFromIndex); },
    [&](EntityNode* entityNode) { withEntityNode(*entityNode, removeFromIndex); },
    [&](BrushNode* brushNode) { withBrushNode(*brushNode, removeFromIndex); },
    [&](PatchNode* patchNode) { withPatchNode(*patchNode, removeFromIndex); }));
}

void NodeIndex::clear()
{
  m_index = std::make_unique<NodeStringIndex>();
}

std::vector<Node*> NodeIndex::doFindNodes(const std::string_view pattern) const
{
  auto result = std::vector<Node*>{};
  m_index->find_matches(pattern, std::back_inserter(result));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

} // namespace tb::mdl
