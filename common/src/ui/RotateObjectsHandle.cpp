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

#include "RotateObjectsHandle.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/Hit.h"
#include "render/Camera.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"

#include "vm/intersection.h"
#include "vm/mat_ext.h"
#include "vm/scalar.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <tuple>

namespace tb::ui
{
namespace
{

template <typename T>
std::tuple<vm::vec<T, 3>, vm::vec<T, 3>, vm::vec<T, 3>> computeAxes(
  const vm::vec3d& handlePos, const vm::vec<T, 3>& cameraPos)
{
  const auto viewDir = vm::normalize(vm::vec<T, 3>{handlePos} - cameraPos);
  return vm::is_equal(std::abs(viewDir.z()), T(1), vm::constants<T>::almost_zero())
           ? std::
               tuple{vm::vec<T, 3>{1, 0, 0}, vm::vec<T, 3>{0, 1, 0}, vm::vec<T, 3>{0, 0, -1}}
           : std::tuple{
               viewDir.x() > T(0) ? vm::vec<T, 3>{-1, 0, 0} : vm::vec<T, 3>{1, 0, 0},
               viewDir.y() > T(0) ? vm::vec<T, 3>{0, -1, 0} : vm::vec<T, 3>{0, 1, 0},
               viewDir.z() > T(0) ? vm::vec<T, 3>{0, 0, -1} : vm::vec<T, 3>{0, 0, 1}};
}

} // namespace

const mdl::HitType::Type RotateObjectsHandle::HandleHitType = mdl::HitType::freeType();

RotateObjectsHandle::Handle::Handle(const vm::vec3d& position)
  : m_position{position}
{
}

RotateObjectsHandle::Handle::~Handle() = default;

double RotateObjectsHandle::Handle::scalingFactor(const render::Camera& camera) const
{
  return double(camera.perspectiveScalingFactor(vm::vec3f{m_position}));
}

double RotateObjectsHandle::Handle::majorRadius()
{
  return double(pref(Preferences::RotateHandleRadius));
}

double RotateObjectsHandle::Handle::minorRadius()
{
  return double(pref(Preferences::HandleRadius));
}

mdl::Hit RotateObjectsHandle::Handle::pickCenterHandle(
  const vm::ray3d& pickRay, const render::Camera& camera) const
{
  if (
    const auto distance = camera.pickPointHandle(
      pickRay, m_position, double(pref(Preferences::HandleRadius))))
  {
    const auto hitPoint = vm::point_at_distance(pickRay, *distance);
    return {HandleHitType, *distance, hitPoint, HitArea::Center};
  }
  return mdl::Hit::NoHit;
}

mdl::Hit RotateObjectsHandle::Handle::pickRotateHandle(
  const vm::ray3d& pickRay, const render::Camera& camera, const HitArea area) const
{
  const auto transform = handleTransform(camera, area);

  if (const auto inverse = vm::invert(transform))
  {
    const auto transformedRay = pickRay.transform(*inverse);
    const auto transformedPosition = *inverse * m_position;
    if (
      const auto transformedDistance = vm::intersect_ray_torus(
        transformedRay, transformedPosition, majorRadius(), minorRadius()))
    {
      const auto transformedHitPoint =
        vm::point_at_distance(transformedRay, *transformedDistance);
      const auto hitPoint = transform * transformedHitPoint;
      const auto distance = vm::dot(hitPoint - pickRay.origin, pickRay.direction);
      return {HandleHitType, distance, hitPoint, area};
    }
  }

  return mdl::Hit::NoHit;
}

vm::mat4x4d RotateObjectsHandle::Handle::handleTransform(
  const render::Camera& camera, const HitArea area) const
{
  const auto scalingFactor = this->scalingFactor(camera);
  if (scalingFactor <= double(0))
  {
    return vm::mat4x4d::zero();
  }

  const auto scalingMatrix =
    vm::scaling_matrix(vm::vec3d{scalingFactor, scalingFactor, scalingFactor});
  switch (area)
  {
  case HitArea::XAxis:
    return vm::mat4x4d::rot_90_y_ccw() * scalingMatrix;
  case HitArea::YAxis:
    return vm::mat4x4d::rot_90_x_cw() * scalingMatrix;
  case HitArea::ZAxis:
  case HitArea::Center:
  case HitArea::None:
    return vm::mat4x4d::identity() * scalingMatrix;
    switchDefault();
  }
}

mdl::Hit RotateObjectsHandle::Handle2D::pick(
  const vm::ray3d& pickRay, const render::Camera& camera) const
{
  switch (vm::find_abs_max_component(camera.direction()))
  {
  case vm::axis::x:
    return mdl::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::XAxis));
  case vm::axis::y:
    return mdl::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::YAxis));
  default:
    return mdl::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::ZAxis));
  }
}

