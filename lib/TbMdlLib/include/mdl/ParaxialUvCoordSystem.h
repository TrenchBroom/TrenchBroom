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

#include <tuple>

namespace tb::mdl
{

/**
 * A UV coordinate system whose axes are snapped to the axis plane which best matches the
 * face plane. Clients should use UvCoordSystem instead of using this class directly.
 */
class ParaxialUvCoordSystem
{
private:
  size_t m_index = 0;
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;
  UvAttributes m_uvAttributes;

  kdl_reflect_decl(ParaxialUvCoordSystem, m_index, m_uAxis, m_vAxis, m_uvAttributes);

private:
  ParaxialUvCoordSystem(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);
  ParaxialUvCoordSystem(const vm::vec3d& normal, const UvAttributes& uvAttributes);

public:
  /**
   * Creates a UV coordinate system snapped to the axis plane which best matches the
   * plane defined by the given points.
   *
   * Returns an error if the given points do not define a plane.
   */
  static Result<ParaxialUvCoordSystem> createFromPoints(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);

  /**
   * Creates a UV coordinate system snapped to the axis plane which best matches the
   * given normal.
   */
  static Result<ParaxialUvCoordSystem> createFromNormal(
    const vm::vec3d& normal, const UvAttributes& uvAttributes);

  /**
   * Creates a paraxial UV coordinate system that approximates the given Valve format UV
   * coordinate system as closely as possible.
   *
   * Returns an error if the given points do not define a plane.
   */
  static Result<ParaxialUvCoordSystem> createFromParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis);

  static size_t planeNormalIndex(const vm::vec3d& normal);
  static std::tuple<vm::vec3d, vm::vec3d, vm::vec3d> axes(size_t index);

  const UvAttributes& uvAttributes() const;
  void copyUvAttributes(const UvAttributes& uvAttributes);

  vm::vec3d uAxis() const;
  vm::vec3d vAxis() const;
  vm::vec3d normal() const;

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
};

} // namespace tb::mdl
