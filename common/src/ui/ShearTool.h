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

class ShearTool : public Tool
{
public:
  static const mdl::HitType::Type ShearToolSideHitType;

private:
  MapDocument& m_document;
  bool m_resizing = false;
  bool m_constrainVertical = false;
  vm::bbox3d m_bboxAtDragStart;
  mdl::Hit m_dragStartHit = mdl::Hit::NoHit;
  vm::vec3d m_dragCumulativeDelta;

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

  /**
   * If inside a drag, returns the bbox at the start of the drag.
   * Otherwise, returns the current bounds(). for rendering sheared bbox.
   */
  vm::bbox3d bboxAtDragStart() const;

  void startShearWithHit(const mdl::Hit& hit);
  void commitShear();
  void cancelShear();
  void shearByDelta(const vm::vec3d& delta);

  const mdl::Hit& dragStartHit() const;

  vm::mat4x4d bboxShearMatrix() const;
  std::optional<vm::polygon3f> shearHandle() const;

  void updatePickedSide(const mdl::PickResult& pickResult);

  bool constrainVertical() const;
  void setConstrainVertical(bool constrainVertical);
};

} // namespace ui
} // namespace tb