mdl::Hit RotateObjectsHandle::Handle2D::pickRotateHandle(
  const vm::ray3d& pickRay, const render::Camera& camera, const HitArea area) const
{
  // Work around imprecision caused by 2D cameras being positioned at map bounds
  // by placing the ray origin on the same plane as the handle itself.
  // Fixes erratic handle selection behaviour at high zoom
  // TODO: This should be removed once we replace the numerically unstable torus
  // intersection code
  auto ray = pickRay;
  switch (area)
  {
  case HitArea::XAxis:
    ray.origin[0] = m_position[0];
    break;

  case HitArea::YAxis:
    ray.origin[1] = m_position[1];
    break;

  case HitArea::ZAxis:
    ray.origin[2] = m_position[2];
    break;

  case HitArea::None:
  case HitArea::Center:
    break;

    switchDefault();
  }

  return Handle::pickRotateHandle(ray, camera, area);
}

void RotateObjectsHandle::Handle2D::renderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  const auto& camera = renderContext.camera();
  if (const auto radius = float(majorRadius() * scalingFactor(renderContext.camera()));
      radius > 0.0f)
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setShowOccludedObjects();

    renderService.setLineWidth(2.0f);
    renderService.setForegroundColor(
      pref(Preferences::axisColor(vm::find_abs_max_component(camera.direction()))));
    renderService.renderCircle(
      vm::vec3f{m_position}, vm::find_abs_max_component(camera.direction()), 64, radius);

    renderService.setForegroundColor(pref(Preferences::HandleColor));
    renderService.renderHandle(vm::vec3f{m_position});
  }
}

void RotateObjectsHandle::Handle2D::renderHighlight(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  RotateObjectsHandle::HitArea area) const
{
  if (const auto radius = float(majorRadius() * scalingFactor(renderContext.camera()));
      radius > 0.0f)
  {
    const auto& camera = renderContext.camera();

    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setShowOccludedObjects();

    switch (area)
    {
    case HitArea::Center:
      renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
      renderService.renderHandleHighlight(vm::vec3f{m_position});
      break;
    case HitArea::XAxis:
    case HitArea::YAxis:
    case HitArea::ZAxis:
      renderService.setLineWidth(3.0f);
      renderService.setForegroundColor(
        pref(Preferences::axisColor(vm::find_abs_max_component(camera.direction()))));
      renderService.renderCircle(
        vm::vec3f{m_position},
        vm::find_abs_max_component(camera.direction()),
        64,
        radius);
      break;
    case HitArea::None:
      break;
      switchDefault();
    }
  }
}

mdl::Hit RotateObjectsHandle::Handle3D::pick(
  const vm::ray3d& pickRay, const render::Camera& camera) const
{
  return mdl::selectClosest(
    pickCenterHandle(pickRay, camera),
    pickRotateHandle(pickRay, camera, HitArea::XAxis),
    pickRotateHandle(pickRay, camera, HitArea::YAxis),
    pickRotateHandle(pickRay, camera, HitArea::ZAxis));
}

void RotateObjectsHandle::Handle3D::renderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
{
  if (const auto radius = float(majorRadius() * scalingFactor(renderContext.camera()));
      radius > 0.0f)
  {
    const auto [xAxis, yAxis, zAxis] =
      computeAxes(m_position, renderContext.camera().position());

    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setShowOccludedObjects();

    renderService.renderCoordinateSystem(
      vm::bbox3f{radius}.translate(vm::vec3f{m_position}));

    renderService.setLineWidth(2.0f);
    renderService.setForegroundColor(pref(Preferences::XAxisColor));
    renderService.renderCircle(
      vm::vec3f{m_position}, vm::axis::x, 64, radius, zAxis, yAxis);
    renderService.setForegroundColor(pref(Preferences::YAxisColor));
    renderService.renderCircle(
      vm::vec3f{m_position}, vm::axis::y, 64, radius, xAxis, zAxis);
    renderService.setForegroundColor(pref(Preferences::ZAxisColor));
    renderService.renderCircle(
      vm::vec3f{m_position}, vm::axis::z, 64, radius, xAxis, yAxis);

    renderService.setForegroundColor(pref(Preferences::HandleColor));
    renderService.renderHandle(vm::vec3f{m_position});
  }
}

