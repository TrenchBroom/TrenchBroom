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

#include "Lasso.h"

#include "FloatType.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"

#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/polygon.h"
#include "vm/segment.h"

namespace TrenchBroom
{
namespace View
{
Lasso::Lasso(
  const Renderer::Camera& camera, const FloatType distance, const vm::vec3& point)
  : m_camera{camera}
  , m_distance{distance}
  , m_start{point}
  , m_cur{point}
{
}

void Lasso::update(const vm::vec3& point)
{
  m_cur = point;
}

bool Lasso::selects(
  const vm::vec3& point, const vm::plane3& plane, const vm::bbox2& box) const
{
  const auto projected = project(point, plane);
  return !vm::is_nan(projected) && box.contains(vm::vec2{projected});
}

bool Lasso::selects(
  const vm::segment3& edge, const vm::plane3& plane, const vm::bbox2& box) const
{
  return selects(edge.center(), plane, box);
}

bool Lasso::selects(
  const vm::polygon3& polygon, const vm::plane3& plane, const vm::bbox2& box) const
{
  return selects(polygon.center(), plane, box);
}

vm::vec3 Lasso::project(const vm::vec3& point, const vm::plane3& plane) const
{
  const auto ray = vm::ray3{m_camera.pickRay(vm::vec3f{point})};
  const auto hitDistance = vm::intersect_ray_plane(ray, plane);
  if (vm::is_nan(hitDistance))
  {
    return vm::vec3::nan();
  }

  const auto hitPoint = vm::point_at_distance(ray, hitDistance);
  return getTransform() * hitPoint;
}

void Lasso::render(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
{
  const auto transform = getTransform();
  const auto [invertible, inverseTransform] = vm::invert(transform);
  assert(invertible);
  unused(invertible);

  const auto box = getBox(transform);
  const auto polygon = std::vector<vm::vec3f>{
    vm::vec3f{inverseTransform * vm::vec3{box.min.x(), box.min.y(), 0.0}},
    vm::vec3f{inverseTransform * vm::vec3{box.min.x(), box.max.y(), 0.0}},
    vm::vec3f{inverseTransform * vm::vec3{box.max.x(), box.max.y(), 0.0}},
    vm::vec3f{inverseTransform * vm::vec3{box.max.x(), box.min.y(), 0.0}},
  };

  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
  renderService.setLineWidth(2.0f);
  renderService.renderPolygonOutline(polygon);

  renderService.setForegroundColor(Color(1.0f, 1.0f, 1.0f, 0.25f));
  renderService.renderFilledPolygon(polygon);
}

vm::plane3 Lasso::getPlane() const
{
  return vm::plane3{
    vm::vec3{m_camera.defaultPoint(static_cast<float>(m_distance))},
    vm::vec3{m_camera.direction()}};
}

vm::mat4x4 Lasso::getTransform() const
{
  return vm::mat4x4{vm::coordinate_system_matrix(
    m_camera.right(),
    m_camera.up(),
    -m_camera.direction(),
    m_camera.defaultPoint(static_cast<float>(m_distance)))};
}

vm::bbox2 Lasso::getBox(const vm::mat4x4& transform) const
{
  const auto start = transform * m_start;
  const auto cur = transform * m_cur;

  const auto min = vm::min(start, cur);
  const auto max = vm::max(start, cur);
  return vm::bbox2{vm::vec2{min}, vm::vec2{max}};
}
} // namespace View
} // namespace TrenchBroom
