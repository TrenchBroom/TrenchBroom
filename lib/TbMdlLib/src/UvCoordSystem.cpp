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

#include "mdl/UvCoordSystem.h"

#include "mdl/UvUtils.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/reflection_impl.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <utility>

namespace tb::mdl
{

kdl_reflect_impl(UvCoordSystemSnapshot);

kdl_reflect_impl(UvCoordSystem);

UvCoordSystem::UvCoordSystem(ParaxialUvCoordSystem system)
  : m_system{std::move(system)}
{
}

UvCoordSystem::UvCoordSystem(ParallelUvCoordSystem system)
  : m_system{std::move(system)}
{
}

std::optional<UvCoordSystemSnapshot> UvCoordSystem::takeSnapshot() const
{
  return std::visit(
    kdl::overload(
      [](const ParaxialUvCoordSystem&) -> std::optional<UvCoordSystemSnapshot> {
        // the UV axes are derived from the face plane, so there is nothing to transfer
        return std::nullopt;
      },
      [](const ParallelUvCoordSystem& system) -> std::optional<UvCoordSystemSnapshot> {
        return UvCoordSystemSnapshot{system.uAxis(), system.vAxis()};
      }),
    m_system);
}

void UvCoordSystem::restoreSnapshot(const UvCoordSystemSnapshot& snapshot)
{
  std::visit(
    kdl::overload(
      [](ParaxialUvCoordSystem&) { contract_assert(false); },
      [&](ParallelUvCoordSystem& system) {
        system.setAxes(snapshot.uAxis, snapshot.vAxis);
      }),
    m_system);
}

vm::vec3d UvCoordSystem::uAxis() const
{
  return std::visit([](const auto& system) { return system.uAxis(); }, m_system);
}

vm::vec3d UvCoordSystem::vAxis() const
{
  return std::visit([](const auto& system) { return system.vAxis(); }, m_system);
}

vm::vec3d UvCoordSystem::normal() const
{
  return std::visit([](const auto& system) { return system.normal(); }, m_system);
}

void UvCoordSystem::resetCache(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const UvAttributes& uvAttributes)
{
  std::visit(
    [&](auto& system) { system.resetCache(point0, point1, point2, uvAttributes); },
    m_system);
}

void UvCoordSystem::reset(const vm::vec3d& normal)
{
  std::visit([&](auto& system) { system.reset(normal); }, m_system);
}

void UvCoordSystem::resetToParaxial(const vm::vec3d& normal, const float angle)
{
  std::visit([&](auto& system) { system.resetToParaxial(normal, angle); }, m_system);
}

void UvCoordSystem::resetToParallel(const vm::vec3d& normal, const float angle)
{
  std::visit([&](auto& system) { system.resetToParallel(normal, angle); }, m_system);
}

vm::vec2f UvCoordSystem::uvCoords(
  const vm::vec3d& point,
  const UvAttributes& uvAttributes,
  const vm::vec2f& textureSize) const
{
  return (computeUvCoords(point, uAxis(), vAxis(), uvAttributes.scale)
          + uvAttributes.offset)
         / textureSize;
}

void UvCoordSystem::setRotation(
  const vm::vec3d& normal, const float oldAngle, const float newAngle)
{
  std::visit(
    [&](auto& system) { system.setRotation(normal, oldAngle, newAngle); }, m_system);
}

void UvCoordSystem::transform(
  const vm::plane3d& oldBoundary,
  const vm::plane3d& newBoundary,
  const vm::mat4x4d& transformation,
  UvAttributes& uvAttributes,
  const vm::vec2f& textureSize,
  const bool lockTexture,
  const vm::vec3d& invariant)
{
  std::visit(
    [&](auto& system) {
      system.transform(
        oldBoundary,
        newBoundary,
        transformation,
        uvAttributes,
        textureSize,
        lockTexture,
        invariant);
    },
    m_system);
}

void UvCoordSystem::setNormal(
  const vm::vec3d& oldNormal,
  const vm::vec3d& newNormal,
  const UvAttributes& uvAttributes,
  const WrapStyle style)
{
  if (oldNormal != newNormal)
  {
    std::visit(
      [&](auto& system) {
        switch (style)
        {
        case WrapStyle::Rotation:
          system.updateNormalWithRotation(oldNormal, newNormal, uvAttributes);
          break;
        case WrapStyle::Projection:
          system.updateNormalWithProjection(newNormal, uvAttributes);
          break;
        }
      },
      m_system);
  }
}

void UvCoordSystem::translate(
  const vm::vec3d& normal,
  const vm::vec3d& up,
  const vm::vec3d& right,
  const vm::vec2f& offset,
  UvAttributes& uvAttributes) const
{
  const auto toPlane = vm::plane_projection_matrix(0.0, normal);
  const auto fromPlane = vm::invert(toPlane);
  const auto transform = *fromPlane * vm::mat4x4d::zero_out<2>() * toPlane;
  const auto transformedUAxis = vm::normalize(transform * uAxis());
  const auto transformedVAxis = vm::normalize(transform * vAxis());

  auto verticalAxis = vm::vec3d{};
  auto horizontalAxis = vm::vec3d{};
  size_t uIndex = 0;
  size_t vIndex = 0;

  // Select the texture axis closest to the right view axies for horizontal movement
  if (
    vm::abs(vm::dot(transformedUAxis, right)) > vm::abs(vm::dot(transformedVAxis, right)))
  {
    horizontalAxis = transformedUAxis;
    verticalAxis = transformedVAxis;
    uIndex = 0;
    vIndex = 1;
  }
  else if (
    vm::abs(vm::dot(transformedUAxis, right)) > vm::abs(vm::dot(transformedVAxis, right)))
  {
    horizontalAxis = transformedVAxis;
    verticalAxis = transformedUAxis;
    uIndex = 1;
    vIndex = 0;
  }
  else if (
    vm::abs(vm::dot(transformedUAxis, up)) > vm::abs(vm::dot(transformedVAxis, up)))
  {
    horizontalAxis = transformedVAxis;
    verticalAxis = transformedUAxis;
    uIndex = 1;
    vIndex = 0;
  }
  else if (
    vm::abs(vm::dot(transformedUAxis, up)) > vm::abs(vm::dot(transformedVAxis, up)))
  {
    horizontalAxis = transformedUAxis;
    verticalAxis = transformedVAxis;
    uIndex = 0;
    vIndex = 1;
  }
  else
  {
    // if we cannot make a choice, we better do nothing
    return;
  }


  auto actualOffset = vm::vec2f{};
  if (vm::dot(right, horizontalAxis) >= 0.0)
  {
    actualOffset[uIndex] = -offset.x();
  }
  else
  {
    actualOffset[uIndex] = +offset.x();
  }
  if (vm::dot(up, verticalAxis) >= 0.0)
  {
    actualOffset[vIndex] = -offset.y();
  }
  else
  {
    actualOffset[vIndex] = +offset.y();
  }

  // Flip offset direction when texture scale is negative
  if (uvAttributes.scale.x() < 0.0f)
  {
    actualOffset[0] *= -1.0f;
  }
  if (uvAttributes.scale.y() < 0.0f)
  {
    actualOffset[1] *= -1.0f;
  }

  uvAttributes.offset = uvAttributes.offset + actualOffset;
}

void UvCoordSystem::rotate(
  const vm::vec3d& normal, const float angle, UvAttributes& uvAttributes) const
{
  const auto actualAngle = isRotationInverted(normal) ? -angle : angle;

  uvAttributes.rotation = uvAttributes.rotation + actualAngle;
}

void UvCoordSystem::shear(const vm::vec3d& normal, const vm::vec2f& factors)
{
  std::visit([&](auto& system) { system.shear(normal, factors); }, m_system);
}

vm::mat4x4d UvCoordSystem::toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return computeWorldToUvMatrix(uAxis(), vAxis(), normal(), offset, scale);
}

