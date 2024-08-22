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

#include "OrthographicCamera.h"

#include "vm/forward.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/plane.h"
#include "vm/ray.h"
#include "vm/vec.h"

namespace TrenchBroom
{
namespace Renderer
{
OrthographicCamera::OrthographicCamera()
  : Camera()
  , m_zoomedViewport(this->viewport())
{
}

OrthographicCamera::OrthographicCamera(
  const float nearPlane,
  const float farPlane,
  const Viewport& viewport,
  const vm::vec3f& position,
  const vm::vec3f& direction,
  const vm::vec3f& up)
  : Camera(nearPlane, farPlane, viewport, position, direction, up)
  , m_zoomedViewport(this->viewport())
{
}

const OrthographicCamera::Viewport& OrthographicCamera::zoomedViewport() const
{
  return m_zoomedViewport;
}

std::vector<vm::vec3> OrthographicCamera::viewportVertices() const
{
  const auto w2 = static_cast<float>(zoomedViewport().width) / 2.0f;
  const auto h2 = static_cast<float>(zoomedViewport().height) / 2.0f;

  std::vector<vm::vec3> result(4);
  result[0] = vm::vec3(position() - w2 * right() + h2 * up());
  result[1] = vm::vec3(position() + w2 * right() + h2 * up());
  result[2] = vm::vec3(position() + w2 * right() - h2 * up());
  result[3] = vm::vec3(position() - w2 * right() - h2 * up());
  return result;
}

Camera::ProjectionType OrthographicCamera::doGetProjectionType() const
{
  return Projection_Orthographic;
}

void OrthographicCamera::doValidateMatrices(
  vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const
{
  const auto w2 = static_cast<float>(zoomedViewport().width) / 2.0f;
  const auto h2 = static_cast<float>(zoomedViewport().height) / 2.0f;

  projectionMatrix = vm::ortho_matrix(nearPlane(), farPlane(), -w2, h2, w2, -h2);
  viewMatrix = vm::view_matrix(direction(), up()) * vm::translation_matrix(-position());
}

vm::ray3f OrthographicCamera::doGetPickRay(const vm::vec3f& point) const
{
  const auto v = point - position();
  const auto d = dot(v, direction());
  const auto o = point - d * direction();
  return vm::ray3f(o, direction());
}

void OrthographicCamera::doComputeFrustumPlanes(
  vm::plane3f& topPlane,
  vm::plane3f& rightPlane,
  vm::plane3f& bottomPlane,
  vm::plane3f& leftPlane) const
{
  const auto w2 = static_cast<float>(zoomedViewport().width) / 2.0f;
  const auto h2 = static_cast<float>(zoomedViewport().height) / 2.0f;

  const auto& center = position();
  topPlane = vm::plane3f(center + h2 * up(), up());
  rightPlane = vm::plane3f(center + w2 * right(), right());
  bottomPlane = vm::plane3f(center - h2 * up(), -up());
  leftPlane = vm::plane3f(center - w2 * right(), -right());
}

void OrthographicCamera::doRenderFrustum(
  RenderContext&,
  VboManager& /* vboManager */,
  const float /* size */,
  const Color& /* color */) const
{
}

float OrthographicCamera::doPickFrustum(
  const float /* size */, const vm::ray3f& /* ray */) const
{
  return vm::nan<float>();
}

float OrthographicCamera::doGetPerspectiveScalingFactor(
  const vm::vec3f& /* position */) const
{
  return 1.0f / zoom();
}

void OrthographicCamera::doUpdateZoom()
{
  const auto& unzoomedViewport = viewport();
  m_zoomedViewport = Viewport(
    unzoomedViewport.x,
    unzoomedViewport.y,
    static_cast<int>(vm::round(static_cast<float>(unzoomedViewport.width) / zoom())),
    static_cast<int>(vm::round(static_cast<float>(unzoomedViewport.height) / zoom())));
}
} // namespace Renderer
} // namespace TrenchBroom
