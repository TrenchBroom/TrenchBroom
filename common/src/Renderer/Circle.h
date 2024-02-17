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

#include "Renderer/VertexArray.h"

#include "vecmath/forward.h"
#include "vecmath/util.h"

namespace TrenchBroom
{
namespace Renderer
{
class Circle
{
private:
  VertexArray m_array;
  bool m_filled;

public:
  Circle(float radius, size_t segments, bool filled);
  Circle(float radius, size_t segments, bool filled, float startAngle, float angleLength);
  Circle(
    float radius,
    size_t segments,
    bool filled,
    vm::axis::type axis,
    const vm::vec3f& startAxis,
    const vm::vec3f& endAxis);
  Circle(
    float radius,
    size_t segments,
    bool filled,
    vm::axis::type axis,
    float startAngle,
    float angleLength);

  bool prepared() const;
  void prepare(VboManager& vboManager);
  void render();

private:
  void init3D(
    float radius,
    size_t segments,
    vm::axis::type axis,
    float startAngle,
    float angleLength);
  void init2D(float radius, size_t segments, float startAngle, float angleLength);
};
} // namespace Renderer
} // namespace TrenchBroom
