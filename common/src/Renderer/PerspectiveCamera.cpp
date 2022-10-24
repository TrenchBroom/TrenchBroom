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

#include "PerspectiveCamera.h"

#include "Color.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/VboManager.h"
#include "Renderer/VertexArray.h"

#include <vecmath/forward.h>
#include <vecmath/intersection.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>

#include <limits>
#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
PerspectiveCamera::PerspectiveCamera()
  : Camera()
  , m_fov(90.0)
{
}

PerspectiveCamera::PerspectiveCamera(
  const float fov,
  const float nearPlane,
  const float farPlane,
  const Viewport& viewport,
  const vm::vec3f& position,
  const vm::vec3f& direction,
  const vm::vec3f& up)
  : Camera(nearPlane, farPlane, viewport, position, direction, up)
  , m_fov(fov)
{
  assert(m_fov > 0.0f);
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
  assert(fov > 0.0f);
  if (fov != m_fov)
  {
    m_fov = fov;
    m_valid = false;
    cameraDidChangeNotifier(this);
  }
}

vm::ray3f PerspectiveCamera::doGetPickRay(const vm::vec3f& point) const
{
  const vm::vec3f direction = normalize(point - position());
  return vm::ray3f(position(), direction);
}

float PerspectiveCamera::computeZoomedFov(const float zoom, const float fov)
{
  // Piecewise definition of a function to get a natural feeling zoom
  // - for values below 0.7, use the square root.
  // - for values above 1.2, use the negated inverse (approaches 0 smoothly)
  // - for values in between, linearly interpolate between both
  const auto f1 = std::sqrt(zoom);
  const auto f2 = (-1.0f / zoom + 2.0f);
  float z;
  if (zoom < 0.7f)
  {
    z = f1;
  }
  else if (zoom < 1.2f)
  {
    z = vm::mix(f1, f2, 2.0f * (zoom - 0.7f));
  }
  else
  {
    z = f2;
  }

  return fov * z;
}

Camera::ProjectionType PerspectiveCamera::doGetProjectionType() const
{
  return Projection_Perspective;
}

void PerspectiveCamera::doValidateMatrices(
  vm::mat4x4f& projectionMatrix, vm::mat4x4f& viewMatrix) const
{
  const Viewport& viewport = this->viewport();
  projectionMatrix = vm::perspective_matrix(
    zoomedFov(), nearPlane(), farPlane(), viewport.width, viewport.height);
  viewMatrix = vm::view_matrix(direction(), up()) * vm::translation_matrix(-position());
}

void PerspectiveCamera::doComputeFrustumPlanes(
  vm::plane3f& topPlane,
  vm::plane3f& rightPlane,
  vm::plane3f& bottomPlane,
  vm::plane3f& leftPlane) const
{
  const vm::vec2f frustum = getFrustum();
  const vm::vec3f center = position() + direction() * nearPlane();

  vm::vec3f d = center + up() * frustum.y() - position();
  topPlane = vm::plane3f(position(), normalize(cross(right(), d)));

  d = center + right() * frustum.x() - position();
  rightPlane = vm::plane3f(position(), normalize(cross(d, up())));

  d = center - up() * frustum.y() - position();
  bottomPlane = vm::plane3f(position(), normalize(cross(d, right())));

  d = center - right() * frustum.x() - position();
  leftPlane = vm::plane3f(position(), normalize(cross(up(), d)));
}

void PerspectiveCamera::doRenderFrustum(
  RenderContext& renderContext,
  VboManager& vboManager,
  const float size,
  const Color& color) const
{
  using Vertex = GLVertexTypes::P3C4::Vertex;
  std::vector<Vertex> triangleVertices;
  std::vector<Vertex> lineVertices;
  triangleVertices.reserve(6);
  lineVertices.reserve(8 * 2);

  vm::vec3f verts[4];
  getFrustumVertices(size, verts);

  triangleVertices.emplace_back(position(), Color(color, 0.7f));
  for (size_t i = 0; i < 4; ++i)
  {
    triangleVertices.emplace_back(verts[i], Color(color, 0.2f));
  }
  triangleVertices.emplace_back(verts[0], Color(color, 0.2f));

  for (size_t i = 0; i < 4; ++i)
  {
    lineVertices.emplace_back(position(), color);
    lineVertices.emplace_back(verts[i], color);
  }

  for (size_t i = 0; i < 4; ++i)
  {
    lineVertices.emplace_back(verts[i], color);
    lineVertices.emplace_back(verts[vm::succ(i, 4)], color);
  }

  auto triangleArray = VertexArray::ref(triangleVertices);
  auto lineArray = VertexArray::ref(lineVertices);

  triangleArray.prepare(vboManager);
  lineArray.prepare(vboManager);

  ActiveShader shader(renderContext.shaderManager(), Shaders::VaryingPCShader);
  triangleArray.render(PrimType::TriangleFan);
  lineArray.render(PrimType::Lines);
}

float PerspectiveCamera::doPickFrustum(const float size, const vm::ray3f& ray) const
{
  vm::vec3f verts[4];
  getFrustumVertices(size, verts);

  auto minDistance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < 4; ++i)
  {
    const auto distance =
      vm::intersect_ray_triangle(ray, position(), verts[i], verts[vm::succ(i, 4)]);
    if (!vm::is_nan(distance))
    {
      minDistance = vm::min(distance, minDistance);
    }
  }
  return minDistance;
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
  return vm::vec2f(h, v);
}

float PerspectiveCamera::doGetPerspectiveScalingFactor(const vm::vec3f& position) const
{
  const auto perpDist = perpendicularDistanceTo(position);
  return perpDist / viewportFrustumDistance();
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
} // namespace Renderer
} // namespace TrenchBroom
