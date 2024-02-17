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

#pragma once

#include "FloatType.h"
#include "Macros.h"
#include "Model/TexCoordSystem.h"

#include "vm/forward.h"
#include "vm/vec.h"

#include <memory>
#include <tuple>

namespace TrenchBroom
{
namespace Model
{
class ParallelTexCoordSystemSnapshot : public TexCoordSystemSnapshot
{
private:
  vm::vec3 m_xAxis;
  vm::vec3 m_yAxis;

public:
  ParallelTexCoordSystemSnapshot(const vm::vec3& xAxis, const vm::vec3& yAxis);
  ParallelTexCoordSystemSnapshot(const ParallelTexCoordSystem* coordSystem);

private:
  std::unique_ptr<TexCoordSystemSnapshot> doClone() const override;
  void doRestore(ParallelTexCoordSystem& coordSystem) const override;
  void doRestore(ParaxialTexCoordSystem& coordSystem) const override;
};

class ParallelTexCoordSystem : public TexCoordSystem
{
private:
  vm::vec3 m_xAxis;
  vm::vec3 m_yAxis;

  friend class ParallelTexCoordSystemSnapshot;

public:
  ParallelTexCoordSystem(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs);
  ParallelTexCoordSystem(const vm::vec3& xAxis, const vm::vec3& yAxis);

  static std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> fromParaxial(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs);

private:
  std::unique_ptr<TexCoordSystem> doClone() const override;
  std::unique_ptr<TexCoordSystemSnapshot> doTakeSnapshot() const override;
  void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) override;

  vm::vec3 getXAxis() const override;
  vm::vec3 getYAxis() const override;
  vm::vec3 getZAxis() const override;

  void doResetCache(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) override;
  void doResetTextureAxes(const vm::vec3& normal) override;
  void doResetTextureAxesToParaxial(const vm::vec3& normal, float angle) override;
  void doResetTextureAxesToParallel(const vm::vec3& normal, float angle) override;

  bool isRotationInverted(const vm::vec3& normal) const override;
  vm::vec2f doGetTexCoords(
    const vm::vec3& point,
    const BrushFaceAttributes& attribs,
    const vm::vec2f& textureSize) const override;

  void doSetRotation(const vm::vec3& normal, float oldAngle, float newAngle) override;
  void applyRotation(const vm::vec3& normal, FloatType angle);

  void doTransform(
    const vm::plane3& oldBoundary,
    const vm::plane3& newBoundary,
    const vm::mat4x4& transformation,
    BrushFaceAttributes& attribs,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3& invariant) override;
  float computeTextureAngle(
    const vm::plane3& oldBoundary, const vm::mat4x4& transformation) const;

  void doUpdateNormalWithProjection(
    const vm::vec3& newNormal, const BrushFaceAttributes& attribs) override;
  void doUpdateNormalWithRotation(
    const vm::vec3& oldNormal,
    const vm::vec3& newNormal,
    const BrushFaceAttributes& attribs) override;

  void doShearTexture(const vm::vec3& normal, const vm::vec2f& factors) override;

  float doMeasureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const override;
  void computeInitialAxes(const vm::vec3& normal, vm::vec3& xAxis, vm::vec3& yAxis) const;

  std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> doToParallel(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const override;
  std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> doToParaxial(
    const vm::vec3& point0,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const override;

  deleteCopyAndMove(ParallelTexCoordSystem);
};
} // namespace Model
} // namespace TrenchBroom
