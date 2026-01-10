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

#include "Color.h"

#include "kd/reflection_decl.h"

#include <optional>
#include <string>
#include <variant>

namespace tb::mdl
{

class BrushFace;
class BrushFaceAttributes;

struct ResetAxis
{
  kdl_reflect_decl_empty(ResetAxis);
};

struct ToParaxial
{
  kdl_reflect_decl_empty(ToParaxial);
};

struct ToParallel
{
  kdl_reflect_decl_empty(ToParallel);
};

using AxisOp = std::variant<ResetAxis, ToParaxial, ToParallel>;

std::ostream& operator<<(std::ostream& lhs, const AxisOp& rhs);

struct SetValue
{
  std::optional<float> value;

  kdl_reflect_decl(SetValue, value);
};

struct AddValue
{
  float delta;

  kdl_reflect_decl(AddValue, delta);
};

struct MultiplyValue
{
  float factor;

  kdl_reflect_decl(MultiplyValue, factor);
};

using ValueOp = std::variant<SetValue, AddValue, MultiplyValue>;

std::ostream& operator<<(std::ostream& lhs, const ValueOp& rhs);

struct SetFlags
{
  std::optional<int> value;

  kdl_reflect_decl(SetFlags, value);
};

struct SetFlagBits
{
  int value;

  kdl_reflect_decl(SetFlagBits, value);
};

struct ClearFlagBits
{
  int value;

  kdl_reflect_decl(ClearFlagBits, value);
};

using FlagOp = std::variant<SetFlags, SetFlagBits, ClearFlagBits>;

std::ostream& operator<<(std::ostream& lhs, const FlagOp& rhs);

struct UpdateBrushFaceAttributes
{
  std::optional<std::string> materialName = std::nullopt;
  std::optional<ValueOp> xOffset = std::nullopt;
  std::optional<ValueOp> yOffset = std::nullopt;
  std::optional<ValueOp> rotation = std::nullopt;
  std::optional<ValueOp> xScale = std::nullopt;
  std::optional<ValueOp> yScale = std::nullopt;
  std::optional<FlagOp> surfaceFlags = std::nullopt;
  std::optional<FlagOp> surfaceContents = std::nullopt;
  std::optional<ValueOp> surfaceValue = std::nullopt;
  std::optional<std::optional<Color>> color = std::nullopt;
  std::optional<AxisOp> axis = std::nullopt;

  kdl_reflect_decl(
    UpdateBrushFaceAttributes,
    materialName,
    xOffset,
    yOffset,
    rotation,
    xScale,
    yScale,
    surfaceFlags,
    surfaceContents,
    surfaceValue,
    color,
    axis);
};

UpdateBrushFaceAttributes copyAll(const BrushFaceAttributes& attributes);
UpdateBrushFaceAttributes copyAllExceptContentFlags(
  const BrushFaceAttributes& attributes);

UpdateBrushFaceAttributes resetAll(const BrushFaceAttributes& defaultFaceAttributes);
UpdateBrushFaceAttributes resetAllToParaxial(
  const BrushFaceAttributes& defaultFaceAttributes);

enum class UvPolicy
{
  best,
  next,
  prev,
};

std::ostream& operator<<(std::ostream& lhs, UvPolicy rhs);

enum class UvAxis
{
  u,
  v,
};

std::ostream& operator<<(std::ostream& lhs, UvAxis rhs);

enum class UvDirection
{
  forward,
  backward,
};

std::ostream& operator<<(std::ostream& lhs, UvDirection direction);

UpdateBrushFaceAttributes align(const BrushFace& brushFace, UvPolicy policy);
UpdateBrushFaceAttributes justify(
  const BrushFace& brushFace, UvAxis uv, UvDirection direction, UvPolicy policy);

void evaluate(
  const UpdateBrushFaceAttributes& updateBrushFaceAttributes, BrushFace& brushFace);

} // namespace tb::mdl
