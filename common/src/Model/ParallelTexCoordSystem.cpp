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

#include "ParallelTexCoordSystem.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "FloatType.h"
#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/ParaxialTexCoordSystem.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec.h"

#include <algorithm>
#include <cstddef>

namespace TrenchBroom::Model
{

namespace
{

/**
 * Generates two vectors which are perpendicular to `normal` and perpendicular to each
 * other.
 */
std::tuple<vm::vec3, vm::vec3> computeInitialAxes(const vm::vec3& normal)
{
  const auto xAxis = vm::find_abs_max_component(normal) == vm::axis::z
                       ? vm::normalize(vm::cross(vm::vec3::pos_y(), normal))
                       : vm::normalize(vm::cross(vm::vec3::pos_z(), normal));

  return {xAxis, vm::normalize(vm::cross(xAxis, normal))};
}

/**
 * Rotate CCW by `angle` radians about `normal`.
 */
std::tuple<vm::vec3, vm::vec3> applyRotation(
  const vm::vec3& xAxis,
  const vm::vec3& yAxis,
  const vm::vec3& normal,
  const FloatType angle)
{
  const auto rot = vm::quat3{normal, angle};
  return {rot * xAxis, rot * yAxis};
}

} // namespace

ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(
  const vm::vec3& xAxis, const vm::vec3& yAxis)
  : m_xAxis{xAxis}
  , m_yAxis{yAxis}
{
}

ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(
  const ParallelTexCoordSystem* coordSystem)
  : m_xAxis{coordSystem->xAxis()}
  , m_yAxis{coordSystem->yAxis()}
{
}

std::unique_ptr<TexCoordSystemSnapshot> ParallelTexCoordSystemSnapshot::clone() const
{
  return std::make_unique<ParallelTexCoordSystemSnapshot>(m_xAxis, m_yAxis);
}

void ParallelTexCoordSystemSnapshot::doRestore(ParallelTexCoordSystem& coordSystem) const
{
  coordSystem.m_xAxis = m_xAxis;
  coordSystem.m_yAxis = m_yAxis;
}

void ParallelTexCoordSystemSnapshot::doRestore(
  ParaxialTexCoordSystem& /* coordSystem */) const
{
  ensure(false, "wrong coord system type");
}

/**
 * Constructs a parallel tex coord system where the texture is projected form the face
 * plane
 *
 * @param point0 a point defining the face plane
 * @param point1 a point defining the face plane
 * @param point2 a point defining the face plane
 * @param attribs face attributes
 */
ParallelTexCoordSystem::ParallelTexCoordSystem(
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const BrushFaceAttributes& attribs)
{
  const auto normal = vm::normalize(vm::cross(point2 - point0, point1 - point0));
  std::tie(m_xAxis, m_yAxis) = computeInitialAxes(normal);
  std::tie(m_xAxis, m_yAxis) =
    applyRotation(xAxis(), yAxis(), normal, FloatType(attribs.rotation()));
}

ParallelTexCoordSystem::ParallelTexCoordSystem(
  const vm::vec3& xAxis, const vm::vec3& yAxis)
  : m_xAxis{xAxis}
  , m_yAxis{yAxis}
{
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParallelTexCoordSystem::
  fromParaxial(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs)
{
  const auto tempParaxial = ParaxialTexCoordSystem{point0, point1, point2, attribs};
  return {
    ParallelTexCoordSystem{tempParaxial.xAxis(), tempParaxial.yAxis()}.clone(), attribs};
}

std::unique_ptr<TexCoordSystem> ParallelTexCoordSystem::clone() const
{
  return std::make_unique<ParallelTexCoordSystem>(xAxis(), yAxis());
}

std::unique_ptr<TexCoordSystemSnapshot> ParallelTexCoordSystem::takeSnapshot() const
{
  return std::make_unique<ParallelTexCoordSystemSnapshot>(this);
}

void ParallelTexCoordSystem::restoreSnapshot(const TexCoordSystemSnapshot& snapshot)
{
  snapshot.doRestore(*this);
}

vm::vec3 ParallelTexCoordSystem::xAxis() const
{
  return m_xAxis;
}

vm::vec3 ParallelTexCoordSystem::yAxis() const
{
  return m_yAxis;
}

vm::vec3 ParallelTexCoordSystem::zAxis() const
{
  return vm::normalize(vm::cross(xAxis(), yAxis()));
}

void ParallelTexCoordSystem::resetCache(
  const vm::vec3& /* point0 */,
  const vm::vec3& /* point1 */,
  const vm::vec3& /* point2 */,
  const BrushFaceAttributes& /* attribs */)
{
  // no-op
}

void ParallelTexCoordSystem::resetTextureAxes(const vm::vec3& normal)
{
  std::tie(m_xAxis, m_yAxis) = computeInitialAxes(normal);
}

void ParallelTexCoordSystem::resetTextureAxesToParaxial(
  const vm::vec3& normal, float angle)
{
  const auto index = ParaxialTexCoordSystem::planeNormalIndex(normal);
  std::tie(m_xAxis, m_yAxis, std::ignore) = ParaxialTexCoordSystem::axes(index);
  std::tie(m_xAxis, m_yAxis) = applyRotation(xAxis(), yAxis(), normal, FloatType(angle));
}

void ParallelTexCoordSystem::resetTextureAxesToParallel(
  const vm::vec3& normal, float angle)
{
  std::tie(m_xAxis, m_yAxis) = computeInitialAxes(normal);
  std::tie(m_xAxis, m_yAxis) = applyRotation(xAxis(), yAxis(), normal, FloatType(angle));
}

vm::vec2f ParallelTexCoordSystem::getTexCoords(
  const vm::vec3& point,
  const BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize) const
{
  return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / textureSize;
}

/**
 * Rotates from `oldAngle` to `newAngle`. Both of these are in CCW degrees about
 * the texture normal (`getZAxis()`). The provided `normal` is ignored.
 */
void ParallelTexCoordSystem::setRotation(
  const vm::vec3& /* normal */, const float oldAngle, const float newAngle)
{
  const auto angleDelta = newAngle - oldAngle;
  if (angleDelta != 0.0f)
  {
    std::tie(m_xAxis, m_yAxis) =
      applyRotation(xAxis(), yAxis(), zAxis(), FloatType(vm::to_radians(angleDelta)));
  }
}

void ParallelTexCoordSystem::transform(
  const vm::plane3& oldBoundary,
  const vm::plane3& newBoundary,
  const vm::mat4x4& transformation,
  BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize,
  const bool lockTexture,
  const vm::vec3& oldInvariant)
{
  if (attribs.xScale() == 0.0f || attribs.yScale() == 0.0f)
  {
    return;
  }

  // when texture lock is off, just project the current texturing
  if (!lockTexture)
  {
    updateNormalWithProjection(newBoundary.normal, attribs);
    return;
  }

  const auto effectiveTransformation = transformation;

  // determine the rotation by which the texture coordinate system will be rotated about
  // its normal
  const auto angleDelta = computeTextureAngle(oldBoundary, effectiveTransformation);
  const auto newAngle =
    vm::correct(vm::normalize_degrees(attribs.rotation() + angleDelta), 4);
  assert(!vm::is_nan(newAngle));
  attribs.setRotation(newAngle);

  // calculate the current texture coordinates of the face's center
  const auto oldInvariantTechCoords =
    computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();
  assert(!vm::is_nan(oldInvariantTechCoords));

  // compute the new texture axes
  const auto worldToTexSpace = toMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1});

