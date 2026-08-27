/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "vm/bbox.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace tb::mdl
{

class Node;
class WorldNode;

/**
 * A stable, branch-local identity for a node that existed when a workspace was forked.
 *
 * The identity deliberately is not stored in a map node. Map files and linked group IDs
 * retain their existing semantics; a workspace identity only lives as long as its branch.
 */
using WorkspaceNodeId = std::size_t;

enum class WorkspaceChangeKind
{
  Added,
  Removed,
  Changed,
  Reparented,
  /** A set of sibling brushes was replaced by an exact lower-count decomposition. */
  BrushesOptimized,
};

struct WorkspaceChange
{
  WorkspaceChangeKind kind;
  WorkspaceNodeId nodeId = 0;
  const Node* baseNode = nullptr;
  const Node* branchNode = nullptr;
  std::optional<WorkspaceNodeId> baseParentId;
  std::optional<WorkspaceNodeId> branchParentId;
  /** Populated for BrushesOptimized. */
  size_t baseBrushCount = 0u;
  /** Populated for BrushesOptimized. */
  size_t branchBrushCount = 0u;
};

enum class WorkspaceMergeOperationKind
{
  Add,
  Remove,
  Replace,
  Reparent,
};

struct WorkspaceMergeOperation
{
  WorkspaceMergeOperationKind kind;
  WorkspaceNodeId nodeId = 0;
  /** For Add, this is the root of a recursively cloned branch subtree. */
  const Node* branchNode = nullptr;
  std::optional<WorkspaceNodeId> parentId;
};

enum class WorkspaceMergeConflictKind
{
  /** The node that was present at fork time no longer occurs in the live tree. */
  MissingLiveNode,
  /** Both the live map and the workspace changed the node's own contents. */
  LiveNodeChanged,
  /** Both the live map and the workspace moved the node. */
  LiveParentChanged,
  /** A new branch node's intended live parent no longer occurs in the live tree. */
  MissingLiveParent,
  /** An existing node was moved below a parent that only exists in the branch. */
  UnsupportedBranchParent,
};

struct WorkspaceMergeConflict
{
  WorkspaceMergeConflictKind kind;
  WorkspaceNodeId nodeId;
};

struct WorkspaceMergePlan
{
  std::vector<WorkspaceMergeOperation> operations;
  std::vector<WorkspaceMergeConflict> conflicts;

  bool canApply() const;
};

/**
 * A private workspace for a branch of a world node.
 *
 * The base is a deep Node clone. A workspace can either own a convenience in-memory
 * branch clone, or attach to an externally owned WorldNode (for example, the WorldNode
 * of a separate MapWindow). The original source-node addresses are used only as
 * in-memory identities while computing a plan against the same live world; they are
 * never persisted or exposed in map data.
 */
class MapWorkspace
{
private:
  struct NodeRecord;

  std::unique_ptr<WorldNode> m_baseWorld;
  std::unique_ptr<WorldNode> m_ownedBranchWorld;
  WorldNode* m_branchWorld;
  vm::bbox3d m_worldBounds;
  std::vector<NodeRecord> m_nodes;

  std::vector<WorkspaceChange> rawChanges() const;

public:
  /** Creates a self-contained in-memory branch clone. */
  MapWorkspace(const WorldNode& sourceWorld, vm::bbox3d worldBounds);
  /**
   * Attaches an externally owned branch whose initial preorder node structure must match
   * sourceWorld. The caller must keep branchWorld alive and must not replace its root for
   * the lifetime of this workspace.
   */
  MapWorkspace(
    const WorldNode& sourceWorld, WorldNode& branchWorld, vm::bbox3d worldBounds);
  ~MapWorkspace();

  MapWorkspace(MapWorkspace&&) noexcept;
  MapWorkspace& operator=(MapWorkspace&&) noexcept;

  MapWorkspace(const MapWorkspace&) = delete;
  MapWorkspace& operator=(const MapWorkspace&) = delete;

  const WorldNode& baseWorld() const;
  WorldNode& branchWorld();
  const WorldNode& branchWorld() const;

  /** Returns a branch-local identity, or nullopt for a node added after the fork. */
  std::optional<WorkspaceNodeId> nodeId(const Node& branchNode) const;
  const Node* baseNode(WorkspaceNodeId nodeId) const;
  const Node* sourceNode(WorkspaceNodeId nodeId) const;

  std::vector<WorkspaceChange> changes() const;

  /**
   * Compares this branch to the supplied current live world and creates a plan which a
   * UI-layer adapter can apply in one Map transaction. This class never mutates
   * liveWorld.
   */
  WorkspaceMergePlan planMerge(const WorldNode& liveWorld) const;
};

} // namespace tb::mdl
