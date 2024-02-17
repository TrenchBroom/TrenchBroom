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

#include "Color.h"

#include "kdl/reflection_decl.h"

#include <vecmath/forward.h>

#include <optional>
#include <string>
#include <string_view>

namespace TrenchBroom
{
namespace Assets
{
class Texture;
}

namespace Model
{

class BrushFaceAttributes
{
public:
  static const std::string NoTextureName;

private:
  std::string m_textureName;

  vm::vec2f m_offset;
  vm::vec2f m_scale;
  float m_rotation;

  std::optional<int> m_surfaceContents;
  std::optional<int> m_surfaceFlags;
  std::optional<float> m_surfaceValue;

  std::optional<Color> m_color;

public:
  explicit BrushFaceAttributes(std::string_view textureName);
  BrushFaceAttributes(const BrushFaceAttributes& other);
  BrushFaceAttributes(std::string_view textureName, const BrushFaceAttributes& other);

  BrushFaceAttributes& operator=(BrushFaceAttributes other);

  kdl_reflect_decl(
    BrushFaceAttributes,
    m_textureName,
    m_offset,
    m_scale,
    m_rotation,
    m_surfaceContents,
    m_surfaceFlags,
    m_surfaceValue,
    m_color);

  friend void swap(BrushFaceAttributes& lhs, BrushFaceAttributes& rhs);

  const std::string& textureName() const;

  const vm::vec2f& offset() const;
  float xOffset() const;
  float yOffset() const;
  vm::vec2f modOffset(const vm::vec2f& offset, const vm::vec2f& textureSize) const;

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

  bool setTextureName(const std::string& textureName);
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

} // namespace Model
} // namespace TrenchBroom
