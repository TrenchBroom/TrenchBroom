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

#include "kd/reflection_impl.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/vec_io.h" // IWYU pragma: keep

namespace tb::mdl
{

kdl_reflect_impl(UvCoordSystemSnapshot);

UvCoordSystem::UvCoordSystem() = default;

UvCoordSystem::~UvCoordSystem() = default;

vm::vec2f UvCoordSystem::uvCoords(
  const vm::vec3d& point,
  const UvAttributes& uvAttributes,
  const vm::vec2f& textureSize) const
{
  return (computeUvCoords(point, uAxis(), vAxis(), uvAttributes.scale)
          + uvAttributes.offset)
         / textureSize;
}

bool operator==(const UvCoordSystem& lhs, const UvCoordSystem& rhs)
{
  return lhs.uAxis() == rhs.uAxis() && lhs.vAxis() == rhs.vAxis();
}

bool operator!=(const UvCoordSystem& lhs, const UvCoordSystem& rhs)
{
  return !(lhs == rhs);
}

void UvCoordSystem::setNormal(
  const vm::vec3d& oldNormal,
  const vm::vec3d& newNormal,
  const UvAttributes& uvAttributes,
  const WrapStyle style)
{
  if (oldNormal != newNormal)
  {
    switch (style)
    {
    case WrapStyle::Rotation:
      updateNormalWithRotation(oldNormal, newNormal, uvAttributes);
      break;
    case WrapStyle::Projection:
      updateNormalWithProjection(newNormal, uvAttributes);
      break;
    }
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

vm::mat4x4d UvCoordSystem::toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return computeWorldToUvMatrix(uAxis(), vAxis(), normal(), offset, scale);
}

vm::mat4x4d UvCoordSystem::fromMatrix(
  const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return computeUvToWorldMatrix(uAxis(), vAxis(), normal(), offset, scale);
}

} // namespace tb::mdl
