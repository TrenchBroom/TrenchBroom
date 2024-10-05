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

#include "ParallelUVCoordSystem.h"

#include "Ensure.h"
#include "Model/BrushFace.h"
#include "Model/ParaxialUVCoordSystem.h"

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
std::tuple<vm::vec3d, vm::vec3d> computeInitialAxes(const vm::vec3d& normal)
{
  const auto uAxis = vm::find_abs_max_component(normal) == vm::axis::z
                       ? vm::normalize(vm::cross(vm::vec3d::pos_y(), normal))
                       : vm::normalize(vm::cross(vm::vec3d::pos_z(), normal));

  return {uAxis, vm::normalize(vm::cross(uAxis, normal))};
}

/**
 * Rotate CCW by `angle` radians about `normal`.
 */
std::tuple<vm::vec3d, vm::vec3d> applyRotation(
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const vm::vec3d& normal,
  const double angle)
{
  const auto rot = vm::quatd{normal, angle};
  return {rot * uAxis, rot * vAxis};
}

} // namespace

ParallelUVCoordSystemSnapshot::ParallelUVCoordSystemSnapshot(
  const vm::vec3d& uAxis, const vm::vec3d& vAxis)
  : m_uAxis{uAxis}
  , m_vAxis{vAxis}
{
}

ParallelUVCoordSystemSnapshot::ParallelUVCoordSystemSnapshot(
  const ParallelUVCoordSystem* coordSystem)
  : m_uAxis{coordSystem->uAxis()}
  , m_vAxis{coordSystem->vAxis()}
{
}

std::unique_ptr<UVCoordSystemSnapshot> ParallelUVCoordSystemSnapshot::clone() const
{
  return std::make_unique<ParallelUVCoordSystemSnapshot>(m_uAxis, m_vAxis);
}

void ParallelUVCoordSystemSnapshot::doRestore(ParallelUVCoordSystem& coordSystem) const
{
  coordSystem.m_uAxis = m_uAxis;
  coordSystem.m_vAxis = m_vAxis;
}

void ParallelUVCoordSystemSnapshot::doRestore(
  ParaxialUVCoordSystem& /* coordSystem */) const
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
ParallelUVCoordSystem::ParallelUVCoordSystem(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const BrushFaceAttributes& attribs)
{
  const auto normal = vm::normalize(vm::cross(point2 - point0, point1 - point0));
  std::tie(m_uAxis, m_vAxis) = computeInitialAxes(normal);
  std::tie(m_uAxis, m_vAxis) =
    applyRotation(uAxis(), vAxis(), normal, double(attribs.rotation()));
}

ParallelUVCoordSystem::ParallelUVCoordSystem(
  const vm::vec3d& uAxis, const vm::vec3d& vAxis)
  : m_uAxis{uAxis}
  , m_vAxis{vAxis}
{
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParallelUVCoordSystem::
  fromParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs)
{
  const auto tempParaxial = ParaxialUVCoordSystem{point0, point1, point2, attribs};
  return {
    ParallelUVCoordSystem{tempParaxial.uAxis(), tempParaxial.vAxis()}.clone(), attribs};
}

std::unique_ptr<UVCoordSystem> ParallelUVCoordSystem::clone() const
{
  return std::make_unique<ParallelUVCoordSystem>(uAxis(), vAxis());
}

std::unique_ptr<UVCoordSystemSnapshot> ParallelUVCoordSystem::takeSnapshot() const
{
  return std::make_unique<ParallelUVCoordSystemSnapshot>(this);
}

void ParallelUVCoordSystem::restoreSnapshot(const UVCoordSystemSnapshot& snapshot)
{
  snapshot.doRestore(*this);
}

vm::vec3d ParallelUVCoordSystem::uAxis() const
{
  return m_uAxis;
}

vm::vec3d ParallelUVCoordSystem::vAxis() const
{
  return m_vAxis;
}

vm::vec3d ParallelUVCoordSystem::normal() const
{
  return vm::normalize(vm::cross(uAxis(), vAxis()));
}

void ParallelUVCoordSystem::resetCache(
  const vm::vec3d& /* point0 */,
  const vm::vec3d& /* point1 */,
  const vm::vec3d& /* point2 */,
  const BrushFaceAttributes& /* attribs */)
{
  // no-op
}

void ParallelUVCoordSystem::reset(const vm::vec3d& normal)
{
  std::tie(m_uAxis, m_vAxis) = computeInitialAxes(normal);
}

void ParallelUVCoordSystem::resetToParaxial(const vm::vec3d& normal, float angle)
{
  const auto index = ParaxialUVCoordSystem::planeNormalIndex(normal);
  std::tie(m_uAxis, m_vAxis, std::ignore) = ParaxialUVCoordSystem::axes(index);
  std::tie(m_uAxis, m_vAxis) = applyRotation(uAxis(), vAxis(), normal, double(angle));
}

