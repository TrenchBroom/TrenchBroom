/*
 Copyright (C) 2026 Thomas Jones

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

#include "Macros.h"
#include "mdl/UVCoordSystem.h"

#include "vm/vec.h"

#include <memory>

namespace tb::mdl
{

class PrimitiveUVCoordSystemSnapshot : public UVCoordSystemSnapshot
{
private:
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;
  vm::vec2f m_offset;

public:
  PrimitiveUVCoordSystemSnapshot(
    const vm::vec3d& uAxis, const vm::vec3d& vAxis, const vm::vec2f& offset);
  explicit PrimitiveUVCoordSystemSnapshot(const PrimitiveUVCoordSystem* coordSystem);

  std::unique_ptr<UVCoordSystemSnapshot> clone() const override;

private:
  void doRestore(ParallelUVCoordSystem& coordSystem) const override;
  void doRestore(ParaxialUVCoordSystem& coordSystem) const override;
  void doRestore(PrimitiveUVCoordSystem& coordSystem) const override;
};

/**
 * A UV coordinate system for Quake 3 brush primitives. The authoritative data are the
 * UV axes and the offset, which map world space points to normalized UV space (in
 * texture repeats) independently of the texture size. The UV attributes (offset, scale
 * and rotation) are derived from the axes on the fly.
 */
class PrimitiveUVCoordSystem : public UVCoordSystem
{
private:
  vm::vec3d m_uAxis;
  vm::vec3d m_vAxis;
  vm::vec2f m_offset;

  friend class PrimitiveUVCoordSystemSnapshot;

public:
  PrimitiveUVCoordSystem(
    const vm::vec3d& uAxis, const vm::vec3d& vAxis, const vm::vec2f& offset);
  PrimitiveUVCoordSystem(
    const vm::vec3d& normal,
    const UVAttributes& uvAttributes,
    const vm::vec2f& textureSize);

  std::unique_ptr<UVCoordSystem> clone() const override;
  std::unique_ptr<UVCoordSystemSnapshot> takeSnapshot() const override;
  void restoreSnapshot(const UVCoordSystemSnapshot& snapshot) override;

  vm::vec3d uAxis() const override;
  vm::vec3d vAxis() const override;
  vm::vec3d normal() const override;

  UVAttributes uvAttributes(const vm::vec2f& textureSize) const override;
  void setUVAttributes(
    const UVAttributes& uvAttributes, const vm::vec2f& textureSize) override;

  void resetCache(
    const vm::vec3d& point0, const vm::vec3d& point1, const vm::vec3d& point2) override;
  void reset(const vm::vec3d& normal) override;
  void resetToParaxial(const vm::vec3d& normal, float angle) override;
  void resetToParallel(const vm::vec3d& normal, float angle) override;

  using UVCoordSystem::uvCoords;
  vm::vec2f uvCoords(const vm::vec3d& point, const vm::vec2f& textureSize) const override;

  void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant) override;

  void shear(const vm::vec3d& normal, const vm::vec2f& factors) override;

  float measureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const override;

  std::unique_ptr<UVCoordSystem> toParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec2f& textureSize) const override;
  std::unique_ptr<UVCoordSystem> toParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec2f& textureSize) const override;

private:
  void setAxes(
    const vm::vec3d& normal,
    const UVAttributes& uvAttributes,
    const vm::vec2f& textureSize);

  bool isRotationInverted(const vm::vec3d& normal) const override;

  void updateNormalWithProjection(const vm::vec3d& newNormal) override;
  void updateNormalWithRotation(
    const vm::vec3d& oldNormal, const vm::vec3d& newNormal) override;

  deleteCopyAndMove(PrimitiveUVCoordSystem);
};

} // namespace tb::mdl
