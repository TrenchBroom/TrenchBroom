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

#include "gl/PerspectiveCamera.h"

#include "kd/contracts.h"

#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <limits>

namespace tb::gl
{

PerspectiveCamera::PerspectiveCamera() = default;

PerspectiveCamera::PerspectiveCamera(
  const float fov,
  const float nearPlane,
  const float farPlane,
  const Viewport& viewport,
  const vm::vec3f& position,
  const vm::vec3f& direction,
  const vm::vec3f& up)
  : Camera{nearPlane, farPlane, viewport, position, direction, up}
  , m_fov{fov}
{
  contract_pre(m_fov > 0.0f);
}

float PerspectiveCamera::fov() const
{
  return m_fov;
}

float PerspectiveCamera::zoomedFov() const
{
  return PerspectiveCamera::computeZoomedFov(zoom(), fov());
}

void PerspectiveCamera::setFov(const float fov)
{
  contract_pre(fov > 0.0f);

  if (fov != m_fov)
  {
    m_fov = fov;
    m_valid = false;
    cameraDidChangeNotifier(this);
  }
}


void PerspectiveCamera::frustumPlanes(
  vm::plane3f& topPlane,
  vm::plane3f& rightPlane,
  vm::plane3f& bottomPlane,
  vm::plane3f& leftPlane) const
{
  const auto frustum = getFrustum();
  const auto center = position() + direction() * nearPlane();

  auto d = center + up() * frustum.y() - position();
  topPlane = vm::plane3f(position(), normalize(cross(right(), d)));

  d = center + right() * frustum.x() - position();
  rightPlane = vm::plane3f(position(), normalize(cross(d, up())));

  d = center - up() * frustum.y() - position();
  bottomPlane = vm::plane3f(position(), normalize(cross(d, right())));

  d = center - right() * frustum.x() - position();
  leftPlane = vm::plane3f(position(), normalize(cross(up(), d)));
}

vm::ray3f PerspectiveCamera::pickRay(const vm::vec3f& point) const
{
  const auto direction = vm::normalize(point - position());
  return {position(), direction};
}

float PerspectiveCamera::perspectiveScalingFactor(const vm::vec3f& position) const
{
  const auto perpDist = perpendicularDistanceTo(position);
  return perpDist / viewportFrustumDistance();
}

float PerspectiveCamera::pickFrustum(const float size, const vm::ray3f& ray) const
{
  vm::vec3f verts[4];
  getFrustumVertices(size, verts);

  auto minDistance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < 4; ++i)
  {
    if (
      const auto distance =
        vm::intersect_ray_triangle(ray, position(), verts[i], verts[vm::succ(i, 4)]))
    {
      minDistance = vm::min(*distance, minDistance);
    }
  }
  return minDistance;
}

float PerspectiveCamera::computeZoomedFov(const float zoom, const float fov)
{
  // Piecewise definition of a function to get a natural feeling zoom
  // - for values below 0.7, use the square root.
  // - for values above 1.2, use the negated inverse (approaches 0 smoothly)
  // - for values in between, linearly interpolate between both
  const auto f1 = std::sqrt(zoom);
  const auto f2 = (-1.0f / zoom + 2.0f);
  const auto z = zoom < 0.7f   ? f1
                 : zoom < 1.2f ? vm::mix(f1, f2, 2.0f * (zoom - 0.7f))
                               : f2;

  return fov * z;
}

Camera::ProjectionType PerspectiveCamera::projectionType() const
{
  return ProjectionType::Perspective;
}

void PerspectiveCamera::doValidateMatrices(
  vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const
{
  const auto& viewport = this->viewport();
  projectionMatrix = vm::perspective_matrix(
    zoomedFov(), nearPlane(), farPlane(), viewport.width, viewport.height);
  viewMatrix = vm::view_matrix(direction(), up()) * vm::translation_matrix(-position());
}

void PerspectiveCamera::getFrustumVertices(const float size, vm::vec3f (&verts)[4]) const
{
  const auto frustum = getFrustum();

  verts[0] = position()
             + (direction() * nearPlane() + frustum.y() * up() - frustum.x() * right())
                 / nearPlane() * size; // top left
  verts[1] = position()
             + (direction() * nearPlane() + frustum.y() * up() + frustum.x() * right())
                 / nearPlane() * size; // top right
  verts[2] = position()
             + (direction() * nearPlane() - frustum.y() * up() + frustum.x() * right())
                 / nearPlane() * size; // bottom right
  verts[3] = position()
             + (direction() * nearPlane() - frustum.y() * up() - frustum.x() * right())
                 / nearPlane() * size; // bottom left
}

vm::vec2f PerspectiveCamera::getFrustum() const
{
  const auto& viewport = this->viewport();
  const auto v = std::tan(vm::to_radians(zoomedFov()) / 2.0f) * 0.75f * nearPlane();
  const auto h =
    v * static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
  return vm::vec2f{h, v};
}

float PerspectiveCamera::viewportFrustumDistance() const
{
  const auto height = static_cast<float>(viewport().height);
  return (height / 2.0f) / std::tan(vm::to_radians(zoomedFov()) / 2.0f);
}

bool PerspectiveCamera::isValidZoom(const float zoom) const
{
  const auto zoomedFov = PerspectiveCamera::computeZoomedFov(zoom, fov());
  return vm::contains(zoomedFov, 1.0f, 150.0f);
}

void PerspectiveCamera::doUpdateZoom() {}

} // namespace tb::gl
