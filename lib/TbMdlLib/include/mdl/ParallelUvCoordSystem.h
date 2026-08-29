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

#pragma once

#include "base/Result.h"
#include "mdl/UvAttributes.h"

#include "kd/reflection_decl.h"

#include "vm/mat.h"
#include "vm/plane.h"
#include "vm/vec.h"

namespace tb::mdl
{

/**
 * A UV coordinate system whose axes are independent of the face plane. Clients should use
 * UvCoordSystem instead of using this class directly.
 */
class ParallelUvCoordSystem
{
private:
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;
  UvAttributes m_uvAttributes;

  kdl_reflect_decl(ParallelUvCoordSystem, m_uAxis, m_vAxis, m_uvAttributes);

private:
  ParallelUvCoordSystem(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);
  ParallelUvCoordSystem(
    const vm::vec3d& uAxis, const vm::vec3d& vAxis, const UvAttributes& uvAttributes);

public:
  /**
   * Creates a UV coordinate system projected from the plane defined by the given points,
   * with its axes rotated by the rotation of the given UV attributes.
   *
   * Returns an error if the given points do not define a plane.
   */
  static Result<ParallelUvCoordSystem> createFromPoints(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);

  /**
   * Creates a UV coordinate system with the given axes, e.g. as read from a Valve format
   * map file.
   */
  static Result<ParallelUvCoordSystem> createFromAxes(
    const vm::vec3d& uAxis, const vm::vec3d& vAxis, const UvAttributes& uvAttributes);

  /**
   * Creates a parallel UV coordinate system with axes matching the given paraxial UV
   * coordinate system.
   *
   * Returns an error if the given points do not define a plane.
   */
  static Result<ParallelUvCoordSystem> createFromParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);

  const UvAttributes& uvAttributes() const;
  void copyUvAttributes(const UvAttributes& uvAttributes);

  vm::vec3d uAxis() const;
  vm::vec3d vAxis() const;
  vm::vec3d normal() const;
  void setAxes(const vm::vec3d& uAxis, const vm::vec3d& vAxis);

  void resetCache(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2);

  void reset(const vm::vec3d& normal);
  void resetToParaxial(const vm::vec3d& normal, float angle);
  void resetToParallel(const vm::vec3d& normal, float angle);

  void setRotation(const vm::vec3d& normal, float oldAngle, float newAngle);

  void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant);

  void shear(const vm::vec3d& normal, const vm::vec2f& factors);

  float measureAngle(const vm::vec2f& center, const vm::vec2f& point) const;

  bool isRotationInverted(const vm::vec3d& normal) const;

  void updateNormalWithProjection(const vm::vec3d& newNormal);
  void updateNormalWithRotation(const vm::vec3d& oldNormal, const vm::vec3d& newNormal);

private:
  float computeRotationAngle(
    const vm::plane3d& oldBoundary, const vm::mat4x4d& transformation) const;
};

} // namespace tb::mdl
