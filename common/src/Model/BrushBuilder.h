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

#include <kdl/result_forward.h>

#include <vecmath/bbox.h>

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Model {
class Brush;
class ModelFactory;

enum class BrushError;
enum class MapFormat;

class BrushBuilder {
private:
  MapFormat m_mapFormat;
  const vm::bbox3 m_worldBounds;
  const BrushFaceAttributes m_defaultAttribs;

public:
  BrushBuilder(MapFormat mapFormat, const vm::bbox3& worldBounds);
  BrushBuilder(
    MapFormat mapFormat, const vm::bbox3& worldBounds, const BrushFaceAttributes& defaultAttribs);

  kdl::result<Brush, BrushError> createCube(FloatType size, const std::string& textureName) const;
  kdl::result<Brush, BrushError> createCube(
    FloatType size, const std::string& leftTexture, const std::string& rightTexture,
    const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture,
    const std::string& bottomTexture) const;

  kdl::result<Brush, BrushError> createCuboid(
    const vm::vec3& size, const std::string& textureName) const;
  kdl::result<Brush, BrushError> createCuboid(
    const vm::vec3& size, const std::string& leftTexture, const std::string& rightTexture,
    const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture,
    const std::string& bottomTexture) const;

  kdl::result<Brush, BrushError> createCuboid(
    const vm::bbox3& bounds, const std::string& textureName) const;
  kdl::result<Brush, BrushError> createCuboid(
    const vm::bbox3& bounds, const std::string& leftTexture, const std::string& rightTexture,
    const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture,
    const std::string& bottomTexture) const;

  kdl::result<Brush, BrushError> createBrush(
    const std::vector<vm::vec3>& points, const std::string& textureName) const;
  kdl::result<Brush, BrushError> createBrush(
    const Polyhedron3& polyhedron, const std::string& textureName) const;
};
} // namespace Model
} // namespace TrenchBroom
