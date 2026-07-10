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

#include "mdl/HitFilter.h"
#include "ui/RotateHandle.h"

#include "vm/vec.h"

#include <functional>
#include <memory>

namespace tb
{
namespace mdl
{
class Grid;
}

namespace render
{
class RenderBatch;
class RenderContext;
} // namespace render

namespace ui
{
class InputState;
class Tool;
class ToolController;

/**
 * Per-drag state for a ring handle drag. A fresh tracker is created by
 * RotateHandleDelegate::beginRingDrag() when a drag starts, and is owned by the drag
 * delegate for the duration of that one drag. Any snapshot state a tool needs (e.g. the
 * value being dragged from) should be held by the concrete tracker, not by the tool.
 */
class RingDragTracker
{
public:
  virtual ~RingDragTracker();

  virtual void apply(const vm::vec3d& center, const vm::vec3d& axis, double angle) = 0;
  virtual void end() = 0;
  virtual void cancel() = 0;
};

/**
 * Implemented by tools that own a RotateHandle gizmo (a center-point handle plus X/Y/Z
 * rotation rings) and want to reuse the generic ring-drag/move-center ToolController
 * parts below instead of duplicating that wiring.
 */
class RotateHandleDelegate
{
public:
  virtual ~RotateHandleDelegate();

  virtual Tool& tool() = 0;
  virtual const Tool& tool() const = 0;

  virtual const mdl::Grid& grid() const = 0;

  virtual RotateHandle& handle() = 0;

  virtual double handleSnapAngle() const = 0;

  virtual void handleClicked(RotateHandle::HitArea area);

  virtual vm::vec3d handleCenter() const = 0;
  virtual void setHandleCenter(const vm::vec3d& position) = 0;

  virtual std::unique_ptr<RingDragTracker> beginRingDrag() = 0;
};

/**
 * Implemented by tools that own a single freely draggable point handle and want to reuse
 * the generic point-handle ToolController part below instead of duplicating the pick,
 * drag and hover-highlight wiring.
 */
class PointHandleDelegate
{
public:
  virtual ~PointHandleDelegate();

  virtual Tool& tool() = 0;
  virtual const Tool& tool() const = 0;

  virtual const mdl::Grid& grid() const = 0;

  virtual vm::vec3d handlePosition() const = 0;
  virtual void setHandlePosition(const vm::vec3d& position) = 0;

  virtual void renderHighlight(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const = 0;
};

/**
 * Returns whether a mouse-down event should start a point handle drag. Evaluated
 * alongside the hit filter passed to createPointHandlePart, so each call site can keep
 * its own mouse button / modifier key gating rules.
 */
using AcceptPointHandleDrag = std::function<bool(const InputState&)>;

/**
 * Forces the selection guide to be shown while any of the given handle hit types is under
 * the cursor. Shared by the tool controllers that host a RotateHandle so that they don't
 * each re-implement the same ToolController::setRenderOptions logic.
 */
void setHandleRenderOptions(
  const InputState& inputState,
  render::RenderContext& renderContext,
  mdl::HitType::Type handleHitType);

std::unique_ptr<ToolController> createRingHandlePart2D(RotateHandleDelegate& delegate);
std::unique_ptr<ToolController> createRingHandlePart3D(RotateHandleDelegate& delegate);
std::unique_ptr<ToolController> createCenterHandlePart2D(RotateHandleDelegate& delegate);
std::unique_ptr<ToolController> createCenterHandlePart3D(RotateHandleDelegate& delegate);

std::unique_ptr<ToolController> createPointHandlePart(
  PointHandleDelegate& delegate,
  mdl::HitFilter hitFilter,
  AcceptPointHandleDrag acceptDrag);

} // namespace ui
} // namespace tb
