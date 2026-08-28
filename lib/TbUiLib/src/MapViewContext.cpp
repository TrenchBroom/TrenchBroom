/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/MapViewContext.h"

#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Grid.h"
#include "mdl/HitAdapter.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Picking.h"
#include "mdl/PickResult.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"

#include "vm/util.h"

namespace tb::ui
{
namespace
{

MapViewNodeReference makeNodeReference(const mdl::Node& node, const mdl::Map& map)
{
  return {node.pathFrom(map.worldNode()), node.name()};
}

} // namespace

MapViewContext captureMapViewContext(
  const mdl::Map& map, const gl::Camera& camera, const MapViewType viewType)
{
  auto result = MapViewContext{};
  result.document.path = map.path();
  result.document.filename = map.filename();
  result.document.revision = map.modificationCount();
  result.document.modified = map.modified();
  result.viewType = viewType;

  result.camera.orthographicProjection = camera.orthographicProjection();
  result.camera.viewport = camera.viewport();
  result.camera.nearPlane = camera.nearPlane();
  result.camera.farPlane = camera.farPlane();
  result.camera.zoom = camera.zoom();
  result.camera.position = camera.position();
  result.camera.direction = camera.direction();
  result.camera.up = camera.up();
  result.camera.right = camera.right();

  const auto& grid = map.grid();
  result.grid.size = grid.size();
  result.grid.actualSize = grid.actualSize();
  result.grid.snap = grid.snap();
  result.grid.visible = grid.visible();
  result.currentMaterialName = map.currentMaterialName();

  const auto& selection = map.selection();
  result.selectedNodes.reserve(selection.nodes.size());
  for (const auto* node : selection.nodes)
  {
    result.selectedNodes.push_back(makeNodeReference(*node, map));
  }

  result.selectedFaces.reserve(selection.brushFaces.size());
  for (const auto& face : selection.brushFaces)
  {
    result.selectedFaces.push_back(
      {makeNodeReference(*face.node(), map), face.faceIndex()});
  }

  const auto& editorContext = map.editorContext();
  const auto* currentLayer = editorContext.currentLayer();
  const auto layers = map.worldNode().allLayers();
  result.layers.reserve(layers.size());
  for (const auto* layer : layers)
  {
    result.layers.push_back({
      makeNodeReference(*layer, map),
      editorContext.visible(*layer),
      layer == currentLayer,
    });
  }

  return result;
}

MapViewPickResult pickMapView(
  mdl::Map& map, const gl::Camera& camera, const float x, const float y)
{
  const auto ray = vm::ray3d{camera.pickRay(x, y)};
  auto result = MapViewPickResult{ray, {}};
  auto pickResult =
    camera.perspectiveProjection()
      ? mdl::PickResult::byDistance()
      : mdl::PickResult::bySize(vm::find_abs_max_component(ray.direction));
  mdl::pick(map, ray, pickResult);

  result.hits.reserve(pickResult.size());
  for (const auto& hit : pickResult.all())
  {
    auto pickHit = MapViewPickHit{
      hit.type(),
      hit.distance(),
      hit.error(),
      hit.hitPoint(),
      std::nullopt,
      std::nullopt};

    if (const auto face = mdl::hitToFaceHandle(hit))
    {
      pickHit.node = makeNodeReference(*face->node(), map);
      pickHit.faceIndex = face->faceIndex();
    }
    else if (const auto* node = mdl::hitToNode(hit))
    {
      pickHit.node = makeNodeReference(*node, map);
    }

    result.hits.push_back(std::move(pickHit));
  }

  return result;
}

} // namespace tb::ui
