/*
 Copyright (C) 2026 Thomas Jones

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

#include "mdl/PrimitiveUVCoordSystem.h"

#include "mdl/ParallelUVCoordSystem.h"
#include "mdl/ParaxialUVCoordSystem.h"

#include "kd/contracts.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <cmath>

namespace tb::mdl
{

PrimitiveUVCoordSystemSnapshot::PrimitiveUVCoordSystemSnapshot(
  const vm::vec3d& uAxis, const vm::vec3d& vAxis, const vm::vec2f& offset)
  : m_uAxis{uAxis}
  , m_vAxis{vAxis}
  , m_offset{offset}
{
}

PrimitiveUVCoordSystemSnapshot::PrimitiveUVCoordSystemSnapshot(
  const PrimitiveUVCoordSystem* coordSystem)
  : m_uAxis{coordSystem->m_uAxis}
  , m_vAxis{coordSystem->m_vAxis}
  , m_offset{coordSystem->m_offset}
{
}

std::unique_ptr<UVCoordSystemSnapshot> PrimitiveUVCoordSystemSnapshot::clone() const
{
  return std::make_unique<PrimitiveUVCoordSystemSnapshot>(m_uAxis, m_vAxis, m_offset);
}

void PrimitiveUVCoordSystemSnapshot::doRestore(
  ParallelUVCoordSystem& /* coordSystem */) const
{
  contract_assert(false);
}

void PrimitiveUVCoordSystemSnapshot::doRestore(
  ParaxialUVCoordSystem& /* coordSystem */) const
{
  contract_assert(false);
}

void PrimitiveUVCoordSystemSnapshot::doRestore(PrimitiveUVCoordSystem& coordSystem) const
{
  coordSystem.m_uAxis = m_uAxis;
  coordSystem.m_vAxis = m_vAxis;
  coordSystem.m_offset = m_offset;
}

PrimitiveUVCoordSystem::PrimitiveUVCoordSystem(
  const vm::vec3d& uAxis, const vm::vec3d& vAxis, const vm::vec2f& offset)
  : m_uAxis{uAxis}
  , m_vAxis{vAxis}
  , m_offset{offset}
{
}

PrimitiveUVCoordSystem::PrimitiveUVCoordSystem(
  const vm::vec3d& normal, const UVAttributes& uvAttributes, const vm::vec2f& textureSize)
{
  setAxes(normal, uvAttributes, textureSize);
}

std::unique_ptr<UVCoordSystem> PrimitiveUVCoordSystem::clone() const
{
  return std::make_unique<PrimitiveUVCoordSystem>(m_uAxis, m_vAxis, m_offset);
}

std::unique_ptr<UVCoordSystemSnapshot> PrimitiveUVCoordSystem::takeSnapshot() const
{
  return std::make_unique<PrimitiveUVCoordSystemSnapshot>(this);
}

void PrimitiveUVCoordSystem::restoreSnapshot(const UVCoordSystemSnapshot& snapshot)
{
  snapshot.doRestore(*this);
}

vm::vec3d PrimitiveUVCoordSystem::uAxis() const
{
  return vm::normalize(m_uAxis);
}

vm::vec3d PrimitiveUVCoordSystem::vAxis() const
{
  return vm::normalize(m_vAxis);
}

vm::vec3d PrimitiveUVCoordSystem::normal() const
{
  return vm::normalize(vm::cross(m_uAxis, m_vAxis));
}