  // The formula for texturing is:
  //
  //     uv = worldToTexSpace * point
  //
  // We want to find a new worldToTexSpace matrix, ?, such that
  // transformed points have the same uv coords as they did
  // without the transform, with the old worldToTexSpace matrix:
  //
  //     uv = ? * transform * point
  //
  // The solution for ? is (worldToTexSpace * transform_inverse)
  const auto inverseTransform = invert(effectiveTransformation);
  const auto newWorldToTexSpace = worldToTexSpace * *inverseTransform;

  // extract the new m_xAxis and m_yAxis from newWorldToTexSpace.
  // note, the matrix is in column major format.
  for (size_t i = 0; i < 3; i++)
  {
    m_xAxis[i] = newWorldToTexSpace[i][0];
    m_yAxis[i] = newWorldToTexSpace[i][1];
  }
  assert(!vm::is_nan(xAxis()));
  assert(!vm::is_nan(yAxis()));

  // determine the new texture coordinates of the transformed center of the face, sans
  // offsets
  const auto newInvariant = effectiveTransformation * oldInvariant;
  const auto newInvariantTexCoords = computeTexCoords(newInvariant, attribs.scale());

  // since the center should be invariant, the offsets are determined by the difference of
  // the current and the original texture coordinates of the center
  const auto newOffset = correct(
    attribs.modOffset(oldInvariantTechCoords - newInvariantTexCoords, textureSize), 4);
  assert(!vm::is_nan(newOffset));
  attribs.setOffset(newOffset);
}

