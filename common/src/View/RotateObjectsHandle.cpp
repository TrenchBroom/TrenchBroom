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

#include "RotateObjectsHandle.h"

#include "FloatType.h"
#include "Macros.h"
#include "Model/Hit.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"

#include "kdl/string_utils.h"

#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/scalar.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <tuple>

namespace TrenchBroom
{
namespace View
{
template <typename T>
std::tuple<vm::vec<T, 3>, vm::vec<T, 3>, vm::vec<T, 3>> computeAxes(
  const vm::vec3& handlePos, const vm::vec<T, 3>& cameraPos)
{
  vm::vec<T, 3> xAxis, yAxis, zAxis;
  const auto viewDir = vm::normalize(vm::vec<T, 3>(handlePos) - cameraPos);
  if (vm::is_equal(std::abs(viewDir.z()), T(1.0), vm::constants<T>::almost_zero()))
  {
    xAxis = vm::vec<T, 3>::pos_x();
    yAxis = vm::vec<T, 3>::pos_y();
  }
  else
  {
    xAxis = viewDir.x() > T(0.0) ? vm::vec<T, 3>::neg_x() : vm::vec<T, 3>::pos_x();
    yAxis = viewDir.y() > T(0.0) ? vm::vec<T, 3>::neg_y() : vm::vec<T, 3>::pos_y();
  }
  zAxis = viewDir.z() > T(0.0) ? vm::vec<T, 3>::neg_z() : vm::vec<T, 3>::pos_z();

  return {xAxis, yAxis, zAxis};
}

const Model::HitType::Type RotateObjectsHandle::HandleHitType =
  Model::HitType::freeType();

RotateObjectsHandle::Handle::Handle(const vm::vec3& position)
  : m_position(position)
{
}

RotateObjectsHandle::Handle::~Handle() = default;

FloatType RotateObjectsHandle::Handle::scalingFactor(const Renderer::Camera& camera) const
{
  return static_cast<FloatType>(camera.perspectiveScalingFactor(vm::vec3f(m_position)));
}

FloatType RotateObjectsHandle::Handle::majorRadius()
{
  return static_cast<FloatType>(pref(Preferences::RotateHandleRadius));
}

FloatType RotateObjectsHandle::Handle::minorRadius()
{
  return static_cast<FloatType>(pref(Preferences::HandleRadius));
}

Model::Hit RotateObjectsHandle::Handle::pickCenterHandle(
  const vm::ray3& pickRay, const Renderer::Camera& camera) const
{
  const FloatType distance = camera.pickPointHandle(
    pickRay, m_position, static_cast<FloatType>(pref(Preferences::HandleRadius)));
  if (vm::is_nan(distance))
  {
    return Model::Hit::NoHit;
  }
  else
  {
    return Model::Hit(
      HandleHitType, distance, vm::point_at_distance(pickRay, distance), HitArea::Center);
  }
}

Model::Hit RotateObjectsHandle::Handle::pickRotateHandle(
  const vm::ray3& pickRay, const Renderer::Camera& camera, const HitArea area) const
{
  const auto transform = handleTransform(camera, area);

  const auto [invertible, inverse] = vm::invert(transform);
  if (!invertible)
  {
    return Model::Hit::NoHit;
  }

  if (invertible)
  {
    const auto transformedRay = pickRay.transform(inverse);
    const auto transformedPosition = inverse * m_position;
    const auto transformedDistance = vm::intersect_ray_torus(
      transformedRay, transformedPosition, majorRadius(), minorRadius());
    if (!vm::is_nan(transformedDistance))
    {
      const auto transformedHitPoint =
        vm::point_at_distance(transformedRay, transformedDistance);
      const auto hitPoint = transform * transformedHitPoint;
      const auto distance = vm::dot(hitPoint - pickRay.origin, pickRay.direction);
      return Model::Hit(HandleHitType, distance, hitPoint, area);
    }
  }

  return Model::Hit::NoHit;
}

vm::mat4x4 RotateObjectsHandle::Handle::handleTransform(
  const Renderer::Camera& camera, const HitArea area) const
{
  const auto scalingFactor = this->scalingFactor(camera);
  if (scalingFactor <= FloatType(0.0))
  {
    return vm::mat4x4::zero();
  }

  const auto scalingMatrix =
    vm::scaling_matrix(vm::vec3(scalingFactor, scalingFactor, scalingFactor));
  switch (area)
  {
  case HitArea::XAxis:
    return vm::mat4x4::rot_90_y_ccw() * scalingMatrix;
  case HitArea::YAxis:
    return vm::mat4x4::rot_90_x_cw() * scalingMatrix;
  case HitArea::ZAxis:
  case HitArea::Center:
  case HitArea::None:
    return vm::mat4x4::identity() * scalingMatrix;
    switchDefault();
  }
}

Model::Hit RotateObjectsHandle::Handle2D::pick(
  const vm::ray3& pickRay, const Renderer::Camera& camera) const
{
  switch (vm::find_abs_max_component(camera.direction()))
  {
  case vm::axis::x:
    return Model::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::XAxis));
  case vm::axis::y:
    return Model::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::YAxis));
  default:
    return Model::selectClosest(
      pickCenterHandle(pickRay, camera),
      pickRotateHandle(pickRay, camera, HitArea::ZAxis));
  }
}