void ParallelUVCoordSystem::resetToParallel(const vm::vec3d& normal, float angle)
{
  std::tie(m_uAxis, m_vAxis) = computeInitialAxes(normal);
  std::tie(m_uAxis, m_vAxis) = applyRotation(uAxis(), vAxis(), normal, double(angle));
}

vm::vec2f ParallelUVCoordSystem::uvCoords(
  const vm::vec3d& point,
  const BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize) const
{
  return (computeUVCoords(point, attribs.scale()) + attribs.offset()) / textureSize;
}

/**
 * Rotates from `oldAngle` to `newAngle`. Both of these are in CCW degrees about
 * the texture normal (`getZAxis()`). The provided `normal` is ignored.
 */
void ParallelUVCoordSystem::setRotation(
  const vm::vec3d& /* normal */, const float oldAngle, const float newAngle)
{
  const auto angleDelta = newAngle - oldAngle;
  if (angleDelta != 0.0f)
  {
    std::tie(m_uAxis, m_vAxis) =
      applyRotation(uAxis(), vAxis(), normal(), double(vm::to_radians(angleDelta)));
  }
}

void ParallelUVCoordSystem::transform(
  const vm::plane3d& oldBoundary,
  const vm::plane3d& newBoundary,
  const vm::mat4x4d& transformation,
  BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize,
  const bool lockAlignment,
  const vm::vec3d& oldInvariant)
{
  if (attribs.xScale() == 0.0f || attribs.yScale() == 0.0f)
  {
    return;
  }

  // when texture lock is off, just project the current texturing
  if (!lockAlignment)
  {
    updateNormalWithProjection(newBoundary.normal, attribs);
    return;
  }

  const auto effectiveTransformation = transformation;

  // determine the rotation by which the UV coordinate system will be rotated about
  // its normal
  const auto angleDelta = computeRotationAngle(oldBoundary, effectiveTransformation);
  const auto newAngle =
    vm::correct(vm::normalize_degrees(attribs.rotation() + angleDelta), 4);
  assert(!vm::is_nan(newAngle));
  attribs.setRotation(newAngle);

  // calculate the current UV coordinates of the face's center
  const auto oldInvariantUVCoords =
    computeUVCoords(oldInvariant, attribs.scale()) + attribs.offset();
  assert(!vm::is_nan(oldInvariantUVCoords));

  // compute the new UV axes
  const auto worldToTexSpace = toMatrix(vm::vec2f{0, 0}, vm::vec2f{1, 1});

  // The formula for UV is:
  //
  //     uv = worldToTexSpace * point
  //
  // We want to find a new worldToTexSpace matrix, ?, such that
  // transformed points have the same UV coords as they did
  // without the transform, with the old worldToTexSpace matrix:
  //
  //     uv = ? * transform * point
  //
  // The solution for ? is (worldToTexSpace * transform_inverse)
  const auto inverseTransform = invert(effectiveTransformation);
  const auto newWorldToUVSpace = worldToTexSpace * *inverseTransform;

  // extract the new m_uAxis and m_vAxis from newWorldToUVSpace.
  // note, the matrix is in column major format.
  for (size_t i = 0; i < 3; i++)
  {
    m_uAxis[i] = newWorldToUVSpace[i][0];
    m_vAxis[i] = newWorldToUVSpace[i][1];
  }
  assert(!vm::is_nan(uAxis()));
  assert(!vm::is_nan(vAxis()));

  // determine the new texture coordinates of the transformed center of the face, sans
  // offsets
  const auto newInvariant = effectiveTransformation * oldInvariant;
  const auto newInvariantUVCoords = computeUVCoords(newInvariant, attribs.scale());

  // since the center should be invariant, the offsets are determined by the difference of
  // the current and the original texture coordinates of the center
  const auto newOffset = vm::correct(
    attribs.modOffset(oldInvariantUVCoords - newInvariantUVCoords, textureSize), 4);
  assert(!vm::is_nan(newOffset));
  attribs.setOffset(newOffset);
}

void ParallelUVCoordSystem::shear(const vm::vec3d& /* normal */, const vm::vec2f& f)
{
  // clang-format off
  const auto shear = vm::mat4x4d{
     1.0, f[0], 0.0, 0.0, 
    f[1],  1.0, 0.0, 0.0, 
     0.0,  0.0, 1.0, 0.0, 
     0.0,  0.0, 0.0, 1.0};
  // clang-format on

  const auto toMatrix =
    vm::coordinate_system_matrix(uAxis(), vAxis(), normal(), vm::vec3d::zero());
  const auto fromMatrix = vm::invert(toMatrix);

  const auto transform = *fromMatrix * shear * toMatrix;
  m_uAxis = transform * uAxis();
  m_vAxis = transform * vAxis();
}

