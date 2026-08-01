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

#include "render/TextAnchor.h"

#include "vm/bbox.h"
#include "vm/util.h"

namespace tb
{
namespace gl
{
class Camera;
}

namespace render
{
class RenderBatch;
class RenderContext;

/**
 * Anchors a bounds size label ("X: 64") to the midpoint of one edge of the bounds, in a
 * 2D (top-down or side) view. Exposed so its alignment logic can be tested directly.
 */
class SizeTextAnchor2D : public TextAnchor3D
{
private:
  const vm::bbox3d& m_bounds;
  const vm::axis::type m_axis;
  const gl::Camera& m_camera;

public:
  SizeTextAnchor2D(
    const vm::bbox3d& bounds, vm::axis::type axis, const gl::Camera& camera);

private:
  vm::vec3f basePosition() const override;
  TextAlignment::Type alignment() const override;
  vm::vec2f extraOffsets(TextAlignment::Type alignment) const override;
};

/**
 * Anchors a bounds size label ("X: 64") to the midpoint of one edge of the bounds, in a
 * 3D view, picking the edge and alignment that face the camera. Exposed so its
 * positioning logic can be tested directly.
 */
class SizeTextAnchor3D : public TextAnchor3D
{
private:
  const vm::bbox3d& m_bounds;
  const vm::axis::type m_axis;
  const gl::Camera& m_camera;

public:
  SizeTextAnchor3D(
    const vm::bbox3d& bounds, vm::axis::type axis, const gl::Camera& camera);

private:
  vm::vec3f basePosition() const override;
  TextAlignment::Type alignment() const override;
  vm::vec2f extraOffsets(TextAlignment::Type alignment) const override;
};

class SelectionBoundsRenderer
{
private:
  const vm::bbox3d m_bounds;

public:
  explicit SelectionBoundsRenderer(const vm::bbox3d& bounds);

  void render(RenderContext& renderContext, RenderBatch& renderBatch);

private:
  void renderBounds(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderSize(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderSize2D(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderSize3D(RenderContext& renderContext, RenderBatch& renderBatch);
  void renderMinMax(RenderContext& renderContext, RenderBatch& renderBatch);
};

} // namespace render
} // namespace tb