void ParallelTexCoordSystem::shearTexture(
  const vm::vec3& /* normal */, const vm::vec2f& f)
{
  const vm::mat4x4 shear(
    1.0, f[0], 0.0, 0.0, f[1], 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

  const auto toMatrix =
    vm::coordinate_system_matrix(xAxis(), yAxis(), zAxis(), vm::vec3::zero());
  const auto fromMatrix = vm::invert(toMatrix);

  const auto transform = *fromMatrix * shear * toMatrix;
  m_xAxis = transform * xAxis();
  m_yAxis = transform * yAxis();
}

/**
 * Measures the angle between the line from `center` to `point` and the texture space X
 * axis, in CCW degrees about the texture normal. Returns this, added to `currentAngle`
 * (also in CCW degrees).
 */
float ParallelTexCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  const auto vec = vm::vec3f{point - center};
  const auto angleInRadians =
    vm::measure_angle(vm::normalize(vec), vm::vec3f::pos_x(), vm::vec3f::pos_z());
  return currentAngle + vm::to_degrees(angleInRadians);
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParallelTexCoordSystem::
  toParallel(
    const vm::vec3&,
    const vm::vec3&,
    const vm::vec3&,
    const BrushFaceAttributes& attribs) const
{
  return {clone(), attribs};
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParallelTexCoordSystem::
  toParaxial(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const
{
  return ParaxialTexCoordSystem::fromParallel(
    point0, point1, point2, attribs, xAxis(), yAxis());
}

bool ParallelTexCoordSystem::isRotationInverted(const vm::vec3& /* normal */) const
{
  return false;
}

void ParallelTexCoordSystem::updateNormalWithProjection(
  const vm::vec3& newNormal, const BrushFaceAttributes& /* attribs */)
{
  // Goal: (m_xAxis, m_yAxis) define the texture projection that was used for a face with
  // oldNormal. We want to update (m_xAxis, m_yAxis) to be usable on a face with
  // newNormal. Since this is the "projection" method (attempts to emulate
  // ParaxialTexCoordSystem), we want to modify (m_xAxis, m_yAxis) as little as possible
  // and only make 90 degree rotations if necessary.

  // Method: build a cube where the front face is the old texture projection (m_xAxis,
  // m_yAxis) and the other 5 faces are 90 degree rotations from that. Use the "face"
  // whose texture normal (cross product of the x and y axis) is closest to newNormal (the
  // new face normal).

  auto possibleTexAxes = std::vector<std::pair<vm::vec3, vm::vec3>>{};
  possibleTexAxes.emplace_back(xAxis(), yAxis()); // possibleTexAxes[0] = front
  possibleTexAxes.emplace_back(yAxis(), xAxis()); // possibleTexAxes[1] = back
  const auto rotations = std::vector<vm::quat3>{
    vm::quat3{
      vm::normalize(xAxis()),
      vm::to_radians(90.0)}, // possibleTexAxes[2]= bottom (90 degrees CCW about m_xAxis)
    vm::quat3{vm::normalize(xAxis()), vm::to_radians(-90.0)}, // possibleTexAxes[3] = top
    vm::quat3{vm::normalize(yAxis()), vm::to_radians(90.0)},  // possibleTexAxes[4] = left
    vm::quat3{
      vm::normalize(yAxis()), vm::to_radians(-90.0)}, // possibleTexAxes[5] = right
  };
  for (const auto& rotation : rotations)
  {
    possibleTexAxes.emplace_back(rotation * xAxis(), rotation * yAxis());
  }
  assert(possibleTexAxes.size() == 6);

  auto possibleTexAxesNormals = std::vector<vm::vec3>{};
  for (const auto& axes : possibleTexAxes)
  {
    const auto texNormal = vm::normalize(vm::cross(axes.first, axes.second));
    possibleTexAxesNormals.push_back(texNormal);
  }
  assert(possibleTexAxesNormals.size() == 6);

  // Find the index in possibleTexAxesNormals of the normal closest to the newNormal (face
  // normal)
  auto cosAngles = std::vector<FloatType>{};
  for (const auto& texNormal : possibleTexAxesNormals)
  {
    const auto cosAngle = vm::dot(texNormal, newNormal);
    cosAngles.push_back(cosAngle);
  }
  assert(cosAngles.size() == 6);

  const auto index = std::distance(
    cosAngles.begin(), std::max_element(cosAngles.begin(), cosAngles.end()));
  assert(index >= 0);
  assert(index < 6);

  // Skip 0 because it is "no change".
  // Skip 1 because it's a 180 degree flip, we prefer to just project the "front" texture
  // axes.
  if (index >= 2)
  {
    const auto& axes = possibleTexAxes[static_cast<size_t>(index)];
    m_xAxis = axes.first;
    m_yAxis = axes.second;
  }
}

void ParallelTexCoordSystem::updateNormalWithRotation(
  const vm::vec3& oldNormal,
  const vm::vec3& newNormal,
  const BrushFaceAttributes& /* attribs */)
{
  const auto cross = vm::cross(oldNormal, newNormal);
  if (cross == vm::vec3::zero())
  {
    // oldNormal and newNormal are either the same or opposite.
    // in this case, no need to update the texture axes.
    return;
  }

  const auto axis = vm::normalize(cross);
  const auto angle = vm::measure_angle(newNormal, oldNormal, axis);
  const auto rotation = vm::quat3{axis, angle};

  m_xAxis = rotation * xAxis();
  m_yAxis = rotation * yAxis();
}

float ParallelTexCoordSystem::computeTextureAngle(
  const vm::plane3& oldBoundary, const vm::mat4x4& transformation) const
{
  const auto rotationScale = vm::strip_translation(transformation);
  const auto oldNormal = oldBoundary.normal;
  const auto newNormal = vm::normalize(rotationScale * oldNormal);

  const auto nonTextureRotation = vm::quat{oldNormal, newNormal};
  const auto newXAxis = vm::normalize(rotationScale * xAxis());
  const auto nonXAxis = vm::normalize(nonTextureRotation * yAxis());
  const auto angle = vm::to_degrees(vm::measure_angle(nonXAxis, newXAxis, newNormal));
  return float(angle);
}

} // namespace TrenchBroom::Model
