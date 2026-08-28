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

#pragma once

#include "gl/Camera.h"
#include "mdl/Node.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{
/**
 * The identity of a map editor viewport. The values are deliberately semantic rather
 * than tied to a particular map view layout, so they can also describe a separate
 * preview window.
 */
enum class MapViewType
{
  ThreeD,
  XY,
  XZ,
  YZ,
};

struct MapViewCameraContext
{
  bool orthographicProjection = false;
  gl::Camera::Viewport viewport;
  float nearPlane = 0.0f;
  float farPlane = 0.0f;
  float zoom = 1.0f;
  vm::vec3f position;
  vm::vec3f direction;
  vm::vec3f up;
  vm::vec3f right;
};

struct MapViewDocumentContext
{
  std::filesystem::path path;
  std::string filename;
  size_t revision = 0u;
  bool modified = false;
};

struct MapViewGridContext
{
  int size = 0;
  double actualSize = 0.0;
  bool snap = false;
  bool visible = false;
};

/**
 * A reference that is meaningful only for the revision in which it was captured.
 * Node paths intentionally avoid exposing object addresses through the automation
 * boundary.
 */
struct MapViewNodeReference
{
  mdl::NodePath path;
  std::string name;
};

struct MapViewFaceReference
{
  MapViewNodeReference node;
  size_t faceIndex = 0u;
};

struct MapViewLayerContext
{
  MapViewNodeReference node;
  bool visible = false;
  bool current = false;
};

/**
 * A self-contained, semantic description of a single map view. It deliberately does
 * not include QWidget or model pointers, allowing an automation client to inspect one
 * view while the user continues working in another one.
 */
struct MapViewContext
{
  MapViewDocumentContext document;
  MapViewType viewType = MapViewType::ThreeD;
  MapViewCameraContext camera;
  MapViewGridContext grid;
  std::string currentMaterialName;
  std::vector<MapViewNodeReference> selectedNodes;
  std::vector<MapViewFaceReference> selectedFaces;
  std::vector<MapViewLayerContext> layers;
};

struct MapViewPickHit
{
  uint64_t type = 0u;
  double distance = 0.0;
  double error = 0.0;
  vm::vec3d point;
  std::optional<MapViewNodeReference> node;
  std::optional<size_t> faceIndex;
};

struct MapViewPickResult
{
  vm::ray3d ray;
  std::vector<MapViewPickHit> hits;
};

/**
 * Captures map state shared by all map view types. The caller supplies the view type
 * and camera so the result represents the exact viewport used for a screenshot or
 * screen-coordinate pick.
 */
MapViewContext captureMapViewContext(
  const mdl::Map& map, const gl::Camera& camera, MapViewType viewType);

/**
 * Performs the same model pick as a map view without changing mouse, focus, selection,
 * or camera state. Hit ordering follows the view's projection: distance in 3D and
 * projected size in orthographic views.
 */
MapViewPickResult pickMapView(mdl::Map& map, const gl::Camera& camera, float x, float y);

} // namespace ui
} // namespace tb
