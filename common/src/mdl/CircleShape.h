/*
 Copyright (C) 2024 Kristian Duske

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

#include "kdl/overload.h"
#include "kdl/reflection_decl.h"

#include <variant>

namespace tb::mdl
{
struct VertexAlignedCircle;
struct ScalableCircle;

struct EdgeAlignedCircle
{
  size_t numSides = 8;

  EdgeAlignedCircle();
  explicit EdgeAlignedCircle(size_t numSides);
  explicit EdgeAlignedCircle(const VertexAlignedCircle& circleShape);
  explicit EdgeAlignedCircle(const ScalableCircle& circleShape);

  kdl_reflect_decl(EdgeAlignedCircle, numSides);
};

struct VertexAlignedCircle
{
  size_t numSides = 8;

  VertexAlignedCircle();
  explicit VertexAlignedCircle(size_t numSides);
  explicit VertexAlignedCircle(const EdgeAlignedCircle& circleShape);
  explicit VertexAlignedCircle(const ScalableCircle& circleShape);

  kdl_reflect_decl(VertexAlignedCircle, numSides);
};

struct ScalableCircle
{
  size_t precision = 0;

  ScalableCircle();
  explicit ScalableCircle(size_t precision);
  explicit ScalableCircle(const VertexAlignedCircle& circleShape);
  explicit ScalableCircle(const EdgeAlignedCircle& circleShape);

  kdl_reflect_decl(ScalableCircle, precision);
};

using CircleShape = std::variant<EdgeAlignedCircle, VertexAlignedCircle, ScalableCircle>;

template <typename To>
To convertCircleShape(const CircleShape& from)
{
  return std::visit(
    kdl::overload(
      [](const EdgeAlignedCircle& edgeAligned) { return To{edgeAligned}; },
      [](const VertexAlignedCircle& vertexAligned) { return To{vertexAligned}; },
      [](const ScalableCircle& scalable) { return To{scalable}; }),
    from);
}

} // namespace tb::mdl
