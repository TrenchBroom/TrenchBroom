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

#include "mdl/Map_Geometry.h"

#include "Ensure.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/AddRemoveNodesCommand.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/BrushVertexCommands.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/Grid.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/Polyhedron3.h"
#include "mdl/SetLinkIdsCommand.h"
#include "mdl/SwapNodeContentsCommand.h"
#include "mdl/Transaction.h"
#include "mdl/VertexHandleManager.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/result_fold.h"
#include "kd/string_format.h"
#include "kd/task_manager.h"

#include <ranges>

namespace tb::mdl
{

kdl_reflect_impl(TransformVerticesResult);

bool transformSelection(
  Map& map, const std::string& commandName, const vm::mat4x4d& transformation)
{
  if (map.vertexHandles().anySelected())
  {
    return transformVertices(map, map.vertexHandles().selectedHandles(), transformation)
      .success;
  }

  auto nodesToTransform = std::vector<Node*>{};
  auto entitiesToTransform = std::unordered_map<EntityNodeBase*, size_t>{};

  for (auto* node : map.selection().nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* groupNode) {
        nodesToTransform.push_back(groupNode);
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, EntityNode* entityNode) {
        if (!entityNode->hasChildren())
        {
          nodesToTransform.push_back(entityNode);
        }
        else
        {
          entityNode->visitChildren(thisLambda);
        }
      },
      [&](BrushNode* brushNode) {
        nodesToTransform.push_back(brushNode);
        entitiesToTransform[brushNode->entity()]++;
      },
      [&](PatchNode* patchNode) {
        nodesToTransform.push_back(patchNode);
        entitiesToTransform[patchNode->entity()]++;
      }));
  }

  // add entities if all of their children are transformed
  for (const auto& [entityNode, transformedChildCount] : entitiesToTransform)
  {
    if (
      transformedChildCount == entityNode->childCount()
      && !isWorldspawn(entityNode->entity().classname()))
    {
      nodesToTransform.push_back(entityNode);
    }
  }

  using TransformResult = Result<std::pair<Node*, NodeContents>>;

  const auto alignmentLock = pref(Preferences::AlignmentLock);
  const auto updateAngleProperty =
    map.world()->entityPropertyConfig().updateAnglePropertyAfterTransform;

  auto tasks =
    nodesToTransform | std::views::transform([&](auto& node) {
      return std::function{[&]() {
        return node->accept(kdl::overload(
          [&](WorldNode*) -> TransformResult { ensure(false, "Unexpected world node"); },
          [&](LayerNode*) -> TransformResult { ensure(false, "Unexpected layer node"); },
          [&](GroupNode* groupNode) -> TransformResult {
            auto group = groupNode->group();
            group.transform(transformation);
            return std::make_pair(groupNode, NodeContents{std::move(group)});
          },
          [&](EntityNode* entityNode) -> TransformResult {
            auto entity = entityNode->entity();
            entity.transform(transformation, updateAngleProperty);
            return std::make_pair(entityNode, NodeContents{std::move(entity)});
          },
          [&](BrushNode* brushNode) -> TransformResult {
            const auto* containingGroup = brushNode->containingGroup();
            const bool lockAlignment =
            alignmentLock
            || (containingGroup && containingGroup->closed() && collectLinkedNodes({map.world()}, *brushNode).size() > 1);

            auto brush = brushNode->brush();
            return brush.transform(map.worldBounds(), transformation, lockAlignment)
                   | kdl::and_then([&]() -> TransformResult {
                       return std::make_pair(brushNode, NodeContents{std::move(brush)});
                     });
          },
          [&](PatchNode* patchNode) -> TransformResult {
            auto patch = patchNode->patch();
            patch.transform(transformation);
            return std::make_pair(patchNode, NodeContents{std::move(patch)});
          }));
      }};
    });

  const auto success = map.taskManager().run_tasks_and_wait(tasks) | kdl::fold
                       | kdl::transform([&](auto nodesToUpdate) {
                           return updateNodeContents(
                             map,
                             commandName,
                             std::move(nodesToUpdate),
                             collectContainingGroups(map.selection().nodes));
                         })
                       | kdl::value_or(false);

  if (success)
  {
    map.pushRepeatableCommand([&, commandName, transformation]() {
      transformSelection(map, commandName, transformation);
    });
  }

  return success;
}

bool translateSelection(mdl::Map& map, const vm::vec3d& delta)
{
  return transformSelection(map, "Translate Objects", vm::translation_matrix(delta));
}