UVAttributes PrimitiveUVCoordSystem::uvAttributes(const vm::vec2f& textureSize) const
{
  const auto uLength = vm::length(m_uAxis);
  const auto vLength = vm::length(m_vAxis);
  contract_assert(uLength > 0.0 && vLength > 0.0);

  // The reference frame is the one that setAxes used, which is the frame for the
  // normal opposite to the UV normal (see computeInitialAxes).
  const auto [baseUAxis, baseVAxis] = computeInitialAxes(-normal());

  const auto unitUAxis = m_uAxis / uLength;
  const auto cos = vm::dot(unitUAxis, baseUAxis);
  const auto sin = -vm::dot(unitUAxis, baseVAxis);

  auto result = UVAttributes{};
  result.offset = m_offset * textureSize;
  result.scale = vm::vec2f{
    float(1.0 / (uLength * double(textureSize.x()))),
    float(1.0 / (vLength * double(textureSize.y()))),
  };
  result.rotation =
    float(vm::correct(vm::normalize_degrees(vm::to_degrees(std::atan2(sin, cos))), 4));
  return result;
}

void PrimitiveUVCoordSystem::setUVAttributes(
  const UVAttributes& uvAttributes, const vm::vec2f& textureSize)
{
  setAxes(-normal(), uvAttributes, textureSize);
}

void PrimitiveUVCoordSystem::resetCache(
  const vm::vec3d& /* point0 */,
  const vm::vec3d& /* point1 */,
  const vm::vec3d& /* point2 */)
{
  // no-op
}

void PrimitiveUVCoordSystem::reset(const vm::vec3d& normal)
{
  std::tie(m_uAxis, m_vAxis) = computeInitialAxes(normal);
}

void PrimitiveUVCoordSystem::resetToParaxial(const vm::vec3d& normal, const float angle)
{
  const auto index = ParaxialUVCoordSystem::planeNormalIndex(normal);
  std::tie(m_uAxis, m_vAxis, std::ignore) = ParaxialUVCoordSystem::axes(index);

  const auto rot = vm::quatd{normal, double(angle)};
  m_uAxis = rot * m_uAxis;
  m_vAxis = rot * m_vAxis;
}

void PrimitiveUVCoordSystem::resetToParallel(const vm::vec3d& normal, const float angle)
{
  std::tie(m_uAxis, m_vAxis) = computeInitialAxes(normal);

  const auto rot = vm::quatd{normal, double(angle)};
  m_uAxis = rot * m_uAxis;
  m_vAxis = rot * m_vAxis;
}

vm::vec2f PrimitiveUVCoordSystem::uvCoords(
  const vm::vec3d& point, const vm::vec2f& /* textureSize */) const
{
  return vm::vec2f{
           float(vm::dot(point, m_uAxis)),
           float(vm::dot(point, m_vAxis)),
         }
         + m_offset;
}

void PrimitiveUVCoordSystem::transform(
  const vm::plane3d& /* oldBoundary */,
  const vm::plane3d& newBoundary,
  const vm::mat4x4d& transformation,
  const vm::vec2f& /* textureSize */,
  const bool lockTexture,
  const vm::vec3d& /* invariant */)
{
  // when texture lock is off, just project the current texturing
  if (!lockTexture)
  {
    updateNormalWithProjection(newBoundary.normal);
    return;
  }

  const auto inverseTransform = vm::invert(transformation);
  if (!inverseTransform)
  {
    return;
  }

  // The projection maps a world space point to normalized UV space. To keep the UV
  // coordinates of transformed points invariant, we compose the projection with the
  // inverse transformation.
  const auto n = normal();
  const auto worldToUV = vm::mat4x4d{
    m_uAxis[0],
    m_uAxis[1],
    m_uAxis[2],
    double(m_offset.x()),
    m_vAxis[0],
    m_vAxis[1],
    m_vAxis[2],
    double(m_offset.y()),
    n[0],
    n[1],
    n[2],
    0.0,
    0.0,
    0.0,
    0.0,
    1.0};

  const auto newWorldToUV = worldToUV * *inverseTransform;

  // note, the matrix is in column major format.
  for (size_t i = 0; i < 3; ++i)
  {
    m_uAxis[i] = newWorldToUV[i][0];
    m_vAxis[i] = newWorldToUV[i][1];
  }
  m_offset = vm::vec2f{float(newWorldToUV[3][0]), float(newWorldToUV[3][1])};

  contract_assert(!vm::is_nan(m_uAxis));
  contract_assert(!vm::is_nan(m_vAxis));
  contract_assert(!vm::is_nan(m_offset));
}

