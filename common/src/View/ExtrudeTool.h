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
#include "Model/BrushFaceHandle.h"
#include "Model/HitType.h"
#include "NotifierConnection.h"
#include "View/Tool.h"

#include "kdl/reflection_decl.h"

#include "vm/forward.h"
#include "vm/line.h"
#include "vm/plane.h"
#include "vm/vec.h"

#include <memory>
#include <variant>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class BrushFace;
class Hit;
class Node;
class PickResult;
} // namespace Model

namespace Renderer
{
class Camera;
}

namespace View
{
class Grid;
class MapDocument;
class Selection;

/**
 * Similar to Model::BrushFaceHandle but caches the Brush state at the beginning of the
 * drag. We need this to be able to make decisions about the drag before reverting the
 * transaction.
 */
struct ExtrudeDragHandle
{
  Model::BrushFaceHandle faceHandle;
  Model::Brush brushAtDragStart;

  explicit ExtrudeDragHandle(Model::BrushFaceHandle faceHandle);

  const Model::BrushFace& faceAtDragStart() const;
  vm::vec3 faceNormal() const;

  kdl_reflect_decl(ExtrudeDragHandle, faceHandle);
};

struct ExtrudeDragState
{
  /** The drag handles when the drag started. */
  std::vector<ExtrudeDragHandle> initialDragHandles;
  /** The faces being dragged. */
  std::vector<Model::BrushFaceHandle> currentDragFaces;

  /** Whether or not to create new brushes by splitting the selected brushes. */
  bool splitBrushes = false;
  /** The total drag distance so far. */
  vm::vec3 totalDelta = {0, 0, 0};

  kdl_reflect_decl(
    ExtrudeDragState, initialDragHandles, currentDragFaces, splitBrushes, totalDelta);
};

struct ExtrudeHitData
{
  Model::BrushFaceHandle face;
  std::variant<vm::plane3, vm::line3> dragReference;
  vm::vec3 initialHandlePosition;

  kdl_reflect_decl(ExtrudeHitData, face, dragReference, initialHandlePosition);
};

/**
 * Tool for extruding faces along their normals (Shift+LMB Drag).
 *
 * Also:
 *  - split brushes outward/inward (Ctrl+Shift+LMB Drag)
 *  - move faces (Alt+Shift+LMB Drag, 2D views only)
 */
class ExtrudeTool : public Tool
{
public:
  static const Model::HitType::Type ExtrudeHitType;

private:
  std::weak_ptr<MapDocument> m_document;
  /**
   * Propsed drag handles for the next drag. Should only be accessed when m_dragging is
   * false. This needs to be cached here so that it is shared between multiple views.
   * Otherwise, we cannot show the proposed drag handles in all views.
   */
  std::vector<ExtrudeDragHandle> m_proposedDragHandles;
  bool m_dragging;

  NotifierConnection m_notifierConnection;

public:
  explicit ExtrudeTool(std::weak_ptr<MapDocument> document);

  bool applies() const;

  const Grid& grid() const;

  Model::Hit pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult) const;
  Model::Hit pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult) const;

  /**
   * Returns the current proposed drag handles as per the last call to
   * updateProposedDragHandles.
   */
  const std::vector<ExtrudeDragHandle>& proposedDragHandles() const;

  /**
   * Updates the proposed drag handles according to the given picking result.
   */
  void updateProposedDragHandles(const Model::PickResult& pickResult);

  static std::vector<Model::BrushFaceHandle> getDragFaces(
    const std::vector<ExtrudeDragHandle>& dragHandles);

  void beginExtrude();
  bool extrude(const vm::vec3& faceDelta, ExtrudeDragState& dragState);

  void beginMove();
  bool move(const vm::vec3& delta, ExtrudeDragState& dragState);

  void commit(const ExtrudeDragState& dragState);
  void cancel();

private:
  void connectObservers();
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void selectionDidChange(const Selection& selection);
};
} // namespace View
} // namespace TrenchBroom
