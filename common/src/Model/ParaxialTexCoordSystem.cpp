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

#include "ParaxialTexCoordSystem.h"

#include "Assets/Texture.h"
#include "Ensure.h"
#include "FloatType.h"
#include "Model/BrushFace.h"
#include "Model/ParallelTexCoordSystem.h"

#include "vm/plane.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <array>
#include <cassert>
#include <cmath>
#include <optional>
#include <utility> // for std::pair

namespace TrenchBroom
{
namespace Model
{
const vm::vec3 ParaxialTexCoordSystem::BaseAxes[] = {
  vm::vec3(0.0, 0.0, 1.0),
  vm::vec3(1.0, 0.0, 0.0),
  vm::vec3(0.0, -1.0, 0.0),
  vm::vec3(0.0, 0.0, -1.0),
  vm::vec3(1.0, 0.0, 0.0),
  vm::vec3(0.0, -1.0, 0.0),
  vm::vec3(1.0, 0.0, 0.0),
  vm::vec3(0.0, 1.0, 0.0),
  vm::vec3(0.0, 0.0, -1.0),
  vm::vec3(-1.0, 0.0, 0.0),
  vm::vec3(0.0, 1.0, 0.0),
  vm::vec3(0.0, 0.0, -1.0),
  vm::vec3(0.0, 1.0, 0.0),
  vm::vec3(1.0, 0.0, 0.0),
  vm::vec3(0.0, 0.0, -1.0),
  vm::vec3(0.0, -1.0, 0.0),
  vm::vec3(1.0, 0.0, 0.0),
  vm::vec3(0.0, 0.0, -1.0),
};

ParaxialTexCoordSystem::ParaxialTexCoordSystem(
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const BrushFaceAttributes& attribs)
  : m_index(0)
{
  resetCache(point0, point1, point2, attribs);
}

ParaxialTexCoordSystem::ParaxialTexCoordSystem(
  const vm::vec3& normal, const BrushFaceAttributes& attribs)
  : m_index(0)
{
  setRotation(normal, 0.0f, attribs.rotation());
}

ParaxialTexCoordSystem::ParaxialTexCoordSystem(
  const size_t index, const vm::vec3& xAxis, const vm::vec3& yAxis)
  : m_index(index)
  , m_xAxis(xAxis)
  , m_yAxis(yAxis)
{
}

size_t ParaxialTexCoordSystem::planeNormalIndex(const vm::vec3& normal)
{
  size_t bestIndex = 0;
  FloatType bestDot = static_cast<FloatType>(0.0);
  for (size_t i = 0; i < 6; ++i)
  {
    const FloatType curDot = dot(normal, BaseAxes[i * 3]);
    if (curDot > bestDot)
    { // no need to use -altaxis for qbsp, but -oldaxis is necessary
      bestDot = curDot;
      bestIndex = i;
    }
  }
  return bestIndex;
}

void ParaxialTexCoordSystem::axes(const size_t index, vm::vec3& xAxis, vm::vec3& yAxis)
{
  vm::vec3 temp;
  axes(index, xAxis, yAxis, temp);
}

void ParaxialTexCoordSystem::axes(
  size_t index, vm::vec3& xAxis, vm::vec3& yAxis, vm::vec3& projectionAxis)
{
  xAxis = BaseAxes[index * 3 + 1];
  yAxis = BaseAxes[index * 3 + 2];
  projectionAxis = BaseAxes[(index / 2) * 6];
}

vm::plane3 ParaxialTexCoordSystem::planeFromPoints(
  const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2)
{
  const vm::vec3 normal = normalize(cross(point2 - point0, point1 - point0));
  return vm::plane3(point0, normal);
}

std::unique_ptr<TexCoordSystem> ParaxialTexCoordSystem::doClone() const
{
  return std::make_unique<ParaxialTexCoordSystem>(m_index, m_xAxis, m_yAxis);
}

std::unique_ptr<TexCoordSystemSnapshot> ParaxialTexCoordSystem::doTakeSnapshot() const
{
  return std::unique_ptr<TexCoordSystemSnapshot>();
}

void ParaxialTexCoordSystem::doRestoreSnapshot(
  const TexCoordSystemSnapshot& /* snapshot */)
{
  ensure(false, "unsupported");
}

vm::vec3 ParaxialTexCoordSystem::getXAxis() const
{
  return m_xAxis;
}

vm::vec3 ParaxialTexCoordSystem::getYAxis() const
{
  return m_yAxis;
}

vm::vec3 ParaxialTexCoordSystem::getZAxis() const
{
  return BaseAxes[m_index * 3 + 0];
}

void ParaxialTexCoordSystem::doResetCache(
  const vm::vec3& point0,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const BrushFaceAttributes& attribs)
{
  const vm::vec3 normal = planeFromPoints(point0, point1, point2).normal;
  setRotation(normal, 0.0f, attribs.rotation());
}

void ParaxialTexCoordSystem::doResetTextureAxes(const vm::vec3& /* normal */) {}
void ParaxialTexCoordSystem::doResetTextureAxesToParaxial(
  const vm::vec3& /* normal */, const float /* angle */)
{
}
void ParaxialTexCoordSystem::doResetTextureAxesToParallel(
  const vm::vec3& /* normal */, const float /* angle */)
{
}

bool ParaxialTexCoordSystem::isRotationInverted(const vm::vec3& normal) const
{
  const size_t index = planeNormalIndex(normal);
  return index % 2 == 0;
}

vm::vec2f ParaxialTexCoordSystem::doGetTexCoords(
  const vm::vec3& point,
  const BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize) const
{
  return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / textureSize;
}

void ParaxialTexCoordSystem::doSetRotation(
  const vm::vec3& normal, const float /* oldAngle */, const float newAngle)
{
  m_index = planeNormalIndex(normal);
  axes(m_index, m_xAxis, m_yAxis);
  rotateAxes(m_xAxis, m_yAxis, vm::to_radians(static_cast<FloatType>(newAngle)), m_index);
}

void ParaxialTexCoordSystem::doTransform(
  const vm::plane3& oldBoundary,
  const vm::plane3& newBoundary,
  const vm::mat4x4& transformation,
  BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize,
  bool lockTexture,
  const vm::vec3& oldInvariant)
{
  const auto offset = transformation * vm::vec3::zero();
  const auto& oldNormal = oldBoundary.normal;
  auto newNormal = newBoundary.normal;
  assert(vm::is_unit(newNormal, vm::C::almost_zero()));

  // fix some rounding errors - if the old and new texture axes are almost the same, use
  // the old axis
  if (vm::is_equal(newNormal, oldNormal, 0.01))
  {
    newNormal = oldNormal;
  }

  if (!lockTexture || attribs.xScale() == 0.0f || attribs.yScale() == 0.0f)
  {
    setRotation(newNormal, attribs.rotation(), attribs.rotation());
    return;
  }

  // calculate the current texture coordinates of the origin
  const auto oldInvariantTexCoords =
    computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();

  // project the texture axes onto the boundary plane along the texture Z axis
  const auto scale = vm::vec2{attribs.scale()};
  const auto boundaryOffset = oldBoundary.project_point(vm::vec3::zero(), getZAxis());
  const auto oldXAxis = oldBoundary.project_point(m_xAxis * scale.x(), getZAxis());
  const auto oldYAxis = oldBoundary.project_point(m_yAxis * scale.y(), getZAxis());
  if (boundaryOffset && oldXAxis && oldYAxis)
  {
    const auto oldXAxisOnBoundary = *oldXAxis - *boundaryOffset;
    const auto oldYAxisOnBoundary = *oldYAxis - *boundaryOffset;

    // transform the projected texture axes and compensate the translational component
    const auto transformedXAxis = transformation * oldXAxisOnBoundary - offset;
    const auto transformedYAxis = transformation * oldYAxisOnBoundary - offset;

    const bool preferX = textureSize.x() >= textureSize.y();

    // obtain the new texture plane norm and the new base texture axes
    vm::vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
    const auto newIndex = planeNormalIndex(newNormal);
    axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);

    const auto newTexturePlane = vm::plane3{0.0, newProjectionAxis};

    // project the transformed texture axes onto the new texture projection plane
    const auto projectedTransformedXAxis =
      newTexturePlane.project_point(transformedXAxis);
    const auto projectedTransformedYAxis =
      newTexturePlane.project_point(transformedYAxis);
    assert(
      !vm::is_nan(projectedTransformedXAxis) && !vm::is_nan(projectedTransformedYAxis));

    const auto normalizedXAxis = vm::normalize(projectedTransformedXAxis);
    const auto normalizedYAxis = vm::normalize(projectedTransformedYAxis);

    // determine the rotation angle from the dot product of the new base axes and the
    // transformed, projected and normalized texture axes
    auto cosX = float(vm::dot(newBaseXAxis, normalizedXAxis));
    auto cosY = float(vm::dot(newBaseYAxis, normalizedYAxis));
    assert(!vm::is_nan(cosX));
    assert(!vm::is_nan(cosY));

    auto radX = std::acos(cosX);
    if (vm::dot(vm::cross(newBaseXAxis, normalizedXAxis), newProjectionAxis) < 0.0)
    {
      radX *= -1.0f;
    }

    auto radY = std::acos(cosY);
    if (vm::dot(vm::cross(newBaseYAxis, normalizedYAxis), newProjectionAxis) < 0.0)
    {
      radY *= -1.0f;
    }

    // TODO: be smarter about choosing between the X and Y axis rotations - sometimes
    // either one can be better
    auto rad = preferX ? radX : radY;

    // for some reason, when the texture plane normal is the Y axis, we must rotation
    // clockwise
    const auto planeNormIndex = (newIndex / 2) * 6;
    if (planeNormIndex == 12)
    {
      rad *= -1.0f;
    }

    const auto newRotation = vm::correct(vm::normalize_degrees(vm::to_degrees(rad)), 4);
    doSetRotation(newNormal, newRotation, newRotation);

    // finally compute the scaling factors
    auto newScale = vm::correct(
      vm::vec2f{
        float(length(projectedTransformedXAxis)),
        float(length(projectedTransformedYAxis)),
      },
      4);

    // the sign of the scaling factors depends on the angle between the new texture axis
    // and the projected transformed axis
    if (vm::dot(m_xAxis, normalizedXAxis) < 0.0)
    {
      newScale[0] *= -1.0f;
    }
    if (vm::dot(m_yAxis, normalizedYAxis) < 0.0)
    {
      newScale[1] *= -1.0f;
    }

    // compute the parameters of the transformed texture coordinate system
    const auto newInvariant = transformation * oldInvariant;

    // determine the new texture coordinates of the transformed center of the face, sans
    // offsets
    const auto newInvariantTexCoords = computeTexCoords(newInvariant, newScale);

    // since the center should be invariant, the offsets are determined by the difference
    // of the current and the original texture coordiknates of the center
    const auto newOffset = vm::correct(
      attribs.modOffset(oldInvariantTexCoords - newInvariantTexCoords, textureSize), 4);

    assert(!vm::is_nan(newOffset));
    assert(!vm::is_nan(newScale));
    assert(!vm::is_nan(newRotation));
    assert(!vm::is_zero(newScale.x(), vm::Cf::almost_zero()));
    assert(!vm::is_zero(newScale.y(), vm::Cf::almost_zero()));

    attribs.setOffset(newOffset);
    attribs.setScale(newScale);
    attribs.setRotation(newRotation);
  }
}

void ParaxialTexCoordSystem::doUpdateNormalWithProjection(
  const vm::vec3& newNormal, const BrushFaceAttributes& attribs)
{
  setRotation(newNormal, attribs.rotation(), attribs.rotation());
}

void ParaxialTexCoordSystem::doUpdateNormalWithRotation(
  const vm::vec3& /* oldNormal */,
  const vm::vec3& newNormal,
  const BrushFaceAttributes& attribs)
{
  // not supported; fall back to doUpdateNormalWithProjection
  doUpdateNormalWithProjection(newNormal, attribs);
}

void ParaxialTexCoordSystem::doShearTexture(
  const vm::vec3& /* normal */, const vm::vec2f& /* factors */)
{
  // not supported
}

float ParaxialTexCoordSystem::doMeasureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  const auto rot = vm::quatf(vm::vec3f::pos_z(), -vm::to_radians(currentAngle));
  const auto vec = rot * vm::vec3f(point - center);

