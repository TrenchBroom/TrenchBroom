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

#include <array>
#include <optional>
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
    node->accept(kdl::overload(
      [&](
        auto&& thisLambda, WorldNode& worldNode) { worldNode.visitChildren(thisLambda); },
      [&](
        auto&& thisLambda, LayerNode& layerNode) { layerNode.visitChildren(thisLambda); },
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
        return node->accept(kdl::overload(
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
                       return std::make_pair(&brushNode, NodeContents{std::move(brush)});
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

    const auto result = map.executeAndStore(std::make_unique<BrushEdgeCommand>(
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

    const auto result = map.executeAndStore(std::make_unique<BrushFaceCommand>(
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

    const auto result = map.executeAndStore(std::make_unique<BrushVertexCommand>(
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

    const auto result = map.executeAndStore(std::make_unique<BrushVertexCommand>(
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
  const Map& map, const BrushOptimizationCandidate& candidate)
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
        for (const auto* sourceNode : map.selection().brushes)
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

} // namespace

bool canOptimizeSelectedBrushes(const Map& map)
{
  const auto& selection = map.selection();
  if (!selection.hasOnlyBrushes() || selection.brushes.size() < 2u)
  {
    return false;
  }

  const auto* parent = selection.brushes.front()->parent();
  return std::ranges::all_of(selection.brushes, [&](const auto* brushNode) {
    return brushNode->parent() == parent && isAxisAlignedCuboid(brushNode->brush());
  });
}

std::vector<BrushOptimizationCandidate> createSelectedBrushOptimizationCandidates(
  const Map& map)
{
  if (!canOptimizeSelectedBrushes(map))
  {
    return {};
  }

  const auto inputBounds = map.selection().brushes
                           | std::views::transform([](const auto* brushNode) {
                               return brushNode->brush().bounds();
                             })
                           | kdl::ranges::to<std::vector>();
  auto candidates = createBrushOptimizationCandidates(inputBounds);
  std::erase_if(candidates, [&](const auto& candidate) {
    return !preservesVisibleFaceAttributes(map, candidate);
  });
  return candidates;
}

bool applyBrushOptimizationCandidate(
  Map& map, const BrushOptimizationCandidate& candidate)
{
  if (!canOptimizeSelectedBrushes(map) || candidate.bounds.empty())
  {
    return false;
  }

  const auto sourceNodes = std::vector<BrushNode*>{map.selection().brushes};
  const auto sourceBrushes =
    sourceNodes
    | std::views::transform([](const auto* brushNode) { return &brushNode->brush(); })
    | kdl::ranges::to<std::vector>();

  const auto builder = BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaultUvAttributes,
    map.gameInfo().gameConfig.faceAttribsConfig.defaultSurfaceAttributes};
  auto brushes = std::vector<Brush>{};
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

  auto nodesToAdd = brushes | std::views::transform([](auto&& brush) -> Node* {
                      return new BrushNode{std::move(brush)};
                    })
                    | kdl::ranges::to<std::vector>();
  auto& parent = *sourceNodes.front()->parent();

  deselectAll(map);
  const auto addedNodes = addNodes(map, {{&parent, nodesToAdd}});
  if (addedNodes.size() != nodesToAdd.size())
  {
    return false;
  }
  removeNodes(map, kdl::vec_static_cast<Node*>(sourceNodes));
  selectNodes(map, addedNodes);
  return true;
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
