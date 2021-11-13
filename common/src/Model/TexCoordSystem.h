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

#include "Model/BrushFaceAttributes.h"

#include <vecmath/vec.h>

#include <memory>
#include <tuple>

namespace TrenchBroom {
namespace Model {
class ParallelTexCoordSystem;
class ParaxialTexCoordSystem;
class TexCoordSystem;

class TexCoordSystemSnapshot {
public:
  virtual ~TexCoordSystemSnapshot();
  void restore(TexCoordSystem& coordSystem) const;
  std::unique_ptr<TexCoordSystemSnapshot> clone() const;

private:
  virtual std::unique_ptr<TexCoordSystemSnapshot> doClone() const = 0;
  virtual void doRestore(ParallelTexCoordSystem& coordSystem) const = 0;
  virtual void doRestore(ParaxialTexCoordSystem& coordSystem) const = 0;

  friend class ParallelTexCoordSystem;
  friend class ParaxialTexCoordSystem;
};

enum class WrapStyle
{
  Projection,
  Rotation
};

class TexCoordSystem {
public:
  TexCoordSystem();
  virtual ~TexCoordSystem();

  friend bool operator==(const TexCoordSystem& lhs, const TexCoordSystem& rhs);
  friend bool operator!=(const TexCoordSystem& lhs, const TexCoordSystem& rhs);

  std::unique_ptr<TexCoordSystem> clone() const;
  std::unique_ptr<TexCoordSystemSnapshot> takeSnapshot() const;

  vm::vec3 xAxis() const;
  vm::vec3 yAxis() const;

  void resetCache(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs);
  void resetTextureAxes(const vm::vec3& normal);
  void resetTextureAxesToParaxial(const vm::vec3& normal, float angle);
  void resetTextureAxesToParallel(const vm::vec3& normal, float angle);

  vm::vec2f getTexCoords(
    const vm::vec3& point, const BrushFaceAttributes& attribs, const vm::vec2f& textureSize) const;

  void setRotation(const vm::vec3& normal, float oldAngle, float newAngle);
  void transform(
    const vm::plane3& oldBoundary, const vm::plane3& newBoundary, const vm::mat4x4& transformation,
    BrushFaceAttributes& attribs, const vm::vec2f& textureSize, bool lockTexture,
    const vm::vec3& invariant);
  void updateNormal(
    const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs,
    const WrapStyle style);

  void moveTexture(
    const vm::vec3& normal, const vm::vec3& up, const vm::vec3& right, const vm::vec2f& offset,
    BrushFaceAttributes& attribs) const;
  void rotateTexture(const vm::vec3& normal, float angle, BrushFaceAttributes& attribs) const;
  void shearTexture(const vm::vec3& normal, const vm::vec2f& factors);

  vm::mat4x4 toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;
  vm::mat4x4 fromMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;
  float measureAngle(float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const;

  std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> toParallel(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const;
  std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> toParaxial(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const;

private:
  virtual std::unique_ptr<TexCoordSystem> doClone() const = 0;
  virtual std::unique_ptr<TexCoordSystemSnapshot> doTakeSnapshot() const = 0;
  virtual void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) = 0;
  friend class TexCoordSystemSnapshot;

  virtual vm::vec3 getXAxis() const = 0;
  virtual vm::vec3 getYAxis() const = 0;
  virtual vm::vec3 getZAxis() const = 0;

  virtual void doResetCache(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs) = 0;
  virtual void doResetTextureAxes(const vm::vec3& normal) = 0;
  virtual void doResetTextureAxesToParaxial(const vm::vec3& normal, float angle) = 0;
  virtual void doResetTextureAxesToParallel(const vm::vec3& normal, float angle) = 0;

  virtual bool isRotationInverted(const vm::vec3& normal) const = 0;
  virtual vm::vec2f doGetTexCoords(
    const vm::vec3& point, const BrushFaceAttributes& attribs,
    const vm::vec2f& textureSize) const = 0;

  virtual void doSetRotation(const vm::vec3& normal, float oldAngle, float newAngle) = 0;
  virtual void doTransform(
    const vm::plane3& oldBoundary, const vm::plane3& newBoundary, const vm::mat4x4& transformation,
    BrushFaceAttributes& attribs, const vm::vec2f& textureSize, bool lockTexture,
    const vm::vec3& invariant) = 0;
  virtual void doUpdateNormalWithProjection(
    const vm::vec3& newNormal, const BrushFaceAttributes& attribs) = 0;
  virtual void doUpdateNormalWithRotation(
    const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) = 0;

  virtual void doShearTexture(const vm::vec3& normal, const vm::vec2f& factors) = 0;

  virtual float doMeasureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const = 0;

  virtual std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> doToParallel(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const = 0;
  virtual std::tuple<std::unique_ptr<TexCoordSystem>, BrushFaceAttributes> doToParaxial(
    const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2,
    const BrushFaceAttributes& attribs) const = 0;

protected:
  vm::vec2f computeTexCoords(const vm::vec3& point, const vm::vec2f& scale) const;

  template <typename T> T safeScale(const T value) const {
    return vm::is_equal(value, T(0.0), vm::constants<T>::almost_zero()) ? static_cast<T>(1.0)
                                                                        : value;
  }

  template <typename T1, typename T2>
  vm::vec<T1, 3> safeScaleAxis(const vm::vec<T1, 3>& axis, const T2 factor) const {
    return axis / safeScale(T1(factor));
  }

  deleteCopyAndMove(TexCoordSystem)
};
} // namespace Model
} // namespace TrenchBroom
