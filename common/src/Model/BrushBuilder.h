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

#include "BrushFaceAttributes.h"
#include "FloatType.h"
#include "Model/Polyhedron3.h"
#include "Result.h"

#include "vm/bbox.h"

#include <string>
#include <vector>

namespace TrenchBroom::Model
{
class Brush;
class ModelFactory;
enum class MapFormat;

class BrushBuilder
{
private:
  MapFormat m_mapFormat;
  const vm::bbox3 m_worldBounds;
  const BrushFaceAttributes m_defaultAttribs;

public:
  BrushBuilder(MapFormat mapFormat, const vm::bbox3& worldBounds);
  BrushBuilder(
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    const BrushFaceAttributes& defaultAttribs);

  Result<Brush> createCube(FloatType size, const std::string& materialName) const;
  Result<Brush> createCube(
    FloatType size,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createCuboid(const vm::vec3& size, const std::string& materialName) const;
  Result<Brush> createCuboid(
    const vm::vec3& size,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createCuboid(
    const vm::bbox3& bounds, const std::string& materialName) const;
  Result<Brush> createCuboid(
    const vm::bbox3& bounds,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createBrush(
    const std::vector<vm::vec3>& points, const std::string& materialName) const;
  Result<Brush> createBrush(
    const Polyhedron3& polyhedron, const std::string& materialName) const;
};
} // namespace TrenchBroom::Model
