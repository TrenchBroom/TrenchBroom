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

#include "Macros.h"
#include "mdl/BrushFaceAttributes.h"

#include "vm/mat.h"
#include "vm/plane.h"
#include "vm/vec.h"

#include <memory>
#include <tuple>

namespace tb::mdl
{
class ParallelUVCoordSystem;
class ParaxialUVCoordSystem;
class UVCoordSystem;

class UVCoordSystemSnapshot
{
public:
  virtual ~UVCoordSystemSnapshot();
  void restore(UVCoordSystem& coordSystem) const;
  virtual std::unique_ptr<UVCoordSystemSnapshot> clone() const = 0;

private:
  virtual void doRestore(ParallelUVCoordSystem& coordSystem) const = 0;
  virtual void doRestore(ParaxialUVCoordSystem& coordSystem) const = 0;

  friend class ParallelUVCoordSystem;
  friend class ParaxialUVCoordSystem;
};

enum class WrapStyle
{
  Projection,
  Rotation
};

class UVCoordSystem
{
public:
  UVCoordSystem();
  virtual ~UVCoordSystem();

  friend bool operator==(const UVCoordSystem& lhs, const UVCoordSystem& rhs);
  friend bool operator!=(const UVCoordSystem& lhs, const UVCoordSystem& rhs);

  virtual std::unique_ptr<UVCoordSystem> clone() const = 0;
  virtual std::unique_ptr<UVCoordSystemSnapshot> takeSnapshot() const = 0;
  virtual void restoreSnapshot(const UVCoordSystemSnapshot& snapshot) = 0;

  virtual vm::vec3d uAxis() const = 0;
  virtual vm::vec3d vAxis() const = 0;
  virtual vm::vec3d normal() const = 0;

  virtual void resetCache(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs) = 0;
  virtual void reset(const vm::vec3d& normal) = 0;
  virtual void resetToParaxial(const vm::vec3d& normal, float angle) = 0;
  virtual void resetToParallel(const vm::vec3d& normal, float angle) = 0;

  virtual vm::vec2f uvCoords(
    const vm::vec3d& point,
    const BrushFaceAttributes& attribs,
    const vm::vec2f& textureSize) const = 0;

  virtual void setRotation(const vm::vec3d& normal, float oldAngle, float newAngle) = 0;
  virtual void transform(
    const vm::plane3d& oldBoundary,
    const vm::plane3d& newBoundary,
    const vm::mat4x4d& transformation,
    BrushFaceAttributes& attribs,
    const vm::vec2f& textureSize,
    bool lockTexture,
    const vm::vec3d& invariant) = 0;
  void setNormal(
    const vm::vec3d& oldNormal,
    const vm::vec3d& newNormal,
    const BrushFaceAttributes& attribs,
    WrapStyle style);

  void translate(
    const vm::vec3d& normal,
    const vm::vec3d& up,
    const vm::vec3d& right,
    const vm::vec2f& offset,
    BrushFaceAttributes& attribs) const;
  void rotate(const vm::vec3d& normal, float angle, BrushFaceAttributes& attribs) const;
  virtual void shear(const vm::vec3d& normal, const vm::vec2f& factors) = 0;

  vm::mat4x4d toMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;
  vm::mat4x4d fromMatrix(const vm::vec2f& offset, const vm::vec2f& scale) const;

  virtual float measureAngle(
    float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const = 0;

  virtual std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> toParallel(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs) const = 0;
  virtual std::tuple<std::unique_ptr<UVCoordSystem>, BrushFaceAttributes> toParaxial(
    const vm::vec3d& point0,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const BrushFaceAttributes& attribs) const = 0;

private:
  friend class UVCoordSystemSnapshot;

  virtual bool isRotationInverted(const vm::vec3d& normal) const = 0;

  virtual void updateNormalWithProjection(
    const vm::vec3d& newNormal, const BrushFaceAttributes& attribs) = 0;
  virtual void updateNormalWithRotation(
    const vm::vec3d& oldNormal,
    const vm::vec3d& newNormal,
    const BrushFaceAttributes& attribs) = 0;

protected:
  vm::vec2f computeUVCoords(const vm::vec3d& point, const vm::vec2f& scale) const;

  template <typename T>
  static T safeScale(const T value)
  {
    return vm::is_equal(value, T(0.0), vm::constants<T>::almost_zero())
             ? static_cast<T>(1.0)
             : value;
  }

  template <typename T1, typename T2>
  static vm::vec<T1, 3> safeScaleAxis(const vm::vec<T1, 3>& axis, const T2 factor)
  {
    return axis / safeScale(T1(factor));
  }

  deleteCopyAndMove(UVCoordSystem);
};

} // namespace tb::mdl
