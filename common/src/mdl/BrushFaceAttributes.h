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

#include "kdl/reflection_decl.h"

#include "vm/vec.h"

#include <optional>
#include <string>
#include <string_view>

namespace tb::mdl
{
class Material;

class BrushFaceAttributes
{
public:
  static const std::string NoMaterialName;

private:
  std::string m_materialName;

  vm::vec2f m_offset = vm::vec2f{0, 0};
  vm::vec2f m_scale = vm::vec2f{1, 1};
  float m_rotation = 0.0f;

  std::optional<int> m_surfaceContents;
  std::optional<int> m_surfaceFlags;
  std::optional<float> m_surfaceValue;

  std::optional<Color> m_color;

public:
  explicit BrushFaceAttributes(std::string_view materialName);
  BrushFaceAttributes(std::string_view materialName, const BrushFaceAttributes& other);

  kdl_reflect_decl(
    BrushFaceAttributes,
    m_materialName,
    m_offset,
    m_scale,
    m_rotation,
    m_surfaceContents,
    m_surfaceFlags,
    m_surfaceValue,
    m_color);

  const std::string& materialName() const;

  const vm::vec2f& offset() const;
  float xOffset() const;
  float yOffset() const;
  vm::vec2f modOffset(const vm::vec2f& offset, const vm::vec2f& size) const;

  const vm::vec2f& scale() const;
  float xScale() const;
  float yScale() const;

  float rotation() const;

  bool hasSurfaceAttributes() const;
  const std::optional<int>& surfaceContents() const;
  const std::optional<int>& surfaceFlags() const;
  const std::optional<float>& surfaceValue() const;

  bool hasColor() const;
  const std::optional<Color>& color() const;

  bool valid() const;

  bool setMaterialName(const std::string& materialName);
  bool setOffset(const vm::vec2f& offset);
  bool setXOffset(float xOffset);
  bool setYOffset(float yOffset);
  bool setScale(const vm::vec2f& scale);
  bool setXScale(float xScale);
  bool setYScale(float yScale);
  bool setRotation(float rotation);
  bool setSurfaceContents(const std::optional<int>& surfaceContents);
  bool setSurfaceFlags(const std::optional<int>& surfaceFlags);
  bool setSurfaceValue(const std::optional<float>& surfaceValue);
  bool setColor(const std::optional<Color>& color);
};

} // namespace tb::mdl
