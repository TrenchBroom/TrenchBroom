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

#include "Lasso.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"

#include "kdl/optional_utils.h"

#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/polygon.h"
#include "vm/segment.h"

namespace TrenchBroom::View
{

Lasso::Lasso(
  const Renderer::Camera& camera, const double distance, const vm::vec3d& point)
  : m_camera{camera}
  , m_distance{distance}
  , m_start{point}
  , m_cur{point}
{
}

void Lasso::update(const vm::vec3d& point)
{
  m_cur = point;
}

bool Lasso::selects(
  const vm::vec3d& point, const vm::plane3d& plane, const vm::bbox2d& box) const
{
  if (const auto projected = project(point, plane))
  {
    return box.contains(vm::vec2d{*projected});
  }
  return false;
}

bool Lasso::selects(
  const vm::segment3d& edge, const vm::plane3d& plane, const vm::bbox2d& box) const
{
  return selects(edge.center(), plane, box);
}

bool Lasso::selects(
  const vm::polygon3d& polygon, const vm::plane3d& plane, const vm::bbox2d& box) const
{
  return selects(polygon.center(), plane, box);
}

std::optional<vm::vec3d> Lasso::project(
  const vm::vec3d& point, const vm::plane3d& plane) const
{
  const auto ray = vm::ray3d{m_camera.pickRay(vm::vec3f{point})};
  return kdl::optional_transform(
    vm::intersect_ray_plane(ray, plane), [&](const auto hitDistance) {
      const auto hitPoint = vm::point_at_distance(ray, hitDistance);
      return getTransform() * hitPoint;
    });
}

void Lasso::render(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
{
  const auto transform = getTransform();
  const auto inverseTransform = vm::invert(transform);

  const auto box = getBox(transform);
  const auto polygon = std::vector{
    vm::vec3f{*inverseTransform * vm::vec3d{box.min.x(), box.min.y(), 0.0}},
    vm::vec3f{*inverseTransform * vm::vec3d{box.min.x(), box.max.y(), 0.0}},
    vm::vec3f{*inverseTransform * vm::vec3d{box.max.x(), box.max.y(), 0.0}},
    vm::vec3f{*inverseTransform * vm::vec3d{box.max.x(), box.min.y(), 0.0}},
  };

  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
  renderService.setLineWidth(2.0f);
  renderService.renderPolygonOutline(polygon);

  renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 0.25f));
  renderService.renderFilledPolygon(polygon);
}

vm::plane3d Lasso::getPlane() const
{
  return vm::plane3d{
    vm::vec3d{m_camera.defaultPoint(static_cast<float>(m_distance))},
    vm::vec3d{m_camera.direction()}};
}

vm::mat4x4d Lasso::getTransform() const
{
  return vm::mat4x4d{vm::coordinate_system_matrix(
    m_camera.right(),
    m_camera.up(),
    -m_camera.direction(),
    m_camera.defaultPoint(static_cast<float>(m_distance)))};
}

vm::bbox2d Lasso::getBox(const vm::mat4x4d& transform) const
{
  const auto start = transform * m_start;
  const auto cur = transform * m_cur;

  const auto min = vm::min(start, cur);
  const auto max = vm::max(start, cur);
  return vm::bbox2d{vm::vec2d{min}, vm::vec2d{max}};
}

} // namespace TrenchBroom::View
