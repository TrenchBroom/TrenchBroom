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

#include "TexCoordSystem.h"

#include "Model/BrushFace.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

namespace TrenchBroom
{
namespace Model
{
TexCoordSystemSnapshot::~TexCoordSystemSnapshot() = default;

void TexCoordSystemSnapshot::restore(TexCoordSystem& coordSystem) const
{
  coordSystem.doRestoreSnapshot(*this);
}

std::unique_ptr<TexCoordSystemSnapshot> TexCoordSystemSnapshot::clone() const
{
  return doClone();
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

std::unique_ptr<TexCoordSystem> TexCoordSystem::clone() const
{
  return doClone();
}

std::unique_ptr<TexCoordSystemSnapshot> TexCoordSystem::takeSnapshot() const
{
  return doTakeSnapshot();
}

vm::vec3 TexCoordSystem::xAxis() const
{
  return getXAxis();
}

vm::vec3 TexCoordSystem::yAxis() const
{
  return getYAxis();
}

void TexCoordSystem::resetCache(
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const BrushFaceAttributes& attribs)
{
  doResetCache(point0, point1, point2, attribs);
}

void TexCoordSystem::resetTextureAxes(const vm::vec3& normal)
{
  doResetTextureAxes(normal);
}

void TexCoordSystem::resetTextureAxesToParaxial(const vm::vec3& normal, const float angle)
{
  doResetTextureAxesToParaxial(normal, angle);
}

void TexCoordSystem::resetTextureAxesToParallel(const vm::vec3& normal, const float angle)
{
  doResetTextureAxesToParaxial(normal, angle);
}

vm::vec2f TexCoordSystem::getTexCoords(
  const vm::vec3& point,
  const BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize) const
{
  return doGetTexCoords(point, attribs, textureSize);
}

void TexCoordSystem::setRotation(
  const vm::vec3& normal, const float oldAngle, const float newAngle)
{
  doSetRotation(normal, oldAngle, newAngle);
}

void TexCoordSystem::transform(
  const vm::plane3& oldBoundary,
  const vm::plane3& newBoundary,
  const vm::mat4x4& transformation,
  BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize,
  bool lockTexture,
  const vm::vec3& invariant)
{
  doTransform(
    oldBoundary,
    newBoundary,
    transformation,
    attribs,
    textureSize,
    lockTexture,
    invariant);
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
      doUpdateNormalWithRotation(oldNormal, newNormal, attribs);
      break;
    case WrapStyle::Projection:
      doUpdateNormalWithProjection(newNormal, attribs);
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
  const auto [invertible, fromPlane] = invert(toPlane);
  const auto transform = fromPlane * vm::mat4x4::zero_out<2>() * toPlane;
  const auto texX = normalize(transform * getXAxis());
  const auto texY = normalize(transform * getYAxis());
  assert(invertible);
  unused(invertible);

  vm::vec3 vAxis, hAxis;
  size_t xIndex = 0;
  size_t yIndex = 0;

  // we prefer to use the texture axis which is closer to the XY plane for horizontal
  // movement
  if (std::abs(texX.z()) < std::abs(texY.z()))
  {
    hAxis = texX;
    vAxis = texY;
    xIndex = 0;
    yIndex = 1;
  }
  else if (std::abs(texY.z()) < std::abs(texX.z()))
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

    if (std::abs(dot(right, texX)) > std::abs(dot(right, texY)))
    {
      // the right view axis is closer to the X texture axis
      hAxis = texX;
      vAxis = texY;
      xIndex = 0;
      yIndex = 1;
    }
    else if (std::abs(dot(right, texY)) > std::abs(dot(right, texX)))
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
      if (std::abs(dot(up, texY)) > std::abs(dot(up, texX)))
      {
        // the up view axis is closer to the Y texture axis
        hAxis = texX;
        vAxis = texY;
        xIndex = 0;
        yIndex = 1;
      }
      else if (std::abs(dot(up, texX)) > std::abs(dot(up, texY)))
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

  vm::vec2f actualOffset;
  if (dot(right, hAxis) >= 0.0)
  {
    actualOffset[xIndex] = -offset.x();
  }
  else
  {
    actualOffset[xIndex] = +offset.x();
  }
  if (dot(up, vAxis) >= 0.0)
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
  const float actualAngle = isRotationInverted(normal) ? -angle : angle;
  attribs.setRotation(attribs.rotation() + actualAngle);
}

void TexCoordSystem::shearTexture(const vm::vec3& normal, const vm::vec2f& factors)
{
  doShearTexture(normal, factors);
}

vm::mat4x4 TexCoordSystem::toMatrix(const vm::vec2f& o, const vm::vec2f& s) const
{
  const vm::vec3 x = safeScaleAxis(getXAxis(), s.x());
  const vm::vec3 y = safeScaleAxis(getYAxis(), s.y());
  const vm::vec3 z = getZAxis();

  return vm::mat4x4(
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
    1.0);
}

vm::mat4x4 TexCoordSystem::fromMatrix(
  const vm::vec2f& offset, const vm::vec2f& scale) const
{
  const auto [invertible, result] = invert(toMatrix(offset, scale));
  assert(invertible);
  unused(invertible);
  return result;
}

float TexCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  return doMeasureAngle(currentAngle, center, point);
}

vm::vec2f TexCoordSystem::computeTexCoords(
  const vm::vec3& point, const vm::vec2f& scale) const
{
  return vm::vec2f(
    dot(point, safeScaleAxis(getXAxis(), scale.x())),
    dot(point, safeScaleAxis(getYAxis(), scale.y())));
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> TexCoordSystem::
  toParallel(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const
{
  return doToParallel(point0, point1, point2, attribs);
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> TexCoordSystem::
  toParaxial(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const
{
  return doToParaxial(point0, point1, point2, attribs);
}
} // namespace Model
} // namespace TrenchBroom