  const auto angleInRadians =
    vm::Cf::two_pi()
    - vm::measure_angle(vm::normalize(vec), vm::vec3f::pos_x(), vm::vec3f::pos_z());
  return vm::to_degrees(angleInRadians);
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParaxialTexCoordSystem::
  doToParallel(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const
{
  return ParallelTexCoordSystem::fromParaxial(point0, point1, point2, attribs);
}

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParaxialTexCoordSystem::
  doToParaxial(
    const vm::vec3&,
    const vm::vec3&,
    const vm::vec3&,
    const BrushFaceAttributes& attribs) const
{
  // Already in the requested format
  return {clone(), attribs};
}

void ParaxialTexCoordSystem::rotateAxes(
  vm::vec3& xAxis,
  vm::vec3& yAxis,
  const FloatType angleInRadians,
  const size_t planeNormIndex) const
{
  const vm::vec3 rotAxis =
    vm::cross(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
  const vm::quat3 rot(rotAxis, angleInRadians);
  xAxis = vm::correct(rot * xAxis);
  yAxis = vm::correct(rot * yAxis);
}

namespace FromParallel
{
struct ParaxialAttribs
{
  float rotation = 0.0;
  vm::vec2f scale = vm::vec2f::fill(1.0f);
  vm::vec2f offset;
};

struct ParaxialAttribsNoOffset
{
  float rotate = 0.0;
  vm::vec2f scale = vm::vec2f::fill(1.0f);
};

static std::pair<size_t, size_t> getSTAxes(const vm::vec3f& snappedNormal)
{
  if (snappedNormal[0] != 0.0f)
  {
    return std::make_pair(1, 2);
  }
  else if (snappedNormal[1] != 0.0f)
  {
    return std::make_pair(0, 2);
  }
  else
  {
    return std::make_pair(0, 1);
  }
}

static vm::vec2f projectToAxisPlane(
  const vm::vec3f& snappedNormal, const vm::vec3f& point)
{
  const auto axes = getSTAxes(snappedNormal);
  return vm::vec2f(point[axes.first], point[axes.second]);
}

static void textureAxisFromFacePlane(
  const vm::plane3& facePlane,
  vm::vec3f& xVecOut,
  vm::vec3f& yVecOut,
  vm::vec3f& snappedNormal)
{
  vm::vec3 xVec, yVec, projectionAxis;
  ParaxialTexCoordSystem::axes(
    ParaxialTexCoordSystem::planeNormalIndex(facePlane.normal),
    xVec,
    yVec,
    projectionAxis);

  xVecOut = vm::vec3f(xVec);
  yVecOut = vm::vec3f(yVec);
  snappedNormal = -vm::vec3f(projectionAxis);
}

static vm::mat2x2f mat2x2_rotation_degrees(const float degrees)
{
  const float r = vm::to_radians(degrees);
  const float cosr = std::cos(r);
  const float sinr = std::sin(r);

  return {cosr, -sinr, sinr, cosr};
}

static float mat2x2_extract_rotation_degrees(const vm::mat2x2f& m)
{
  const vm::vec2f point =
    m * vm::vec2f(1, 0); // choice of this matters if there's shearing
  const float rotation = std::atan2(point[1], point[0]);
  return vm::to_degrees(rotation);
}

static vm::vec2f getTexCoordsAtPoint(
  const ParaxialAttribs& attribs, const vm::plane3& facePlane, const vm::vec3& point)
{
  BrushFaceAttributes tempAttribs("");
  tempAttribs.setRotation(attribs.rotation);
  tempAttribs.setScale(attribs.scale);
  tempAttribs.setOffset(attribs.offset);

  auto temp = ParaxialTexCoordSystem(facePlane.normal, tempAttribs);
  return temp.getTexCoords(point, tempAttribs, vm::vec2f(1.0f, 1.0f));
}

static ParaxialAttribs appendOffset(
  const ParaxialAttribsNoOffset& attribs, const vm::vec2f& offset)
{
  ParaxialAttribs result;
  result.rotation = attribs.rotate;
  result.scale = attribs.scale;
  result.offset = offset;
  return result;
}

static float clockwiseDegreesBetween(vm::vec2f start, vm::vec2f end)
{
  start = vm::normalize(start);
  end = vm::normalize(end);

  const float cosAngle = vm::max(-1.0f, vm::min(1.0f, vm::dot(start, end)));
  const float unsignedDegrees = vm::to_degrees(std::acos(cosAngle));

  if (unsignedDegrees < 0.000001f)
  {
    return 0.0f;
  }

  // get a normal for the rotation plane using the right-hand rule
  // if this is pointing up (vm::vec3f(0,0,1)), it's counterclockwise rotation.
  // if this is pointing down (vm::vec3f(0,0,-1)), it's clockwise rotation.
  const vm::vec3f rotationNormal = vm::normalize(
    vm::cross(vm::vec3f(start[0], start[1], 0.0f), vm::vec3f(end[0], end[1], 0.0f)));

  const float normalsCosAngle = vm::dot(rotationNormal, vm::vec3f(0, 0, 1));
  if (normalsCosAngle >= 0)
  {
    // counterclockwise rotation
    return -unsignedDegrees;
  }
  // clockwise rotation
  return unsignedDegrees;
}

static std::optional<ParaxialAttribsNoOffset> extractParaxialAttribs(
  vm::mat2x2f M, const vm::plane3& facePlane, const bool preserveX)
{
  // Check for shear, because we might tweak M to remove it
  {
    vm::vec2f Xvec = vm::vec2f(M[0][0], M[1][0]);
    vm::vec2f Yvec = vm::vec2f(M[0][1], M[1][1]);
    const float cosAngle = vm::dot(vm::normalize(Xvec), vm::normalize(Yvec));

    if (std::fabs(cosAngle) > 0.001f)
    {
      // Detected shear

      if (preserveX)
      {
        const float degreesToY = clockwiseDegreesBetween(Xvec, Yvec);
        const bool clockwise = (degreesToY > 0.0f);

        // turn 90 degrees from Xvec
        const vm::vec2f newYdir = vm::normalize(vm::vec2f(vm::cross(
          vm::vec3f(0, 0, clockwise ? -1.0f : 1.0f), vm::vec3f(Xvec[0], Xvec[1], 0.0))));

        // scalar projection of the old Yvec onto newYDir to get the new Yscale
        const float newYscale = vm::dot(Yvec, newYdir);
        Yvec = newYdir * static_cast<float>(newYscale);
      }
      else
      {
        const float degreesToX = clockwiseDegreesBetween(Yvec, Xvec);
        const bool clockwise = (degreesToX > 0.0f);

        // turn 90 degrees from Yvec
        const vm::vec2f newXdir = vm::normalize(vm::vec2f(vm::cross(
          vm::vec3f(0, 0, clockwise ? -1.0f : 1.0f), vm::vec3f(Yvec[0], Yvec[1], 0.0))));

        // scalar projection of the old Xvec onto newXDir to get the new Xscale
        const float newXscale = vm::dot(Xvec, newXdir);
        Xvec = newXdir * static_cast<float>(newXscale);
      }

      // recheck, they should be perpendicular now
      const float newCosAngle = vm::dot(vm::normalize(Xvec), vm::normalize(Yvec));
      assert(fabs(newCosAngle) <= 0.001);
      unused(newCosAngle);

      // update M
      M[0][0] = Xvec[0];
      M[1][0] = Xvec[1];

      M[0][1] = Yvec[0];
      M[1][1] = Yvec[1];
    }
  }

  // extract abs(scale)
  const double absXScale = sqrt(pow(M[0][0], 2.0) + pow(M[1][0], 2.0));
  const double absYScale = sqrt(pow(M[0][1], 2.0) + pow(M[1][1], 2.0));
  const vm::mat2x2f applyAbsScaleM{
    static_cast<float>(absXScale), 0.0f, 0.0f, static_cast<float>(absYScale)};

  vm::vec3f vecs[2];
  vm::vec3f snappedNormal;
  textureAxisFromFacePlane(facePlane, vecs[0], vecs[1], snappedNormal);

  const vm::vec2f sAxis = projectToAxisPlane(snappedNormal, vecs[0]);
  const vm::vec2f tAxis = projectToAxisPlane(snappedNormal, vecs[1]);

  // This is an identity matrix possibly with negative signs.
  const vm::mat2x2f axisFlipsM{sAxis[0], sAxis[1], tAxis[0], tAxis[1]};

  // M can be built like this and the orider guides how we
  // strip off components of it later in this function.
  //
  // M = scaleM * rotateM * axisFlipsM;

  // strip off the magnitude component of the scale, and `axisFlipsM`.
  auto [applyAbsScaleMInvOk, applyAbsScaleMInv] = vm::invert(applyAbsScaleM);
  auto [axisFlipsMInvOk, axisFlipsMInv] = vm::invert(axisFlipsM);

  if (!applyAbsScaleMInvOk || !axisFlipsMInvOk)
  {
    return std::nullopt;
  }

  const vm::mat2x2f flipRotate = applyAbsScaleMInv * M * axisFlipsMInv;

  // We don't know the signs on the scales, which will mess up figuring out the rotation,
  // so try all 4 combinations
  static const std::array<float, 2> negativeOneAndOne{-1.0, 1.0};
  for (const float xScaleSign : negativeOneAndOne)
  {
    for (const float yScaleSign : negativeOneAndOne)
    {
      // "apply" - matrix constructed to apply a guessed value
      // "guess" - this matrix might not be what we think

      const vm::mat2x2f applyGuessedFlipM{xScaleSign, 0, 0, yScaleSign};

      auto [invOk, inv] = vm::invert(applyGuessedFlipM);
      if (!invOk)
      {
        continue;
      }
      const vm::mat2x2f rotateMGuess = inv * flipRotate;
      const float angleGuess = mat2x2_extract_rotation_degrees(rotateMGuess);

      const vm::mat2x2f applyAngleGuessM = mat2x2_rotation_degrees(angleGuess);
      const vm::mat2x2f Mguess =
        applyGuessedFlipM * applyAbsScaleM * applyAngleGuessM * axisFlipsM;

      if (
        std::fabs(M[0][0] - Mguess[0][0]) < 0.001f
        && std::fabs(M[0][1] - Mguess[0][1]) < 0.001f
        && std::fabs(M[1][0] - Mguess[1][0]) < 0.001f
        && std::fabs(M[1][1] - Mguess[1][1]) < 0.001f)
      {

        ParaxialAttribsNoOffset reversed;
        reversed.rotate = angleGuess;
        reversed.scale[0] = xScaleSign / static_cast<float>(absXScale);
        reversed.scale[1] = yScaleSign / static_cast<float>(absYScale);
        return reversed;
      }
    }
  }

  return std::nullopt;
}

static std::optional<ParaxialAttribs> texCoordMatrixToParaxial(
  const vm::plane3& faceplane,
  const vm::mat4x4f& worldToTexSpace,
  const std::array<vm::vec3f, 3>& facePoints)
{
  // First get the un-rotated, un-scaled unit texture vecs (based on the face plane).
  vm::vec3f snappedNormal;
  vm::vec3f unrotatedVecs[2];
  textureAxisFromFacePlane(faceplane, unrotatedVecs[0], unrotatedVecs[1], snappedNormal);

  // Grab the UVs of the 3 reference points
  vm::vec2f facepointsUVs[3];
  for (size_t i = 0; i < 3; ++i)
  {
    facepointsUVs[i] = vm::vec2f(worldToTexSpace * vm::vec4f(facePoints[i], 1.0f));
  }

  // Project the 3 reference points onto the axis plane. They are now 2d points.
  vm::vec2f facepointsProjected[3];
  for (size_t i = 0; i < 3; ++i)
  {
    facepointsProjected[i] = projectToAxisPlane(snappedNormal, facePoints[i]);
  }

  // Now make 2 vectors out of our 3 points (so we are ignoring translation for now)
  const vm::vec2f p0p1 = facepointsProjected[1] - facepointsProjected[0];
  const vm::vec2f p0p2 = facepointsProjected[2] - facepointsProjected[0];

  const vm::vec2f p0p1UV = facepointsUVs[1] - facepointsUVs[0];
  const vm::vec2f p0p2UV = facepointsUVs[2] - facepointsUVs[0];

  /*
  Find a 2x2 transformation matrix that maps p0p1 to p0p1UV, and p0p2 to p0p2UV

      [ a b ] [ p0p1.x ] = [ p0p1UV.x ]
      [ c d ] [ p0p1.y ]   [ p0p1UV.y ]

      [ a b ] [ p0p2.x ] = [ p0p1UV.x ]
      [ c d ] [ p0p2.y ]   [ p0p2UV.y ]

  writing as a system of equations:

      a * p0p1.x + b * p0p1.y = p0p1UV.x
      c * p0p1.x + d * p0p1.y = p0p1UV.y
      a * p0p2.x + b * p0p2.y = p0p2UV.x
      c * p0p2.x + d * p0p2.y = p0p2UV.y

  back to a matrix equation, with the unknowns in a column vector:

     [ p0p1UV.x ]   [ p0p1.x p0p1.y 0       0      ] [ a ]
     [ p0p1UV.y ] = [ 0       0     p0p1.x p0p1.y  ] [ b ]
     [ p0p2UV.x ]   [ p0p2.x p0p2.y 0       0      ] [ c ]
     [ p0p2UV.y ]   [ 0       0     p0p2.x p0p2.y  ] [ d ]

   */
  const vm::mat4x4f M{
    p0p1[0],
    p0p1[1],
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    p0p1[0],
    p0p1[1],
    p0p2[0],
    p0p2[1],
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    p0p2[0],
    p0p2[1]};

  const auto [mInvOk, Minv] = vm::invert(M);
  if (!mInvOk)
  {
    return std::nullopt;
  }
  const vm::vec4f abcd = Minv * vm::vec4f(p0p1UV[0], p0p1UV[1], p0p2UV[0], p0p2UV[1]);

  const vm::mat2x2f texPlaneToUV{abcd[0], abcd[1], abcd[2], abcd[3]};

  const auto result = extractParaxialAttribs(texPlaneToUV, faceplane, false);
  if (!result.has_value())
  {
    return std::nullopt;
  }

  // figure out texture offset by testing one point.
  // NOTE: the choice of point shouldn't matter in the case when the conversion is
  // lossless (no shearing). However, if there is shearing (which we can't capture in the
  // paraxial format), this test point should be somewhere on the face, because the
  // texture may only be aligned properly around this point.
  const vm::vec3f testPoint = facePoints[0];
  const vm::vec2f testActualUV = getTexCoordsAtPoint(
    appendOffset(*result, vm::vec2f(0, 0)), faceplane, vm::vec3(testPoint));
  const vm::vec2f testDesiredUV = vm::vec2f(worldToTexSpace * vm::vec4f(testPoint, 1.0f));
  return appendOffset(*result, testDesiredUV - testActualUV);
}

/**
 * Converts the given Valve tex coord system to matrix form, such that
 *
 *            [     s      ]
 *  M * vec = [     t      ]
 *            [distOffPlane]
 *            [     1      ]
 *
 * where vec is a world space position that we want to compute the s/t coordinates of,
 * s/t are the texture coordinates in pixels (same units as texture size),
 * and distOffPlane is the distance of `vec` off the face plane in world space.
 */
static vm::mat4x4f valveTo4x4Matrix(
  const vm::plane3& facePlane,
  const BrushFaceAttributes& attribs,
  const vm::vec3& xAxis,
  const vm::vec3& yAxis)
{
  vm::mat4x4f result;

  // fill in columns 0..2
  for (size_t i = 0; i < 3; ++i)
  {
    // column, row
    result[i][0] = static_cast<float>(xAxis[i]) / attribs.scale().x();
    result[i][1] = static_cast<float>(yAxis[i]) / attribs.scale().y();
    result[i][2] = static_cast<float>(facePlane.normal[i]);
    result[i][3] = 0.0f;
  }
  // column 3
  result[3][0] = attribs.offset().x();
  result[3][1] = attribs.offset().y();
  result[3][2] = static_cast<float>(-facePlane.distance);
  result[3][3] = 1.0f;

  return result;
}
} // namespace FromParallel

std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> ParaxialTexCoordSystem::
  fromParallel(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs,
    const vm::vec3& xAxis,
    const vm::vec3& yAxis)
{
  const vm::plane3 facePlane = planeFromPoints(point0, point1, point2);
  const vm::mat4x4f worldToTexSpace =
    FromParallel::valveTo4x4Matrix(facePlane, attribs, xAxis, yAxis);
  const auto facePoints =
    std::array<vm::vec3f, 3>{vm::vec3f(point0), vm::vec3f(point1), vm::vec3f(point2)};

  const auto conversionResult =
    FromParallel::texCoordMatrixToParaxial(facePlane, worldToTexSpace, facePoints);

  BrushFaceAttributes newAttribs = attribs;
  if (conversionResult.has_value())
  {
    newAttribs.setOffset(conversionResult->offset);
    newAttribs.setScale(conversionResult->scale);
    newAttribs.setRotation(conversionResult->rotation);
  }
  else
  {
    newAttribs.setOffset(vm::vec2f::zero());
    newAttribs.setScale(vm::vec2f::one());
    newAttribs.setRotation(0.0f);
  }

  return {
    std::make_unique<ParaxialTexCoordSystem>(point0, point1, point2, newAttribs),
    newAttribs};
}
} // namespace Model
} // namespace TrenchBroom
