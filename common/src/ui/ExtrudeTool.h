/*
 Copyright (C) 2010 Kristian Duske

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

#include "NotifierConnection.h"
#include "mdl/Brush.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/HitType.h"
#include "ui/Tool.h"

#include "kdl/reflection_decl.h"

#include "vm/line.h"
#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <memory>
#include <variant>
#include <vector>

namespace tb
{
namespace mdl
{
class BrushFace;
class Grid;
class Hit;
class Node;
class PickResult;
struct SelectionChange;
} // namespace mdl

namespace render
{
class Camera;
}

namespace ui
{
class MapDocument;

/**
 * Similar to mdl::BrushFaceHandle but caches the Brush state at the beginning of the
 * drag. We need this to be able to make decisions about the drag before reverting the
 * transaction.
 */
struct ExtrudeDragHandle
{
  mdl::BrushFaceHandle faceHandle;
  mdl::Brush brushAtDragStart;

  explicit ExtrudeDragHandle(mdl::BrushFaceHandle faceHandle);

  const mdl::BrushFace& faceAtDragStart() const;
  vm::vec3d faceNormal() const;

  kdl_reflect_decl(ExtrudeDragHandle, faceHandle);
};

struct ExtrudeDragState
{
  /** The drag handles when the drag started. */
  std::vector<ExtrudeDragHandle> initialDragHandles;
  /** The faces being dragged. */
  std::vector<mdl::BrushFaceHandle> currentDragFaces;

  /** Whether or not to create new brushes by splitting the selected brushes. */
  bool splitBrushes = false;
  /** The total drag distance so far. */
  vm::vec3d totalDelta = {0, 0, 0};

  kdl_reflect_decl(
    ExtrudeDragState, initialDragHandles, currentDragFaces, splitBrushes, totalDelta);
};

struct ExtrudeHitData
{
  mdl::BrushFaceHandle face;
  std::variant<vm::plane3d, vm::line3d> dragReference;
  vm::vec3d initialHandlePosition;

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
  static const mdl::HitType::Type ExtrudeHitType;

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

  const mdl::Grid& grid() const;

  mdl::Hit pick2D(const vm::ray3d& pickRay, const mdl::PickResult& pickResult) const;
  mdl::Hit pick3D(const vm::ray3d& pickRay, const mdl::PickResult& pickResult) const;

  /**
   * Returns the current proposed drag handles as per the last call to
   * updateProposedDragHandles.
   */
  const std::vector<ExtrudeDragHandle>& proposedDragHandles() const;

  /**
   * Updates the proposed drag handles according to the given picking result.
   */
  void updateProposedDragHandles(const mdl::PickResult& pickResult);

  static std::vector<mdl::BrushFaceHandle> getDragFaces(
    const std::vector<ExtrudeDragHandle>& dragHandles);

  void beginExtrude();
  bool extrude(const vm::vec3d& faceDelta, ExtrudeDragState& dragState);

  void beginMove();
  bool move(const vm::vec3d& delta, ExtrudeDragState& dragState);

  void commit(const ExtrudeDragState& dragState);
  void cancel();

private:
  void connectObservers();
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void selectionDidChange(const mdl::SelectionChange& selectionChange);
};
} // namespace ui
} // namespace tb
