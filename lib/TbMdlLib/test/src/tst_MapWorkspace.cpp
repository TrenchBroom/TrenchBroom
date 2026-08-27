/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/MapFormat.h"
#include "mdl/MapWorkspace.h"
#include "mdl/WorldNode.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

constexpr auto WorldBounds = vm::bbox3d{8192.0};

struct SourceTree
{
  WorldNode world{{}, {}, MapFormat::Standard};
  GroupNode* firstGroup = new GroupNode{Group{"first"}};
  GroupNode* secondGroup = new GroupNode{Group{"second"}};
  GroupNode* brushGroup = new GroupNode{Group{"brushes"}};
  EntityNode* entity = new EntityNode{Entity{{{"classname", "light"}}}};
  BrushNode* firstBrush = nullptr;
  BrushNode* secondBrush = nullptr;

  SourceTree()
  {
    firstGroup->addChild(entity);
    const auto builder = BrushBuilder{world.mapFormat(), WorldBounds};
    firstBrush = new BrushNode{
      builder.createCuboid(vm::bbox3d{{0, 0, 0}, {32, 64, 64}}, "material")
      | kdl::value()};
    secondBrush = new BrushNode{
      builder.createCuboid(vm::bbox3d{{32, 0, 0}, {64, 64, 64}}, "material")
      | kdl::value()};
    brushGroup->addChildren({firstBrush, secondBrush});
    world.defaultLayer()->addChildren({firstGroup, secondGroup, brushGroup});
  }
};

EntityNode& branchEntity(MapWorkspace& workspace)
{
  auto* firstGroup =
    static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[0]);
  return *static_cast<EntityNode*>(firstGroup->children()[0]);
}

std::unique_ptr<WorldNode> cloneWorld(const WorldNode& world)
{
  return std::unique_ptr<WorldNode>{
    static_cast<WorldNode*>(world.cloneRecursively(WorldBounds))};
}

void setProperty(EntityNode& node, const std::string& key, const std::string& value)
{
  auto entity = node.entity();
  entity.addOrUpdateProperty(key, value);
  node.setEntity(std::move(entity));
}

} // namespace

