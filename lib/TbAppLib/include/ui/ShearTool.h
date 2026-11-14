/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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

#include "mdl/Hit.h"
#include "mdl/HitType.h"
#include "ui/ScaleTool.h"
#include "ui/Tool.h"

#include "vm/bbox.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <optional>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class Grid;
class PickResult;
} // namespace mdl

namespace ui
{
class MapDocument;

struct ShearDragState
{
  /** The side being dragged. */
  BBoxSide side;
  /** The bounds of the selection when the drag started. */
  vm::bbox3d bboxAtDragStart;
  /** The total drag distance so far. */
  vm::vec3d totalDelta = {0, 0, 0};
};

class ShearTool : public Tool
{
public:
  static const mdl::HitType::Type ShearToolSideHitType;

private:
  MapDocument& m_document;
  bool m_dragging = false;
  mdl::Hit m_pickedSide = mdl::Hit::NoHit;

public:
  explicit ShearTool(MapDocument& document);
  ~ShearTool() override;

  const mdl::Grid& grid() const;

  bool applies() const;

  void pickBackSides(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    mdl::PickResult& pickResult) const;
  void pick2D(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    mdl::PickResult& pickResult) const;
  void pick3D(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    mdl::PickResult& pickResult) const;

public:
  vm::bbox3d bounds() const;

  bool dragging() const;

  void beginShear();
  void commitShear(const ShearDragState& dragState);
  void cancelShear();
  void shearByDelta(const vm::vec3d& delta, ShearDragState& dragState);

  /**
   * Returns the side of the current bounds that was picked by the last call to
   * updatePickedSide, if any.
   */
  std::optional<vm::polygon3f> shearHandle() const;

  void updatePickedSide(const mdl::PickResult& pickResult);
};

} // namespace ui
} // namespace tb
