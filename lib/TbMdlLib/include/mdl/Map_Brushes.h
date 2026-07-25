/*
 Copyright (C) 2025 Kristian Duske

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

#include "vm/plane.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <vector>

namespace tb::mdl
{
class BrushFaceAttributes;
class Map;
class UvCoordSystemSnapshot;

enum class UvAxis;
enum class UvSign;
enum class UvPolicy;
enum class UvFitMode;

struct UpdateBrushFaceAttributes;

enum class WrapStyle;

enum class UvJustifyDirection
{
  Left,
  Right,
  Up,
  Down,
};

enum class UvFitDirection
{
  Horizontal,
  Vertical,
};

bool createBrush(Map& map, const std::vector<vm::vec3d>& points);

bool setBrushFaceAttributes(Map& map, const UpdateBrushFaceAttributes& update);

bool copyUv(
  Map& map,
  const UvCoordSystemSnapshot& coordSystemSnapshot,
  const BrushFaceAttributes& attribs,
  const vm::plane3d& sourceFacePlane,
  WrapStyle wrapStyle);

bool translateUv(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::vec2f& delta);

bool rotateUv(Map& map, float angle);

bool shearUv(Map& map, const vm::vec2f& factors);

bool flipUv(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  vm::direction cameraRelativeFlipDirection);

void alignUv(Map& map, UvPolicy uvPolicy);
void justifyUv(Map& map, UvJustifyDirection uvJustifyDirection, UvPolicy uvPolicy);
void fitUv(
  Map& map, UvFitDirection uvFitDirection, UvPolicy uvPolicy, UvFitMode uvFitMode);
void autoFitUv(Map& map);

} // namespace tb::mdl
