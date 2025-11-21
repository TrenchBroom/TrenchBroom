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

#include "ParaxialUVCoordSystem.h"

#include "mdl/BrushFace.h"
#include "mdl/ParallelUVCoordSystem.h"

#include "kd/contracts.h"

#include "vm/plane.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <array>
#include <cmath>
#include <optional>

namespace tb::mdl
{
namespace
{

const vm::vec3d BaseAxes[] = {
  {0.0, 0.0, 1.0},
  {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {0.0, 0.0, -1.0},
  {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, -1.0},
  {-1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, -1.0},
  {0.0, 1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 0.0, -1.0},
  {0.0, -1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 0.0, -1.0},
};

struct ParaxialAttribs
{
  float rotation = 0.0;
  vm::vec2f scale = vm::vec2f{1, 1};
  vm::vec2f offset;
};

struct ParaxialAttribsNoOffset
{
  float rotate = 0.0;
  vm::vec2f scale = vm::vec2f{1, 1};
};

std::tuple<size_t, size_t> getSTAxes(const vm::vec3f& snappedNormal)
{
  if (snappedNormal[0] != 0.0f)
  {
    return {1, 2};
  }
  if (snappedNormal[1] != 0.0f)
  {
    return {0, 2};
  }
  return {0, 1};
}

vm::vec2f projectToAxisPlane(const vm::vec3f& snappedNormal, const vm::vec3f& point)
{
  const auto [s, t] = getSTAxes(snappedNormal);
  return {point[s], point[t]};
}

std::tuple<vm::vec3f, vm::vec3f, vm::vec3f> uvAxesFromFacePlane(
  const vm::plane3d& facePlane)
{
  const auto index = ParaxialUVCoordSystem::planeNormalIndex(facePlane.normal);
  const auto [uAxis, vAxis, pAxis] = ParaxialUVCoordSystem::axes(index);

  return {
    vm::vec3f{uAxis},
    vm::vec3f{vAxis},
    -vm::vec3f{pAxis},
  };
}

std::tuple<vm::vec3d, vm::vec3d> rotateAxes(
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis,
  const double angleInRadians,
  const size_t planeNormIndex)
{
  const auto rotAxis =
    vm::cross(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
  const auto rot = vm::quatd{rotAxis, angleInRadians};

  return {
    vm::correct(rot * uAxis),
    vm::correct(rot * vAxis),
  };
}

vm::mat2x2f mat2x2_rotation_degrees(const float degrees)
{
  const auto r = vm::to_radians(degrees);
  const auto cosr = std::cos(r);
  const auto sinr = std::sin(r);

  return {cosr, -sinr, sinr, cosr};
}

float mat2x2_extract_rotation_degrees(const vm::mat2x2f& m)
{
  const auto point = m * vm::vec2f{1, 0}; // choice of this matters if there's shearing
  const auto rotation = std::atan2(point[1], point[0]);
  return vm::to_degrees(rotation);
}

vm::vec2f getUVCoordsAtPoint(
  const ParaxialAttribs& attribs, const vm::plane3d& facePlane, const vm::vec3d& point)
{
  auto tempAttribs = BrushFaceAttributes{""};
  tempAttribs.setRotation(attribs.rotation);
  tempAttribs.setScale(attribs.scale);
  tempAttribs.setOffset(attribs.offset);

  auto temp = ParaxialUVCoordSystem{facePlane.normal, tempAttribs};
  return temp.uvCoords(point, tempAttribs, vm::vec2f{1.0f, 1.0f});
}

ParaxialAttribs appendOffset(
  const ParaxialAttribsNoOffset& attribs, const vm::vec2f& offset)
{
  return {attribs.rotate, attribs.scale, offset};
}

float clockwiseDegreesBetween(vm::vec2f start, vm::vec2f end)
{
  start = vm::normalize(start);
  end = vm::normalize(end);

  const auto cosAngle = vm::max(-1.0f, vm::min(1.0f, vm::dot(start, end)));
  const auto unsignedDegrees = vm::to_degrees(std::acos(cosAngle));

  if (unsignedDegrees < 0.000001f)
  {
    return 0.0f;
  }

  // get a normal for the rotation plane using the right-hand rule if this is pointing up
  // (vm::vec3f(0,0,1)), it's counterclockwise rotation. if this is pointing down
  // (vm::vec3f(0,0,-1)), it's clockwise rotation.
  const auto rotationNormal =
    vm::normalize(vm::cross(vm::vec3f{start.xy(), 0.0f}, vm::vec3f{end.xy(), 0.0f}));

  const auto normalsCosAngle = vm::dot(rotationNormal, vm::vec3f{0, 0, 1});
  if (normalsCosAngle >= 0)
  {
    // counterclockwise rotation
    return -unsignedDegrees;
  }
  // clockwise rotation
  return unsignedDegrees;
}

std::optional<ParaxialAttribsNoOffset> extractParaxialAttribs(
  vm::mat2x2f M, const vm::plane3d& facePlane, const bool preserveU)
{
  // Check for shear, because we might tweak M to remove it
  {
    auto uVec = vm::vec2f{M[0][0], M[1][0]};
    auto vVec = vm::vec2f{M[0][1], M[1][1]};
    const auto cosAngle = vm::dot(vm::normalize(uVec), vm::normalize(vVec));

    if (std::fabs(cosAngle) > 0.001f)
    {
      // Detected shear

      if (preserveU)
      {
        const auto degreesToV = clockwiseDegreesBetween(uVec, vVec);
        const auto clockwise = (degreesToV > 0.0f);

        // turn 90 degrees from xVec
        const auto newVdir = vm::normalize(vm::vec2f{vm::cross(
          vm::vec3f{0, 0, clockwise ? -1.0f : 1.0f}, vm::vec3f{uVec.xy(), 0.0})});

        // scalar projection of the old vVec onto newVDir to get the new vScale
        const auto newVscale = vm::dot(vVec, newVdir);
        vVec = newVdir * float(newVscale);
      }
      else
      {
        const auto degreesToU = clockwiseDegreesBetween(vVec, uVec);
        const auto clockwise = (degreesToU > 0.0f);

        // turn 90 degrees from Yvec
        const auto newUdir = vm::normalize(vm::vec2f{vm::cross(
          vm::vec3f{0, 0, clockwise ? -1.0f : 1.0f}, vm::vec3f{vVec.xy(), 0.0})});

        // scalar projection of the old uVec onto newUDir to get the new uScale
        const auto newUscale = vm::dot(uVec, newUdir);
        uVec = newUdir * float(newUscale);
      }

      // recheck, they should be perpendicular now
      const auto newCosAngle = vm::dot(vm::normalize(uVec), vm::normalize(vVec));
      contract_assert(fabs(newCosAngle) <= 0.001);

      // update M
      M[0][0] = uVec[0];
      M[1][0] = uVec[1];

      M[0][1] = vVec[0];
      M[1][1] = vVec[1];
    }
  }

  // extract abs(scale)
  const auto absUScale = sqrt(pow(M[0][0], 2.0) + pow(M[1][0], 2.0));
  const auto absVScale = sqrt(pow(M[0][1], 2.0) + pow(M[1][1], 2.0));
  const auto applyAbsScaleM = vm::mat2x2f{float(absUScale), 0.0f, 0.0f, float(absVScale)};

  const auto [v1, v2, snappedNormal] = uvAxesFromFacePlane(facePlane);
  const auto uAxis = projectToAxisPlane(snappedNormal, v1);
  const auto vAxis = projectToAxisPlane(snappedNormal, v2);

  // This is an identity matrix possibly with negative signs.
  const auto axisFlipsM = vm::mat2x2f{uAxis[0], uAxis[1], vAxis[0], vAxis[1]};

  // M can be built like this and the orider guides how we strip off components of it
  // later in this function.
  //
  // M = scaleM * rotateM * axisFlipsM;

  // strip off the magnitude component of the scale, and `axisFlipsM`.
  auto applyAbsScaleMInv = vm::invert(applyAbsScaleM);
  auto axisFlipsMInv = vm::invert(axisFlipsM);

  if (!applyAbsScaleMInv || !axisFlipsMInv)
  {
    return std::nullopt;
  }

  const auto flipRotate = *applyAbsScaleMInv * M * *axisFlipsMInv;

  // We don't know the signs on the scales, which will mess up figuring out the rotation,
  // so try all 4 combinations
  constexpr auto negativeOneAndOne = std::array<float, 2>{-1.0, 1.0};
  for (const auto uScaleSign : negativeOneAndOne)
  {
    for (const auto vScaleSign : negativeOneAndOne)
    {
      // "apply" - matrix constructed to apply a guessed value
      // "guess" - this matrix might not be what we think

      const auto applyGuessedFlipM = vm::mat2x2f{uScaleSign, 0, 0, vScaleSign};

      if (const auto inv = vm::invert(applyGuessedFlipM))
      {
        const auto rotateMGuess = *inv * flipRotate;
        const auto angleGuess = mat2x2_extract_rotation_degrees(rotateMGuess);

        const auto applyAngleGuessM = mat2x2_rotation_degrees(angleGuess);
        const auto Mguess =
          applyGuessedFlipM * applyAbsScaleM * applyAngleGuessM * axisFlipsM;

        if (
          std::fabs(M[0][0] - Mguess[0][0]) < 0.001f
          && std::fabs(M[0][1] - Mguess[0][1]) < 0.001f
          && std::fabs(M[1][0] - Mguess[1][0]) < 0.001f
          && std::fabs(M[1][1] - Mguess[1][1]) < 0.001f)
        {
          return ParaxialAttribsNoOffset{
            angleGuess,
            {
              uScaleSign / float(absUScale),
              vScaleSign / float(absVScale),
            },
          };
        }
      }
    }
  }

  return std::nullopt;
}

std::optional<ParaxialAttribs> uvCoordMatrixToParaxial(
  const vm::plane3d& faceplane,
  const vm::mat4x4f& worldToUVSpace,
  const std::array<vm::vec3f, 3>& facePoints)
{
  // First get the un-rotated, un-scaled unit UV vecs (based on the face plane).
  const auto [unrotU, unrotV, snappedNormal] = uvAxesFromFacePlane(faceplane);

  // Grab the UVs of the 3 reference points
  vm::vec2f facepointsUVs[3];
  for (size_t i = 0; i < 3; ++i)
  {
    facepointsUVs[i] = vm::vec2f{worldToUVSpace * vm::vec4f{facePoints[i], 1.0f}};
  }

  // Project the 3 reference points onto the axis plane. They are now 2d points.
  vm::vec2f facepointsProjected[3];
  for (size_t i = 0; i < 3; ++i)
  {
    facepointsProjected[i] = projectToAxisPlane(snappedNormal, facePoints[i]);
  }

  // Now make 2 vectors out of our 3 points (so we are ignoring translation for now)
  const auto p0p1 = facepointsProjected[1] - facepointsProjected[0];
  const auto p0p2 = facepointsProjected[2] - facepointsProjected[0];

  const auto p0p1UV = facepointsUVs[1] - facepointsUVs[0];
  const auto p0p2UV = facepointsUVs[2] - facepointsUVs[0];

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
  const auto M = vm::mat4x4f{
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

  const auto Minv = vm::invert(M);
  if (!Minv)
  {
    return std::nullopt;
  }

  const auto abcd = *Minv * vm::vec4f{p0p1UV[0], p0p1UV[1], p0p2UV[0], p0p2UV[1]};
  const auto uvPlaneToUV = vm::mat2x2f{abcd[0], abcd[1], abcd[2], abcd[3]};

  const auto result = extractParaxialAttribs(uvPlaneToUV, faceplane, false);
  if (!result)
  {
    return std::nullopt;
  }

  // figure out texture offset by testing one point.
  // NOTE: the choice of point shouldn't matter in the case when the conversion is
  // lossless (no shearing). However, if there is shearing (which we can't capture in the
  // paraxial format), this test point should be somewhere on the face, because the
  // texture may only be aligned properly around this point.
  const auto testPoint = facePoints[0];
  const auto testActualUV = getUVCoordsAtPoint(
    appendOffset(*result, vm::vec2f{0, 0}), faceplane, vm::vec3d{testPoint});
  const auto testDesiredUV = vm::vec2f{worldToUVSpace * vm::vec4f{testPoint, 1.0f}};
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
vm::mat4x4f valveTo4x4Matrix(
  const vm::plane3d& facePlane,
  const BrushFaceAttributes& attribs,
  const vm::vec3d& uAxis,
  const vm::vec3d& vAxis)
{
  auto result = vm::mat4x4f{};

  // fill in columns 0..2
  for (size_t i = 0; i < 3; ++i)
  {
    // column, row
    result[i][0] = float(uAxis[i]) / attribs.scale().x();
    result[i][1] = float(vAxis[i]) / attribs.scale().y();
    result[i][2] = float(facePlane.normal[i]);
    result[i][3] = 0.0f;
  }
  // column 3
  result[3][0] = attribs.offset().x();
  result[3][1] = attribs.offset().y();
  result[3][2] = float(-facePlane.distance);
  result[3][3] = 1.0f;

  return result;
}
} // namespace

ParaxialUVCoordSystem::ParaxialUVCoordSystem(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const BrushFaceAttributes& attribs)
{
  resetCache(point0, point1, point2, attribs);
}

ParaxialUVCoordSystem::ParaxialUVCoordSystem(
  const vm::vec3d& normal, const BrushFaceAttributes& attribs)
{
  setRotation(normal, 0.0f, attribs.rotation());
}

ParaxialUVCoordSystem::ParaxialUVCoordSystem(
  const size_t index, const vm::vec3d& uAxis, const vm::vec3d& vAxis)
  : m_index{index}
  , m_uAxis{uAxis}
  , m_vAxis{vAxis}
{
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParaxialUVCoordSystem::
  fromParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis)
{
  const auto facePlane = vm::from_points(point0, point1, point2);
  const auto worldToTexSpace = valveTo4x4Matrix(*facePlane, attribs, uAxis, vAxis);
  const auto facePoints = std::array<vm::vec3f, 3>{
    vm::vec3f{point0},
    vm::vec3f{point1},
    vm::vec3f{point2},
  };

  const auto conversionResult =
    uvCoordMatrixToParaxial(*facePlane, worldToTexSpace, facePoints);

  auto newAttribs = attribs;
  if (conversionResult.has_value())
  {
    newAttribs.setOffset(conversionResult->offset);
    newAttribs.setScale(conversionResult->scale);
    newAttribs.setRotation(conversionResult->rotation);
  }
  else
  {
    newAttribs.setOffset(vm::vec2f{0, 0});
    newAttribs.setScale(vm::vec2f{1, 1});
    newAttribs.setRotation(0.0f);
  }

  return {
    std::make_unique<ParaxialUVCoordSystem>(point0, point1, point2, newAttribs),
    newAttribs,
  };
}

size_t ParaxialUVCoordSystem::planeNormalIndex(const vm::vec3d& normal)
{
  size_t bestIndex = 0;
  auto bestDot = double(0);
  for (size_t i = 0; i < 6; ++i)
  {
    const auto curDot = vm::dot(normal, BaseAxes[i * 3]);
    if (curDot > bestDot)
    { // no need to use -altaxis for qbsp, but -oldaxis is necessary
      bestDot = curDot;
      bestIndex = i;
    }
  }
  return bestIndex;
}

std::tuple<vm::vec3d, vm::vec3d, vm::vec3d> ParaxialUVCoordSystem::axes(
  const size_t index)
{
  return {
    BaseAxes[index * 3 + 1],
    BaseAxes[index * 3 + 2],
    BaseAxes[(index / 2) * 6],
  };
}

std::unique_ptr<UVCoordSystem> ParaxialUVCoordSystem::clone() const
{
  return std::make_unique<ParaxialUVCoordSystem>(m_index, m_uAxis, m_vAxis);
}

std::unique_ptr<UVCoordSystemSnapshot> ParaxialUVCoordSystem::takeSnapshot() const
{
  return nullptr;
}

void ParaxialUVCoordSystem::restoreSnapshot(const UVCoordSystemSnapshot& /* snapshot */)
{
  contract_assert(false);
}

vm::vec3d ParaxialUVCoordSystem::uAxis() const
{
  return m_uAxis;
}

vm::vec3d ParaxialUVCoordSystem::vAxis() const
{
  return m_vAxis;
}

vm::vec3d ParaxialUVCoordSystem::normal() const
{
  return BaseAxes[m_index * 3 + 0];
}

void ParaxialUVCoordSystem::resetCache(
  const vm::vec3d& point0,
  const vm::vec3d& point1,
  const vm::vec3d& point2,
  const BrushFaceAttributes& attribs)
{
  if (const auto normal = vm::plane_normal(point0, point1, point2))
  {
    setRotation(*normal, 0.0f, attribs.rotation());
  }
}

void ParaxialUVCoordSystem::reset(const vm::vec3d& /* normal */) {}

void ParaxialUVCoordSystem::resetToParaxial(
  const vm::vec3d& /* normal */, const float /* angle */)
{
}

void ParaxialUVCoordSystem::resetToParallel(
  const vm::vec3d& /* normal */, const float /* angle */)
{
}

vm::vec2f ParaxialUVCoordSystem::uvCoords(
  const vm::vec3d& point,
  const BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize) const
{
  return (computeUVCoords(point, attribs.scale()) + attribs.offset()) / textureSize;
}

void ParaxialUVCoordSystem::setRotation(
  const vm::vec3d& normal, const float /* oldAngle */, const float newAngle)
{
  m_index = planeNormalIndex(normal);
  std::tie(m_uAxis, m_vAxis, std::ignore) = axes(m_index);
  std::tie(m_uAxis, m_vAxis) =
    rotateAxes(m_uAxis, m_vAxis, vm::to_radians(double(newAngle)), m_index);
}

void ParaxialUVCoordSystem::transform(
  const vm::plane3d& oldBoundary,
  const vm::plane3d& newBoundary,
  const vm::mat4x4d& transformation,
  BrushFaceAttributes& attribs,
  const vm::vec2f& textureSize,
  bool lockTexture,
  const vm::vec3d& oldInvariant)
{
  const auto offset = transformation * vm::vec3d{0, 0};
  auto newBoundaryNormal = newBoundary.normal;
  contract_assert(vm::is_unit(newBoundaryNormal, vm::Cd::almost_zero()));

  // fix some rounding errors - if the old and new texture axes are almost the same, use
  // the old axis
  if (vm::is_equal(newBoundaryNormal, oldBoundary.normal, 0.01))
  {
    newBoundaryNormal = oldBoundary.normal;
  }

  if (!lockTexture || attribs.xScale() == 0.0f || attribs.yScale() == 0.0f)
  {
    setRotation(newBoundaryNormal, attribs.rotation(), attribs.rotation());
    return;
  }

  // calculate the current UV coordinates of the origin
  const auto oldInvariantUVCoords =
    computeUVCoords(oldInvariant, attribs.scale()) + attribs.offset();

  // project the UV axes onto the boundary plane along the normal axis
  const auto scale = vm::vec2d{attribs.scale()};
  const auto boundaryOffset = oldBoundary.project_point(vm::vec3d{0, 0, 0}, normal());
  const auto oldUAxis = oldBoundary.project_point(m_uAxis * scale.x(), normal());
  const auto oldVAxis = oldBoundary.project_point(m_vAxis * scale.y(), normal());
  if (boundaryOffset && oldUAxis && oldVAxis)
  {
    const auto oldUAxisOnBoundary = *oldUAxis - *boundaryOffset;
    const auto oldVAxisOnBoundary = *oldVAxis - *boundaryOffset;

    // transform the projected texture axes and compensate the translational component
    const auto transformedUAxis = transformation * oldUAxisOnBoundary - offset;
    const auto transformedVAxis = transformation * oldVAxisOnBoundary - offset;

    const bool preferU = textureSize.x() >= textureSize.y();

    // obtain the new texture plane norm and the new base texture axes
    const auto newIndex = planeNormalIndex(newBoundaryNormal);
    const auto [newBaseUAxis, newBaseVAxis, newUVNormal] = axes(newIndex);

    const auto newUVPlane = vm::plane3d{0.0, newUVNormal};

    // project the transformed texture axes onto the new texture projection plane
    const auto projectedTransformedUAxis = newUVPlane.project_point(transformedUAxis);
    const auto projectedTransformedVAxis = newUVPlane.project_point(transformedVAxis);
    contract_assert(
      !vm::is_nan(projectedTransformedUAxis) && !vm::is_nan(projectedTransformedVAxis));

    const auto normalizedUAxis = vm::normalize(projectedTransformedUAxis);
    const auto normalizedVAxis = vm::normalize(projectedTransformedVAxis);

    // determine the rotation angle from the dot product of the new base axes and the
    // transformed, projected and normalized texture axes
    const auto cosU = float(vm::dot(newBaseUAxis, normalizedUAxis));
    const auto cosV = float(vm::dot(newBaseVAxis, normalizedVAxis));
    contract_assert(!vm::is_nan(cosU));
    contract_assert(!vm::is_nan(cosV));

    auto radU = std::acos(cosU);
    if (vm::dot(vm::cross(newBaseUAxis, normalizedUAxis), newUVNormal) < 0.0)
    {
      radU *= -1.0f;
    }

    auto radV = std::acos(cosV);
    if (vm::dot(vm::cross(newBaseVAxis, normalizedVAxis), newUVNormal) < 0.0)
    {
      radV *= -1.0f;
    }

    // TODO: be smarter about choosing between the X and Y axis rotations - sometimes
    // either one can be better
    auto rad = preferU ? radU : radV;

    // for some reason, when the texture plane normal is the Y axis, we must rotation
    // clockwise
    const auto planeNormIndex = (newIndex / 2) * 6;
    if (planeNormIndex == 12)
    {
      rad *= -1.0f;
    }

    const auto newRotation = vm::correct(vm::normalize_degrees(vm::to_degrees(rad)), 4);
    setRotation(newBoundaryNormal, newRotation, newRotation);

    // finally compute the scaling factors
    auto newScale = vm::correct(
      vm::vec2f{
        float(vm::length(projectedTransformedUAxis)),
        float(vm::length(projectedTransformedVAxis)),
      },
      4);

    // the sign of the scaling factors depends on the angle between the new texture axis
    // and the projected transformed axis
    if (vm::dot(m_uAxis, normalizedUAxis) < 0.0)
    {
      newScale[0] *= -1.0f;
    }
    if (vm::dot(m_vAxis, normalizedVAxis) < 0.0)
    {
      newScale[1] *= -1.0f;
    }

    // compute the parameters of the transformed texture coordinate system
    const auto newInvariant = transformation * oldInvariant;

    // determine the new texture coordinates of the transformed center of the face, sans
    // offsets
    const auto newInvariantUVCoords = computeUVCoords(newInvariant, newScale);

    // since the center should be invariant, the offsets are determined by the difference
    // of the current and the original texture coordiknates of the center
    const auto newOffset = vm::correct(
      attribs.modOffset(oldInvariantUVCoords - newInvariantUVCoords, textureSize), 4);

    contract_assert(!vm::is_nan(newOffset));
    contract_assert(!vm::is_nan(newScale));
    contract_assert(!vm::is_nan(newRotation));
    contract_assert(!vm::is_zero(newScale.x(), vm::Cf::almost_zero()));
    contract_assert(!vm::is_zero(newScale.y(), vm::Cf::almost_zero()));

    attribs.setOffset(newOffset);
    attribs.setScale(newScale);
    attribs.setRotation(newRotation);
  }
}

void ParaxialUVCoordSystem::shear(
  const vm::vec3d& /* normal */, const vm::vec2f& /* factors */)
{
  // not supported
}

float ParaxialUVCoordSystem::measureAngle(
  const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const
{
  const auto rot = vm::quatf{vm::vec3f{0, 0, 1}, -vm::to_radians(currentAngle)};
  const auto vec = rot * vm::vec3f{point - center};

  const auto angleInRadians =
    vm::Cf::two_pi()
    - vm::measure_angle(vm::normalize(vec), vm::vec3f{1, 0, 0}, vm::vec3f{0, 0, 1});
  return vm::to_degrees(angleInRadians);
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParaxialUVCoordSystem::
  toParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs) const
{
  return ParallelUVCoordSystem::fromParaxial(point0, point1, point2, attribs);
}

std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> ParaxialUVCoordSystem::
  toParaxial(
    const vm::vec3d&,
    const vm::vec3d&,
    const vm::vec3d&,
    const BrushFaceAttributes& attribs) const
{
  // Already in the requested format
  return {clone(), attribs};
}

bool ParaxialUVCoordSystem::isRotationInverted(const vm::vec3d& normal) const
{
  const auto index = planeNormalIndex(normal);
  return index % 2 == 0;
}

void ParaxialUVCoordSystem::updateNormalWithProjection(
  const vm::vec3d& newNormal, const BrushFaceAttributes& attribs)
{
  setRotation(newNormal, attribs.rotation(), attribs.rotation());
}

void ParaxialUVCoordSystem::updateNormalWithRotation(
  const vm::vec3d& /* oldNormal */,
  const vm::vec3d& newNormal,
  const BrushFaceAttributes& attribs)
{
  // not supported; fall back to doUpdateNormalWithProjection
  updateNormalWithProjection(newNormal, attribs);
}

} // namespace tb::mdl