TEST_CASE("MapWorkspace")
{
  SECTION("can attach to the externally owned world of a separate branch document")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    const auto workspace = MapWorkspace{source.world, *branch, WorldBounds};

    const auto& entity = static_cast<const EntityNode&>(
      *static_cast<const GroupNode*>(
         workspace.branchWorld().defaultLayer()->children()[0])
         ->children()[0]);
    const auto id = workspace.nodeId(entity);

    REQUIRE(id);
    CHECK(&workspace.branchWorld() == branch.get());
    CHECK(workspace.baseNode(*id) != &entity);
    CHECK(workspace.changes().empty());
    CHECK(workspace.planMerge(source.world).canApply());
  }

  SECTION("reconstructs a detached branch from exported durable identities")
  {
    auto source = SourceTree{};
    auto base = cloneWorld(source.world);
    auto branch = cloneWorld(source.world);
    auto original = MapWorkspace{source.world, *branch, WorldBounds};
    auto& entity = branchEntity(original);
    const auto entityId = *original.nodeId(entity);
    setProperty(entity, "style", "2");

    const auto identities = original.exportNodeIdentities();
    auto restored = MapWorkspace{*base, *branch, WorldBounds, identities};

    CHECK_FALSE(restored.hasAttachedSource());
    CHECK(restored.sourceNode(entityId) == nullptr);
    const auto detachedPlan = restored.planMerge(source.world);
    REQUIRE_FALSE(detachedPlan.canApply());
    REQUIRE(detachedPlan.conflicts.size() == 1u);
    CHECK(
      detachedPlan.conflicts.front().kind
      == WorkspaceMergeConflictKind::UnattachedLiveSource);

    restored.attachSource(source.world);
    CHECK(restored.hasAttachedSource());
    CHECK(restored.sourceNode(entityId) == source.entity);
    const auto plan = restored.planMerge(source.world);
    REQUIRE(plan.canApply());
    REQUIRE(plan.operations.size() == 1u);
    CHECK(plan.operations.front().kind == WorkspaceMergeOperationKind::Replace);
    CHECK(plan.operations.front().nodeId == entityId);
  }

  SECTION("retains reparenting and branch-only descendants across reconstruction")
  {
    auto source = SourceTree{};
    auto base = cloneWorld(source.world);
    auto branch = cloneWorld(source.world);
    auto original = MapWorkspace{source.world, *branch, WorldBounds};
    auto* firstGroup = static_cast<GroupNode*>(branch->defaultLayer()->children()[0]);
    auto* secondGroup = static_cast<GroupNode*>(branch->defaultLayer()->children()[1]);
    auto* entity = firstGroup->children().front();
    const auto entityId = *original.nodeId(*entity);
    firstGroup->removeChild(entity);
    secondGroup->addChild(entity);
    secondGroup->addChild(new EntityNode{Entity{{{"classname", "info_player_start"}}}});

    auto restored =
      MapWorkspace{*base, *branch, WorldBounds, original.exportNodeIdentities()};
    restored.attachSource(source.world);
    const auto plan = restored.planMerge(source.world);

    REQUIRE(plan.canApply());
    CHECK(
      std::count_if(
        plan.operations.begin(),
        plan.operations.end(),
        [&](const auto& operation) {
          return operation.kind == WorkspaceMergeOperationKind::Reparent
                 && operation.nodeId == entityId;
        })
      == 1u);
    CHECK(
      std::count_if(
        plan.operations.begin(),
        plan.operations.end(),
        [](const auto& operation) {
          return operation.kind == WorkspaceMergeOperationKind::Add;
        })
      == 1u);
  }

  SECTION("validates imported identity tables and refuses unsafe source topology")
  {
    auto source = SourceTree{};
    auto base = cloneWorld(source.world);
    auto branch = cloneWorld(source.world);
    const auto original = MapWorkspace{source.world, *branch, WorldBounds};
    const auto identities = original.exportNodeIdentities();

    auto duplicateIds = identities;
    duplicateIds.push_back(duplicateIds.back());
    CHECK_THROWS_AS(
      (MapWorkspace{*base, *branch, WorldBounds, std::move(duplicateIds)}),
      std::invalid_argument);

    auto staleBranchPath = identities;
    staleBranchPath.back().branchPath = WorkspaceNodePath{999u};
    CHECK_THROWS_AS(
      (MapWorkspace{*base, *branch, WorldBounds, std::move(staleBranchPath)}),
      std::invalid_argument);

    auto restored = MapWorkspace{*base, *branch, WorldBounds, identities};
    auto changedSource = cloneWorld(source.world);
    auto* firstGroup =
      static_cast<GroupNode*>(changedSource->defaultLayer()->children()[0]);
    auto* secondGroup =
      static_cast<GroupNode*>(changedSource->defaultLayer()->children()[1]);
    auto* entity = firstGroup->children().front();
    firstGroup->removeChild(entity);
    secondGroup->addChild(entity);

    CHECK_THROWS_AS(restored.attachSource(*changedSource), std::invalid_argument);
    CHECK_FALSE(restored.hasAttachedSource());
  }

  SECTION("allows independent source-only contents edits after explicit attachment")
  {
    auto source = SourceTree{};
    auto base = cloneWorld(source.world);
    auto branch = cloneWorld(source.world);
    const auto original = MapWorkspace{source.world, *branch, WorldBounds};
    auto restored =
      MapWorkspace{*base, *branch, WorldBounds, original.exportNodeIdentities()};
    auto secondGroup = source.secondGroup->group();
    secondGroup.setName("source-only");
    source.secondGroup->setGroup(std::move(secondGroup));

    restored.attachSource(source.world);
    const auto plan = restored.planMerge(source.world);
    CHECK(plan.canApply());
    CHECK(plan.operations.empty());
    CHECK(restored.planMerge(source.world).operations.empty());
  }

  SECTION("retains three-way conflict behavior after reconstruction")
  {
    SECTION("merges compatible independent branch and source edits repeatedly")
    {
      auto source = SourceTree{};
      auto base = cloneWorld(source.world);
      auto branch = cloneWorld(source.world);
      auto original = MapWorkspace{source.world, *branch, WorldBounds};
      auto& entity = branchEntity(original);
      const auto entityId = *original.nodeId(entity);
      setProperty(entity, "style", "2");

      auto secondGroup = source.secondGroup->group();
      secondGroup.setName("source-only");
      source.secondGroup->setGroup(std::move(secondGroup));

      auto restored =
        MapWorkspace{*base, *branch, WorldBounds, original.exportNodeIdentities()};
      restored.attachSource(source.world);
      const auto firstPlan = restored.planMerge(source.world);
      const auto secondPlan = restored.planMerge(source.world);

      REQUIRE(firstPlan.canApply());
      REQUIRE(secondPlan.canApply());
      REQUIRE(firstPlan.operations.size() == 1u);
      REQUIRE(secondPlan.operations.size() == firstPlan.operations.size());
      CHECK(firstPlan.operations.front().kind == WorkspaceMergeOperationKind::Replace);
      CHECK(firstPlan.operations.front().nodeId == entityId);
      CHECK(secondPlan.operations.front().kind == firstPlan.operations.front().kind);
      CHECK(secondPlan.operations.front().nodeId == firstPlan.operations.front().nodeId);
    }

    SECTION("detects divergent contents edits")
    {
      auto source = SourceTree{};
      auto base = cloneWorld(source.world);
      auto branch = cloneWorld(source.world);
      auto original = MapWorkspace{source.world, *branch, WorldBounds};
      auto& entity = branchEntity(original);
      const auto entityId = *original.nodeId(entity);
      setProperty(entity, "style", "2");
      setProperty(*source.entity, "style", "3");

      auto restored =
        MapWorkspace{*base, *branch, WorldBounds, original.exportNodeIdentities()};
      restored.attachSource(source.world);
      const auto plan = restored.planMerge(source.world);

      REQUIRE_FALSE(plan.canApply());
      REQUIRE(plan.conflicts.size() == 1u);
      CHECK(plan.conflicts.front().kind == WorkspaceMergeConflictKind::LiveNodeChanged);
      CHECK(plan.conflicts.front().nodeId == entityId);
    }

    SECTION("detects a live edit when the branch deleted its node")
    {
      auto source = SourceTree{};
      auto base = cloneWorld(source.world);
      auto branch = cloneWorld(source.world);
      auto original = MapWorkspace{source.world, *branch, WorldBounds};
      auto* firstGroup = static_cast<GroupNode*>(branch->defaultLayer()->children()[0]);
      auto* entity = firstGroup->children().front();
      const auto entityId = *original.nodeId(*entity);
      firstGroup->removeChild(entity);
      const auto removedEntity = std::unique_ptr<Node>{entity};
      setProperty(*source.entity, "style", "3");

      auto restored =
        MapWorkspace{*base, *branch, WorldBounds, original.exportNodeIdentities()};
      restored.attachSource(source.world);
      const auto plan = restored.planMerge(source.world);

      REQUIRE(removedEntity);
      REQUIRE_FALSE(plan.canApply());
      CHECK(
        std::count_if(
          plan.conflicts.begin(),
          plan.conflicts.end(),
          [&](const auto& conflict) {
            return conflict.kind == WorkspaceMergeConflictKind::LiveNodeChanged
                   && conflict.nodeId == entityId;
          })
        == 1u);
    }
  }

  SECTION("reports direct content, reparenting, and an added subtree root")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto& entity = branchEntity(workspace);
    const auto entityId = *workspace.nodeId(entity);
    auto* firstGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[0]);
    auto* secondGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[1]);

    setProperty(entity, "style", "2");
    firstGroup->removeChild(&entity);
    secondGroup->addChild(&entity);
    auto* addedGroup = new GroupNode{Group{"new group"}};
    addedGroup->addChild(new EntityNode{Entity{{{"classname", "info_player_start"}}}});
    secondGroup->addChild(addedGroup);

    const auto changes = workspace.changes();
    CHECK(
      std::count_if(
        changes.begin(),
        changes.end(),
        [&](const auto& change) {
          return change.kind == WorkspaceChangeKind::Changed && change.nodeId == entityId;
        })
      == 1u);
    CHECK(
      std::count_if(
        changes.begin(),
        changes.end(),
        [&](const auto& change) {
          return change.kind == WorkspaceChangeKind::Reparented
                 && change.nodeId == entityId;
        })
      == 1u);
    CHECK(
      std::count_if(
        changes.begin(),
        changes.end(),
        [](const auto& change) { return change.kind == WorkspaceChangeKind::Added; })
      == 1u);
    const auto plan = workspace.planMerge(source.world);
    REQUIRE(plan.canApply());
    CHECK(
      std::count_if(
        plan.operations.begin(),
        plan.operations.end(),
        [](const auto& operation) {
          return operation.kind == WorkspaceMergeOperationKind::Add;
        })
      == 1u);

    secondGroup->removeChild(addedGroup);
    delete addedGroup;
    const auto changesAfterRemoval = workspace.changes();
    CHECK(
      std::count_if(
        changesAfterRemoval.begin(),
        changesAfterRemoval.end(),
        [](const auto& change) { return change.kind == WorkspaceChangeKind::Added; })
      == 0u);
  }

  SECTION("plans a clean merge and detects concurrent live changes")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto& entity = branchEntity(workspace);
    const auto entityId = *workspace.nodeId(entity);
    setProperty(entity, "style", "2");

    auto plan = workspace.planMerge(source.world);
    REQUIRE(plan.canApply());
    REQUIRE(plan.operations.size() == 1u);
    CHECK(plan.operations.front().kind == WorkspaceMergeOperationKind::Replace);
    CHECK(plan.operations.front().nodeId == entityId);

    setProperty(*source.entity, "style", "3");
    plan = workspace.planMerge(source.world);
    REQUIRE_FALSE(plan.canApply());
    REQUIRE(plan.conflicts.size() == 1u);
    CHECK(plan.conflicts.front().kind == WorkspaceMergeConflictKind::LiveNodeChanged);
    CHECK(plan.conflicts.front().nodeId == entityId);
  }

  SECTION("reports a removed subtree only at its root")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* layer = workspace.branchWorld().defaultLayer();
    auto* removedGroup = layer->children().front();
    const auto groupId = *workspace.nodeId(*removedGroup);
    const auto entityId = *workspace.nodeId(*removedGroup->children().front());

    layer->removeChild(removedGroup);
    const auto removedSubtree = std::unique_ptr<Node>{removedGroup};
    REQUIRE(removedSubtree);
    const auto changes = workspace.changes();
    REQUIRE(changes.size() == 1u);
    CHECK(changes.front().kind == WorkspaceChangeKind::Removed);
    CHECK(changes.front().nodeId == groupId);
    CHECK(changes.front().nodeId != entityId);

    const auto plan = workspace.planMerge(source.world);
    REQUIRE(plan.canApply());
    REQUIRE(plan.operations.size() == 1u);
    CHECK(plan.operations.front().kind == WorkspaceMergeOperationKind::Remove);
    CHECK(plan.operations.front().nodeId == groupId);

    setProperty(*source.entity, "style", "3");
    const auto conflictingPlan = workspace.planMerge(source.world);
    REQUIRE_FALSE(conflictingPlan.canApply());
    CHECK(
      std::count_if(
        conflictingPlan.conflicts.begin(),
        conflictingPlan.conflicts.end(),
        [&](const auto& conflict) {
          return conflict.kind == WorkspaceMergeConflictKind::LiveNodeChanged
                 && conflict.nodeId == entityId;
        })
      == 1u);
  }

  SECTION("summarizes an exact brush optimization without weakening merge conflicts")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* brushGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[2]);
    auto* firstBrush = brushGroup->children()[0];
    auto* secondBrush = brushGroup->children()[1];
    brushGroup->removeChild(firstBrush);
    brushGroup->removeChild(secondBrush);
    const auto removedFirstBrush = std::unique_ptr<Node>{firstBrush};
    const auto removedSecondBrush = std::unique_ptr<Node>{secondBrush};
    const auto builder = BrushBuilder{workspace.branchWorld().mapFormat(), WorldBounds};
    auto* optimizedBrush = new BrushNode{
      builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
      | kdl::value()};
    brushGroup->addChild(optimizedBrush);

    const auto changes = workspace.changes();
    REQUIRE(changes.size() == 1u);
    CHECK(changes.front().kind == WorkspaceChangeKind::BrushesOptimized);
    CHECK(changes.front().nodeId == 0u);
    CHECK(changes.front().baseBrushCount == 2u);
    CHECK(changes.front().branchBrushCount == 1u);

    auto plan = workspace.planMerge(source.world);
    REQUIRE(plan.canApply());
    CHECK(plan.operations.size() == 3u);

    auto firstSourceBrush = source.firstBrush->brush();
    REQUIRE(firstSourceBrush
              .transform(WorldBounds, vm::translation_matrix(vm::vec3d{16, 0, 0}), false)
              .is_success());
    source.firstBrush->setBrush(std::move(firstSourceBrush));
    plan = workspace.planMerge(source.world);
    REQUIRE_FALSE(plan.canApply());
    CHECK(
      std::count_if(
        plan.conflicts.begin(),
        plan.conflicts.end(),
        [&](const auto& conflict) {
          return conflict.kind == WorkspaceMergeConflictKind::LiveNodeChanged;
        })
      == 1u);
  }

  SECTION("does not summarize a lower-count brush replacement with changed attributes")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* brushGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[2]);
    auto* firstBrush = brushGroup->children()[0];
    auto* secondBrush = brushGroup->children()[1];
    brushGroup->removeChild(firstBrush);
    brushGroup->removeChild(secondBrush);
    const auto removedFirstBrush = std::unique_ptr<Node>{firstBrush};
    const auto removedSecondBrush = std::unique_ptr<Node>{secondBrush};
    const auto builder = BrushBuilder{workspace.branchWorld().mapFormat(), WorldBounds};
    brushGroup->addChild(new BrushNode{
      builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "different_material")
      | kdl::value()});

    const auto changes = workspace.changes();
    CHECK(changes.size() == 3u);
    CHECK(std::ranges::none_of(changes, [](const auto& change) {
      return change.kind == WorkspaceChangeKind::BrushesOptimized;
    }));
  }

  SECTION("summarizes multiple independent brush optimization cohorts under one parent")
  {
    auto source = SourceTree{};
    const auto builder = BrushBuilder{source.world.mapFormat(), WorldBounds};
    source.brushGroup->addChildren(
      {new BrushNode{
         builder.createCuboid(vm::bbox3d{{0, 0, 128}, {32, 64, 192}}, "material")
         | kdl::value()},
       new BrushNode{
         builder.createCuboid(vm::bbox3d{{32, 0, 128}, {64, 64, 192}}, "material")
         | kdl::value()}});
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* brushGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[2]);
    auto removedBrushes = std::vector<std::unique_ptr<Node>>{};
    const auto branchBrushes = brushGroup->children();
    for (auto* brush : branchBrushes)
    {
      brushGroup->removeChild(brush);
      removedBrushes.emplace_back(brush);
    }
    brushGroup->addChildren(
      {new BrushNode{
         builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
         | kdl::value()},
       new BrushNode{
         builder.createCuboid(vm::bbox3d{{0, 0, 128}, {64, 64, 192}}, "material")
         | kdl::value()}});

    const auto changes = workspace.changes();
    REQUIRE(changes.size() == 1u);
    CHECK(changes.front().kind == WorkspaceChangeKind::BrushesOptimized);
    CHECK(changes.front().baseBrushCount == 4u);
    CHECK(changes.front().branchBrushCount == 2u);
  }

  SECTION("rejects moving an existing node below a branch-only parent")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* firstGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[0]);
    auto* secondGroup =
      static_cast<GroupNode*>(workspace.branchWorld().defaultLayer()->children()[1]);
    auto* entity = firstGroup->children().front();
    firstGroup->removeChild(entity);
    auto* newGroup = new GroupNode{Group{"branch-only"}};
    secondGroup->addChild(newGroup);
    newGroup->addChild(entity);

    const auto plan = workspace.planMerge(source.world);
    REQUIRE_FALSE(plan.canApply());
    REQUIRE(
      std::count_if(
        plan.conflicts.begin(),
        plan.conflicts.end(),
        [](const auto& conflict) {
          return conflict.kind == WorkspaceMergeConflictKind::UnsupportedBranchParent;
        })
      == 1u);
  }

  SECTION("reports only top-level removed roots and orders removals deepest first")
  {
    auto source = SourceTree{};
    auto branch = cloneWorld(source.world);
    auto workspace = MapWorkspace{source.world, *branch, WorldBounds};
    auto* layer = workspace.branchWorld().defaultLayer();
    auto* firstGroup = static_cast<GroupNode*>(layer->children()[0]);
    auto* secondGroup = static_cast<GroupNode*>(layer->children()[1]);
    auto* entity = firstGroup->children().front();
    const auto entityId = *workspace.nodeId(*entity);
    const auto secondGroupId = *workspace.nodeId(*secondGroup);

    firstGroup->removeChild(entity);
    const auto removedEntity = std::unique_ptr<Node>{entity};
    layer->removeChild(secondGroup);
    const auto removedGroup = std::unique_ptr<Node>{secondGroup};
    REQUIRE(removedEntity);
    REQUIRE(removedGroup);

    const auto changes = workspace.changes();
    CHECK(
      std::count_if(
        changes.begin(),
        changes.end(),
        [](const auto& change) { return change.kind == WorkspaceChangeKind::Removed; })
      == 2u);

    auto plan = workspace.planMerge(source.world);
    REQUIRE(plan.canApply());
    REQUIRE(plan.operations.size() == 2u);
    CHECK(plan.operations.front().kind == WorkspaceMergeOperationKind::Remove);
    CHECK(plan.operations.front().nodeId == entityId);
    CHECK(plan.operations.back().kind == WorkspaceMergeOperationKind::Remove);
    CHECK(plan.operations.back().nodeId == secondGroupId);

    setProperty(*source.entity, "style", "3");
    plan = workspace.planMerge(source.world);
    REQUIRE_FALSE(plan.canApply());
    REQUIRE(plan.conflicts.size() == 1u);
    CHECK(plan.conflicts.front().kind == WorkspaceMergeConflictKind::LiveNodeChanged);
  }
}

} // namespace tb::mdl