Model::Hit RotateObjectsHandle::Handle2D::pickRotateHandle(
  const vm::ray3& pickRay, const Renderer::Camera& camera, const HitArea area) const
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
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
{
  const auto& camera = renderContext.camera();
  const auto radius =
    static_cast<float>(majorRadius() * scalingFactor(renderContext.camera()));
  if (radius <= 0.0f)
  {
    return;
  }

  Renderer::RenderService renderService(renderContext, renderBatch);
  renderService.setShowOccludedObjects();

  renderService.setLineWidth(2.0f);
  renderService.setForegroundColor(
    pref(Preferences::axisColor(vm::find_abs_max_component(camera.direction()))));
  renderService.renderCircle(
    vm::vec3f(m_position), vm::find_abs_max_component(camera.direction()), 64, radius);

  renderService.setForegroundColor(pref(Preferences::HandleColor));
  renderService.renderHandle(vm::vec3f(m_position));
}

void RotateObjectsHandle::Handle2D::renderHighlight(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  RotateObjectsHandle::HitArea area) const
{
  const auto radius =
    static_cast<float>(majorRadius() * scalingFactor(renderContext.camera()));
  if (radius <= 0.0f)
  {
    return;
  }

  const auto& camera = renderContext.camera();

  Renderer::RenderService renderService(renderContext, renderBatch);
  renderService.setShowOccludedObjects();

  switch (area)
  {
  case HitArea::Center:
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f(m_position));
    break;
  case HitArea::XAxis:
  case HitArea::YAxis:
  case HitArea::ZAxis:
    renderService.setLineWidth(3.0f);
    renderService.setForegroundColor(
      pref(Preferences::axisColor(vm::find_abs_max_component(camera.direction()))));
    renderService.renderCircle(
      vm::vec3f(m_position), vm::find_abs_max_component(camera.direction()), 64, radius);
    break;
  case HitArea::None:
    break;
    switchDefault();
  }
}

Model::Hit RotateObjectsHandle::Handle3D::pick(
  const vm::ray3& pickRay, const Renderer::Camera& camera) const
{
  return Model::selectClosest(
    pickCenterHandle(pickRay, camera),
    pickRotateHandle(pickRay, camera, HitArea::XAxis),
    pickRotateHandle(pickRay, camera, HitArea::YAxis),
    pickRotateHandle(pickRay, camera, HitArea::ZAxis));
}

