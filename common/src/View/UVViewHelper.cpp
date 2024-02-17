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

#include "UVViewHelper.h"

#include "Assets/Texture.h"
#include "FloatType.h"
#include "Model/BrushFace.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Preferences.h"
#include "Renderer/OrthographicCamera.h"
#include "View/UVView.h"

#include "vm/intersection.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

namespace TrenchBroom
{
namespace View
{
UVViewHelper::UVViewHelper(Renderer::OrthographicCamera& camera)
  : m_camera(camera)
  , m_zoomValid(false)
  , m_faceHandle()
  , m_subDivisions(1, 1)
{
}

bool UVViewHelper::valid() const
{
  return m_faceHandle.has_value();
}

const Model::BrushFace* UVViewHelper::face() const
{
  if (m_faceHandle.has_value())
  {
    return &m_faceHandle->face();
  }
  else
  {
    return nullptr;
  }
}

const Assets::Texture* UVViewHelper::texture() const
{
  if (!valid())
    return nullptr;
  return face()->texture();
}

void UVViewHelper::setFaceHandle(std::optional<Model::BrushFaceHandle> faceHandle)
{
  if (faceHandle != m_faceHandle)
  {
    m_faceHandle = std::move(faceHandle);
    if (m_faceHandle != std::nullopt)
    {
      resetCamera();
      resetOrigin();
    }
  }
}

void UVViewHelper::cameraViewportChanged()
{
  // If the user selects a face before the texturing view was shown for the first time,
  // the size of the view might still have been off, resulting in invalid zoom factors.
  // Therefore we must reset the zoom whenever the viewport changes until a valid zoom
  // factor can be computed.
  if (valid() && !m_zoomValid)
    resetZoom();
}

const vm::vec2i& UVViewHelper::subDivisions() const
{
  return m_subDivisions;
}

vm::vec2 UVViewHelper::stripeSize() const
{
  assert(valid());

  const Assets::Texture* texture = face()->texture();
  if (texture == nullptr)
    return vm::vec2::zero();
  const FloatType width =
    static_cast<FloatType>(texture->width()) / static_cast<FloatType>(m_subDivisions.x());
  const FloatType height = static_cast<FloatType>(texture->height())
                           / static_cast<FloatType>(m_subDivisions.y());
  return vm::vec2(width, height);
}

void UVViewHelper::setSubDivisions(const vm::vec2i& subDivisions)
{
  m_subDivisions = subDivisions;
}

const vm::vec3 UVViewHelper::origin() const
{
  assert(valid());

  return m_origin;
}

const vm::vec2f UVViewHelper::originInFaceCoords() const
{
  const vm::mat4x4 toFace =
    face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  return vm::vec2f(toFace * origin());
}

const vm::vec2f UVViewHelper::originInTexCoords() const
{
  assert(valid());

  const vm::mat4x4 toFace = face()->toTexCoordSystemMatrix(
    face()->attributes().offset(), face()->attributes().scale(), true);
  return vm::vec2f(toFace * origin());
}

void UVViewHelper::setOriginInFaceCoords(const vm::vec2f& originInFaceCoords)
{
  const vm::mat4x4 fromFace =
    face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  m_origin = fromFace * vm::vec3(originInFaceCoords);
}

const Renderer::OrthographicCamera& UVViewHelper::camera() const
{
  return m_camera;
}

float UVViewHelper::cameraZoom() const
{
  return m_camera.zoom();
}

void UVViewHelper::pickTextureGrid(
  const vm::ray3& ray,
  const Model::HitType::Type hitTypes[2],
  Model::PickResult& pickResult) const
{
  assert(valid());

  const auto* texture = face()->texture();
  if (texture != nullptr)
  {
    const auto& boundary = face()->boundary();
    const FloatType distance = vm::intersect_ray_plane(ray, boundary);
    const vm::vec3 hitPointInWorldCoords = vm::point_at_distance(ray, distance);
    const vm::vec2f hitPointInTexCoords = vm::vec2f(
      face()->toTexCoordSystemMatrix(
        face()->attributes().offset(), face()->attributes().scale(), true)
      * hitPointInWorldCoords);
    const vm::vec2f hitPointInViewCoords = texToViewCoords(hitPointInTexCoords);

    // X and Y distance in texels to the closest grid intersection.
    // (i.e. so the X component is the distance to the closest vertical gridline, and the
    // Y the distance to the closest horizontal gridline.)
    const vm::vec2f distanceFromGridTexCoords =
      computeDistanceFromTextureGrid(vm::vec3(hitPointInTexCoords, 0.0f));
    const vm::vec2f closestPointsOnGridInTexCoords[2] = {
      hitPointInTexCoords
        + vm::vec2f(
          distanceFromGridTexCoords.x(), 0.0f), // closest point on a vertical gridline
      hitPointInTexCoords
        + vm::vec2f(
          0.0f, distanceFromGridTexCoords.y()), // closest point on a horizontal gridline
    };

    // FIXME: should be measured in points so the grid isn't harder to hit with high-DPI
    const float distToClosestGridInViewCoords[2] = {
      vm::distance(
        hitPointInViewCoords, texToViewCoords(closestPointsOnGridInTexCoords[0])),
      vm::distance(
        hitPointInViewCoords, texToViewCoords(closestPointsOnGridInTexCoords[1]))};

    // FIXME: factor out and share with other tools
    constexpr float maxDistance = static_cast<float>(5.0);

    for (size_t i = 0; i < 2; ++i)
    {
      const float error = distToClosestGridInViewCoords[i];

      if (error <= maxDistance)
      {
        const vm::vec2 stripeSize = UVViewHelper::stripeSize();
        const int index =
          static_cast<int>(vm::round(hitPointInTexCoords[i] / stripeSize[i]));
        pickResult.addHit(
          Model::Hit(hitTypes[i], distance, hitPointInWorldCoords, index, error));
      }
    }
  }
}

vm::vec2f UVViewHelper::snapDelta(const vm::vec2f& delta, const vm::vec2f& distance) const
{
  const float zoom = cameraZoom();

  vm::vec2f result;
  for (size_t i = 0; i < 2; ++i)
  {
    if (vm::abs(distance[i]) < 4.0f / zoom)
      result[i] = delta[i] + distance[i];
    else
      result[i] = vm::round(delta[i]);
  }
  return result;
}

vm::vec2f UVViewHelper::computeDistanceFromTextureGrid(const vm::vec3& position) const
{
  const vm::vec2 stripe = stripeSize();
  assert(stripe.x() != 0.0 && stripe.y() != 0);

  const vm::vec2 closest = snap(position.xy(), stripe);
  return vm::vec2f(closest - position.xy());
}

void UVViewHelper::computeOriginHandleVertices(
  vm::vec3& x1, vm::vec3& x2, vm::vec3& y1, vm::vec3& y2) const
{
  assert(valid());

  const auto toTex =
    face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  const auto toWorld =
    face()->fromTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  computeLineVertices(vm::vec2(originInFaceCoords()), x1, x2, y1, y2, toTex, toWorld);
}

void UVViewHelper::computeScaleHandleVertices(
  const vm::vec2& pos, vm::vec3& x1, vm::vec3& x2, vm::vec3& y1, vm::vec3& y2) const
{
  assert(valid());

  const vm::mat4x4 toTex = face()->toTexCoordSystemMatrix(
    face()->attributes().offset(), face()->attributes().scale(), true);
  const vm::mat4x4 toWorld = face()->fromTexCoordSystemMatrix(
    face()->attributes().offset(), face()->attributes().scale(), true);
  computeLineVertices(pos, x1, x2, y1, y2, toTex, toWorld);
}

void UVViewHelper::computeLineVertices(
  const vm::vec2& pos,
  vm::vec3& x1,
  vm::vec3& x2,
  vm::vec3& y1,
  vm::vec3& y2,
  const vm::mat4x4& toTex,
  const vm::mat4x4& toWorld) const
{
  const auto viewportVertices = toTex * m_camera.viewportVertices();
  const auto viewportBounds =
    vm::bbox3::merge_all(std::begin(viewportVertices), std::end(viewportVertices));
  const auto& min = viewportBounds.min;
  const auto& max = viewportBounds.max;

  x1 = toWorld * vm::vec3(pos.x(), min.y(), 0.0);
  x2 = toWorld * vm::vec3(pos.x(), max.y(), 0.0);
  y1 = toWorld * vm::vec3(min.x(), pos.y(), 0.0);
  y2 = toWorld * vm::vec3(max.x(), pos.y(), 0.0);
}

vm::vec2f UVViewHelper::texToViewCoords(const vm::vec2f& pos) const
{
  const vm::vec3 posInWorldCoords =
    face()->fromTexCoordSystemMatrix(
      face()->attributes().offset(), face()->attributes().scale(), true)
    * vm::vec3(pos, 0.0f);
  const vm::vec2f posInViewCoords = m_camera.project(vm::vec3f(posInWorldCoords)).xy();
  return posInViewCoords;
}

void UVViewHelper::resetOrigin()
{
  assert(valid());

  const auto toTex =
    face()->toTexCoordSystemMatrix(vm::vec2f::zero(), vm::vec2f::one(), true);
  const auto texVertices = toTex * face()->vertexPositions();

  const auto toCam = vm::mat4x4(m_camera.viewMatrix());
  const auto camVertices = toCam * face()->vertexPositions();

  // The origin is at the "lower left" corner of the bounding box.
  auto originFace = texVertices[0];
  auto originCam = camVertices[0];
  for (size_t i = 1; i < texVertices.size(); ++i)
  {
    auto vertexCam = camVertices[i];
    for (size_t j = 0; j < 2; ++j)
    {
      if (vertexCam[j] < originCam[j])
      {
        originCam[j] = vertexCam[j];
        originFace[j] = texVertices[i][j];
      }
    }
  }

  setOriginInFaceCoords(vm::vec2f(originFace));
}

void UVViewHelper::resetCamera()
{
  assert(valid());

  const auto& normal = face()->boundary().normal;
  vm::vec3 right;

  if (vm::abs(dot(vm::vec3::pos_z(), normal)) < FloatType(1.0))
  {
    right = normalize(cross(vm::vec3::pos_z(), normal));
  }
  else
  {
    right = vm::vec3::pos_x();
  }
  const auto up = normalize(cross(normal, right));

  m_camera.setNearPlane(-1.0f);
  m_camera.setFarPlane(+1.0f);
  m_camera.setDirection(vm::vec3f(-normal), vm::vec3f(up));

  const auto position = face()->boundsCenter();
  m_camera.moveTo(vm::vec3f(position));
  resetZoom();
}

void UVViewHelper::resetZoom()
{
  assert(valid());

  auto w = static_cast<float>(m_camera.viewport().width);
  auto h = static_cast<float>(m_camera.viewport().height);

  if (w <= 1.0f || h <= 1.0f)
  {
    return;
  }

  if (w > 80.0f)
  {
    w -= 80.0f;
  }
  if (h > 80.0f)
  {
    h -= 80.0f;
  }

  const auto bounds = computeFaceBoundsInCameraCoords();
  const auto boundsSize = vm::vec3f(bounds.size());

  auto zoom = 3.0f;
  zoom = vm::min(zoom, w / boundsSize.x());
  zoom = vm::min(zoom, h / boundsSize.y());
  if (zoom > 0.0f)
  {
    m_camera.setZoom(zoom);
    m_zoomValid = true;
  }
}

vm::bbox3 UVViewHelper::computeFaceBoundsInCameraCoords() const
{
  assert(valid());

  const auto transform = vm::coordinate_system_matrix(
    vm::vec3(m_camera.right()),
    vm::vec3(m_camera.up()),
    vm::vec3(-m_camera.direction()),
    vm::vec3(m_camera.position()));

  vm::bbox3 result;
  const auto vertices = face()->vertices();
  auto it = std::begin(vertices);
  auto end = std::end(vertices);

  result.min = result.max = transform * (*it++)->position();
  while (it != end)
  {
    result = merge(result, transform * (*it++)->position());
  }
  return result;
}
} // namespace View
} // namespace TrenchBroom