void RotateObjectsHandle::Handle3D::renderHighlight(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  RotateObjectsHandle::HitArea area) const
{
  if (const auto radius = float(majorRadius() * scalingFactor(renderContext.camera()));
      radius > 0.0f)
  {
    const auto [xAxis, yAxis, zAxis] =
      computeAxes(m_position, renderContext.camera().position());

    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setShowOccludedObjects();

    switch (area)
    {
    case HitArea::Center:
      renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
      renderService.renderHandleHighlight(vm::vec3f{m_position});
      renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
      renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
      renderService.renderString(
        fmt::format("{}", fmt::streamed(m_position)), vm::vec3f{m_position});
      break;
    case HitArea::XAxis:
      renderService.setForegroundColor(pref(Preferences::XAxisColor));
      renderService.setLineWidth(3.0f);
      renderService.renderCircle(
        vm::vec3f{m_position}, vm::axis::x, 64, radius, zAxis, yAxis);
      break;
    case HitArea::YAxis:
      renderService.setForegroundColor(pref(Preferences::YAxisColor));
      renderService.setLineWidth(3.0f);
      renderService.renderCircle(
        vm::vec3f{m_position}, vm::axis::y, 64, radius, xAxis, zAxis);
      break;
    case HitArea::ZAxis:
      renderService.setForegroundColor(pref(Preferences::ZAxisColor));
      renderService.setLineWidth(3.0f);
      renderService.renderCircle(
        vm::vec3f{m_position}, vm::axis::z, 64, radius, xAxis, yAxis);
      break;
    case HitArea::None:
      break;
      switchDefault();
    }
  }
}

mdl::Hit RotateObjectsHandle::Handle3D::pickRotateHandle(
  const vm::ray3d& pickRay, const render::Camera& camera, const HitArea area) const
{
  if (const auto hit = Handle::pickRotateHandle(pickRay, camera, area); hit.isMatch())
  {
    const auto hitVector = hit.hitPoint() - m_position;

    const auto [xAxis, yAxis, zAxis] = computeAxes(m_position, pickRay.origin);
    if (
      vm::dot(hitVector, xAxis) >= 0.0 && vm::dot(hitVector, yAxis) >= 0.0
      && vm::dot(hitVector, zAxis) >= 0.0)
    {
      return hit;
    }
  }

  return mdl::Hit::NoHit;
}

RotateObjectsHandle::RotateObjectsHandle()
  : m_handle2D{m_position}
  , m_handle3D{m_position}
{
}

const vm::vec3d& RotateObjectsHandle::position() const
{
  return m_position;
}

void RotateObjectsHandle::setPosition(const vm::vec3d& position)
{
  m_position = position;
}

mdl::Hit RotateObjectsHandle::pick2D(
  const vm::ray3d& pickRay, const render::Camera& camera) const
{
  return m_handle2D.pick(pickRay, camera);
}

mdl::Hit RotateObjectsHandle::pick3D(
  const vm::ray3d& pickRay, const render::Camera& camera) const
{
  return m_handle3D.pick(pickRay, camera);
}

double RotateObjectsHandle::majorHandleRadius(const render::Camera& camera) const
{
  return Handle::majorRadius() * m_handle3D.scalingFactor(camera);
}

double RotateObjectsHandle::minorHandleRadius(const render::Camera& camera) const
{
  return Handle::minorRadius() * m_handle3D.scalingFactor(camera);
}

vm::vec3d RotateObjectsHandle::rotationAxis(const HitArea area) const
{
  switch (area)
  {
  case HitArea::XAxis:
    return vm::vec3d{1, 0, 0};
  case HitArea::YAxis:
    return vm::vec3d{0, 1, 0};
  case HitArea::ZAxis:
    return vm::vec3d{0, 0, 1};
  case HitArea::None:
  case HitArea::Center:
    return vm::vec3d{0, 0, 1};
    switchDefault();
  }
}

void RotateObjectsHandle::renderHandle2D(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_handle2D.renderHandle(renderContext, renderBatch);
}

void RotateObjectsHandle::renderHandle3D(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_handle3D.renderHandle(renderContext, renderBatch);
}

void RotateObjectsHandle::renderHighlight2D(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const HitArea area)
{
  m_handle2D.renderHighlight(renderContext, renderBatch, area);
}

void RotateObjectsHandle::renderHighlight3D(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const HitArea area)
{
  m_handle3D.renderHighlight(renderContext, renderBatch, area);
}

} // namespace tb::ui
