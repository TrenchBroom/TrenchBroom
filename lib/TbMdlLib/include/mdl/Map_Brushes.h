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
class UVCoordSystemSnapshot;

enum class UvAxis;
enum class UvSign;
enum class UvPolicy;

struct UpdateBrushFaceAttributes;

enum class WrapStyle;

bool createBrush(Map& map, const std::vector<vm::vec3d>& points);

bool setBrushFaceAttributes(Map& map, const UpdateBrushFaceAttributes& update);

bool copyUV(
  Map& map,
  const UVCoordSystemSnapshot& coordSystemSnapshot,
  const BrushFaceAttributes& attribs,
  const vm::plane3d& sourceFacePlane,
  WrapStyle wrapStyle);

bool translateUV(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::vec2f& delta);

bool rotateUV(Map& map, float angle);

bool shearUV(Map& map, const vm::vec2f& factors);

bool flipUV(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  vm::direction cameraRelativeFlipDirection);

void alignUV(Map& map, UvPolicy uvPolicy);
void justifyUV(Map& map, UvAxis uvAxis, UvSign uvSign, UvPolicy uvPolicy);
void fitUV(Map& map, UvAxis uvAxis, UvSign uvSign, UvPolicy uvPolicy);

} // namespace tb::mdl
