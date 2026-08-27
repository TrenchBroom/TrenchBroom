/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "mdl/MapWorkspace.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map_Geometry.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace tb::mdl
{
namespace
{

bool sameEntityContents(const Entity& lhs, const Entity& rhs)
{
  auto lhsProperties = lhs.properties();
  auto rhsProperties = rhs.properties();
  const auto propertyKey = [](const auto& property) {
    return std::tie(property.key(), property.value());
  };
  std::ranges::sort(lhsProperties, {}, propertyKey);
  std::ranges::sort(rhsProperties, {}, propertyKey);
  auto lhsProtected = lhs.protectedProperties();
  auto rhsProtected = rhs.protectedProperties();
  std::ranges::sort(lhsProtected);
  std::ranges::sort(rhsProtected);
  return lhsProperties == rhsProperties && lhsProtected == rhsProtected;
}

bool sameBrushContents(const Brush& lhs, const Brush& rhs)
{
  if (lhs.faceCount() != rhs.faceCount())
  {
    return false;
  }
  return std::ranges::all_of(lhs.faces(), [&](const auto& lhsFace) {
    const auto rhsFaceIndex = rhs.findFace(lhsFace.boundary());
    if (!rhsFaceIndex)
    {
      return false;
    }
    const auto& rhsFace = rhs.face(*rhsFaceIndex);
    return lhsFace.materialName() == rhsFace.materialName()
           && lhsFace.surfaceAttributes() == rhsFace.surfaceAttributes()
           && lhsFace.uvAttributes() == rhsFace.uvAttributes()
           && vm::is_equal(lhsFace.uAxis(), rhsFace.uAxis(), vm::Cd::almost_zero())
           && vm::is_equal(lhsFace.vAxis(), rhsFace.vAxis(), vm::Cd::almost_zero());
  });
}

bool sameNodeContents(const Node& lhs, const Node& rhs)
{
  if (
    lhs.visibilityState() != rhs.visibilityState() || lhs.lockState() != rhs.lockState())
  {
    return false;
  }

  if (const auto* left = dynamic_cast<const WorldNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const WorldNode*>(&rhs);
    return right != nullptr && left->entity() == right->entity();
  }
  if (const auto* left = dynamic_cast<const LayerNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const LayerNode*>(&rhs);
    return right != nullptr && left->layer() == right->layer();
  }
  if (const auto* left = dynamic_cast<const GroupNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const GroupNode*>(&rhs);
    return right != nullptr && left->group() == right->group()
           && left->linkId() == right->linkId();
  }
  if (const auto* left = dynamic_cast<const EntityNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const EntityNode*>(&rhs);
    return right != nullptr && sameEntityContents(left->entity(), right->entity());
  }
  if (const auto* left = dynamic_cast<const BrushNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const BrushNode*>(&rhs);
    return right != nullptr && sameBrushContents(left->brush(), right->brush());
  }
  if (const auto* left = dynamic_cast<const PatchNode*>(&lhs))
  {
    const auto* right = dynamic_cast<const PatchNode*>(&rhs);
    return right != nullptr && left->patch() == right->patch();
  }
  return false;
}

void collectNodes(const Node& node, std::vector<const Node*>& result)
{
  result.push_back(&node);
  for (const auto* child : node.children())
  {
    collectNodes(*child, result);
  }
}

bool sameNodeType(const Node& lhs, const Node& rhs)
{
  return (dynamic_cast<const WorldNode*>(&lhs) != nullptr)
           == (dynamic_cast<const WorldNode*>(&rhs) != nullptr)
         && (dynamic_cast<const LayerNode*>(&lhs) != nullptr)
              == (dynamic_cast<const LayerNode*>(&rhs) != nullptr)
         && (dynamic_cast<const GroupNode*>(&lhs) != nullptr)
              == (dynamic_cast<const GroupNode*>(&rhs) != nullptr)
         && (dynamic_cast<const EntityNode*>(&lhs) != nullptr)
              == (dynamic_cast<const EntityNode*>(&rhs) != nullptr)
         && (dynamic_cast<const BrushNode*>(&lhs) != nullptr)
              == (dynamic_cast<const BrushNode*>(&rhs) != nullptr)
         && (dynamic_cast<const PatchNode*>(&lhs) != nullptr)
              == (dynamic_cast<const PatchNode*>(&rhs) != nullptr);
}

} // namespace

struct MapWorkspace::NodeRecord
{
  WorkspaceNodeId id;
  const Node* sourceNode;
  const Node* baseNode;
  const Node* branchNode;
  std::optional<WorkspaceNodeId> baseParentId;
};

bool WorkspaceMergePlan::canApply() const
{
  return conflicts.empty();
}

MapWorkspace::MapWorkspace(const WorldNode& sourceWorld, const vm::bbox3d worldBounds)
  : m_baseWorld{static_cast<WorldNode*>(sourceWorld.cloneRecursively(worldBounds))}
  , m_ownedBranchWorld{static_cast<WorldNode*>(sourceWorld.cloneRecursively(worldBounds))}
  , m_branchWorld{m_ownedBranchWorld.get()}
  , m_worldBounds{worldBounds}
{
  auto sourceNodes = std::vector<const Node*>{};
  auto baseNodes = std::vector<const Node*>{};
  auto branchNodes = std::vector<const Node*>{};
  collectNodes(sourceWorld, sourceNodes);
  collectNodes(*m_baseWorld, baseNodes);
  collectNodes(*m_branchWorld, branchNodes);

  // Deep clones preserve tree order, allowing us to assign identity without modifying
  // map nodes or relying on file serialization.
  m_nodes.reserve(sourceNodes.size());
  for (size_t i = 0; i < sourceNodes.size(); ++i)
  {
    const auto* sourceParent = sourceNodes[i]->parent();
    const auto parentIt = std::ranges::find(sourceNodes, sourceParent);
    const auto parentId = sourceParent == nullptr
                            ? std::optional<WorkspaceNodeId>{}
                            : std::optional<WorkspaceNodeId>{static_cast<WorkspaceNodeId>(
                                std::distance(sourceNodes.begin(), parentIt) + 1u)};
    m_nodes.push_back({i + 1u, sourceNodes[i], baseNodes[i], branchNodes[i], parentId});
  }
}

MapWorkspace::MapWorkspace(
  const WorldNode& sourceWorld, WorldNode& branchWorld, const vm::bbox3d worldBounds)
  : m_baseWorld{static_cast<WorldNode*>(sourceWorld.cloneRecursively(worldBounds))}
  , m_branchWorld{&branchWorld}
  , m_worldBounds{worldBounds}
{
  auto sourceNodes = std::vector<const Node*>{};
  auto baseNodes = std::vector<const Node*>{};
  auto branchNodes = std::vector<const Node*>{};
  collectNodes(sourceWorld, sourceNodes);
  collectNodes(*m_baseWorld, baseNodes);
  collectNodes(branchWorld, branchNodes);

  if (
    sourceNodes.size() != branchNodes.size()
    || !std::ranges::equal(
      sourceNodes, branchNodes, [](const auto* source, const auto* branch) {
        return sameNodeType(*source, *branch);
      }))
  {
    throw std::invalid_argument{
      "The branch world must have the source world's preorder node structure when "
      "attached"};
  }

  // Deep clones preserve tree order, allowing us to assign identity without modifying
  // map nodes or relying on file serialization.
  m_nodes.reserve(sourceNodes.size());
  for (size_t i = 0; i < sourceNodes.size(); ++i)
  {
    const auto* sourceParent = sourceNodes[i]->parent();
    const auto parentIt = std::ranges::find(sourceNodes, sourceParent);
    const auto parentId = sourceParent == nullptr
                            ? std::optional<WorkspaceNodeId>{}
                            : std::optional<WorkspaceNodeId>{static_cast<WorkspaceNodeId>(
                                std::distance(sourceNodes.begin(), parentIt) + 1u)};
    m_nodes.push_back({i + 1u, sourceNodes[i], baseNodes[i], branchNodes[i], parentId});
  }
}

MapWorkspace::~MapWorkspace() = default;
MapWorkspace::MapWorkspace(MapWorkspace&&) noexcept = default;
MapWorkspace& MapWorkspace::operator=(MapWorkspace&&) noexcept = default;

const WorldNode& MapWorkspace::baseWorld() const
{
  return *m_baseWorld;
}

WorldNode& MapWorkspace::branchWorld()
{
  return *m_branchWorld;
}

const WorldNode& MapWorkspace::branchWorld() const
{
  return *m_branchWorld;
}

std::optional<WorkspaceNodeId> MapWorkspace::nodeId(const Node& branchNode) const
{
  const auto it = std::ranges::find(m_nodes, &branchNode, &NodeRecord::branchNode);
  if (it == m_nodes.end())
  {
    return std::nullopt;
  }
  return it->id;
}

const Node* MapWorkspace::baseNode(const WorkspaceNodeId nodeId) const
{
  const auto it = std::ranges::find(m_nodes, nodeId, &NodeRecord::id);
  return it == m_nodes.end() ? nullptr : it->baseNode;
}

const Node* MapWorkspace::sourceNode(const WorkspaceNodeId nodeId) const
{
  const auto it = std::ranges::find(m_nodes, nodeId, &NodeRecord::id);
  return it == m_nodes.end() ? nullptr : it->sourceNode;
}

std::vector<WorkspaceChange> MapWorkspace::rawChanges() const
{
  auto result = std::vector<WorkspaceChange>{};
  auto branchNodes = std::vector<const Node*>{};
  collectNodes(*m_branchWorld, branchNodes);
  const auto branchSet =
    std::unordered_set<const Node*>{branchNodes.begin(), branchNodes.end()};

  for (const auto& record : m_nodes)
  {
    if (!branchSet.contains(record.branchNode))
    {
      const auto parentIsAlsoRemoved =
        record.baseParentId
        && !branchSet.contains(m_nodes[*record.baseParentId - 1u].branchNode);
      if (!parentIsAlsoRemoved)
      {
        result.push_back(
          {WorkspaceChangeKind::Removed,
           record.id,
           record.baseNode,
           nullptr,
           record.baseParentId,
           {}});
      }
      continue;
    }

    if (!sameNodeContents(*record.baseNode, *record.branchNode))
    {
      result.push_back(
        {WorkspaceChangeKind::Changed,
         record.id,
         record.baseNode,
         record.branchNode,
         record.baseParentId,
         {}});
    }

    const auto branchParentId = record.branchNode->parent() == nullptr
                                  ? std::optional<WorkspaceNodeId>{}
                                  : nodeId(*record.branchNode->parent());
    if (branchParentId != record.baseParentId)
    {
      result.push_back(
        {WorkspaceChangeKind::Reparented,
         record.id,
         record.baseNode,
         record.branchNode,
         record.baseParentId,
         branchParentId});
    }
  }

  for (const auto* branchNode : branchNodes)
  {
    if (!nodeId(*branchNode))
    {
      const auto parentId = branchNode->parent() == nullptr
                              ? std::optional<WorkspaceNodeId>{}
                              : nodeId(*branchNode->parent());
      if (parentId)
      {
        result.push_back(
          {WorkspaceChangeKind::Added, 0, nullptr, branchNode, {}, parentId});
      }
    }
  }
  return result;
}

std::vector<WorkspaceChange> MapWorkspace::changes() const
{
  const auto raw = rawChanges();
  struct BrushReplacement
  {
    WorkspaceNodeId parentId;
    std::vector<size_t> rawChangeIndices;
    std::vector<const BrushNode*> baseBrushes;
    std::vector<const BrushNode*> branchBrushes;
  };

  auto replacements = std::vector<BrushReplacement>{};
  const auto replacementFor = [&](const WorkspaceNodeId parentId) -> BrushReplacement& {
    const auto it =
      std::ranges::find(replacements, parentId, &BrushReplacement::parentId);
    if (it != replacements.end())
    {
      return *it;
    }
    replacements.push_back({parentId, {}, {}, {}});
    return replacements.back();
  };

  for (size_t index = 0u; index < raw.size(); ++index)
  {
    const auto& change = raw[index];
    if (
      change.kind == WorkspaceChangeKind::Removed && change.baseParentId
      && dynamic_cast<const BrushNode*>(change.baseNode) != nullptr)
    {
      auto& replacement = replacementFor(*change.baseParentId);
      replacement.rawChangeIndices.push_back(index);
      replacement.baseBrushes.push_back(static_cast<const BrushNode*>(change.baseNode));
    }
    else if (
      change.kind == WorkspaceChangeKind::Added && change.branchParentId
      && dynamic_cast<const BrushNode*>(change.branchNode) != nullptr)
    {
      auto& replacement = replacementFor(*change.branchParentId);
      replacement.rawChangeIndices.push_back(index);
      replacement.branchBrushes.push_back(
        static_cast<const BrushNode*>(change.branchNode));
    }
  }

  auto replacementByFirstChange = std::unordered_map<size_t, WorkspaceChange>{};
  auto summarizedRawChanges = std::unordered_set<size_t>{};
  for (const auto& replacement : replacements)
  {
    if (
      replacement.baseBrushes.empty() || replacement.branchBrushes.empty()
      || !isBrushOptimizationResult(
        m_baseWorld->mapFormat(),
        m_worldBounds,
        replacement.baseBrushes,
        replacement.branchBrushes))
    {
      continue;
    }

    const auto firstChange = *std::ranges::min_element(replacement.rawChangeIndices);
    replacementByFirstChange.emplace(
      firstChange,
      WorkspaceChange{
        WorkspaceChangeKind::BrushesOptimized,
        0u,
        baseNode(replacement.parentId),
        m_nodes[replacement.parentId - 1u].branchNode,
        replacement.parentId,
        replacement.parentId,
        replacement.baseBrushes.size(),
        replacement.branchBrushes.size()});
    summarizedRawChanges.insert(
      replacement.rawChangeIndices.begin(), replacement.rawChangeIndices.end());
  }

  auto result = std::vector<WorkspaceChange>{};
  result.reserve(raw.size());
  for (size_t index = 0u; index < raw.size(); ++index)
  {
    if (const auto it = replacementByFirstChange.find(index);
        it != replacementByFirstChange.end())
    {
      result.push_back(it->second);
    }
    if (!summarizedRawChanges.contains(index))
    {
      result.push_back(raw[index]);
    }
  }
  return result;
}

WorkspaceMergePlan MapWorkspace::planMerge(const WorldNode& liveWorld) const
{
  auto plan = WorkspaceMergePlan{};
  auto liveNodes = std::vector<const Node*>{};
  collectNodes(liveWorld, liveNodes);
  const auto liveSet =
    std::unordered_set<const Node*>{liveNodes.begin(), liveNodes.end()};
  const auto changesToMerge = rawChanges();
  auto removedIds = std::unordered_set<WorkspaceNodeId>{};
  for (const auto& change : changesToMerge)
  {
    if (change.kind == WorkspaceChangeKind::Removed)
    {
      removedIds.insert(change.nodeId);
    }
  }
  const auto sourceToId = [&] {
    auto result = std::unordered_map<const Node*, WorkspaceNodeId>{};
    for (const auto& record : m_nodes)
    {
      result.emplace(record.sourceNode, record.id);
    }
    return result;
  }();

  const auto liveParentId = [&](const Node& node) -> std::optional<WorkspaceNodeId> {
    if (const auto* parent = node.parent())
    {
      if (const auto it = sourceToId.find(parent); it != sourceToId.end())
      {
        return it->second;
      }
    }
    return {};
  };

  // A single remove operation deletes a whole subtree. Although changes() reports only
  // its root, do not silently discard a concurrent live edit to a descendant.
  for (const auto& change : changesToMerge)
  {
    if (change.kind != WorkspaceChangeKind::Removed)
    {
      continue;
    }
    for (const auto& record : m_nodes)
    {
      if (
        record.id == change.nodeId || !change.baseNode->isAncestorOf(*record.baseNode)
        || !liveSet.contains(record.sourceNode))
      {
        continue;
      }
      if (!sameNodeContents(*record.baseNode, *record.sourceNode))
      {
        plan.conflicts.push_back(
          {WorkspaceMergeConflictKind::LiveNodeChanged, record.id});
      }
      else if (liveParentId(*record.sourceNode) != record.baseParentId)
      {
        plan.conflicts.push_back(
          {WorkspaceMergeConflictKind::LiveParentChanged, record.id});
      }
    }
  }

  for (const auto& change : changesToMerge)
  {
    if (change.kind == WorkspaceChangeKind::Added)
    {
      // Adding a subtree is one recursive clone operation. Its newly-created children
      // are therefore represented by the root addition rather than duplicate commands.
      if (change.branchNode->parent() && !nodeId(*change.branchNode->parent()))
      {
        continue;
      }
      if (
        change.branchParentId
        && !liveSet.contains(m_nodes[*change.branchParentId - 1u].sourceNode))
      {
        plan.conflicts.push_back({WorkspaceMergeConflictKind::MissingLiveParent, 0});
      }
      else
      {
        plan.operations.push_back(
          {WorkspaceMergeOperationKind::Add,
           0,
           change.branchNode,
           change.branchParentId});
      }
      continue;
    }

    const auto& record = m_nodes[change.nodeId - 1u];
    if (!liveSet.contains(record.sourceNode))
    {
      if (change.kind != WorkspaceChangeKind::Removed)
      {
        plan.conflicts.push_back(
          {WorkspaceMergeConflictKind::MissingLiveNode, change.nodeId});
      }
      continue;
    }

    if (
      (change.kind == WorkspaceChangeKind::Changed
       || change.kind == WorkspaceChangeKind::Removed)
      && !sameNodeContents(*record.baseNode, *record.sourceNode))
    {
      plan.conflicts.push_back(
        {WorkspaceMergeConflictKind::LiveNodeChanged, change.nodeId});
      continue;
    }
    if (
      change.kind == WorkspaceChangeKind::Reparented
      && liveParentId(*record.sourceNode) != record.baseParentId)
    {
      plan.conflicts.push_back(
        {WorkspaceMergeConflictKind::LiveParentChanged, change.nodeId});
      continue;
    }
    if (
      change.kind == WorkspaceChangeKind::Reparented && change.branchParentId
      && !liveSet.contains(m_nodes[*change.branchParentId - 1u].sourceNode))
    {
      plan.conflicts.push_back(
        {WorkspaceMergeConflictKind::MissingLiveParent, change.nodeId});
      continue;
    }
    if (change.kind == WorkspaceChangeKind::Reparented && !change.branchParentId)
    {
      // The merge protocol identifies live parents by fork-time identities. A branch-only
      // parent has no live counterpart, and cloning it would duplicate this existing
      // node.
      plan.conflicts.push_back(
        {WorkspaceMergeConflictKind::UnsupportedBranchParent, change.nodeId});
      continue;
    }

    switch (change.kind)
    {
    case WorkspaceChangeKind::Removed:
      // Deleting an ancestor removes its entire subtree, so the merge adapter must
      // receive only the roots of deleted subtrees.
      if (!change.baseParentId || !removedIds.contains(*change.baseParentId))
      {
        plan.operations.push_back(
          {WorkspaceMergeOperationKind::Remove, change.nodeId, nullptr, {}});
      }
      break;
    case WorkspaceChangeKind::Changed:
      plan.operations.push_back(
        {WorkspaceMergeOperationKind::Replace, change.nodeId, change.branchNode, {}});
      break;
    case WorkspaceChangeKind::Reparented:
      plan.operations.push_back(
        {WorkspaceMergeOperationKind::Reparent,
         change.nodeId,
         change.branchNode,
         change.branchParentId});
      break;
    case WorkspaceChangeKind::Added:
    case WorkspaceChangeKind::BrushesOptimized:
      // Merge planning consumes rawChanges(), which never emits summary changes.
      break;
    }
  }
  const auto operationPhase = [](const WorkspaceMergeOperationKind kind) {
    switch (kind)
    {
    case WorkspaceMergeOperationKind::Add:
      return 0;
    case WorkspaceMergeOperationKind::Replace:
    case WorkspaceMergeOperationKind::Reparent:
      return 1;
    case WorkspaceMergeOperationKind::Remove:
      return 2;
    }
    return 0;
  };
  std::stable_sort(
    plan.operations.begin(),
    plan.operations.end(),
    [&](const auto& lhs, const auto& rhs) {
      const auto lhsPhase = operationPhase(lhs.kind);
      const auto rhsPhase = operationPhase(rhs.kind);
      if (lhsPhase != rhsPhase)
      {
        return lhsPhase < rhsPhase;
      }
      if (lhs.kind == WorkspaceMergeOperationKind::Add)
      {
        return lhs.branchNode->depth() < rhs.branchNode->depth();
      }
      if (lhs.kind == WorkspaceMergeOperationKind::Remove)
      {
        return baseNode(lhs.nodeId)->depth() > baseNode(rhs.nodeId)->depth();
      }
      return lhs.nodeId < rhs.nodeId
             || (lhs.nodeId == rhs.nodeId
                 && static_cast<int>(lhs.kind) < static_cast<int>(rhs.kind));
    });
  return plan;
}

} // namespace tb::mdl
