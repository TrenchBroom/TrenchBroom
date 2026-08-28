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

#include "base/Logger.h"
#include "mdl/AddRemoveNodesCommand.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/BrushOptimization.h"
#include "mdl/BrushVertexCommands.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
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
#include "mdl/NodeHandles.h"
#include "mdl/PatchNode.h"
#include "mdl/Polyhedron3.h"
#include "mdl/SetLinkIdsCommand.h"
#include "mdl/SwapNodeContentsCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/result_fold.h"
#include "kd/string_format.h"
#include "kd/task_manager.h"
#include "kd/vector_utils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <tuple>
#include <utility>

namespace tb::mdl
{

kdl_reflect_impl(TransformVerticesResult);

bool transformSelection(
  Map& map, const std::string& commandName, const vm::mat4x4d& transformation)
{
  if (map.nodeHandles().anyHandleSelected<VertexHandle>())
  {
    const auto selectedVertexPositions =
      VertexHandle::getPositions(map.nodeHandles().selectedHandles<VertexHandle>());
    return transformVertices(map, selectedVertexPositions, transformation).success;
  }

  auto nodesToTransform = std::vector<Node*>{};
  auto entitiesToTransform = std::unordered_map<EntityNodeBase*, size_t>{};

  for (auto* node : map.selection().nodes)
  {
    node->accept(
      kdl::overload(
        [&](auto&& thisLambda, WorldNode& worldNode) {
          worldNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, LayerNode& layerNode) {
          layerNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, GroupNode& groupNode) {
          nodesToTransform.push_back(&groupNode);
          groupNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, EntityNode& entityNode) {
          if (!entityNode.hasChildren())
          {
            nodesToTransform.push_back(&entityNode);
          }
          else
          {
            entityNode.visitChildren(thisLambda);
          }
        },
        [&](BrushNode& brushNode) {
          nodesToTransform.push_back(&brushNode);
          entitiesToTransform[brushNode.entity()]++;
        },
        [&](PatchNode& patchNode) {
          nodesToTransform.push_back(&patchNode);
          entitiesToTransform[patchNode.entity()]++;
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

  const auto alignmentLock = map.editorContext().alignmentLock();
  const auto updateAngleProperty =
    map.worldNode().entityPropertyConfig().updateAnglePropertyAfterTransform;

  auto tasks =
    nodesToTransform | std::views::transform([&](auto& node) {
      return std::function{[&]() {
        return node->accept(
          kdl::overload(
            [&](WorldNode&) -> TransformResult { contract_assert(false); },
            [&](LayerNode&) -> TransformResult { contract_assert(false); },
            [&](GroupNode& groupNode) -> TransformResult {
              auto group = groupNode.group();
              group.transform(transformation);
              return std::make_pair(&groupNode, NodeContents{std::move(group)});
            },
            [&](EntityNode& entityNode) -> TransformResult {
              auto entity = entityNode.entity();
              entity.transform(transformation, updateAngleProperty);
              return std::make_pair(&entityNode, NodeContents{std::move(entity)});
            },
            [&](BrushNode& brushNode) -> TransformResult {
              const auto* containingGroup = brushNode.containingGroup();
              const bool lockAlignment =
            alignmentLock
            || (containingGroup && containingGroup->closed() && collectLinkedNodes({&map.worldNode()}, brushNode).size() > 1);

              auto brush = brushNode.brush();
              return brush.transform(map.worldBounds(), transformation, lockAlignment)
                     | kdl::and_then([&]() -> TransformResult {
                         return std::make_pair(
                           &brushNode, NodeContents{std::move(brush)});
                       });
            },
            [&](PatchNode& patchNode) -> TransformResult {
              auto patch = patchNode.patch();
              patch.transform(transformation);
              return std::make_pair(&patchNode, NodeContents{std::move(patch)});
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
  Map& map, const std::vector<vm::vec3d>& vertexPositions, const vm::mat4x4d& transform)
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
                 map.worldBounds(),
                 verticesToMove,
                 transform,
                 map.editorContext().uvLock())
               | kdl::transform([&]() {
                   auto newPositions =
                     brush.findClosestVertexPositions(transform * verticesToMove);
                   kdl::vec_append(newVertexPositions, std::move(newPositions));
                 })
               | kdl::if_error([&](auto e) {
                   map.logger().error() << "Could not move brush vertices: " << e.msg;
                 })
               | kdl::is_success();
      },
      [](BezierPatch&) { return true; }));

  if (!newNodes)
  {
    return TransformVerticesResult{false, false};
  }

  kdl::vec_sort_and_remove_duplicates(newVertexPositions);
  const auto hasRemainingVertices = !newVertexPositions.empty();

  auto commandName =
    kdl::str_plural(vertexPositions.size(), "Move Brush Vertex", "Move Brush Vertices");
  auto transaction = Transaction{map, commandName};

  const auto changedLinkedGroups = collectContainingGroups(
    *newNodes | std::views::keys | kdl::ranges::to<std::vector>());

  auto command = std::make_unique<BrushVertexCommand>(
    std::move(commandName), std::move(*newNodes), vertexPositions, newVertexPositions);

  if (!map.executeAndStore(std::move(command)))
  {
    transaction.cancel();
    return TransformVerticesResult{false, false};
  }

  setHasPendingChanges(changedLinkedGroups, true);

  if (!transaction.commit())
  {
    return TransformVerticesResult{false, false};
  }

  return {true, hasRemainingVertices};
}

bool transformEdges(
  Map& map, const std::vector<vm::segment3d>& edgePositions, const vm::mat4x4d& transform)
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
                 map.worldBounds(), edgesToMove, transform, map.editorContext().uvLock())
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestEdgePositions(
                     edgesToMove | std::views::transform([&](const auto& edge) {
                       return edge.transform(transform);
                     })
                     | kdl::ranges::to<std::vector>());
                   kdl::vec_append(newEdgePositions, std::move(newPositions));
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

    const auto result = map.executeAndStore(
      std::make_unique<BrushEdgeCommand>(
        commandName, std::move(*newNodes), edgePositions, newEdgePositions));

    if (!result)
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
  Map& map, const std::vector<vm::polygon3d>& facePositions, const vm::mat4x4d& transform)
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
                 map.worldBounds(), facesToMove, transform, map.editorContext().uvLock())
               | kdl::transform([&]() {
                   auto newPositions = brush.findClosestFacePositions(
                     facesToMove | std::views::transform([&](const auto& face) {
                       return face.transform(transform);
                     })
                     | kdl::ranges::to<std::vector>());
                   kdl::vec_append(newFacePositions, std::move(newPositions));
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

    const auto result = map.executeAndStore(
      std::make_unique<BrushFaceCommand>(
        commandName, std::move(*newNodes), facePositions, newFacePositions));

    if (!result)
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

    const auto result = map.executeAndStore(
      std::make_unique<BrushVertexCommand>(
        commandName,
        std::move(*newNodes),
        std::vector<vm::vec3d>{},
        std::vector<vm::vec3d>{vertexPosition}));

    if (!result)
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

    const auto result = map.executeAndStore(
      std::make_unique<BrushVertexCommand>(
        commandName, std::move(*newNodes), vertexPositions, std::vector<vm::vec3d>{}));

    if (!result)
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
          originalBrush.snapVertices(
            map.worldBounds(), snapTo, map.editorContext().uvLock())
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
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
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
             auto& parentNode = !map.selection().brushes.empty()
                                  ? *map.selection().brushes.front()->parent()
                                : !map.selection().brushFaces.empty()
                                  ? *map.selection().brushFaces.front().node()->parent()
                                  : parentForNodes(map);

             auto* brushNode = new BrushNode{std::move(b)};

             auto transaction = Transaction{map, "CSG Convex Merge"};
             deselectAll(map);
             if (addNodes(map, {{&parentNode, {brushNode}}}).empty())
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

namespace
{

double overlapLength(
  const double lhsMin, const double lhsMax, const double rhsMin, const double rhsMax)
{
  return std::max(0.0, std::min(lhsMax, rhsMax) - std::max(lhsMin, rhsMin));
}

double faceOverlapArea(const vm::bbox3d& lhs, const vm::bbox3d& rhs, const size_t axis)
{
  const auto uAxis = (axis + 1u) % 3u;
  const auto vAxis = (axis + 2u) % 3u;
  return overlapLength(lhs.min[uAxis], lhs.max[uAxis], rhs.min[uAxis], rhs.max[uAxis])
         * overlapLength(lhs.min[vAxis], lhs.max[vAxis], rhs.min[vAxis], rhs.max[vAxis]);
}

bool candidateFaceIsInternal(
  const BrushOptimizationCandidate& candidate,
  const size_t boxIndex,
  const size_t axis,
  const bool positive)
{
  const auto& bounds = candidate.bounds[boxIndex];
  const auto plane = positive ? bounds.max[axis] : bounds.min[axis];
  const auto uAxis = (axis + 1u) % 3u;
  const auto vAxis = (axis + 2u) % 3u;
  const auto faceArea = bounds.size()[uAxis] * bounds.size()[vAxis];

  auto coveredArea = 0.0;
  for (size_t otherIndex = 0u; otherIndex < candidate.bounds.size(); ++otherIndex)
  {
    if (otherIndex == boxIndex)
    {
      continue;
    }

    const auto& other = candidate.bounds[otherIndex];
    const auto otherPlane = positive ? other.min[axis] : other.max[axis];
    if (vm::is_equal(plane, otherPlane, vm::Cd::almost_zero()))
    {
      coveredArea += faceOverlapArea(bounds, other, axis);
    }
  }
  return vm::is_equal(faceArea, coveredArea, vm::Cd::almost_zero());
}

bool compatibleVisibleFaceAttributes(const BrushFace& lhs, const BrushFace& rhs)
{
  return lhs.materialName() == rhs.materialName()
         && lhs.surfaceAttributes() == rhs.surfaceAttributes()
         && lhs.uvAttributes() == rhs.uvAttributes()
         && vm::is_equal(lhs.uAxis(), rhs.uAxis(), vm::Cd::almost_zero())
         && vm::is_equal(lhs.vAxis(), rhs.vAxis(), vm::Cd::almost_zero());
}

bool preservesVisibleFaceAttributes(
  const std::vector<BrushNode*>& sourceNodes, const BrushOptimizationCandidate& candidate)
{
  for (size_t boxIndex = 0u; boxIndex < candidate.bounds.size(); ++boxIndex)
  {
    const auto& bounds = candidate.bounds[boxIndex];
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
      for (const auto positive : {false, true})
      {
        if (candidateFaceIsInternal(candidate, boxIndex, axis, positive))
        {
          continue;
        }

        const auto normal =
          (positive ? 1.0 : -1.0) * vm::vec3d::axis(vm::axis::type(axis));
        const auto plane = positive ? bounds.max[axis] : bounds.min[axis];
        const BrushFace* referenceFace = nullptr;
        for (const auto* sourceNode : sourceNodes)
        {
          const auto& sourceBrush = sourceNode->brush();
          const auto sourcePlane =
            positive ? sourceBrush.bounds().max[axis] : sourceBrush.bounds().min[axis];
          if (
            !vm::is_equal(plane, sourcePlane, vm::Cd::almost_zero())
            || faceOverlapArea(bounds, sourceBrush.bounds(), axis)
                 <= vm::Cd::almost_zero())
          {
            continue;
          }

          const auto faceIndex = sourceBrush.findFace(normal);
          contract_assert(faceIndex.has_value());
          const auto& sourceFace = sourceBrush.face(*faceIndex);
          if (referenceFace == nullptr)
          {
            referenceFace = &sourceFace;
          }
          else if (!compatibleVisibleFaceAttributes(*referenceFace, sourceFace))
          {
            return false;
          }
        }

        if (referenceFace == nullptr)
        {
          return false;
        }
      }
    }
  }
  return true;
}

constexpr auto PrismOptimizationEpsilon = 0.01;
constexpr auto PrismOptimizationStrategyCount = 24u;

struct ConvexPrismPiece
{
  size_t axis;
  double min;
  double max;
  std::vector<vm::vec2d> footprint;
  std::vector<size_t> sourceIndices;
};

double cross2d(const vm::vec2d& lhs, const vm::vec2d& rhs)
{
  return lhs.x() * rhs.y() - lhs.y() * rhs.x();
}

double signedPolygonArea(const std::vector<vm::vec2d>& polygon)
{
  auto result = 0.0;
  for (size_t i = 0u; i < polygon.size(); ++i)
  {
    result += cross2d(polygon[i], polygon[(i + 1u) % polygon.size()]);
  }
  return result / 2.0;
}

double polygonArea(const std::vector<vm::vec2d>& polygon)
{
  return std::abs(signedPolygonArea(polygon));
}

bool pointsEqual(const vm::vec2d& lhs, const vm::vec2d& rhs)
{
  return vm::squared_distance(lhs, rhs)
         <= PrismOptimizationEpsilon * PrismOptimizationEpsilon;
}

std::vector<vm::vec2d> convexHull(std::vector<vm::vec2d> points)
{
  std::ranges::sort(points, [](const auto& lhs, const auto& rhs) {
    return std::tuple{lhs.x(), lhs.y()} < std::tuple{rhs.x(), rhs.y()};
  });
  points.erase(std::unique(points.begin(), points.end(), pointsEqual), points.end());
  if (points.size() < 3u)
  {
    return {};
  }

  const auto appendPoint = [](std::vector<vm::vec2d>& hull, const vm::vec2d& point) {
    while (hull.size() >= 2u
           && cross2d(hull.back() - hull[hull.size() - 2u], point - hull.back())
                <= PrismOptimizationEpsilon)
    {
      hull.pop_back();
    }
    hull.push_back(point);
  };

  auto lower = std::vector<vm::vec2d>{};
  for (const auto& point : points)
  {
    appendPoint(lower, point);
  }
  auto upper = std::vector<vm::vec2d>{};
  for (const auto& point : points | std::views::reverse)
  {
    appendPoint(upper, point);
  }
  lower.pop_back();
  upper.pop_back();
  kdl::vec_append(lower, upper);
  return lower;
}

std::array<size_t, 2u> planarAxes(const size_t axis)
{
  return {(axis + 1u) % 3u, (axis + 2u) % 3u};
}

vm::vec2d projectPoint(const vm::vec3d& point, const size_t axis)
{
  const auto axes = planarAxes(axis);
  return {point[axes[0]], point[axes[1]]};
}

vm::vec3d unprojectPoint(
  const vm::vec2d& point, const size_t axis, const double coordinate)
{
  const auto axes = planarAxes(axis);
  auto result = vm::vec3d::zero();
  result[axis] = coordinate;
  result[axes[0]] = point.x();
  result[axes[1]] = point.y();
  return result;
}

double segmentOverlapLength(
  const vm::vec2d& a, const vm::vec2d& b, const vm::vec2d& c, const vm::vec2d& d)
{
  const auto ab = b - a;
  const auto lengthSquared = vm::squared_length(ab);
  if (lengthSquared <= PrismOptimizationEpsilon * PrismOptimizationEpsilon)
  {
    return 0.0;
  }
  const auto length = std::sqrt(lengthSquared);
  if (
    std::abs(cross2d(ab, c - a)) > PrismOptimizationEpsilon * length
    || std::abs(cross2d(ab, d - a)) > PrismOptimizationEpsilon * length)
  {
    return 0.0;
  }

  const auto t1 = vm::dot(c - a, ab) / lengthSquared;
  const auto t2 = vm::dot(d - a, ab) / lengthSquared;
  const auto overlap = std::min(1.0, std::max(t1, t2)) - std::max(0.0, std::min(t1, t2));
  return std::max(0.0, overlap) * length;
}

double sharedBoundaryLength(
  const std::vector<vm::vec2d>& lhs, const std::vector<vm::vec2d>& rhs)
{
  auto result = 0.0;
  for (size_t i = 0u; i < lhs.size(); ++i)
  {
    for (size_t j = 0u; j < rhs.size(); ++j)
    {
      result += segmentOverlapLength(
        lhs[i], lhs[(i + 1u) % lhs.size()], rhs[j], rhs[(j + 1u) % rhs.size()]);
    }
  }
  return result;
}

vm::vec2d lineIntersection(
  const vm::vec2d& a, const vm::vec2d& b, const vm::vec2d& c, const vm::vec2d& d)
{
  const auto ab = b - a;
  const auto cd = d - c;
  const auto denominator = cross2d(ab, cd);
  if (std::abs(denominator) <= PrismOptimizationEpsilon)
  {
    return b;
  }
  return a + (cross2d(c - a, cd) / denominator) * ab;
}

std::vector<vm::vec2d> intersectConvexPolygons(
  std::vector<vm::vec2d> subject, const std::vector<vm::vec2d>& clip)
{
  for (size_t edgeIndex = 0u; edgeIndex < clip.size(); ++edgeIndex)
  {
    const auto& edgeStart = clip[edgeIndex];
    const auto& edgeEnd = clip[(edgeIndex + 1u) % clip.size()];
    const auto input = std::move(subject);
    subject.clear();
    if (input.empty())
    {
      break;
    }

    const auto inside = [&](const auto& point) {
      return cross2d(edgeEnd - edgeStart, point - edgeStart) >= -PrismOptimizationEpsilon;
    };
    auto previous = input.back();
    auto previousInside = inside(previous);
    for (const auto& current : input)
    {
      const auto currentInside = inside(current);
      if (currentInside != previousInside)
      {
        subject.push_back(lineIntersection(previous, current, edgeStart, edgeEnd));
      }
      if (currentInside)
      {
        subject.push_back(current);
      }
      previous = current;
      previousInside = currentInside;
    }
  }
  return subject;
}

std::optional<ConvexPrismPiece> extractConvexPrism(
  const Brush& brush, const size_t axis, const size_t sourceIndex)
{
  const auto axisType = vm::axis::type(axis);
  const auto negativeFaceIndex = brush.findFace(-vm::vec3d::axis(axisType));
  const auto positiveFaceIndex = brush.findFace(vm::vec3d::axis(axisType));
  if (!negativeFaceIndex || !positiveFaceIndex)
  {
    return std::nullopt;
  }

  for (size_t faceIndex = 0u; faceIndex < brush.faceCount(); ++faceIndex)
  {
    if (faceIndex == *negativeFaceIndex || faceIndex == *positiveFaceIndex)
    {
      continue;
    }
    if (std::abs(brush.face(faceIndex).normal()[axis]) > PrismOptimizationEpsilon)
    {
      return std::nullopt;
    }
  }

  auto footprint =
    brush.face(*positiveFaceIndex).vertexPositions()
    | std::views::transform([&](const auto& point) { return projectPoint(point, axis); })
    | kdl::ranges::to<std::vector>();
  footprint = convexHull(std::move(footprint));
  if (footprint.size() < 3u || polygonArea(footprint) <= PrismOptimizationEpsilon)
  {
    return std::nullopt;
  }

  const auto& bounds = brush.bounds();
  return ConvexPrismPiece{
    axis, bounds.min[axis], bounds.max[axis], std::move(footprint), {sourceIndex}};
}

std::optional<std::vector<ConvexPrismPiece>> extractCommonConvexPrisms(
  const std::vector<BrushNode*>& brushNodes)
{
  for (size_t axis = 0u; axis < 3u; ++axis)
  {
    auto pieces = std::vector<ConvexPrismPiece>{};
    for (size_t index = 0u; index < brushNodes.size(); ++index)
    {
      auto piece = extractConvexPrism(brushNodes[index]->brush(), axis, index);
      if (!piece)
      {
        pieces.clear();
        break;
      }
      pieces.push_back(std::move(*piece));
    }
    if (pieces.empty())
    {
      continue;
    }

    const auto min = pieces.front().min;
    const auto max = pieces.front().max;
    if (std::ranges::any_of(pieces, [&](const auto& piece) {
          return !vm::is_equal(piece.min, min, PrismOptimizationEpsilon)
                 || !vm::is_equal(piece.max, max, PrismOptimizationEpsilon);
        }))
    {
      continue;
    }

    auto overlaps = false;
    for (size_t i = 0u; i < pieces.size() && !overlaps; ++i)
    {
      for (size_t j = i + 1u; j < pieces.size(); ++j)
      {
        if (
          polygonArea(intersectConvexPolygons(pieces[i].footprint, pieces[j].footprint))
          > PrismOptimizationEpsilon)
        {
          overlaps = true;
          break;
        }
      }
    }
    if (!overlaps)
    {
      return pieces;
    }
  }
  return std::nullopt;
}

std::optional<ConvexPrismPiece> mergePrismPieces(
  const ConvexPrismPiece& lhs, const ConvexPrismPiece& rhs)
{
  const auto sharedLength = sharedBoundaryLength(lhs.footprint, rhs.footprint);
  if (sharedLength <= PrismOptimizationEpsilon)
  {
    return std::nullopt;
  }

  auto points = lhs.footprint;
  kdl::vec_append(points, rhs.footprint);
  auto footprint = convexHull(std::move(points));
  const auto sourceArea = polygonArea(lhs.footprint) + polygonArea(rhs.footprint);
  const auto mergedArea = polygonArea(footprint);
  const auto areaTolerance =
    PrismOptimizationEpsilon * std::max(1.0, std::sqrt(sourceArea));
  if (std::abs(mergedArea - sourceArea) > areaTolerance)
  {
    return std::nullopt;
  }

  auto sourceIndices = lhs.sourceIndices;
  kdl::vec_append(sourceIndices, rhs.sourceIndices);
  std::ranges::sort(sourceIndices);
  return ConvexPrismPiece{
    lhs.axis, lhs.min, lhs.max, std::move(footprint), std::move(sourceIndices)};
}

struct PrismMerge
{
  size_t lhs;
  size_t rhs;
  double sharedLength;
  ConvexPrismPiece piece;
};

std::vector<ConvexPrismPiece> greedilyMergePrisms(
  std::vector<ConvexPrismPiece> pieces, const uint32_t strategy)
{
  auto random = std::mt19937{strategy};
  while (true)
  {
    auto merges = std::vector<PrismMerge>{};
    for (size_t lhs = 0u; lhs < pieces.size(); ++lhs)
    {
      for (size_t rhs = lhs + 1u; rhs < pieces.size(); ++rhs)
      {
        if (auto piece = mergePrismPieces(pieces[lhs], pieces[rhs]))
        {
          merges.push_back(
            {lhs,
             rhs,
             sharedBoundaryLength(pieces[lhs].footprint, pieces[rhs].footprint),
             std::move(*piece)});
        }
      }
    }
    if (merges.empty())
    {
      return pieces;
    }

    std::ranges::shuffle(merges, random);
    const auto score = [&](const auto& merge) {
      const auto area = polygonArea(merge.piece.footprint);
      const auto vertexCount = merge.piece.footprint.size();
      switch (strategy % 4u)
      {
      case 0u:
        return merge.sharedLength;
      case 1u:
        return area;
      case 2u:
        return -static_cast<double>(vertexCount);
      case 3u:
        return area / static_cast<double>(vertexCount);
      }
      switchDefault();
      return 0.0;
    };
    const auto best = std::ranges::max_element(merges, {}, score);
    auto next = std::vector<ConvexPrismPiece>{};
    next.reserve(pieces.size() - 1u);
    for (size_t index = 0u; index < pieces.size(); ++index)
    {
      if (index != best->lhs && index != best->rhs)
      {
        next.push_back(std::move(pieces[index]));
      }
    }
    next.push_back(std::move(best->piece));
    pieces = std::move(next);
  }
}

bool samePrismDecomposition(
  const std::vector<ConvexPrismPiece>& lhs, const std::vector<ConvexPrismPiece>& rhs)
{
  if (lhs.size() != rhs.size())
  {
    return false;
  }
  const auto pieceKey = [](const auto& piece) {
    const auto centroid =
      std::accumulate(piece.footprint.begin(), piece.footprint.end(), vm::vec2d::zero())
      / static_cast<double>(piece.footprint.size());
    return std::tuple{centroid.x(), centroid.y(), polygonArea(piece.footprint)};
  };
  auto sortedLhs = lhs;
  auto sortedRhs = rhs;
  std::ranges::sort(sortedLhs, {}, pieceKey);
  std::ranges::sort(sortedRhs, {}, pieceKey);
  for (size_t index = 0u; index < sortedLhs.size(); ++index)
  {
    if (
      sortedLhs[index].footprint.size() != sortedRhs[index].footprint.size()
      || std::abs(
           polygonArea(sortedLhs[index].footprint)
           - polygonArea(sortedRhs[index].footprint))
           > PrismOptimizationEpsilon)
    {
      return false;
    }
    const auto lhsHull = convexHull(sortedLhs[index].footprint);
    const auto rhsHull = convexHull(sortedRhs[index].footprint);
    if (!std::ranges::equal(lhsHull, rhsHull, pointsEqual))
    {
      return false;
    }
  }
  return true;
}

bool prismFaceAttributesAreCompatible(
  const Brush& candidate,
  const ConvexPrismPiece& piece,
  const std::vector<BrushNode*>& sourceNodes)
{
  for (const auto& candidateFace : candidate.faces())
  {
    const BrushFace* referenceFace = nullptr;
    for (const auto sourceIndex : piece.sourceIndices)
    {
      const auto& sourceBrush = sourceNodes[sourceIndex]->brush();
      const auto sourceFaceIndex = sourceBrush.findFace(candidateFace.boundary());
      if (!sourceFaceIndex)
      {
        continue;
      }
      const auto& sourceFace = sourceBrush.face(*sourceFaceIndex);
      if (referenceFace == nullptr)
      {
        referenceFace = &sourceFace;
      }
      else if (!compatibleVisibleFaceAttributes(*referenceFace, sourceFace))
      {
        return false;
      }
    }
    if (referenceFace == nullptr)
    {
      return false;
    }
  }
  return true;
}

double prismInternalFaceArea(const std::vector<ConvexPrismPiece>& pieces)
{
  auto result = 0.0;
  for (size_t lhs = 0u; lhs < pieces.size(); ++lhs)
  {
    for (size_t rhs = lhs + 1u; rhs < pieces.size(); ++rhs)
    {
      result += sharedBoundaryLength(pieces[lhs].footprint, pieces[rhs].footprint)
                * (pieces[lhs].max - pieces[lhs].min);
    }
  }
  return result;
}

std::optional<BrushOptimizationCandidate> makePrismCandidate(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<BrushNode*>& sourceNodes,
  const std::vector<ConvexPrismPiece>& pieces)
{
  const auto builder = BrushBuilder{mapFormat, worldBounds};
  auto brushes = std::vector<Brush>{};
  brushes.reserve(pieces.size());
  for (const auto& piece : pieces)
  {
    auto points = std::vector<vm::vec3d>{};
    points.reserve(piece.footprint.size() * 2u);
    for (const auto& point : piece.footprint)
    {
      points.push_back(unprojectPoint(point, piece.axis, piece.min));
      points.push_back(unprojectPoint(point, piece.axis, piece.max));
    }

    // cloneFaceAttributesFrom below replaces all face attributes. The initial material
    // only lets BrushBuilder construct the candidate without a Map instance.
    auto brushResult = builder.createBrush(points, "__tb_optimized_brush__");
    if (brushResult.is_error())
    {
      return std::nullopt;
    }
    auto brush = std::move(brushResult).value();
    if (!prismFaceAttributesAreCompatible(brush, piece, sourceNodes))
    {
      return std::nullopt;
    }
    auto sourceBrushes = piece.sourceIndices
                         | std::views::transform([&](const auto sourceIndex) {
                             return &sourceNodes[sourceIndex]->brush();
                           })
                         | kdl::ranges::to<std::vector>();
    brush.cloneFaceAttributesFrom(sourceBrushes);
    brushes.push_back(std::move(brush));
  }

  return BrushOptimizationCandidate{
    {}, prismInternalFaceArea(pieces), std::move(brushes)};
}

std::vector<BrushOptimizationCandidate> createConvexPrismOptimizationCandidates(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<BrushNode*>& sourceNodes,
  const std::vector<ConvexPrismPiece>& inputPieces)
{
  auto decompositions = std::vector<std::vector<ConvexPrismPiece>>{};
  for (uint32_t strategy = 0u; strategy < PrismOptimizationStrategyCount; ++strategy)
  {
    auto pieces = greedilyMergePrisms(inputPieces, strategy);
    if (
      pieces.size() >= inputPieces.size()
      || std::ranges::any_of(decompositions, [&](const auto& existing) {
           return samePrismDecomposition(existing, pieces);
         }))
    {
      continue;
    }
    decompositions.push_back(std::move(pieces));
  }

  auto candidates = std::vector<BrushOptimizationCandidate>{};
  for (const auto& decomposition : decompositions)
  {
    if (
      auto candidate =
        makePrismCandidate(mapFormat, worldBounds, sourceNodes, decomposition))
    {
      candidates.push_back(std::move(*candidate));
    }
  }
  std::ranges::sort(candidates, [](const auto& lhs, const auto& rhs) {
    return std::tuple{lhs.brushCount(), lhs.internalFaceArea}
           < std::tuple{rhs.brushCount(), rhs.internalFaceArea};
  });
  return candidates;
}

} // namespace

bool canOptimizeBrushes(const std::vector<BrushNode*>& brushNodes)
{
  if (brushNodes.size() < 2u)
  {
    return false;
  }

  const auto* parent = brushNodes.front()->parent();
  if (std::ranges::any_of(
        brushNodes, [&](const auto* brushNode) { return brushNode->parent() != parent; }))
  {
    return false;
  }

  return std::ranges::all_of(
           brushNodes,
           [](const auto* brushNode) { return isAxisAlignedCuboid(brushNode->brush()); })
         || extractCommonConvexPrisms(brushNodes).has_value();
}

namespace
{

std::vector<BrushOptimizationCandidate> createExactBrushOptimizationCandidates(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<BrushNode*>& brushNodes)
{
  if (!canOptimizeBrushes(brushNodes))
  {
    return {};
  }

  if (std::ranges::all_of(brushNodes, [](const auto* brushNode) {
        return isAxisAlignedCuboid(brushNode->brush());
      }))
  {
    const auto inputBounds = brushNodes
                             | std::views::transform([](const auto* brushNode) {
                                 return brushNode->brush().bounds();
                               })
                             | kdl::ranges::to<std::vector>();
    auto candidates = createBrushOptimizationCandidates(inputBounds);
    std::erase_if(candidates, [&](const auto& candidate) {
      return !preservesVisibleFaceAttributes(brushNodes, candidate);
    });
    return candidates;
  }

  const auto inputPieces = extractCommonConvexPrisms(brushNodes);
  return inputPieces ? createConvexPrismOptimizationCandidates(
                         mapFormat, worldBounds, brushNodes, *inputPieces)
                     : std::vector<BrushOptimizationCandidate>{};
}

std::vector<Brush> candidateBrushes(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<BrushNode*>& sourceNodes,
  const BrushOptimizationCandidate& candidate)
{
  if (!candidate.brushes.empty())
  {
    return candidate.brushes;
  }

  const auto sourceBrushes =
    sourceNodes | std::views::transform([](const auto* node) { return &node->brush(); })
    | kdl::ranges::to<std::vector>();
  const auto builder = BrushBuilder{mapFormat, worldBounds};
  auto result = std::vector<Brush>{};
  result.reserve(candidate.bounds.size());
  for (const auto& bounds : candidate.bounds)
  {
    auto brushResult = builder.createCuboid(bounds, "__tb_optimized_brush__");
    if (brushResult.is_error())
    {
      return {};
    }
    auto brush = std::move(brushResult).value();
    brush.cloneFaceAttributesFrom(sourceBrushes);
    result.push_back(std::move(brush));
  }
  return result;
}

std::vector<std::vector<BrushNode*>> findExactBrushOptimizationCohorts(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<BrushNode*>& brushNodes)
{
  auto parents = std::vector<size_t>(brushNodes.size());
  std::iota(parents.begin(), parents.end(), 0u);
  const auto findRoot = [&](auto&& self, const size_t index) -> size_t {
    if (parents[index] != index)
    {
      parents[index] = self(self, parents[index]);
    }
    return parents[index];
  };
  const auto join = [&](const size_t lhs, const size_t rhs) {
    const auto lhsRoot = findRoot(findRoot, lhs);
    const auto rhsRoot = findRoot(findRoot, rhs);
    if (lhsRoot != rhsRoot)
    {
      parents[rhsRoot] = lhsRoot;
    }
  };

  for (size_t lhs = 0u; lhs < brushNodes.size(); ++lhs)
  {
    for (size_t rhs = lhs + 1u; rhs < brushNodes.size(); ++rhs)
    {
      if (
        brushNodes[lhs]->parent() == brushNodes[rhs]->parent()
        && !createExactBrushOptimizationCandidates(
              mapFormat, worldBounds, std::vector{brushNodes[lhs], brushNodes[rhs]})
              .empty())
      {
        join(lhs, rhs);
      }
    }
  }

  auto cohortsByRoot = std::vector<std::vector<BrushNode*>>(brushNodes.size());
  for (size_t index = 0u; index < brushNodes.size(); ++index)
  {
    cohortsByRoot[findRoot(findRoot, index)].push_back(brushNodes[index]);
  }

  auto cohorts = std::vector<std::vector<BrushNode*>>{};
  for (auto& cohort : cohortsByRoot)
  {
    if (
      cohort.size() >= 2u
      && !createExactBrushOptimizationCandidates(mapFormat, worldBounds, cohort).empty())
    {
      cohorts.push_back(std::move(cohort));
    }
  }
  return cohorts;
}

std::optional<std::vector<size_t>> findMatchingBrushSubset(
  const std::vector<Brush>& candidate,
  const std::vector<const BrushNode*>& resultNodes,
  const std::vector<bool>& alreadyMatched)
{
  auto matched = alreadyMatched;
  auto indices = std::vector<size_t>{};
  const auto sameBrushContents = [](const Brush& lhs, const Brush& rhs) {
    constexpr auto floatSerializationEpsilon = 0.011f;
    constexpr auto doubleSerializationEpsilon = 0.011;
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
      const auto& lhsUv = lhsFace.uvAttributes();
      const auto& rhsUv = rhsFace.uvAttributes();
      return lhsFace.materialName() == rhsFace.materialName()
             && lhsFace.surfaceAttributes() == rhsFace.surfaceAttributes()
             && vm::is_equal(lhsUv.offset, rhsUv.offset, floatSerializationEpsilon)
             && vm::is_equal(lhsUv.scale, rhsUv.scale, floatSerializationEpsilon)
             && vm::is_equal(lhsUv.rotation, rhsUv.rotation, floatSerializationEpsilon)
             && vm::is_equal(lhsFace.uAxis(), rhsFace.uAxis(), doubleSerializationEpsilon)
             && vm::is_equal(
               lhsFace.vAxis(), rhsFace.vAxis(), doubleSerializationEpsilon);
    });
  };
  for (const auto& brush : candidate)
  {
    auto index = resultNodes.size();
    for (size_t resultIndex = 0u; resultIndex < resultNodes.size(); ++resultIndex)
    {
      if (
        !matched[resultIndex]
        && sameBrushContents(brush, resultNodes[resultIndex]->brush()))
      {
        index = resultIndex;
        break;
      }
    }
    if (index == resultNodes.size())
    {
      return std::nullopt;
    }
    matched[index] = true;
    indices.push_back(index);
  }
  return indices;
}

} // namespace

std::vector<BrushOptimizationCandidate> createBrushOptimizationCandidates(
  const Map& map, const std::vector<BrushNode*>& brushNodes)
{
  return createExactBrushOptimizationCandidates(
    map.worldNode().mapFormat(), map.worldBounds(), brushNodes);
}

bool isBrushOptimizationResult(
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  const std::vector<const BrushNode*>& sourceNodes,
  const std::vector<const BrushNode*>& resultNodes)
{
  if (sourceNodes.size() < 2u || resultNodes.size() >= sourceNodes.size())
  {
    return false;
  }

  const auto source =
    sourceNodes
    | std::views::transform([](const auto* node) { return const_cast<BrushNode*>(node); })
    | kdl::ranges::to<std::vector>();
  const auto cohorts = findExactBrushOptimizationCohorts(mapFormat, worldBounds, source);
  const auto cohortSourceCount = std::accumulate(
    cohorts.begin(), cohorts.end(), size_t{0u}, [](const auto sum, const auto& cohort) {
      return sum + cohort.size();
    });
  if (cohortSourceCount != source.size())
  {
    return false;
  }

  auto matchedResults = std::vector<bool>(resultNodes.size(), false);
  for (const auto& cohort : cohorts)
  {
    auto found = false;
    for (const auto& candidate :
         createExactBrushOptimizationCandidates(mapFormat, worldBounds, cohort))
    {
      const auto brushes = candidateBrushes(mapFormat, worldBounds, cohort, candidate);
      if (
        const auto indices =
          findMatchingBrushSubset(brushes, resultNodes, matchedResults))
      {
        for (const auto index : *indices)
        {
          matchedResults[index] = true;
        }
        found = true;
        break;
      }
    }
    if (!found)
    {
      return false;
    }
  }
  return std::ranges::all_of(matchedResults, std::identity{});
}

std::vector<std::vector<BrushNode*>> findBrushOptimizationCohorts(
  const Map& map, const std::vector<BrushNode*>& brushNodes)
{
  return findExactBrushOptimizationCohorts(
    map.worldNode().mapFormat(), map.worldBounds(), brushNodes);
}

bool applyBrushOptimizationCandidate(
  Map& map,
  const std::vector<BrushNode*>& brushNodes,
  const BrushOptimizationCandidate& candidate)
{
  if (!canOptimizeBrushes(brushNodes) || candidate.brushCount() == 0u)
  {
    return false;
  }

  auto brushes = candidate.brushes;
  if (brushes.empty())
  {
    const auto sourceBrushes =
      brushNodes
      | std::views::transform([](const auto* brushNode) { return &brushNode->brush(); })
      | kdl::ranges::to<std::vector>();
    const auto builder = BrushBuilder{
      map.worldNode().mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
      map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
    brushes.reserve(candidate.bounds.size());
    for (const auto& bounds : candidate.bounds)
    {
      const auto createSuccess = builder.createCuboid(bounds, map.currentMaterialName())
                                 | kdl::transform([&](auto brush) {
                                     brush.cloneFaceAttributesFrom(sourceBrushes);
                                     brushes.push_back(std::move(brush));
                                   })
                                 | kdl::transform_error([&](const auto& e) {
                                     map.logger().error()
                                       << "Could not create optimized brush: " << e.msg;
                                   })
                                 | kdl::is_success();
      if (!createSuccess)
      {
        return false;
      }
    }
  }

  auto nodesToAdd = brushes | std::views::transform([](auto&& brush) -> Node* {
                      return new BrushNode{std::move(brush)};
                    })
                    | kdl::ranges::to<std::vector>();
  auto& parent = *brushNodes.front()->parent();

  deselectAll(map);
  const auto addedNodes = addNodes(map, {{&parent, nodesToAdd}});
  if (addedNodes.size() != nodesToAdd.size())
  {
    return false;
  }
  removeNodes(map, kdl::vec_static_cast<Node*>(brushNodes));
  selectNodes(map, addedNodes);
  return true;
}

bool canOptimizeSelectedBrushes(const Map& map)
{
  const auto& selection = map.selection();
  return selection.hasOnlyBrushes() && canOptimizeBrushes(selection.brushes);
}

std::vector<BrushOptimizationCandidate> createSelectedBrushOptimizationCandidates(
  const Map& map)
{
  return canOptimizeSelectedBrushes(map)
           ? createBrushOptimizationCandidates(map, map.selection().brushes)
           : std::vector<BrushOptimizationCandidate>{};
}

bool applyBrushOptimizationCandidate(
  Map& map, const BrushOptimizationCandidate& candidate)
{
  return canOptimizeSelectedBrushes(map)
         && applyBrushOptimizationCandidate(map, map.selection().brushes, candidate);
}

namespace
{

constexpr auto GeneratedGeometryEpsilon = 0.01;

using EdgeChain = std::vector<vm::vec3d>;

size_t findOrAddVertex(std::vector<vm::vec3d>& vertices, const vm::vec3d& vertex)
{
  const auto it = std::ranges::find_if(vertices, [&](const auto& existing) {
    return vm::squared_distance(existing, vertex)
           <= GeneratedGeometryEpsilon * GeneratedGeometryEpsilon;
  });
  if (it != vertices.end())
  {
    return static_cast<size_t>(std::distance(vertices.begin(), it));
  }

  vertices.push_back(vertex);
  return vertices.size() - 1u;
}

std::optional<std::array<EdgeChain, 2u>> selectedEdgeChains(const Map& map)
{
  const auto edgePositions =
    EdgeHandle::getPositions(map.nodeHandles().selectedHandles<EdgeHandle>());
  if (edgePositions.size() < 2u)
  {
    return std::nullopt;
  }

  auto vertices = std::vector<vm::vec3d>{};
  auto edges = std::vector<std::array<size_t, 2u>>{};
  for (const auto& edge : edgePositions)
  {
    const auto start = findOrAddVertex(vertices, edge.start());
    const auto end = findOrAddVertex(vertices, edge.end());
    if (start == end)
    {
      return std::nullopt;
    }
    edges.push_back({start, end});
  }

  auto adjacency = std::vector<std::vector<size_t>>(vertices.size());
  for (const auto& [start, end] : edges)
  {
    adjacency[start].push_back(end);
    adjacency[end].push_back(start);
  }

  auto chains = std::vector<EdgeChain>{};
  auto visited = std::vector<bool>(vertices.size(), false);
  for (size_t seed = 0u; seed < vertices.size(); ++seed)
  {
    if (visited[seed] || adjacency[seed].empty())
    {
      continue;
    }

    auto component = std::vector<size_t>{};
    auto stack = std::vector<size_t>{seed};
    visited[seed] = true;
    while (!stack.empty())
    {
      const auto vertex = stack.back();
      stack.pop_back();
      component.push_back(vertex);
      for (const auto neighbor : adjacency[vertex])
      {
        if (!visited[neighbor])
        {
          visited[neighbor] = true;
          stack.push_back(neighbor);
        }
      }
    }

    const auto endpoints = component | std::views::filter([&](const auto vertex) {
                             return adjacency[vertex].size() == 1u;
                           })
                           | kdl::ranges::to<std::vector>();
    if (endpoints.size() != 2u || std::ranges::any_of(component, [&](const auto vertex) {
          return adjacency[vertex].size() > 2u;
        }))
    {
      return std::nullopt;
    }

    auto current = std::min(endpoints[0], endpoints[1]);
    auto previous = vertices.size();
    auto chain = EdgeChain{};
    while (true)
    {
      chain.push_back(vertices[current]);
      const auto next = std::ranges::find_if(
        adjacency[current], [&](const auto neighbor) { return neighbor != previous; });
      if (next == adjacency[current].end())
      {
        break;
      }
      previous = std::exchange(current, *next);
    }
    if (chain.size() != component.size())
    {
      return std::nullopt;
    }
    chains.push_back(std::move(chain));
  }

  if (chains.size() != 2u)
  {
    return std::nullopt;
  }

  const auto forwardDistance = vm::squared_distance(chains[0].front(), chains[1].front())
                               + vm::squared_distance(chains[0].back(), chains[1].back());
  const auto reversedDistance =
    vm::squared_distance(chains[0].front(), chains[1].back())
    + vm::squared_distance(chains[0].back(), chains[1].front());
  if (reversedDistance < forwardDistance)
  {
    std::ranges::reverse(chains[1]);
  }

  return std::array<EdgeChain, 2u>{std::move(chains[0]), std::move(chains[1])};
}

std::optional<vm::vec3d> canonicalPlaneNormal(const std::array<EdgeChain, 2u>& chains)
{
  auto points = chains | std::views::join | kdl::ranges::to<std::vector>();
  const auto& origin = points.front();
  for (size_t i = 1u; i < points.size(); ++i)
  {
    for (size_t j = i + 1u; j < points.size(); ++j)
    {
      auto normal = vm::cross(points[i] - origin, points[j] - origin);
      if (vm::squared_length(normal) <= GeneratedGeometryEpsilon)
      {
        continue;
      }

      normal = vm::normalize(normal);
      const auto dominantAxis = vm::find_abs_max_component(normal);
      if (normal[dominantAxis] < 0.0)
      {
        normal = -normal;
      }
      if (std::ranges::all_of(points, [&](const auto& point) {
            return std::abs(vm::dot(point - origin, normal)) <= GeneratedGeometryEpsilon;
          }))
      {
        return normal;
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}

std::vector<double> chainParameters(const EdgeChain& chain)
{
  auto parameters = std::vector<double>(chain.size(), 0.0);
  for (size_t i = 1u; i < chain.size(); ++i)
  {
    parameters[i] = parameters[i - 1u] + vm::distance(chain[i - 1u], chain[i]);
  }
  const auto totalLength = parameters.back();
  if (totalLength > 0.0)
  {
    for (auto& parameter : parameters)
    {
      parameter /= totalLength;
    }
  }
  return parameters;
}

vm::vec3d pointAtChainParameter(
  const EdgeChain& chain, const std::vector<double>& parameters, const double parameter)
{
  const auto upper = std::ranges::upper_bound(parameters, parameter);
  if (upper == parameters.begin())
  {
    return chain.front();
  }
  if (upper == parameters.end())
  {
    return chain.back();
  }

  const auto upperIndex = static_cast<size_t>(std::distance(parameters.begin(), upper));
  const auto lowerIndex = upperIndex - 1u;
  const auto fraction = (parameter - parameters[lowerIndex])
                        / (parameters[upperIndex] - parameters[lowerIndex]);
  return chain[lowerIndex] + fraction * (chain[upperIndex] - chain[lowerIndex]);
}

double polygonAreaOnPlane(const std::vector<vm::vec3d>& vertices, const vm::vec3d& normal)
{
  auto areaVector = vm::vec3d::zero();
  for (size_t i = 0u; i < vertices.size(); ++i)
  {
    areaVector =
      areaVector + vm::cross(vertices[i], vertices[(i + 1u) % vertices.size()]);
  }
  return std::abs(vm::dot(areaVector, normal)) / 2.0;
}

double triangleAreaOnPlane(
  const vm::vec3d& p1, const vm::vec3d& p2, const vm::vec3d& p3, const vm::vec3d& normal)
{
  return std::abs(vm::dot(vm::cross(p2 - p1, p3 - p1), normal)) / 2.0;
}

void appendConvexBridgeSpan(
  std::vector<std::vector<vm::vec3d>>& result,
  std::vector<vm::vec3d> footprint,
  const vm::vec3d& normal)
{
  if (footprint.size() == 3u || Polyhedron3{footprint}.vertexCount() == footprint.size())
  {
    result.push_back(std::move(footprint));
    return;
  }

  contract_assert(footprint.size() == 4u);
  const auto polygonArea = polygonAreaOnPlane(footprint, normal);
  const auto diagonal02Area =
    triangleAreaOnPlane(footprint[0], footprint[1], footprint[2], normal)
    + triangleAreaOnPlane(footprint[0], footprint[2], footprint[3], normal);
  const auto diagonal13Area =
    triangleAreaOnPlane(footprint[0], footprint[1], footprint[3], normal)
    + triangleAreaOnPlane(footprint[1], footprint[2], footprint[3], normal);

  if (std::abs(diagonal02Area - polygonArea) <= GeneratedGeometryEpsilon)
  {
    result.push_back({footprint[0], footprint[1], footprint[2]});
    result.push_back({footprint[0], footprint[2], footprint[3]});
  }
  else if (std::abs(diagonal13Area - polygonArea) <= GeneratedGeometryEpsilon)
  {
    result.push_back({footprint[0], footprint[1], footprint[3]});
    result.push_back({footprint[1], footprint[2], footprint[3]});
  }
}

std::vector<std::vector<vm::vec3d>> bridgeSpans(
  const std::array<EdgeChain, 2u>& chains, const vm::vec3d& normal)
{
  const auto firstParameters = chainParameters(chains[0]);
  const auto secondParameters = chainParameters(chains[1]);
  auto parameters = firstParameters;
  kdl::vec_append(parameters, secondParameters);
  std::ranges::sort(parameters);
  parameters.erase(
    std::unique(
      parameters.begin(),
      parameters.end(),
      [](const auto lhs, const auto rhs) {
        return std::abs(lhs - rhs) <= vm::Cd::almost_zero();
      }),
    parameters.end());

  auto result = std::vector<std::vector<vm::vec3d>>{};
  result.reserve(parameters.size() - 1u);
  for (size_t i = 1u; i < parameters.size(); ++i)
  {
    const auto lower = parameters[i - 1u];
    const auto upper = parameters[i];
    auto footprint = std::vector<vm::vec3d>{
      pointAtChainParameter(chains[0], firstParameters, lower),
      pointAtChainParameter(chains[0], firstParameters, upper),
      pointAtChainParameter(chains[1], secondParameters, upper),
      pointAtChainParameter(chains[1], secondParameters, lower),
    };
    auto uniqueFootprint = std::vector<vm::vec3d>{};
    for (const auto& point : footprint)
    {
      if (std::ranges::none_of(uniqueFootprint, [&](const auto& existing) {
            return vm::squared_distance(point, existing)
                   <= GeneratedGeometryEpsilon * GeneratedGeometryEpsilon;
          }))
      {
        uniqueFootprint.push_back(point);
      }
    }
    if (uniqueFootprint.size() >= 3u)
    {
      appendConvexBridgeSpan(result, std::move(uniqueFootprint), normal);
    }
  }
  return result;
}

std::optional<std::vector<vm::vec3d>> extremeFaceFootprint(
  const Brush& brush, const vm::axis::type axis, const bool positive)
{
  const auto normal = (positive ? 1.0 : -1.0) * vm::vec3d::axis(axis);
  const auto faceIndex = brush.findFace(normal);
  return faceIndex ? std::optional{brush.face(*faceIndex).vertexPositions()}
                   : std::nullopt;
}

void setEqSurfaceFlags(Brush& brush, const int materialFlags)
{
  for (auto& face : brush.faces())
  {
    const auto orientationFlags = face.normal().z() >= 1.0 - GeneratedGeometryEpsilon ? 1
                                  : face.normal().z() <= -1.0 + GeneratedGeometryEpsilon
                                    ? 4
                                    : 2;
    face.setSurfaceAttributes(
      {.contents = 0, .flags = materialFlags | orientationFlags, .value = 0.0f});
  }
}

Result<Brush> createOffsetPrism(
  const BrushBuilder& builder,
  const std::vector<vm::vec3d>& footprint,
  const vm::vec3d& offset1,
  const vm::vec3d& offset2,
  const std::string& materialName,
  const std::optional<int> materialFlags = std::nullopt)
{
  auto points = std::vector<vm::vec3d>{};
  points.reserve(footprint.size() * 2u);
  for (const auto& point : footprint)
  {
    points.push_back(point + offset1);
    points.push_back(point + offset2);
  }

  return builder.createBrush(points, materialName) | kdl::transform([&](auto brush) {
           if (materialFlags)
           {
             setEqSurfaceFlags(brush, *materialFlags);
           }
           return brush;
         });
}

Result<Brush> createAxisPrism(
  const BrushBuilder& builder,
  const std::vector<vm::vec3d>& footprint,
  const vm::axis::type axis,
  const double targetCoordinate,
  const std::string& materialName,
  const std::optional<int> materialFlags = std::nullopt)
{
  const auto sourceCoordinate = footprint.front()[axis];
  const auto offset = (targetCoordinate - sourceCoordinate) * vm::vec3d::axis(axis);
  return createOffsetPrism(
    builder, footprint, vm::vec3d::zero(), offset, materialName, materialFlags);
}

BrushBuilder geometryBrushBuilder(const Map& map)
{
  return BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
}

} // namespace

bool canBridgeSelectedEdgeChains(const Map& map)
{
  const auto chains = selectedEdgeChains(map);
  return chains && canonicalPlaneNormal(*chains).has_value();
}

bool bridgeSelectedEdgeChains(
  Map& map,
  const double thickness,
  const BridgeSurfaceDirection direction,
  const std::string& materialName)
{
  const auto chains = selectedEdgeChains(map);
  if (!chains || thickness <= 0.0)
  {
    return false;
  }
  const auto normal = canonicalPlaneNormal(*chains);
  if (!normal)
  {
    return false;
  }

  const auto [offset1, offset2] = [&]() {
    switch (direction)
    {
    case BridgeSurfaceDirection::Below:
      return std::tuple{-thickness * *normal, vm::vec3d::zero()};
    case BridgeSurfaceDirection::Above:
      return std::tuple{vm::vec3d::zero(), thickness * *normal};
    case BridgeSurfaceDirection::Centered:
      return std::tuple{-0.5 * thickness * *normal, 0.5 * thickness * *normal};
    }
    switchDefault();
  }();

  auto brushes = std::vector<Brush>{};
  for (const auto& footprint : bridgeSpans(*chains, *normal))
  {
    const auto createSuccess =
      createOffsetPrism(
        geometryBrushBuilder(map), footprint, offset1, offset2, materialName)
      | kdl::transform([&](auto brush) { brushes.push_back(std::move(brush)); })
      | kdl::transform_error([&](const auto& e) {
          map.logger().error() << "Could not bridge edge chains: " << e.msg;
        })
      | kdl::is_success();
    if (!createSuccess)
    {
      return false;
    }
  }
  if (brushes.empty())
  {
    return false;
  }

  auto nodes = brushes | kdl::views::as_rvalue
               | std::views::transform(
                 [](auto&& brush) -> Node* { return new BrushNode{std::move(brush)}; })
               | kdl::ranges::to<std::vector>();
  auto& parent = parentForNodes(map, map.selection().nodes);
  auto transaction = Transaction{map, "Bridge Edge Chains"};
  deselectAll(map);
  if (addNodes(map, {{&parent, nodes}}).size() != nodes.size())
  {
    transaction.cancel();
    return false;
  }
  selectNodes(map, nodes);
  return transaction.commit();
}

bool canCreateVolumeToPlane(const Map& map)
{
  return map.selection().hasBrushFaces() || map.selection().hasOnlyBrushes();
}

bool createVolumeToPlane(
  Map& map,
  const vm::axis::type axis,
  const double coordinate,
  const std::string& materialName)
{
  if (!canCreateVolumeToPlane(map))
  {
    return false;
  }

  auto footprints = std::vector<std::vector<vm::vec3d>>{};
  auto referenceNodes = std::vector<Node*>{};
  if (map.selection().hasBrushFaces())
  {
    for (const auto& handle : map.selection().brushFaces)
    {
      const auto& face = handle.face();
      const auto delta = coordinate - face.center()[axis];
      if (
        std::abs(std::abs(face.normal()[axis]) - 1.0) > GeneratedGeometryEpsilon
        || delta * face.normal()[axis] <= GeneratedGeometryEpsilon)
      {
        return false;
      }
      footprints.push_back(face.vertexPositions());
      referenceNodes.push_back(handle.node());
    }
  }
  else
  {
    for (auto* brushNode : map.selection().brushes)
    {
      const auto& bounds = brushNode->brush().bounds();
      const auto positive = coordinate > bounds.max[axis] + GeneratedGeometryEpsilon;
      const auto negative = coordinate < bounds.min[axis] - GeneratedGeometryEpsilon;
      if (!positive && !negative)
      {
        return false;
      }
      const auto footprint = extremeFaceFootprint(brushNode->brush(), axis, positive);
      if (!footprint)
      {
        return false;
      }
      footprints.push_back(*footprint);
      referenceNodes.push_back(brushNode);
    }
  }

  auto brushes = std::vector<Brush>{};
  for (const auto& footprint : footprints)
  {
    const auto createSuccess =
      createAxisPrism(
        geometryBrushBuilder(map), footprint, axis, coordinate, materialName)
      | kdl::transform([&](auto brush) { brushes.push_back(std::move(brush)); })
      | kdl::transform_error([&](const auto& e) {
          map.logger().error() << "Could not create volume to plane: " << e.msg;
        })
      | kdl::is_success();
    if (!createSuccess)
    {
      return false;
    }
  }

  auto nodes = brushes | kdl::views::as_rvalue
               | std::views::transform(
                 [](auto&& brush) -> Node* { return new BrushNode{std::move(brush)}; })
               | kdl::ranges::to<std::vector>();
  auto& parent = parentForNodes(map, referenceNodes);
  auto transaction = Transaction{map, "Create Volume to Plane"};
  deselectAll(map);
  if (addNodes(map, {{&parent, nodes}}).size() != nodes.size())
  {
    transaction.cancel();
    return false;
  }
  selectNodes(map, nodes);
  return transaction.commit();
}

bool canCreateEqWater(const Map& map)
{
  const auto& selection = map.selection();
  return selection.hasOnlyBrushes() && !selection.brushes.empty()
         && std::ranges::all_of(selection.brushes, [&](const auto* brushNode) {
              return brushNode->entity() == &map.worldNode()
                     && extremeFaceFootprint(brushNode->brush(), vm::axis::z, true)
                          .has_value();
            });
}

bool createEqWater(
  Map& map,
  const double surfaceHeight,
  const double surfaceThickness,
  const std::string& waterMaterialName,
  const std::string& surfaceMaterialName)
{
  if (!canCreateEqWater(map) || surfaceThickness <= 0.0)
  {
    return false;
  }

  const auto sourceNodes = std::vector<BrushNode*>{map.selection().brushes};
  const auto builder = geometryBrushBuilder(map);
  auto waterBrushes = std::vector<Brush>{};
  auto surfaceBrushes = std::vector<Brush>{};
  waterBrushes.reserve(sourceNodes.size());
  surfaceBrushes.reserve(sourceNodes.size());

  for (const auto* sourceNode : sourceNodes)
  {
    const auto footprint = extremeFaceFootprint(sourceNode->brush(), vm::axis::z, true);
    contract_assert(footprint.has_value());
    const auto bottomHeight = footprint->front().z();
    if (surfaceHeight <= bottomHeight + GeneratedGeometryEpsilon)
    {
      return false;
    }

    const auto waterSuccess =
      createAxisPrism(
        builder, *footprint, vm::axis::z, surfaceHeight, waterMaterialName, 0)
      | kdl::transform([&](auto brush) { waterBrushes.push_back(std::move(brush)); })
      | kdl::is_success();
    const auto surfaceSuccess =
      createOffsetPrism(
        builder,
        *footprint,
        (std::max(bottomHeight, surfaceHeight - surfaceThickness) - bottomHeight)
          * vm::vec3d::axis(vm::axis::z),
        (surfaceHeight - bottomHeight) * vm::vec3d::axis(vm::axis::z),
        surfaceMaterialName,
        32)
      | kdl::transform([&](auto brush) { surfaceBrushes.push_back(std::move(brush)); })
      | kdl::is_success();
    if (!waterSuccess || !surfaceSuccess)
    {
      map.logger().error() << "Could not create EQ water geometry";
      return false;
    }
  }

  auto* entityNode = new EntityNode{Entity{{
    {EntityPropertyKeys::Classname, "eq_water"},
    {"types", "Water"},
  }}};
  auto waterNodes = waterBrushes | kdl::views::as_rvalue
                    | std::views::transform([](auto&& brush) -> Node* {
                        return new BrushNode{std::move(brush)};
                      })
                    | kdl::ranges::to<std::vector>();
  auto surfaceNodes = surfaceBrushes | kdl::views::as_rvalue
                      | std::views::transform([](auto&& brush) -> Node* {
                          return new BrushNode{std::move(brush)};
                        })
                      | kdl::ranges::to<std::vector>();

  auto& parent = parentForNodes(map, kdl::vec_static_cast<Node*>(sourceNodes));
  auto transaction = Transaction{map, "Create EQ Water"};
  deselectAll(map);
  if (addNodes(map, {{&parent, {entityNode}}}).empty())
  {
    transaction.cancel();
    return false;
  }
  if (addNodes(map, {{entityNode, waterNodes}, {&parent, surfaceNodes}}).empty())
  {
    transaction.cancel();
    return false;
  }

  auto selectedNodes = waterNodes;
  kdl::vec_append(selectedNodes, surfaceNodes);
  selectNodes(map, selectedNodes);
  return transaction.commit();
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
             map.worldNode().mapFormat(),
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
                        kdl::vec_append(toAddForParent, std::move(resultNodes));
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

bool csgSubtract(
  Map& map,
  const std::vector<BrushNode*>& subtrahendNodes,
  const std::vector<BrushNode*>& minuendNodes,
  std::vector<BrushNode*>& replacementNodes)
{
  if (subtrahendNodes.empty() || minuendNodes.empty())
  {
    return false;
  }

  const auto belongsToMap = [&](const BrushNode* node) {
    return node != nullptr && node->isDescendantOf(map.worldNode());
  };
  const auto hasDuplicates = [](const std::vector<BrushNode*>& nodes) {
    return std::ranges::any_of(
      nodes, [&](const auto* node) { return std::ranges::count(nodes, node) > 1; });
  };
  if (
    !std::ranges::all_of(subtrahendNodes, belongsToMap)
    || !std::ranges::all_of(minuendNodes, belongsToMap) || hasDuplicates(subtrahendNodes)
    || hasDuplicates(minuendNodes)
    || std::ranges::any_of(minuendNodes, [&](const auto* minuendNode) {
         return std::ranges::find(subtrahendNodes, minuendNode) != subtrahendNodes.end();
       }))
  {
    return false;
  }

  const auto subtrahends = subtrahendNodes
                           | std::views::transform([](const auto* subtrahendNode) {
                               return &subtrahendNode->brush();
                             })
                           | kdl::ranges::to<std::vector>();

  // Generate every fragment before changing the map. A failed request can therefore
  // never leave a partially cut target behind.
  auto preparedResults = std::vector<std::pair<BrushNode*, std::vector<Brush>>>{};
  preparedResults.reserve(minuendNodes.size());
  for (auto* minuendNode : minuendNodes)
  {
    auto results = minuendNode->brush().subtract(
      map.worldNode().mapFormat(),
      map.worldBounds(),
      map.currentMaterialName(),
      subtrahends);
    if (std::ranges::any_of(
          results, [](const auto& result) { return result.is_error(); }))
    {
      return false;
    }
    preparedResults.emplace_back(minuendNode, kdl::collect_values(std::move(results)));
  }

  auto toAdd = std::map<Node*, std::vector<Node*>>{};
  auto toRemove = std::vector<Node*>{};
  toRemove.reserve(subtrahendNodes.size() + minuendNodes.size());
  for (auto* subtrahendNode : subtrahendNodes)
  {
    toRemove.push_back(subtrahendNode);
  }
  for (auto* minuendNode : minuendNodes)
  {
    toRemove.push_back(minuendNode);
  }

  auto preparedReplacementNodes = std::vector<BrushNode*>{};
  for (auto& [minuendNode, brushes] : preparedResults)
  {
    auto& nodes = toAdd[minuendNode->parent()];
    for (auto& brush : brushes)
    {
      auto* node = new BrushNode{std::move(brush)};
      nodes.push_back(node);
      preparedReplacementNodes.push_back(node);
    }
  }

  auto transaction = Transaction{map, "CSG Subtract"};
  deselectAll(map);
  auto addedNodes = std::vector<Node*>{};
  if (!toAdd.empty())
  {
    addedNodes = addNodes(map, toAdd);
    if (addedNodes.empty())
    {
      transaction.cancel();
      return false;
    }
  }
  removeNodes(map, toRemove);
  selectNodes(map, addedNodes);
  if (!transaction.commit())
  {
    return false;
  }

  replacementNodes = std::move(preparedReplacementNodes);
  return true;
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
    if (addNodes(map, {{&parentForNodes(map, toRemove), {intersectionNode}}}).empty())
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
                   map.worldNode().mapFormat(),
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
                     kdl::vec_append(toAddForParent, fragmentNodes);
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

namespace
{

constexpr auto PlanarPathSweepEpsilon = 0.000001;

double squaredDistance2d(const vm::vec2d& lhs, const vm::vec2d& rhs)
{
  const auto delta = lhs - rhs;
  return vm::dot(delta, delta);
}

bool samePoint2d(const vm::vec2d& lhs, const vm::vec2d& rhs)
{
  return squaredDistance2d(lhs, rhs) <= PlanarPathSweepEpsilon * PlanarPathSweepEpsilon;
}

std::optional<vm::vec2d> normalizedDirection(const vm::vec2d& start, const vm::vec2d& end)
{
  const auto delta = end - start;
  const auto length = std::sqrt(vm::dot(delta, delta));
  if (length <= PlanarPathSweepEpsilon)
  {
    return std::nullopt;
  }
  return delta / length;
}

std::optional<std::vector<vm::vec2d>> simplifyPlanarPath(
  const std::vector<vm::vec2d>& chain)
{
  auto result = std::vector<vm::vec2d>{};
  result.reserve(chain.size());
  for (const auto& point : chain)
  {
    if (result.empty() || !samePoint2d(result.back(), point))
    {
      result.push_back(point);
    }
  }

  if (result.size() < 2u)
  {
    return std::nullopt;
  }

  // Removing a collinear point can expose another one, so repeat until stable.
  auto removedPoint = true;
  while (removedPoint && result.size() > 2u)
  {
    removedPoint = false;
    for (auto i = size_t{1u}; i + 1u < result.size(); ++i)
    {
      const auto before = normalizedDirection(result[i - 1u], result[i]);
      const auto after = normalizedDirection(result[i], result[i + 1u]);
      if (!before || !after)
      {
        return std::nullopt;
      }
      if (
        std::abs(cross2d(*before, *after)) <= PlanarPathSweepEpsilon
        && vm::dot(*before, *after) > 0.0)
      {
        result.erase(result.begin() + static_cast<std::ptrdiff_t>(i));
        removedPoint = true;
        break;
      }
    }
  }
  return result;
}

std::optional<std::pair<vm::vec2d, vm::vec2d>> planarPathJoin(
  const vm::vec2d& point,
  const vm::vec2d& before,
  const vm::vec2d& after,
  const double halfThickness)
{
  const auto beforeNormal = vm::vec2d{-before.y(), before.x()};
  const auto afterNormal = vm::vec2d{-after.y(), after.x()};
  const auto determinant = cross2d(before, after);
  if (std::abs(determinant) <= PlanarPathSweepEpsilon)
  {
    return std::nullopt;
  }

  const auto intersectOffset = [&](const double side) {
    const auto delta = (afterNormal - beforeNormal) * (side * halfThickness);
    const auto beforeDistance = cross2d(delta, after) / determinant;
    return point + beforeNormal * (side * halfThickness) + before * beforeDistance;
  };
  const auto left = intersectOffset(1.0);
  const auto right = intersectOffset(-1.0);

  // Very acute turns create an unhelpfully long spike. Reject them rather than add
  // geometry that fills a much larger area than its intended centreline.
  const auto maximumMiterLength = 2.0 * halfThickness;
  if (
    squaredDistance2d(left, point) > maximumMiterLength * maximumMiterLength
    || squaredDistance2d(right, point) > maximumMiterLength * maximumMiterLength)
  {
    return std::nullopt;
  }
  return std::pair{left, right};
}

bool finitePoint(const vm::vec2d& point)
{
  return std::isfinite(point.x()) && std::isfinite(point.y());
}

bool pointOnSegment(const vm::vec2d& point, const vm::vec2d& start, const vm::vec2d& end)
{
  return std::abs(cross2d(end - start, point - start)) <= PlanarPathSweepEpsilon
         && point.x() >= std::min(start.x(), end.x()) - PlanarPathSweepEpsilon
         && point.x() <= std::max(start.x(), end.x()) + PlanarPathSweepEpsilon
         && point.y() >= std::min(start.y(), end.y()) - PlanarPathSweepEpsilon
         && point.y() <= std::max(start.y(), end.y()) + PlanarPathSweepEpsilon;
}

bool segmentsIntersect(
  const vm::vec2d& a, const vm::vec2d& b, const vm::vec2d& c, const vm::vec2d& d)
{
  const auto abC = cross2d(b - a, c - a);
  const auto abD = cross2d(b - a, d - a);
  const auto cdA = cross2d(d - c, a - c);
  const auto cdB = cross2d(d - c, b - c);
  if (
    ((abC > PlanarPathSweepEpsilon && abD < -PlanarPathSweepEpsilon)
     || (abC < -PlanarPathSweepEpsilon && abD > PlanarPathSweepEpsilon))
    && ((cdA > PlanarPathSweepEpsilon && cdB < -PlanarPathSweepEpsilon) || (cdA < -PlanarPathSweepEpsilon && cdB > PlanarPathSweepEpsilon)))
  {
    return true;
  }
  return pointOnSegment(a, c, d) || pointOnSegment(b, c, d) || pointOnSegment(c, a, b)
         || pointOnSegment(d, a, b);
}

std::optional<std::vector<vm::vec2d>> simplifyPlanarContour(
  const std::vector<vm::vec2d>& source)
{
  auto result = std::vector<vm::vec2d>{};
  result.reserve(source.size());
  for (const auto& point : source)
  {
    if (!finitePoint(point))
    {
      return std::nullopt;
    }
    if (result.empty() || !samePoint2d(result.back(), point))
    {
      result.push_back(point);
    }
  }
  if (result.size() > 1u && samePoint2d(result.front(), result.back()))
  {
    result.pop_back();
  }
  if (result.size() < 3u)
  {
    return std::nullopt;
  }

  auto removedPoint = true;
  while (removedPoint && result.size() > 3u)
  {
    removedPoint = false;
    for (auto i = size_t{0u}; i < result.size(); ++i)
    {
      const auto previous = (i + result.size() - 1u) % result.size();
      const auto next = (i + 1u) % result.size();
      if (
        std::abs(cross2d(result[i] - result[previous], result[next] - result[i]))
        <= PlanarPathSweepEpsilon)
      {
        result.erase(result.begin() + static_cast<std::ptrdiff_t>(i));
        removedPoint = true;
        break;
      }
    }
  }
  if (std::abs(signedPolygonArea(result)) <= PlanarPathSweepEpsilon)
  {
    return std::nullopt;
  }
  for (auto i = size_t{0u}; i < result.size(); ++i)
  {
    for (auto j = i + 1u; j < result.size(); ++j)
    {
      // Neighbouring edges share their endpoint. Every other intersection makes the
      // contour non-simple, including a touch at a non-neighbouring vertex.
      if (j == i + 1u || (i == 0u && j + 1u == result.size()))
      {
        continue;
      }
      if (segmentsIntersect(
            result[i],
            result[(i + 1u) % result.size()],
            result[j],
            result[(j + 1u) % result.size()]))
      {
        return std::nullopt;
      }
    }
  }
  return result;
}

std::optional<std::vector<vm::vec2d>> snapPlanarContour(
  const std::vector<vm::vec2d>& contour, const double gridSize)
{
  if (!std::isfinite(gridSize) || gridSize <= 0.0)
  {
    return std::nullopt;
  }
  auto snapped = std::vector<vm::vec2d>{};
  snapped.reserve(contour.size());
  for (const auto& point : contour)
  {
    snapped.emplace_back(
      std::round(point.x() / gridSize) * gridSize,
      std::round(point.y() / gridSize) * gridSize);
  }
  const auto simplified = simplifyPlanarContour(snapped);
  return simplified && simplified->size() == contour.size() ? simplified : std::nullopt;
}

std::optional<std::vector<vm::vec2d>> insetPlanarContour(
  const std::vector<vm::vec2d>& contour, const double inset, const double gridSize)
{
  if (inset <= 0.0 || !std::isfinite(inset))
  {
    return std::nullopt;
  }
  const auto orientation = signedPolygonArea(contour) > 0.0 ? 1.0 : -1.0;
  auto result = std::vector<vm::vec2d>{};
  result.reserve(contour.size());
  for (auto i = size_t{0u}; i < contour.size(); ++i)
  {
    const auto previous = (i + contour.size() - 1u) % contour.size();
    const auto before = normalizedDirection(contour[previous], contour[i]);
    const auto after =
      normalizedDirection(contour[i], contour[(i + 1u) % contour.size()]);
    if (!before || !after)
    {
      return std::nullopt;
    }
    const auto determinant = cross2d(*before, *after);
    if (std::abs(determinant) <= PlanarPathSweepEpsilon)
    {
      return std::nullopt;
    }
    const auto beforeNormal = vm::vec2d{-before->y(), before->x()} * orientation;
    const auto afterNormal = vm::vec2d{-after->y(), after->x()} * orientation;
    const auto delta = (afterNormal - beforeNormal) * inset;
    const auto beforeDistance = cross2d(delta, *after) / determinant;
    const auto point = contour[i] + beforeNormal * inset + *before * beforeDistance;
    if (!finitePoint(point))
    {
      return std::nullopt;
    }
    result.push_back(point);
  }
  const auto simple = snapPlanarContour(result, gridSize);
  if (
    !simple || simple->size() != contour.size()
    || signedPolygonArea(*simple) * signedPolygonArea(contour) <= PlanarPathSweepEpsilon)
  {
    return std::nullopt;
  }
  return simple;
}

bool isStrictlyConvex(const std::vector<vm::vec2d>& polygon)
{
  const auto orientation = signedPolygonArea(polygon) > 0.0 ? 1.0 : -1.0;
  for (auto i = size_t{0u}; i < polygon.size(); ++i)
  {
    const auto previous = (i + polygon.size() - 1u) % polygon.size();
    const auto next = (i + 1u) % polygon.size();
    if (
      cross2d(polygon[i] - polygon[previous], polygon[next] - polygon[i]) * orientation
      <= PlanarPathSweepEpsilon)
    {
      return false;
    }
  }
  return true;
}

bool pointInOrOnTriangle(
  const vm::vec2d& point,
  const vm::vec2d& a,
  const vm::vec2d& b,
  const vm::vec2d& c,
  const double orientation)
{
  return cross2d(b - a, point - a) * orientation >= -PlanarPathSweepEpsilon
         && cross2d(c - b, point - b) * orientation >= -PlanarPathSweepEpsilon
         && cross2d(a - c, point - c) * orientation >= -PlanarPathSweepEpsilon;
}

std::optional<std::vector<std::array<vm::vec2d, 3u>>> triangulatePlanarContour(
  const std::vector<vm::vec2d>& contour)
{
  auto remaining = contour;
  const auto orientation = signedPolygonArea(contour) > 0.0 ? 1.0 : -1.0;
  auto result = std::vector<std::array<vm::vec2d, 3u>>{};
  result.reserve(contour.size() - 2u);
  while (remaining.size() > 3u)
  {
    auto foundEar = false;
    for (auto i = size_t{0u}; i < remaining.size(); ++i)
    {
      const auto previous = (i + remaining.size() - 1u) % remaining.size();
      const auto next = (i + 1u) % remaining.size();
      const auto triangle =
        std::array{remaining[previous], remaining[i], remaining[next]};
      if (
        cross2d(triangle[1] - triangle[0], triangle[2] - triangle[1]) * orientation
        <= PlanarPathSweepEpsilon)
      {
        continue;
      }
      const auto containsOtherPoint = [&]() {
        for (auto index = size_t{0u}; index < remaining.size(); ++index)
        {
          if (
            index != previous && index != i && index != next
            && pointInOrOnTriangle(
              remaining[index], triangle[0], triangle[1], triangle[2], orientation))
          {
            return true;
          }
        }
        return false;
      }();
      if (containsOtherPoint)
      {
        continue;
      }
      result.push_back(triangle);
      remaining.erase(remaining.begin() + static_cast<std::ptrdiff_t>(i));
      foundEar = true;
      break;
    }
    if (!foundEar)
    {
      return std::nullopt;
    }
  }
  result.push_back({remaining[0], remaining[1], remaining[2]});
  return result;
}

bool appendPlanarProfilePrism(
  std::vector<PlanarProfileBrushSpec>& result,
  const std::vector<vm::vec2d>& footprint,
  const double bottom,
  const double top,
  const std::string& materialName,
  const std::string& role)
{
  if (
    footprint.size() < 3u || !isStrictlyConvex(footprint) || !std::isfinite(bottom)
    || !std::isfinite(top) || top <= bottom || materialName.empty())
  {
    return false;
  }
  auto points = std::vector<vm::vec3d>{};
  points.reserve(footprint.size() * 2u);
  for (const auto& point : footprint)
  {
    points.emplace_back(point.x(), point.y(), bottom);
  }
  for (const auto& point : footprint)
  {
    points.emplace_back(point.x(), point.y(), top);
  }
  result.push_back({{std::move(points), materialName}, role});
  return true;
}

} // namespace

std::optional<std::vector<BrushCreationSpec>> createPlanarPathSweepBrushSpecs(
  const PlanarPathSweepSpec& spec)
{
  if (
    spec.chains.empty() || spec.top <= spec.bottom || spec.thickness <= 0.0
    || spec.materialName.empty())
  {
    return std::nullopt;
  }

  const auto halfThickness = spec.thickness / 2.0;
  auto result = std::vector<BrushCreationSpec>{};
  for (const auto& sourceChain : spec.chains)
  {
    const auto chain = simplifyPlanarPath(sourceChain);
    if (!chain)
    {
      return std::nullopt;
    }

    auto left = std::vector<vm::vec2d>(chain->size());
    auto right = std::vector<vm::vec2d>(chain->size());
    for (auto i = size_t{0u}; i < chain->size(); ++i)
    {
      if (i == 0u || i + 1u == chain->size())
      {
        const auto direction = i == 0u
                                 ? normalizedDirection((*chain)[0], (*chain)[1])
                                 : normalizedDirection((*chain)[i - 1u], (*chain)[i]);
        if (!direction)
        {
          return std::nullopt;
        }
        const auto normal = vm::vec2d{-direction->y(), direction->x()};
        left[i] = (*chain)[i] + normal * halfThickness;
        right[i] = (*chain)[i] - normal * halfThickness;
      }
      else
      {
        const auto before = normalizedDirection((*chain)[i - 1u], (*chain)[i]);
        const auto after = normalizedDirection((*chain)[i], (*chain)[i + 1u]);
        if (!before || !after)
        {
          return std::nullopt;
        }
        const auto join = planarPathJoin((*chain)[i], *before, *after, halfThickness);
        if (!join)
        {
          return std::nullopt;
        }
        left[i] = join->first;
        right[i] = join->second;
      }
    }

    for (auto i = size_t{0u}; i + 1u < chain->size(); ++i)
    {
      const auto quad = std::array{left[i], right[i], right[i + 1u], left[i + 1u]};
      auto points = std::vector<vm::vec3d>{};
      points.reserve(8u);
      for (const auto& point : quad)
      {
        points.emplace_back(point.x(), point.y(), spec.bottom);
      }
      for (const auto& point : quad)
      {
        points.emplace_back(point.x(), point.y(), spec.top);
      }
      result.push_back({std::move(points), spec.materialName});
    }
  }
  return result.empty()
           ? std::nullopt
           : std::optional<std::vector<BrushCreationSpec>>{std::move(result)};
}

std::optional<std::vector<PlanarProfileBrushSpec>> createPlanarProfileBrushSpecs(
  const PlanarProfileSpec& spec)
{
  const auto sourceContour = simplifyPlanarContour(spec.contour);
  const auto contour = sourceContour ? snapPlanarContour(*sourceContour, spec.gridSize)
                                     : std::optional<std::vector<vm::vec2d>>{};
  if (!contour || (spec.bands.empty() && !spec.core))
  {
    return std::nullopt;
  }

  auto result = std::vector<PlanarProfileBrushSpec>{};
  auto outer = *contour;
  auto previousInset = 0.0;
  for (const auto& band : spec.bands)
  {
    if (
      !std::isfinite(band.inset) || !std::isfinite(band.bottom)
      || !std::isfinite(band.top) || band.top <= band.bottom || band.materialName.empty())
    {
      return std::nullopt;
    }
    const auto inset = std::round(band.inset / spec.gridSize) * spec.gridSize;
    const auto bottom = std::round(band.bottom / spec.gridSize) * spec.gridSize;
    const auto top = std::round(band.top / spec.gridSize) * spec.gridSize;
    if (inset <= previousInset || top <= bottom)
    {
      return std::nullopt;
    }
    const auto inner = insetPlanarContour(*contour, inset, spec.gridSize);
    if (!inner || inner->size() != outer.size())
    {
      return std::nullopt;
    }
    for (auto i = size_t{0u}; i < outer.size(); ++i)
    {
      const auto next = (i + 1u) % outer.size();
      const auto footprint =
        std::vector<vm::vec2d>{outer[i], outer[next], (*inner)[next], (*inner)[i]};
      if (!appendPlanarProfilePrism(
            result, footprint, bottom, top, band.materialName, band.role))
      {
        return std::nullopt;
      }
    }
    outer = *inner;
    previousInset = inset;
  }
  if (spec.core)
  {
    const auto bottom = std::round(spec.core->bottom / spec.gridSize) * spec.gridSize;
    const auto top = std::round(spec.core->top / spec.gridSize) * spec.gridSize;
    if (top <= bottom)
    {
      return std::nullopt;
    }
    const auto triangles = triangulatePlanarContour(outer);
    if (!triangles)
    {
      return std::nullopt;
    }
    for (const auto& triangle : *triangles)
    {
      if (!appendPlanarProfilePrism(
            result,
            {triangle.begin(), triangle.end()},
            bottom,
            top,
            spec.core->materialName,
            spec.core->role))
      {
        return std::nullopt;
      }
    }
  }
  return result.empty()
           ? std::nullopt
           : std::optional<std::vector<PlanarProfileBrushSpec>>{std::move(result)};
}

std::optional<std::vector<PlanarProfileBrushSpec>> createProfileExtrusionBrushSpecs(
  const ProfileExtrusionSpec& spec)
{
  if (
    !std::isfinite(spec.minimum) || !std::isfinite(spec.maximum)
    || spec.maximum <= spec.minimum || !std::isfinite(spec.gridSize)
    || spec.gridSize <= 0.0 || spec.materialName.empty() || spec.role.empty())
  {
    return std::nullopt;
  }

  const auto sourceProfile = simplifyPlanarContour(spec.profile);
  const auto profile = sourceProfile ? snapPlanarContour(*sourceProfile, spec.gridSize)
                                     : std::optional<std::vector<vm::vec2d>>{};
  if (!profile)
  {
    return std::nullopt;
  }

  const auto minimum = std::round(spec.minimum / spec.gridSize) * spec.gridSize;
  const auto maximum = std::round(spec.maximum / spec.gridSize) * spec.gridSize;
  if (maximum <= minimum)
  {
    return std::nullopt;
  }

  const auto makePoints = [&](const std::vector<vm::vec2d>& footprint) {
    auto points = std::vector<vm::vec3d>{};
    points.reserve(footprint.size() * 2u);
    const auto appendPoint = [&](const vm::vec2d& point, const double coordinate) {
      switch (spec.plane)
      {
      case ProfileExtrusionPlane::XY:
        points.emplace_back(point.x(), point.y(), coordinate);
        break;
      case ProfileExtrusionPlane::XZ:
        points.emplace_back(point.x(), coordinate, point.y());
        break;
      case ProfileExtrusionPlane::YZ:
        points.emplace_back(coordinate, point.x(), point.y());
        break;
      }
    };
    for (const auto& point : footprint)
    {
      appendPoint(point, minimum);
    }
    for (const auto& point : footprint)
    {
      appendPoint(point, maximum);
    }
    return points;
  };

  const auto appendPrism = [&](
                             std::vector<PlanarProfileBrushSpec>& result,
                             const std::vector<vm::vec2d>& footprint) {
    if (!isStrictlyConvex(footprint))
    {
      return false;
    }
    result.push_back({{makePoints(footprint), spec.materialName}, spec.role});
    return true;
  };

  auto result = std::vector<PlanarProfileBrushSpec>{};
  if (isStrictlyConvex(*profile))
  {
    if (!appendPrism(result, *profile))
    {
      return std::nullopt;
    }
  }
  else
  {
    const auto triangles = triangulatePlanarContour(*profile);
    if (!triangles)
    {
      return std::nullopt;
    }
    for (const auto& triangle : *triangles)
    {
      if (!appendPrism(result, {triangle.begin(), triangle.end()}))
      {
        return std::nullopt;
      }
    }
  }
  return result.empty()
           ? std::nullopt
           : std::optional<std::vector<PlanarProfileBrushSpec>>{std::move(result)};
}

bool createBrushes(
  Map& map,
  Node& parent,
  const std::vector<BrushCreationSpec>& specs,
  std::vector<BrushNode*>& createdBrushes)
{
  createdBrushes.clear();
  if (specs.empty())
  {
    return false;
  }

  // Construct every brush before changing the map. This keeps a multi-brush request
  // atomic even when one late point set cannot form a convex brush.
  auto brushes = std::vector<Brush>{};
  brushes.reserve(specs.size());
  const auto builder = geometryBrushBuilder(map);
  for (const auto& spec : specs)
  {
    if (spec.points.empty() || spec.materialName.empty())
    {
      return false;
    }
    auto brushResult = builder.createBrush(spec.points, spec.materialName);
    if (brushResult.is_error())
    {
      return false;
    }
    brushes.push_back(std::move(brushResult).value());
  }

  // A brush node's child-admission policy depends only on its node type, so probe it
  // before transferring ownership of any nodes to the map command.
  auto firstNode = std::make_unique<BrushNode>(std::move(brushes.front()));
  if (!parent.canAddChild(*firstNode))
  {
    return false;
  }
  auto nodes = std::vector<Node*>{};
  nodes.reserve(brushes.size());
  nodes.push_back(firstNode.release());
  for (auto brush = std::next(brushes.begin()); brush != brushes.end(); ++brush)
  {
    nodes.push_back(new BrushNode{std::move(*brush)});
  }

  auto transaction = Transaction{map, "Create Brushes"};
  deselectAll(map);
  if (addNodes(map, {{&parent, nodes}}).size() != nodes.size())
  {
    transaction.cancel();
    return false;
  }
  createdBrushes.reserve(nodes.size());
  for (auto* node : nodes)
  {
    createdBrushes.push_back(static_cast<BrushNode*>(node));
  }
  selectNodes(map, nodes);
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
                 map.worldBounds(),
                 *faceIndex,
                 delta,
                 map.editorContext().alignmentLock())
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
