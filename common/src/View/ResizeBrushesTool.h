/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/HitType.h"
#include "NotifierConnection.h"
#include "View/Tool.h"

#include <vecmath/forward.h>
#include <vecmath/polygon.h>
#include <vecmath/vec.h>

#include <kdl/reflection_decl.h>

#include <memory>
#include <tuple>
#include <vector>

namespace TrenchBroom {
namespace Model {
class Brush;
class BrushFace;
class BrushFaceHandle;
class BrushNode;
class Hit;
class Node;
class PickResult;
} // namespace Model

namespace Renderer {
class Camera;
}

namespace View {
class MapDocument;
class Selection;

/**
 * Similar to Model::BrushFaceHandle but caches the Brush state at the beginning of the drag.
 * We need this to be able to make decisions about the drag before reverting the transaction.
 */
struct ResizeDragHandle {
  Model::BrushNode* node;
  Model::Brush brushAtDragStart;
  size_t faceIndex;

  explicit ResizeDragHandle(const Model::BrushFaceHandle& handle);

  const Model::BrushFace& faceAtDragStart() const;
  vm::vec3 faceNormal() const;

  kdl_reflect_decl(ResizeDragHandle, node, faceIndex);
};

struct ResizeDragState {
  /** The position at which the drag initiated. */
  vm::vec3 dragOrigin;
  /** The drag handles when the drag started. */
  std::vector<ResizeDragHandle> initialDragHandles;
  /** The faces being dragged. */
  std::vector<Model::BrushFaceHandle> currentDragFaces;

  /** Whether or not to create new brushes by splitting the selected brushes. */
  bool splitBrushes;
  /** The total drag distance so far. */
  vm::vec3 totalDelta;

  kdl_reflect_decl(
    ResizeDragState, dragOrigin, initialDragHandles, currentDragFaces, splitBrushes, totalDelta);
};

/**
 * Tool for extruding faces along their normals (Shift+LMB Drag).
 *
 * Also:
 *  - split brushes outward/inward (Ctrl+Shift+LMB Drag)
 *  - move faces (Alt+Shift+LMB Drag, 2D views only)
 */
class ResizeBrushesTool : public Tool {
public:
  static const Model::HitType::Type Resize3DHitType;
  static const Model::HitType::Type Resize2DHitType;

  using Resize2DHitData = std::vector<Model::BrushFaceHandle>;
  using Resize3DHitData = Model::BrushFaceHandle;

private:
  std::weak_ptr<MapDocument> m_document;
  /**
   * Propsed drag handles for the next drag. Should only be accessed when m_dragging is false.
   * This needs to be cached here so that it is shared between multiple views. Otherwise, we cannot
   * show the proposed drag handles in all views.
   */
  std::vector<ResizeDragHandle> m_proposedDragHandles;
  bool m_dragging;

  NotifierConnection m_notifierConnection;

public:
  explicit ResizeBrushesTool(std::weak_ptr<MapDocument> document);

  bool applies() const;

  Model::Hit pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
  Model::Hit pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult);

  /**
   * Returns the current proposed drag handles as per the last call to updateProposedDragHandles.
   */
  const std::vector<ResizeDragHandle>& proposedDragHandles() const;

  /**
   * Updates the proposed drag handles according to the given picking result.
   */
  void updateProposedDragHandles(const Model::PickResult& pickResult);

  std::optional<ResizeDragState> beginResize(const Model::PickResult& pickResult, bool split);
  bool resize(const vm::ray3& pickRay, const Renderer::Camera& camera, ResizeDragState& dragState);

  std::optional<ResizeDragState> beginMove(const Model::PickResult& pickResult);
  bool move(const vm::ray3& pickRay, const Renderer::Camera& camera, ResizeDragState& dragState);

  void commit(const ResizeDragState& dragState);
  void cancel();

private:
  void connectObservers();
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void selectionDidChange(const Selection& selection);
};
} // namespace View
} // namespace TrenchBroom