bool rotateSelection(
  mdl::Map& map, const vm::vec3d& center, const vm::vec3d& axis, double angle)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::rotation_matrix(axis, angle)
                              * vm::translation_matrix(-center);
  return transformSelection(map, "Rotate Objects", transformation);
}

bool scaleSelection(mdl::Map& map, const vm::bbox3d& oldBBox, const vm::bbox3d& newBBox)
{
  const auto transformation = vm::scale_bbox_matrix(oldBBox, newBBox);
  return transformSelection(map, "Scale Objects", transformation);
}

bool scaleSelection(mdl::Map& map, const vm::vec3d& center, const vm::vec3d& scaleFactors)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::scaling_matrix(scaleFactors)
                              * vm::translation_matrix(-center);
  return transformSelection(map, "Scale Objects", transformation);
}

bool shearSelection(
  mdl::Map& map,
  const vm::bbox3d& box,
  const vm::vec3d& sideToShear,
  const vm::vec3d& delta)
{
  const auto transformation = vm::shear_bbox_matrix(box, sideToShear, delta);
  return transformSelection(map, "Scale Objects", transformation);
}

bool flipSelection(mdl::Map& map, const vm::vec3d& center, const vm::axis::type axis)
{
  const auto transformation = vm::translation_matrix(center)
                              * vm::mirror_matrix<double>(axis)
                              * vm::translation_matrix(-center);
  return transformSelection(map, "Flip Objects", transformation);
}


TransformVerticesResult transformVertices(
  Map& map, std::vector<vm::vec3d> vertexPositions, const vm::mat4x4d& transform)
{
  auto newVertexPositions = std::vector<vm::vec3d>{};
  auto newNodes = applyToNodeContents(
    map.selection().nodes,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        const auto verticesToMove = vertexPositions
                                    | std::views::filter([&](const auto& vertex) {
                                        return brush.hasVertex(vertex);
                                      })
                                    | kdl::ranges::to<std::vector>();
        if (verticesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformVertices(map.worldBounds(), verticesToMove, transform))
        {
          return false;
        }

        return brush.transformVertices(
                 map.worldBounds(), verticesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions =
                     brush.findClosestVertexPositions(transform * verticesToMove);
                   newVertexPositions = kdl::vec_concat(
                     std::move(newVertexPositions), std::move(newPositions));
                 })
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not move brush vertices: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newVertexPositions);

    const auto commandName =
      kdl::str_plural(vertexPositions.size(), "Move Brush Vertex", "Move Brush Vertices");
    auto transaction = Transaction{map, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

    const auto result = map.executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::move(vertexPositions),
      std::move(newVertexPositions)));

    if (!result->success())
    {
      transaction.cancel();
      return TransformVerticesResult{false, false};
    }

    setHasPendingChanges(changedLinkedGroups, true);

    if (!transaction.commit())
    {
      return TransformVerticesResult{false, false};
    }

    const auto* moveVerticesResult =
      dynamic_cast<BrushVertexCommandResult*>(result.get());
    ensure(
      moveVerticesResult != nullptr,
      "command processor returned unexpected command result type");

    return {moveVerticesResult->success(), moveVerticesResult->hasRemainingVertices()};
  }

  return TransformVerticesResult{false, false};
}

