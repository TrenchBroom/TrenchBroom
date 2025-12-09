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

#include "Selection.h"

#include "mdl/BrushFace.h" // IWYU pragma: keep
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/SelectionChange.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include <ranges>

namespace tb::mdl
{
namespace
{

std::vector<EntityNodeBase*> computeAllEntities(
  const Selection& selection, WorldNode& worldNode)
{
  if (!selection.hasAny())
  {
    return {&worldNode};
  }

  auto result = std::vector<EntityNodeBase*>{};
  for (auto* node : selection.nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](auto&& thisLambda, GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](EntityNode* entityNode) { result.push_back(entityNode); },
      [&](BrushNode* brushNode) { result.push_back(brushNode->entity()); },
      [&](PatchNode* patchNode) { result.push_back(patchNode->entity()); }));
  }

  if (result.empty())
  {
    return {&worldNode};
  }

  result = kdl::vec_sort_and_remove_duplicates(std::move(result));
  if (result.size() > 1)
  {
    // filter out worldspawn
    result = result | std::views::filter([](const auto* entityNode) {
               return entityNode->entity().classname()
                      != EntityPropertyValues::WorldspawnClassname;
             })
             | kdl::ranges::to<std::vector>();
  }

  return result;
}

std::vector<BrushNode*> computeAllBrushes(const Selection& selection)
{
  auto result = std::vector<BrushNode*>{};

  for (auto* node : selection.nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](auto&& thisLambda, GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, EntityNode* entityNode) {
        entityNode->visitChildren(thisLambda);
      },
      [&](BrushNode* brushNode) { result.push_back(brushNode); },
      [&](PatchNode*) {}));
  }

  return result;
}

std::vector<BrushFaceHandle> computeAllBrushFaces(
  const Selection& selection, WorldNode& worldNode)
{
  if (selection.hasBrushFaces())
  {
    return selection.brushFaces;
  }

  const auto faces = collectBrushFaces(selection.nodes);
  return faceSelectionWithLinkedGroupConstraints(worldNode, faces).facesToSelect;
}

template <typename T, std::ranges::range S>
auto applyChange(std::vector<T>& s1, S&& remove, S&& add)
{
  for (const auto& x : remove)
  {
    std::erase(s1, x);
  }
  s1.insert(s1.end(), add.begin(), add.end());
}

template <typename T>
auto makeNodeFilter()
{
  return std::views::transform([](auto* n) { return dynamic_cast<T*>(n); })
         | std::views::filter([](auto* n) { return n != nullptr; });
}

} // namespace

kdl_reflect_impl(Selection);

Selection::Selection(Map& map)
  : m_map{&map}
{
}

void Selection::update(const SelectionChange& selectionChange)
{
  applyChange(nodes, selectionChange.deselectedNodes, selectionChange.selectedNodes);
  applyChange(
    groups,
    selectionChange.deselectedNodes | makeNodeFilter<GroupNode>(),
    selectionChange.selectedNodes | makeNodeFilter<GroupNode>());
  applyChange(
    entities,
    selectionChange.deselectedNodes | makeNodeFilter<EntityNode>(),
    selectionChange.selectedNodes | makeNodeFilter<EntityNode>());
  applyChange(
    brushes,
    selectionChange.deselectedNodes | makeNodeFilter<BrushNode>(),
    selectionChange.selectedNodes | makeNodeFilter<BrushNode>());
  applyChange(
    patches,
    selectionChange.deselectedNodes | makeNodeFilter<PatchNode>(),
    selectionChange.selectedNodes | makeNodeFilter<PatchNode>());

  applyChange(
    brushFaces, selectionChange.deselectedBrushFaces, selectionChange.selectedBrushFaces);

  invalidate();
}

void Selection::clear()
{
  nodes.clear();
  groups.clear();
  entities.clear();
  brushes.clear();
  patches.clear();

  invalidate();
}

void Selection::invalidate()
{
  m_cachedAllEntities = std::nullopt;
  m_cachedAllBrushes = std::nullopt;
  m_cachedAllBrushFaces = std::nullopt;
}

bool Selection::hasAny() const
{
  return hasNodes() || hasBrushFaces();
}

bool Selection::hasNodes() const
{
  return !nodes.empty();
}

bool Selection::hasGroups() const
{
  return !groups.empty();
}

bool Selection::hasOnlyGroups() const
{
  return hasNodes() && nodes.size() == groups.size();
}

bool Selection::hasEntities() const
{
  return !entities.empty();
}

bool Selection::hasOnlyEntities() const
{
  return hasNodes() && nodes.size() == entities.size();
}

bool Selection::hasBrushes() const
{
  return !brushes.empty();
}

bool Selection::hasOnlyBrushes() const
{
  return hasNodes() && nodes.size() == brushes.size();
}

bool Selection::hasPatches() const
{
  return !patches.empty();
}

bool Selection::hasOnlyPatches() const
{
  return hasNodes() && nodes.size() == patches.size();
}

bool Selection::hasBrushFaces() const
{
  return !brushFaces.empty();
}

bool Selection::hasAnyBrushFaces() const
{
  return hasBrushFaces() || hasBrushes();
}

const std::vector<EntityNodeBase*>& Selection::allEntities() const
{
  if (!m_cachedAllEntities)
  {
    m_cachedAllEntities = computeAllEntities(*this, m_map->world());
  }

  return *m_cachedAllEntities;
}

const std::vector<BrushNode*>& Selection::allBrushes() const
{
  if (!m_cachedAllBrushes)
  {
    m_cachedAllBrushes = computeAllBrushes(*this);
  }

  return *m_cachedAllBrushes;
}

const std::vector<BrushFaceHandle>& Selection::allBrushFaces() const
{
  if (!m_cachedAllBrushFaces)
  {
    m_cachedAllBrushFaces = computeAllBrushFaces(*this, m_map->world());
  }

  return *m_cachedAllBrushFaces;
}

} // namespace tb::mdl