void RotateObjectsHandle::Handle3D::renderHandle(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
{
  const auto radius =
    static_cast<float>(majorRadius() * scalingFactor(renderContext.camera()));
  if (radius <= 0.0f)
  {
    return;
  }

  const auto [xAxis, yAxis, zAxis] =
    computeAxes(m_position, renderContext.camera().position());

  Renderer::RenderService renderService(renderContext, renderBatch);
  renderService.setShowOccludedObjects();

  renderService.renderCoordinateSystem(
    vm::bbox3f(radius).translate(vm::vec3f(m_position)));

  renderService.setLineWidth(2.0f);
  renderService.setForegroundColor(pref(Preferences::XAxisColor));
  renderService.renderCircle(
    vm::vec3f(m_position), vm::axis::x, 64, radius, zAxis, yAxis);
  renderService.setForegroundColor(pref(Preferences::YAxisColor));
  renderService.renderCircle(
    vm::vec3f(m_position), vm::axis::y, 64, radius, xAxis, zAxis);
  renderService.setForegroundColor(pref(Preferences::ZAxisColor));
  renderService.renderCircle(
    vm::vec3f(m_position), vm::axis::z, 64, radius, xAxis, yAxis);

  renderService.setForegroundColor(pref(Preferences::HandleColor));
  renderService.renderHandle(vm::vec3f(m_position));
}

void RotateObjectsHandle::Handle3D::renderHighlight(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  RotateObjectsHandle::HitArea area) const
{
  const auto radius =
    static_cast<float>(majorRadius() * scalingFactor(renderContext.camera()));
  if (radius <= 0.0f)
  {
    return;
  }

  const auto [xAxis, yAxis, zAxis] =
    computeAxes(m_position, renderContext.camera().position());

  Renderer::RenderService renderService(renderContext, renderBatch);
  renderService.setShowOccludedObjects();

  switch (area)
  {
  case HitArea::Center:
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f(m_position));
    renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
    renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
    renderService.renderString(kdl::str_to_string(m_position), vm::vec3f(m_position));
    break;
  case HitArea::XAxis:
    renderService.setForegroundColor(pref(Preferences::XAxisColor));
    renderService.setLineWidth(3.0f);
    renderService.renderCircle(
      vm::vec3f(m_position), vm::axis::x, 64, radius, zAxis, yAxis);
    break;
  case HitArea::YAxis:
    renderService.setForegroundColor(pref(Preferences::YAxisColor));
    renderService.setLineWidth(3.0f);
    renderService.renderCircle(
      vm::vec3f(m_position), vm::axis::y, 64, radius, xAxis, zAxis);
    break;
  case HitArea::ZAxis:
    renderService.setForegroundColor(pref(Preferences::ZAxisColor));
    renderService.setLineWidth(3.0f);
    renderService.renderCircle(
      vm::vec3f(m_position), vm::axis::z, 64, radius, xAxis, yAxis);
    break;
  case HitArea::None:
    break;
    switchDefault();
  }
}

Model::Hit RotateObjectsHandle::Handle3D::pickRotateHandle(
  const vm::ray3& pickRay, const Renderer::Camera& camera, const HitArea area) const
{
  const auto hit = Handle::pickRotateHandle(pickRay, camera, area);
  if (hit.isMatch())
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

  return Model::Hit::NoHit;
}

RotateObjectsHandle::RotateObjectsHandle()
  : m_handle2D(m_position)
  , m_handle3D(m_position)
{
}

const vm::vec3& RotateObjectsHandle::position() const
{
  return m_position;
}

void RotateObjectsHandle::setPosition(const vm::vec3& position)
{
  m_position = position;
}

Model::Hit RotateObjectsHandle::pick2D(
  const vm::ray3& pickRay, const Renderer::Camera& camera) const
{
  return m_handle2D.pick(pickRay, camera);
}

Model::Hit RotateObjectsHandle::pick3D(
  const vm::ray3& pickRay, const Renderer::Camera& camera) const
{
  return m_handle3D.pick(pickRay, camera);
}

FloatType RotateObjectsHandle::majorHandleRadius(const Renderer::Camera& camera) const
{
  return Handle::majorRadius() * m_handle3D.scalingFactor(camera);
}

FloatType RotateObjectsHandle::minorHandleRadius(const Renderer::Camera& camera) const
{
  return Handle::minorRadius() * m_handle3D.scalingFactor(camera);
}

vm::vec3 RotateObjectsHandle::rotationAxis(const HitArea area) const
{
  switch (area)
  {
  case HitArea::XAxis:
    return vm::vec3::pos_x();
  case HitArea::YAxis:
    return vm::vec3::pos_y();
  case HitArea::ZAxis:
    return vm::vec3::pos_z();
  case HitArea::None:
  case HitArea::Center:
    return vm::vec3::pos_z();
    switchDefault();
  }
}

void RotateObjectsHandle::renderHandle2D(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  m_handle2D.renderHandle(renderContext, renderBatch);
}

void RotateObjectsHandle::renderHandle3D(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  m_handle3D.renderHandle(renderContext, renderBatch);
}

void RotateObjectsHandle::renderHighlight2D(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const HitArea area)
{
  m_handle2D.renderHighlight(renderContext, renderBatch, area);
}

void RotateObjectsHandle::renderHighlight3D(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const HitArea area)
{
  m_handle3D.renderHighlight(renderContext, renderBatch, area);
}
} // namespace View
} // namespace TrenchBroom
