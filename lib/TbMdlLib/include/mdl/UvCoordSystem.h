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

#include "base/Macros.h"
#include "mdl/UvAttributes.h"

#include "kd/reflection_decl.h"

#include "vm/mat.h"
#include "vm/plane.h"
#include "vm/vec.h"

#include <memory>
#include <optional>
#include <tuple>

namespace tb::mdl
{

/**
 * The UV axes of a UV coordinate system, used to transfer the UV alignment of one face to
 * another. Only UV coordinate systems whose axes are independent of the face plane can be
 * snapshotted and restored.
 */
struct UvCoordSystemSnapshot
{
  vm::vec3d uAxis;
  vm::vec3d vAxis;

  kdl_reflect_decl(UvCoordSystemSnapshot, uAxis, vAxis);
};

enum class WrapStyle
{
  Projection,
  Rotation
};

class UvCoordSystem
{
public:
  UvCoordSystem();
  virtual ~UvCoordSystem();

  friend bool operator==(const UvCoordSystem& lhs, const UvCoordSystem& rhs);
  friend bool operator!=(const UvCoordSystem& lhs, const UvCoordSystem& rhs);

  virtual std::unique_ptr<UvCoordSystem> clone() const = 0;

  /**
   * Returns nullopt if this UV coordinate system's axes are derived from the face plane
   * and therefore cannot be transferred to a face with a different plane.
   */
  virtual std::optional<UvCoordSystemSnapshot> takeSnapshot() const = 0;
  virtual void restoreSnapshot(const UvCoordSystemSnapshot& snapshot) = 0;

  virtual vm::vec3d uAxis() const = 0;
  virtual vm::vec3d vAxis() const = 0;
  virtual vm::vec3d normal() const = 0;

  virtual void resetCache(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) = 0;
  virtual void reset(const vm::vec3d& normal) = 0;
  virtual void resetToParaxial(const vm::vec3d& normal, float angle) = 0;
  virtual void resetToParallel(const vm::vec3d& normal, float angle) = 0;

  vm::vec2f uvCoords(
    const vm::vec3d& point,
    const UvAttributes& uvAttributes,
    const vm::vec2f& textureSize) const;

  virtual void setRotation(const vm::vec3d& normal, float oldAngle, float newAngle) = 0;
  virtual void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    UvAttributes& uvAttributes,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant) = 0;
  void setNormal(
    const vm::vec3d& oldNormal,
    const vm::vec3d& newNormal,
    const UvAttributes& uvAttributes,
    WrapStyle style);

  void translate(
    const vm::vec3d& normal,
    const vm::vec3d& up,
    const vm::vec3d& right,
    const vm::vec2f& offset,
    UvAttributes& uvAttributes) const;
  void rotate(const vm::vec3d& normal, float angle, UvAttributes& uvAttributes) const;
  virtual void shear(const vm::vec3d& normal, const vm::vec2f& factors) = 0;

  vm::mat4x4d toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;
  vm::mat4x4d fromMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;

  virtual float measureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const = 0;

  virtual std::tuple<std::unique_ptr<UvCoordSystem>, UvAttributes> toParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) const = 0;
  virtual std::tuple<std::unique_ptr<UvCoordSystem>, UvAttributes> toParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) const = 0;

private:
  virtual bool isRotationInverted(const vm::vec3d& normal) const = 0;

  virtual void updateNormalWithProjection(
    const vm::vec3d& newNormal, const UvAttributes& uvAttributes) = 0;
  virtual void updateNormalWithRotation(
    const vm::vec3d& oldNormal,
    const vm::vec3d& newNormal,
    const UvAttributes& uvAttributes) = 0;

protected:
  deleteCopyAndMove(UvCoordSystem);
};

} // namespace tb::mdl