bool transformEdges(
  Map& map, std::vector<vm::segment3d> edgePositions, const vm::mat4x4d& transform)
{
  auto newEdgePositions = std::vector<vm::segment3d>{};
  auto newNodes = applyToNodeContents(
    map.selection().nodes,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        const auto edgesToMove =
          edgePositions
          | std::views::filter([&](const auto& edge) { return brush.hasEdge(edge); })
          | kdl::ranges::to<std::vector>();
        if (edgesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformEdges(map.worldBounds(), edgesToMove, transform))
        {
          return false;
        }

        return brush.transformEdges(
                 map.worldBounds(), edgesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestEdgePositions(
                     edgesToMove | std::views::transform([&](const auto& edge) {
                       return edge.transform(transform);
                     })
                     | kdl::ranges::to<std::vector>());
                   newEdgePositions = kdl::vec_concat(
                     std::move(newEdgePositions), std::move(newPositions));
                 })
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not move brush edges: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newEdgePositions);

    const auto commandName =
      kdl::str_plural(edgePositions.size(), "Move Brush Edge", "Move Brush Edges");
    auto transaction = Transaction{map, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

    const auto result = map.executeAndStore(std::make_unique<BrushEdgeCommand>(
      commandName,
      std::move(*newNodes),
      std::move(edgePositions),
      std::move(newEdgePositions)));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool transformFaces(
  Map& map, std::vector<vm::polygon3d> facePositions, const vm::mat4x4d& transform)
{
  auto newFacePositions = std::vector<vm::polygon3d>{};
  auto newNodes = applyToNodeContents(
    map.selection().nodes,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        const auto facesToMove =
          facePositions
          | std::views::filter([&](const auto& face) { return brush.hasFace(face); })
          | kdl::ranges::to<std::vector>();
        if (facesToMove.empty())
        {
          return true;
        }

        if (!brush.canTransformFaces(map.worldBounds(), facesToMove, transform))
        {
          return false;
        }

        return brush.transformFaces(
                 map.worldBounds(), facesToMove, transform, pref(Preferences::UVLock))
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestFacePositions(
                     facesToMove | std::views::transform([&](const auto& face) {
                       return face.transform(transform);
                     })
                     | kdl::ranges::to<std::vector>());
                   newFacePositions = kdl::vec_concat(
                     std::move(newFacePositions), std::move(newPositions));
                 })
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not move brush faces: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (newNodes)
  {
    kdl::vec_sort_and_remove_duplicates(newFacePositions);

    const auto commandName =
      kdl::str_plural(facePositions.size(), "Move Brush Face", "Move Brush Faces");
    auto transaction = Transaction{map, commandName};

    auto changedLinkedGroups = collectContainingGroups(
      *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

    const auto result = map.executeAndStore(std::make_unique<BrushFaceCommand>(
      commandName,
      std::move(*newNodes),
      std::move(facePositions),
      std::move(newFacePositions)));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool addVertex(Map& map, const vm::vec3d& vertexPosition)
{
  auto newNodes = applyToNodeContents(
    map.selection().nodes,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        if (!brush.canAddVertex(map.worldBounds(), vertexPosition))
        {
          return false;
        }

        return brush.addVertex(map.worldBounds(), vertexPosition)
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not add brush vertex: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (newNodes)
  {
    const auto commandName = "Add Brush Vertex";
    auto transaction = Transaction{map, commandName};

    const auto changedLinkedGroups = collectContainingGroups(
      *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

    const auto result = map.executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::vector<vm::vec3d>{},
      std::vector<vm::vec3d>{vertexPosition}));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool removeVertices(
  Map& map, const std::string& commandName, std::vector<vm::vec3d> vertexPositions)
{
  auto newNodes = applyToNodeContents(
    map.selection().nodes,
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        const auto verticesToRemove = vertexPositions
                                      | std::views::filter([&](const auto& vertex) {
                                          return brush.hasVertex(vertex);
                                        })
                                      | kdl::ranges::to<std::vector>();
        if (verticesToRemove.empty())
        {
          return true;
        }

        if (!brush.canRemoveVertices(map.worldBounds(), verticesToRemove))
        {
          return false;
        }

        return brush.removeVertices(map.worldBounds(), verticesToRemove)
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not remove brush vertices: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (newNodes)
  {
    auto transaction = Transaction{map, commandName};

    auto changedLinkedGroups = collectContainingGroups(
      *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

    const auto result = map.executeAndStore(std::make_unique<BrushVertexCommand>(
      commandName,
      std::move(*newNodes),
      std::move(vertexPositions),
      std::vector<vm::vec3d>{}));

    if (!result->success())
    {
      transaction.cancel();
      return false;
    }

    setHasPendingChanges(changedLinkedGroups, true);
    return transaction.commit();
  }

  return false;
}

bool snapVertices(Map& map, const double snapTo)
{
  size_t succeededBrushCount = 0;
  size_t failedBrushCount = 0;

  const auto allSelectedBrushes = map.selection().allBrushes();
  const bool applyAndSwapSuccess = applyAndSwap(
    map,
    "Snap Brush Vertices",
    allSelectedBrushes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(allSelectedBrushes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& originalBrush) {
        if (originalBrush.canSnapVertices(map.worldBounds(), snapTo))
        {
          originalBrush.snapVertices(map.worldBounds(), snapTo, pref(Preferences::UVLock))
            | kdl::transform([&]() { succeededBrushCount += 1; })
            | kdl::transform_error([&](auto e) {
                map.logger().error() << "Could not snap vertices: " << e.msg;
                failedBrushCount += 1;
              });
        }
        else
        {
          failedBrushCount += 1;
        }
        return true;
      },
      [](BezierPatch&) { return true; }));

  if (!applyAndSwapSuccess)
  {
    return false;
  }
  if (succeededBrushCount > 0)
  {
    map.logger().info() << fmt::format(
      "Snapped vertices of {} {}",
      succeededBrushCount,
      kdl::str_plural(succeededBrushCount, "brush", "brushes"));
  }
  if (failedBrushCount > 0)
  {
    map.logger().info() << fmt::format(
      "Failed to snap vertices of {} {}",
      failedBrushCount,
      kdl::str_plural(failedBrushCount, "brush", "brushes"));
  }

  return true;
}

bool csgConvexMerge(Map& map)
{
  if (!map.selection().hasBrushFaces() && !map.selection().hasOnlyBrushes())
  {
    return false;
  }

  auto points = std::vector<vm::vec3d>{};

  if (map.selection().hasBrushFaces())
  {
    for (const auto& handle : map.selection().brushFaces)
    {
      for (const auto* vertex : handle.face().vertices())
      {
        points.push_back(vertex->position());
      }
    }
  }
  else if (map.selection().hasOnlyBrushes())
  {
    for (const auto* brushNode : map.selection().brushes)
    {
      for (const auto* vertex : brushNode->brush().vertices())
      {
        points.push_back(vertex->position());
      }
    }
  }

  auto polyhedron = Polyhedron3{std::move(points)};
  if (!polyhedron.polyhedron() || !polyhedron.closed())
  {
    return false;
  }

  const auto builder = BrushBuilder{
    map.world()->mapFormat(),
    map.worldBounds(),
    map.game()->config().faceAttribsConfig.defaults};
  return builder.createBrush(polyhedron, map.currentMaterialName())
         | kdl::transform([&](auto b) {
             b.cloneFaceAttributesFrom(
               map.selection().brushes | std::views::transform([](const auto* brushNode) {
                 return &brushNode->brush();
               })
               | kdl::ranges::to<std::vector>());

             // The nodelist is either empty or contains only brushes.
             const auto toRemove = map.selection().nodes;

             // We could be merging brushes that have different parents; use the parent
             // of the first brush.
             auto* parentNode = static_cast<Node*>(nullptr);
             if (!map.selection().brushes.empty())
             {
               parentNode = map.selection().brushes.front()->parent();
             }
             else if (!map.selection().brushFaces.empty())
             {
               parentNode = map.selection().brushFaces.front().node()->parent();
             }
             else
             {
               parentNode = parentForNodes(map);
             }

             auto* brushNode = new BrushNode{std::move(b)};

             auto transaction = Transaction{map, "CSG Convex Merge"};
             deselectAll(map);
             if (addNodes(map, {{parentNode, {brushNode}}}).empty())
             {
               transaction.cancel();
               return;
             }
             removeNodes(map, toRemove);
             selectNodes(map, {brushNode});
             transaction.commit();
           })
         | kdl::if_error(
           [&](auto e) { map.logger().error() << "Could not create brush: " << e.msg; })
         | kdl::is_success();
}

bool csgSubtract(Map& map)
{
  const auto subtrahendNodes = std::vector<BrushNode*>{map.selection().brushes};
  if (subtrahendNodes.empty())
  {
    return false;
  }

  auto transaction = Transaction{map, "CSG Subtract"};
  // Select touching, but don't delete the subtrahends yet
  selectTouchingNodes(map, false);

  const auto minuendNodes = std::vector<BrushNode*>{map.selection().brushes};
  const auto subtrahends = subtrahendNodes
                           | std::views::transform([](const auto* subtrahendNode) {
                               return &subtrahendNode->brush();
                             })
                           | kdl::ranges::to<std::vector>();

  auto toAdd = std::map<Node*, std::vector<Node*>>{};
  auto toRemove =
    std::vector<Node*>{std::begin(subtrahendNodes), std::end(subtrahendNodes)};

  return minuendNodes | std::views::transform([&](auto* minuendNode) {
           const auto& minuend = minuendNode->brush();
           auto currentSubtractionResults = minuend.subtract(
             map.world()->mapFormat(),
             map.worldBounds(),
             map.currentMaterialName(),
             subtrahends);

           return currentSubtractionResults
                  | std::views::filter([](const auto r) { return r | kdl::is_success(); })
                  | kdl::views::as_rvalue | kdl::fold
                  | kdl::transform([&](auto currentBrushes) {
                      if (!currentBrushes.empty())
                      {
                        auto resultNodes = currentBrushes | kdl::views::as_rvalue
                                           | std::views::transform([&](auto b) {
                                               return new BrushNode{std::move(b)};
                                             })
                                           | kdl::ranges::to<std::vector>();
                        auto& toAddForParent = toAdd[minuendNode->parent()];
                        toAddForParent = kdl::vec_concat(
                          std::move(toAddForParent), std::move(resultNodes));
                      }

                      toRemove.push_back(minuendNode);
                    });
         })
         | kdl::fold | kdl::transform([&]() {
             deselectAll(map);
             const auto added = addNodes(map, toAdd);
             removeNodes(map, toRemove);
             selectNodes(map, added);

             return transaction.commit();
           })
         | kdl::transform_error([&](const auto& e) {
             map.logger().error() << "Could not subtract brushes: " << e;
             transaction.cancel();
             return false;
           })
         | kdl::value();
}

bool csgIntersect(Map& map)
{
  const auto brushes = map.selection().brushes;
  if (brushes.size() < 2u)
  {
    return false;
  }

  auto intersection = brushes.front()->brush();

  bool valid = true;
  for (auto it = std::next(std::begin(brushes)), end = std::end(brushes);
       it != end && valid;
       ++it)
  {
    auto* brushNode = *it;
    const auto& brush = brushNode->brush();
    valid = intersection.intersect(map.worldBounds(), brush) | kdl::if_error([&](auto e) {
              map.logger().error() << "Could not intersect brushes: " << e.msg;
            })
            | kdl::is_success();
  }

  const auto toRemove = std::vector<Node*>{std::begin(brushes), std::end(brushes)};

  auto transaction = Transaction{map, "CSG Intersect"};
  deselectNodes(map, toRemove);

  if (valid)
  {
    auto* intersectionNode = new BrushNode{std::move(intersection)};
    if (addNodes(map, {{parentForNodes(map, toRemove), {intersectionNode}}}).empty())
    {
      transaction.cancel();
      return false;
    }
    removeNodes(map, toRemove);
    selectNodes(map, {intersectionNode});
  }
  else
  {
    removeNodes(map, toRemove);
  }

  return transaction.commit();
}

bool csgHollow(Map& map)
{
  const auto brushNodes = map.selection().brushes;
  if (brushNodes.empty())
  {
    return false;
  }

  bool didHollowAnything = false;
  auto toAdd = std::map<Node*, std::vector<Node*>>{};
  auto toRemove = std::vector<Node*>{};

  for (auto* brushNode : brushNodes)
  {
    const auto& originalBrush = brushNode->brush();

    auto shrunkenBrush = originalBrush;
    shrunkenBrush.expand(map.worldBounds(), -double(map.grid().actualSize()), true)
      | kdl::and_then([&]() {
          didHollowAnything = true;

          return originalBrush.subtract(
                   map.world()->mapFormat(),
                   map.worldBounds(),
                   map.currentMaterialName(),
                   shrunkenBrush)
                 | kdl::fold | kdl::transform([&](auto fragments) {
                     auto fragmentNodes =
                       fragments | kdl::views::as_rvalue
                       | std::views::transform([](auto&& b) {
                           return new BrushNode{std::forward<decltype(b)>(b)};
                         })
                       | kdl::ranges::to<std::vector>();

                     auto& toAddForParent = toAdd[brushNode->parent()];
                     toAddForParent =
                       kdl::vec_concat(std::move(toAddForParent), fragmentNodes);
                     toRemove.push_back(brushNode);
                   });
        })
      | kdl::transform_error(
        [&](const auto& e) { map.logger().error() << "Could not hollow brush: " << e; });
  }

  if (!didHollowAnything)
  {
    return false;
  }

  auto transaction = Transaction{map, "CSG Hollow"};
  deselectAll(map);
  const auto added = addNodes(map, toAdd);
  if (added.empty())
  {
    transaction.cancel();
    return false;
  }
  removeNodes(map, toRemove);
  selectNodes(map, added);

  return transaction.commit();
}

bool extrudeBrushes(
  Map& map, const std::vector<vm::polygon3d>& faces, const vm::vec3d& delta)
{
  const auto nodes = map.selection().nodes;
  return applyAndSwap(
    map,
    "Resize Brushes",
    nodes,
    collectContainingGroups(nodes),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [](Entity&) { return true; },
      [&](Brush& brush) {
        const auto faceIndex = brush.findFace(faces);
        if (!faceIndex)
        {
          // we allow resizing only some of the brushes
          return true;
        }

        return brush.moveBoundary(
                 map.worldBounds(), *faceIndex, delta, pref(Preferences::AlignmentLock))
               | kdl::transform(
                 [&]() { return map.worldBounds().contains(brush.bounds()); })
               | kdl::transform_error([&](auto e) {
                   map.logger().error() << "Could not resize brush: " << e.msg;
                   return false;
                 })
               | kdl::value();
      },
      [](BezierPatch&) { return true; }));
}

} // namespace tb::mdl
