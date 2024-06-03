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

#include "UVCoordSystem.h"

#include "Model/BrushFace.h"
#include "Model/ParallelUVCoordSystem.h"
#include "Model/ParaxialUVCoordSystem.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

namespace TrenchBroom::Model
{

TexCoordSystemSnapshot::~TexCoordSystemSnapshot() = default;

void TexCoordSystemSnapshot::restore(TexCoordSystem& coordSystem) const
{
  coordSystem.restoreSnapshot(*this);
}

TexCoordSystem::TexCoordSystem() = default;

TexCoordSystem::~TexCoordSystem() = default;

bool operator==(const TexCoordSystem& lhs, const TexCoordSystem& rhs)
{
  return lhs.xAxis() == rhs.xAxis() && lhs.yAxis() == rhs.yAxis();
}

bool operator!=(const TexCoordSystem& lhs, const TexCoordSystem& rhs)
{
  return !(lhs == rhs);
}

void TexCoordSystem::updateNormal(
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

void TexCoordSystem::moveTexture(
  const vm::vec3& normal,
  const vm::vec3& up,
  const vm::vec3& right,
  const vm::vec2f& offset,
  BrushFaceAttributes& attribs) const
{
  const auto toPlane = vm::plane_projection_matrix(0.0, normal);
  const auto fromPlane = vm::invert(toPlane);
  const auto transform = *fromPlane * vm::mat4x4::zero_out<2>() * toPlane;
  const auto texX = vm::normalize(transform * xAxis());
  const auto texY = vm::normalize(transform * yAxis());

  auto vAxis = vm::vec3{};
  auto hAxis = vm::vec3{};
  size_t xIndex = 0;
  size_t yIndex = 0;

  // we prefer to use the texture axis which is closer to the XY plane for horizontal
  // movement
  if (vm::abs(texX.z()) < vm::abs(texY.z()))
  {
    hAxis = texX;
    vAxis = texY;
    xIndex = 0;
    yIndex = 1;
  }
  else if (vm::abs(texY.z()) < vm::abs(texX.z()))
  {
    hAxis = texY;
    vAxis = texX;
    xIndex = 1;
    yIndex = 0;
  }
  else
  {
    // both texture axes have the same absolute angle towards the XY plane, prefer the one
    // that is closer to the right view axis for horizontal movement

    if (vm::abs(vm::dot(right, texX)) > vm::abs(vm::dot(right, texY)))
    {
      // the right view axis is closer to the X texture axis
      hAxis = texX;
      vAxis = texY;
      xIndex = 0;
      yIndex = 1;
    }
    else if (vm::abs(vm::dot(right, texY)) > vm::abs(vm::dot(right, texX)))
    {
      // the right view axis is closer to the Y texture axis
      hAxis = texY;
      vAxis = texX;
      xIndex = 1;
      yIndex = 0;
    }
    else
    {
      // the right axis is as close to the X texture axis as to the Y texture axis
      // test the up axis
      if (vm::abs(dot(up, texY)) > vm::abs(dot(up, texX)))
      {
        // the up view axis is closer to the Y texture axis
        hAxis = texX;
        vAxis = texY;
        xIndex = 0;
        yIndex = 1;
      }
      else if (vm::abs(dot(up, texX)) > vm::abs(dot(up, texY)))
      {
        // the up view axis is closer to the X texture axis
        hAxis = texY;
        vAxis = texX;
        xIndex = 1;
        yIndex = 0;
      }
      else
      {
        // this is just bad, better to do nothing
        return;
      }
    }
  }

  auto actualOffset = vm::vec2f{};
  if (vm::dot(right, hAxis) >= 0.0)
  {
    actualOffset[xIndex] = -offset.x();
  }
  else
  {
    actualOffset[xIndex] = +offset.x();
  }
  if (vm::dot(up, vAxis) >= 0.0)
  {
    actualOffset[yIndex] = -offset.y();
  }
  else
  {
    actualOffset[yIndex] = +offset.y();
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

void TexCoordSystem::rotateTexture(
  const vm::vec3& normal, const float angle, BrushFaceAttributes& attribs) const
{
  const auto actualAngle = isRotationInverted(normal) ? -angle : angle;
  attribs.setRotation(attribs.rotation() + actualAngle);
}

vm::mat4x4 TexCoordSystem::toMatrix(const vm::vec2f& o, const vm::vec2f& s) const
{
  const vm::vec3 x = safeScaleAxis(xAxis(), s.x());
  const vm::vec3 y = safeScaleAxis(yAxis(), s.y());
  const vm::vec3 z = zAxis();

  return vm::mat4x4{
    x[0],
    x[1],
    x[2],
    o[0],
    y[0],
    y[1],
    y[2],
    o[1],
    z[0],
    z[1],
    z[2],
    0.0,
    0.0,
    0.0,
    0.0,
    1.0};
}

vm::mat4x4 TexCoordSystem::fromMatrix(
  const vm::vec2f& offset, const vm::vec2f& scale) const
{
  return *invert(toMatrix(offset, scale));
}

vm::vec2f TexCoordSystem::computeTexCoords(
  const vm::vec3& point, const vm::vec2f& scale) const
{
  return vm::vec2f{
    float(vm::dot(point, safeScaleAxis(xAxis(), scale.x()))),
    float(vm::dot(point, safeScaleAxis(yAxis(), scale.y())))};
}

} // namespace TrenchBroom::Model