void PrimitiveUVCoordSystem::shear(const vm::vec3d& /* normal */, const vm::vec2f& f)
{
  // clang-format off
  const auto shear = vm::mat4x4d{
     1.0, f[0], 0.0, 0.0,
    f[1],  1.0, 0.0, 0.0,
     0.0,  0.0, 1.0, 0.0,
     0.0,  0.0, 0.0, 1.0};
  // clang-format on

  const auto toMatrix =
    vm::coordinate_system_matrix(m_uAxis, m_vAxis, normal(), vm::vec3d{0, 0, 0});
  const auto fromMatrix = vm::invert(toMatrix);

  const auto transform = *fromMatrix * shear * toMatrix;
  m_uAxis = transform * m_uAxis;
  m_vAxis = transform * m_vAxis;
}

float PrimitiveUVCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  const auto vec = vm::vec3f{point - center};
  const auto angleInRadians =
    vm::measure_angle(vm::normalize(vec), vm::vec3f{1, 0, 0}, vm::vec3f{0, 0, 1});
  return currentAngle + vm::to_degrees(angleInRadians);
}

std::unique_ptr<UVCoordSystem> PrimitiveUVCoordSystem::toParallel(
  const vm::vec3d& /* point0 */,
  const vm::vec3d& /* point1 */,
  const vm::vec3d& /* point2 */,
  const vm::vec2f& textureSize) const
{
  return std::make_unique<ParallelUVCoordSystem>(
    uAxis(), vAxis(), uvAttributes(textureSize));
}

std::unique_ptr<UVCoordSystem> PrimitiveUVCoordSystem::toParaxial(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const vm::vec2f& textureSize) const
{
  return ParaxialUVCoordSystem::fromParallel(
    point0, point1, point2, uvAttributes(textureSize), uAxis(), vAxis());
}

void PrimitiveUVCoordSystem::setAxes(
  const vm::vec3d& normal, const UVAttributes& uvAttributes, const vm::vec2f& textureSize)
{
  contract_pre(uvAttributes.scale.x() != 0.0f && uvAttributes.scale.y() != 0.0f);

  const auto [baseUAxis, baseVAxis] = computeInitialAxes(normal);

  const auto rot = vm::quatd{normal, double(vm::to_radians(uvAttributes.rotation))};
  const auto unitUAxis = rot * baseUAxis;
  const auto unitVAxis = rot * baseVAxis;

  m_uAxis = unitUAxis / (double(uvAttributes.scale.x()) * double(textureSize.x()));
  m_vAxis = unitVAxis / (double(uvAttributes.scale.y()) * double(textureSize.y()));
  m_offset = uvAttributes.offset / textureSize;
}

bool PrimitiveUVCoordSystem::isRotationInverted(const vm::vec3d& /* normal */) const
{
  return false;
}

void PrimitiveUVCoordSystem::updateNormalWithProjection(const vm::vec3d& newNormal)
{
  std::tie(m_uAxis, m_vAxis) = projectUVAxes(m_uAxis, m_vAxis, newNormal);
}

void PrimitiveUVCoordSystem::updateNormalWithRotation(
  const vm::vec3d& oldNormal, const vm::vec3d& newNormal)
{
  const auto cross = vm::cross(oldNormal, newNormal);
  if (cross == vm::vec3d{0, 0, 0})
  {
    // oldNormal and newNormal are either the same or opposite.
    // in this case, no need to update the texture axes.
    return;
  }

  const auto axis = vm::normalize(cross);
  const auto angle = vm::measure_angle(newNormal, oldNormal, axis);
  const auto rotation = vm::quatd{axis, angle};

  m_uAxis = rotation * m_uAxis;
  m_vAxis = rotation * m_vAxis;
}

} // namespace tb::mdl
