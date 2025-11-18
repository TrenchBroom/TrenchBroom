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

#include "BrushFaceAttributes.h"

#include "kd/reflection_impl.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <string>

namespace tb::mdl
{

const std::string BrushFaceAttributes::NoMaterialName = "__TB_empty";

BrushFaceAttributes::BrushFaceAttributes(std::string_view materialName)
  : m_materialName{materialName}
{
}

BrushFaceAttributes::BrushFaceAttributes(
  std::string_view materialName, const BrushFaceAttributes& other)
  : m_materialName{materialName}
  , m_offset{other.m_offset}
  , m_scale{other.m_scale}
  , m_rotation{other.m_rotation}
  , m_surfaceContents{other.m_surfaceContents}
  , m_surfaceFlags{other.m_surfaceFlags}
  , m_surfaceValue{other.m_surfaceValue}
  , m_color{other.m_color}
{
}

kdl_reflect_impl(BrushFaceAttributes);

const std::string& BrushFaceAttributes::materialName() const
{
  return m_materialName;
}

const vm::vec2f& BrushFaceAttributes::offset() const
{
  return m_offset;
}

float BrushFaceAttributes::xOffset() const
{
  return m_offset.x();
}

float BrushFaceAttributes::yOffset() const
{
  return m_offset.y();
}

vm::vec2f BrushFaceAttributes::modOffset(
  const vm::vec2f& offset, const vm::vec2f& size) const
{
  return offset - snapDown(offset, size);
}

const vm::vec2f& BrushFaceAttributes::scale() const
{
  return m_scale;
}

float BrushFaceAttributes::xScale() const
{
  return m_scale.x();
}

float BrushFaceAttributes::yScale() const
{
  return m_scale.y();
}

float BrushFaceAttributes::rotation() const
{
  return m_rotation;
}

bool BrushFaceAttributes::hasSurfaceAttributes() const
{
  return m_surfaceContents || m_surfaceFlags || m_surfaceValue;
}

const std::optional<int>& BrushFaceAttributes::surfaceContents() const
{
  return m_surfaceContents;
}

const std::optional<int>& BrushFaceAttributes::surfaceFlags() const
{
  return m_surfaceFlags;
}

const std::optional<float>& BrushFaceAttributes::surfaceValue() const
{
  return m_surfaceValue;
}

bool BrushFaceAttributes::hasColor() const
{
  return m_color.has_value();
}

const std::optional<Color>& BrushFaceAttributes::color() const
{
  return m_color;
}

bool BrushFaceAttributes::valid() const
{
  return !vm::is_zero(m_scale.x(), vm::Cf::almost_zero())
         && !vm::is_zero(m_scale.y(), vm::Cf::almost_zero());
}

bool BrushFaceAttributes::setMaterialName(const std::string& materialName)
{
  if (materialName != m_materialName)
  {
    m_materialName = materialName;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setOffset(const vm::vec2f& offset)
{
  if (offset != m_offset)
  {
    m_offset = offset;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setXOffset(const float xOffset)
{
  if (xOffset != m_offset.x())
  {
    m_offset[0] = xOffset;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setYOffset(const float yOffset)
{
  if (yOffset != m_offset.y())
  {
    m_offset[1] = yOffset;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setScale(const vm::vec2f& scale)
{
  if (scale != m_scale)
  {
    m_scale = scale;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setXScale(const float xScale)
{
  if (xScale != m_scale.x())
  {
    m_scale[0] = xScale;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setYScale(const float yScale)
{
  if (yScale != m_scale.y())
  {
    m_scale[1] = yScale;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setRotation(const float rotation)
{
  if (rotation != m_rotation)
  {
    m_rotation = rotation;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setSurfaceContents(const std::optional<int>& surfaceContents)
{
  if (surfaceContents != m_surfaceContents)
  {
    m_surfaceContents = surfaceContents;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setSurfaceFlags(const std::optional<int>& surfaceFlags)
{
  if (surfaceFlags != m_surfaceFlags)
  {
    m_surfaceFlags = surfaceFlags;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setSurfaceValue(const std::optional<float>& surfaceValue)
{
  if (surfaceValue != m_surfaceValue)
  {
    m_surfaceValue = surfaceValue;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setColor(const std::optional<Color>& color)
{
  if (color != m_color)
  {
    m_color = color;
    return true;
  }
  return false;
}

} // namespace tb::mdl
