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
#include "mdl/ParallelUvCoordSystem.h"
#include "mdl/ParaxialUvCoordSystem.h"
#include "mdl/UvAttributes.h"

#include "kd/reflection_decl.h"

#include "vm/mat.h"
#include "vm/plane.h"
#include "vm/vec.h"

#include <optional>
#include <variant>

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

/**
 * The UV coordinate system of a brush face, which maps points on the face to UV coords.
 *
 * Which kind of UV coordinate system a face uses depends on the map format.
 */
class UvCoordSystem
{
private:
  std::variant<ParaxialUvCoordSystem, ParallelUvCoordSystem> m_system;

  kdl_reflect_decl(UvCoordSystem, m_system);

public:
  explicit UvCoordSystem(ParaxialUvCoordSystem system);
  explicit UvCoordSystem(ParallelUvCoordSystem system);

  static constexpr auto wrap = [](auto uvCoordSystem) {
    return UvCoordSystem{std::move(uvCoordSystem)};
  };

  /**
   * Indicates whether this is a UV coordinate system of the given type.
   */
  template <typename T>
  bool is() const
  {
    return std::holds_alternative<T>(m_system);
  }

  /**
   * Returns nullopt if this UV coordinate system's axes are derived from the face plane
   * and therefore cannot be transferred to a face with a different plane.
   */
  std::optional<UvCoordSystemSnapshot> takeSnapshot() const;
  void restoreSnapshot(const UvCoordSystemSnapshot& snapshot);

  UvAttributes uvAttributes() const;

  /**
   * Sets the given UV attributes and updates the UV axes to match their rotation.
   */
  Result<void> setUvAttributes(const vm::vec3d& normal, const UvAttributes& uvAttributes);

  /**
   * Sets the given UV attributes without updating the UV axes.
   */
  void copyUvAttributes(const UvAttributes& uvAttributes);

  vm::vec3d uAxis() const;
  vm::vec3d vAxis() const;
  vm::vec3d normal() const;

  void resetCache(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2);
  void reset(const vm::vec3d& normal);
  void resetToParaxial(const vm::vec3d& normal, float angle);
  void resetToParallel(const vm::vec3d& normal, float angle);

  vm::vec2f uvCoords(const vm::vec3d& point, const vm::vec2f& textureSize) const;

  /**
   * Computes the UV coords of the given point using the given UV attributes instead of
   * this UV coordinate system's own attributes.
   */
  vm::vec2f uvCoords(
    const vm::vec3d& point,
    const UvAttributes& uvAttributes,
    const vm::vec2f& textureSize) const;

  void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant);
  void setNormal(const vm::vec3d& oldNormal, const vm::vec3d& newNormal, WrapStyle style);

  void translate(
    const vm::vec3d& normal,
    const vm::vec3d& up,
    const vm::vec3d& right,
    const vm::vec2f& offset);
  Result<void> rotate(const vm::vec3d& normal, float angle);
  void shear(const vm::vec3d& normal, const vm::vec2f& factors);

  vm::mat4x4d toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;
  vm::mat4x4d fromMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;

  float measureAngle(const vm::vec2f& center, const vm::vec2f& point) const;

  Result<UvCoordSystem> toParallel(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2) const;
  Result<UvCoordSystem> toParaxial(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2) const;

private:
  void setRotation(const vm::vec3d& normal, float oldAngle, float newAngle);
  bool isRotationInverted(const vm::vec3d& normal) const;
};

} // namespace tb::mdl