vm::mat4x4d UvCoordSystem::fromMatrix(
  const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return computeUvToWorldMatrix(uAxis(), vAxis(), normal(), offset, scale);
}

float UvCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  return std::visit(
    [&](const auto& system) { return system.measureAngle(currentAngle, center, point); },
    m_system);
}

std::tuple<UvCoordSystem, UvAttributes> UvCoordSystem::toParallel(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const UvAttributes& uvAttributes) const
{
  return std::visit(
    kdl::overload(
      [&](const ParaxialUvCoordSystem&) -> std::tuple<UvCoordSystem, UvAttributes> {
        return {
          UvCoordSystem{
            ParallelUvCoordSystem::fromParaxial(point0, point1, point2, uvAttributes)},
          uvAttributes};
      },
      [&](
        const ParallelUvCoordSystem& system) -> std::tuple<UvCoordSystem, UvAttributes> {
        // Already in the requested format
        return {UvCoordSystem{system}, uvAttributes};
      }),
    m_system);
}

std::tuple<UvCoordSystem, UvAttributes> UvCoordSystem::toParaxial(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const UvAttributes& uvAttributes) const
{
  return std::visit(
    kdl::overload(
      [&](
        const ParaxialUvCoordSystem& system) -> std::tuple<UvCoordSystem, UvAttributes> {
        // Already in the requested format
        return {UvCoordSystem{system}, uvAttributes};
      },
      [&](
        const ParallelUvCoordSystem& system) -> std::tuple<UvCoordSystem, UvAttributes> {
        auto [paraxial, newUvAttributes] = ParaxialUvCoordSystem::fromParallel(
          point0, point1, point2, uvAttributes, system.uAxis(), system.vAxis());
        return {UvCoordSystem{std::move(paraxial)}, newUvAttributes};
      }),
    m_system);
}

bool UvCoordSystem::isRotationInverted(const vm::vec3d& normal) const
{
  return std::visit(
    [&](const auto& system) { return system.isRotationInverted(normal); }, m_system);
}

} // namespace tb::mdl
