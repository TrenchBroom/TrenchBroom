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
#include "mdl/UvCoordSystem.h"

#include "vm/vec.h"

#include <memory>
#include <optional>

namespace tb::mdl
{

class ParaxialUvCoordSystem : public UvCoordSystem
{
private:
  size_t m_index = 0;
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;

public:
  ParaxialUvCoordSystem(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes);
  ParaxialUvCoordSystem(const vm::vec3d& normal, const UvAttributes& uvAttributes);
  ParaxialUvCoordSystem(size_t index, const vm::vec3d& uAxis, const vm::vec3d& vAxis);

  static std::tuple<std::unique_ptr<UvCoordSystem>, UvAttributes> fromParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis);

  static size_t planeNormalIndex(const vm::vec3d& normal);
  static std::tuple<vm::vec3d, vm::vec3d, vm::vec3d> axes(size_t index);

  std::unique_ptr<UvCoordSystem> clone() const override;
  std::optional<UvCoordSystemSnapshot> takeSnapshot() const override;
  void restoreSnapshot(const UvCoordSystemSnapshot& snapshot) override;

  vm::vec3d uAxis() const override;
  vm::vec3d vAxis() const override;
  vm::vec3d normal() const override;

  void resetCache(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) override;
  void reset(const vm::vec3d& normal) override;
  void resetToParaxial(const vm::vec3d& normal, float angle) override;
  void resetToParallel(const vm::vec3d& normal, float angle) override;

  void setRotation(const vm::vec3d& normal, float oldAngle, float newAngle) override;
  void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    UvAttributes& uvAttributes,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant) override;

  void shear(const vm::vec3d& normal, const vm::vec2f& factors) override;

  float measureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const override;

  std::tuple<std::unique_ptr<UvCoordSystem>, UvAttributes> toParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) const override;
  std::tuple<std::unique_ptr<UvCoordSystem>, UvAttributes> toParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const UvAttributes& uvAttributes) const override;

private:
  bool isRotationInverted(const vm::vec3d& normal) const override;

  void updateNormalWithProjection(
    const vm::vec3d& newNormal, const UvAttributes& uvAttributes) override;
  void updateNormalWithRotation(
    const vm::vec3d& oldNormal,
    const vm::vec3d& newNormal,
    const UvAttributes& uvAttributes) override;

  deleteCopyAndMove(ParaxialUvCoordSystem);
};

} // namespace tb::mdl
