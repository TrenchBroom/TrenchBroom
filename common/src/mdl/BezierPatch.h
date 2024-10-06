/*
 Copyright (C) 2021 Kristian Duske

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

#include "mdl/AssetReference.h"

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <string>
#include <vector>

namespace tb::mdl
{
class Material;

class BezierPatch
{
public:
  using Point = vm::vec<double, 5>;

private:
  size_t m_pointRowCount;
  size_t m_pointColumnCount;
  std::vector<Point> m_controlPoints;
  vm::bbox3d m_bounds;

  std::string m_materialName;
  AssetReference<Material> m_materialReference;

  kdl_reflect_decl(
    BezierPatch,
    m_pointRowCount,
    m_pointColumnCount,
    m_bounds,
    m_controlPoints,
    m_materialName);

public:
  BezierPatch(
    size_t pointRowCount,
    size_t pointColumnCount,
    std::vector<Point> controlPoints,
    std::string materialName);
  ~BezierPatch();

  BezierPatch(const BezierPatch& other);
  BezierPatch(BezierPatch&& other) noexcept;

  BezierPatch& operator=(const BezierPatch& other);
  BezierPatch& operator=(BezierPatch&& other) noexcept;

public: // control points:
  size_t pointRowCount() const;
  size_t pointColumnCount() const;

  size_t quadRowCount() const;
  size_t quadColumnCount() const;

  size_t surfaceRowCount() const;
  size_t surfaceColumnCount() const;

  const std::vector<Point>& controlPoints() const;
  const Point& controlPoint(size_t row, size_t col) const;
  void setControlPoint(size_t row, size_t col, Point controlPoint);

  const vm::bbox3d& bounds() const;

  const std::string& materialName() const;
  void setMaterialName(std::string materialName);

  const Material* material() const;
  bool setMaterial(Material* material);

  void transform(const vm::mat4x4d& transformation);

  std::vector<Point> evaluate(size_t subdivisionsPerSurface) const;
};

} // namespace tb::mdl
