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


#include "vm/bbox.h" // IWYU pragma: keep
#include "vm/forward.h"
#include "vm/plane.h" // IWYU pragma: keep

namespace TrenchBroom::Renderer
{
class Camera;
class RenderBatch;
class RenderContext;
} // namespace TrenchBroom::Renderer

namespace TrenchBroom::View
{

class Lasso
{
private:
  const Renderer::Camera& m_camera;
  const double m_distance;
  const vm::vec3d m_start;
  vm::vec3d m_cur;

public:
  Lasso(const Renderer::Camera& camera, double distance, const vm::vec3d& point);

  void update(const vm::vec3d& point);

  template <typename I, typename O>
  void selected(I cur, I end, O out) const
  {
    const auto plane = getPlane();
    const auto box = getBox(getTransform());
    while (cur != end)
    {
      if (selects(*cur, plane, box))
      {
        out = *cur;
      }
      ++cur;
    }
  }

private:
  bool selects(
    const vm::vec3d& point, const vm::plane3d& plane, const vm::bbox2d& box) const;
  bool selects(
    const vm::segment3d& edge, const vm::plane3d& plane, const vm::bbox2d& box) const;
  bool selects(
    const vm::polygon3d& polygon, const vm::plane3d& plane, const vm::bbox2d& box) const;
  std::optional<vm::vec3d> project(
    const vm::vec3d& point, const vm::plane3d& plane) const;

public:
  void render(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;

private:
  vm::plane3d getPlane() const;
  vm::mat4x4d getTransform() const;
  vm::bbox2d getBox(const vm::mat4x4d& transform) const;
};

} // namespace TrenchBroom::View
