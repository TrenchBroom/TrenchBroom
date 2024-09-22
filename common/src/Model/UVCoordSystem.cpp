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

#include "UVCoordSystem.h"

#include "Model/BrushFace.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

namespace TrenchBroom::Model
{

UVCoordSystemSnapshot::~UVCoordSystemSnapshot() = default;

void UVCoordSystemSnapshot::restore(UVCoordSystem& coordSystem) const
{
  coordSystem.restoreSnapshot(*this);
}

UVCoordSystem::UVCoordSystem() = default;

UVCoordSystem::~UVCoordSystem() = default;

bool operator==(const UVCoordSystem& lhs, const UVCoordSystem& rhs)
{
  return lhs.uAxis() == rhs.uAxis() && lhs.vAxis() == rhs.vAxis();
}

bool operator!=(const UVCoordSystem& lhs, const UVCoordSystem& rhs)
{
  return !(lhs == rhs);
}

void UVCoordSystem::setNormal(
  const vm::vec3& oldNormal,
  const vm::vec3& newNormal,
  const BrushFaceAttributes& attribs,
  const WrapStyle style)
{
  if (oldNormal != newNormal)
  {
    switch (style)
    {
    case WrapStyle::Rotation:
      updateNormalWithRotation(oldNormal, newNormal, attribs);
      break;
    case WrapStyle::Projection:
      updateNormalWithProjection(newNormal, attribs);
      break;
    }
  }
}

void UVCoordSystem::translate(
  const vm::vec3& normal,
  const vm::vec3& up,
  const vm::vec3& right,
  const vm::vec2f& offset,
  BrushFaceAttributes& attribs) const
{
  const auto toPlane = vm::plane_projection_matrix(0.0, normal);
  const auto fromPlane = vm::invert(toPlane);
  const auto transform = *fromPlane * vm::mat4x4::zero_out<2>() * toPlane;
  const auto transformedUAxis = vm::normalize(transform * uAxis());
  const auto transformedVAxis = vm::normalize(transform * vAxis());

  auto verticalAxis = vm::vec3{};
  auto horizontalAxis = vm::vec3{};
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
  if (attribs.scale().x() < 0.0f)
  {
    actualOffset[0] *= -1.0f;
  }
  if (attribs.scale().y() < 0.0f)
  {
    actualOffset[1] *= -1.0f;
  }

  attribs.setOffset(attribs.offset() + actualOffset);
}

void UVCoordSystem::rotate(
  const vm::vec3& normal, const float angle, BrushFaceAttributes& attribs) const
{
  const auto actualAngle = isRotationInverted(normal) ? -angle : angle;
  attribs.setRotation(attribs.rotation() + actualAngle);
}

vm::mat4x4 UVCoordSystem::toMatrix(const vm::vec2f& o, const vm::vec2f& s) const
{
  const vm::vec3 u = safeScaleAxis(uAxis(), s.x());
  const vm::vec3 v = safeScaleAxis(vAxis(), s.y());
  const vm::vec3 n = normal();

  return vm::mat4x4{
    u[0],
    u[1],
    u[2],
    o[0],
    v[0],
    v[1],
    v[2],
    o[1],
    n[0],
    n[1],
    n[2],
    0.0,
    0.0,
    0.0,
    0.0,
    1.0};
}

vm::mat4x4 UVCoordSystem::fromMatrix(
  const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return *invert(toMatrix(offset, scale));
}

vm::vec2f UVCoordSystem::computeUVCoords(
  const vm::vec3& point, const vm::vec2f& scale) const
{
  return vm::vec2f{
    float(vm::dot(point, safeScaleAxis(uAxis(), scale.x()))),
    float(vm::dot(point, safeScaleAxis(vAxis(), scale.y())))};
}

} // namespace TrenchBroom::Model