/**
 * Measures the angle between the line from `center` to `point` and the texture space X
 * axis, in CCW degrees about the texture normal. Returns this, added to `currentAngle`
 * (also in CCW degrees).
 */
float ParallelUVCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  const auto vec = vm::vec3f{point - center};
  const auto angleInRadians =
    vm::measure_angle(vm::normalize(vec), vm::vec3f::pos_x(), vm::vec3f::pos_z());
  return currentAngle + vm::to_degrees(angleInRadians);
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParallelUVCoordSystem::
  toParallel(
    const vm::vec3d&,
    const vm::vec3d&,
    const vm::vec3d&,
    const BrushFaceAttributes& attribs) const
{
  return {clone(), attribs};
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParallelUVCoordSystem::
  toParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs) const
{
  return ParaxialUVCoordSystem::fromParallel(
    point0, point1, point2, attribs, uAxis(), vAxis());
}

bool ParallelUVCoordSystem::isRotationInverted(const vm::vec3d& /* normal */) const
{
  return false;
}

void ParallelUVCoordSystem::updateNormalWithProjection(
  const vm::vec3d& newNormal, const BrushFaceAttributes& /* attribs */)
{
  // Goal: (m_uAxis, m_vAxis) define the UV projection that was used for a face with
  // oldNormal. We want to update (m_uAxis, m_vAxis) to be usable on a face with
  // newNormal. Since this is the "projection" method (attempts to emulate
  // ParaxialUVCoordSystem), we want to modify (m_uAxis, m_vAxis) as little as possible
  // and only make 90 degree rotations if necessary.

  // Method: build a cube where the front face is the old UV projection (m_uAxis, m_vAxis)
  // and the other 5 faces are 90 degree rotations from that. Use the "face" whose UV
  // normal (cross product of the U and V axis) is closest to newNormal (the new face
  // normal).

  auto possibleUVAxes = std::vector<std::pair<vm::vec3d, vm::vec3d>>{};
  possibleUVAxes.emplace_back(uAxis(), vAxis()); // front
  possibleUVAxes.emplace_back(vAxis(), uAxis()); // back
  const auto rotations = std::vector<vm::quatd>{
    vm::quatd{vm::normalize(uAxis()), vm::to_radians(90.0)},  // bottom
    vm::quatd{vm::normalize(uAxis()), vm::to_radians(-90.0)}, // top
    vm::quatd{vm::normalize(vAxis()), vm::to_radians(90.0)},  // left
    vm::quatd{vm::normalize(vAxis()), vm::to_radians(-90.0)}, // right
  };
  for (const auto& rotation : rotations)
  {
    possibleUVAxes.emplace_back(rotation * uAxis(), rotation * vAxis());
  }
  assert(possibleUVAxes.size() == 6);

  auto possibleUVNormals = std::vector<vm::vec3d>{};
  for (const auto& axes : possibleUVAxes)
  {
    const auto normal = vm::normalize(vm::cross(axes.first, axes.second));
    possibleUVNormals.push_back(normal);
  }
  assert(possibleUVNormals.size() == 6);

  // Find the index in possibleUVNormals of the normal closest to the newNormal (face
  // normal)
  auto cosAngles = std::vector<double>{};
  for (const auto& uvNormal : possibleUVNormals)
  {
    const auto cosAngle = vm::dot(uvNormal, newNormal);
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
    const auto& axes = possibleUVAxes[static_cast<size_t>(index)];
    m_uAxis = axes.first;
    m_vAxis = axes.second;
  }
}

void ParallelUVCoordSystem::updateNormalWithRotation(
  const vm::vec3d& oldNormal,
  const vm::vec3d& newNormal,
  const BrushFaceAttributes& /* attribs */)
{
  const auto cross = vm::cross(oldNormal, newNormal);
  if (cross == vm::vec3d::zero())
  {
    // oldNormal and newNormal are either the same or opposite.
    // in this case, no need to update the texture axes.
    return;
  }

  const auto axis = vm::normalize(cross);
  const auto angle = vm::measure_angle(newNormal, oldNormal, axis);
  const auto rotation = vm::quatd{axis, angle};

  m_uAxis = rotation * uAxis();
  m_vAxis = rotation * vAxis();
}

float ParallelUVCoordSystem::computeRotationAngle(
  const vm::plane3d& oldBoundary, const vm::mat4x4d& transformation) const
{
  const auto rotationScale = vm::strip_translation(transformation);
  const auto oldNormal = oldBoundary.normal;
  const auto newNormal = vm::normalize(rotationScale * oldNormal);

  const auto nonTextureRotation = vm::quat{oldNormal, newNormal};
  const auto newUAxis = vm::normalize(rotationScale * uAxis());
  const auto nonUAxis = vm::normalize(nonTextureRotation * vAxis());
  const auto angle = vm::to_degrees(vm::measure_angle(nonUAxis, newUAxis, newNormal));
  return float(angle);
}

} // namespace TrenchBroom::Model
