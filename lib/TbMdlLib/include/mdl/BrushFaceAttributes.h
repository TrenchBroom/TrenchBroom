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

#include "base/Color.h"
#include "mdl/UvAttributes.h"

#include "kd/reflection_decl.h"

#include <optional>

namespace tb::mdl
{

class BrushFaceAttributes
{
private:
  UvAttributes m_uvAttributes;

  std::optional<int> m_surfaceContents;
  std::optional<int> m_surfaceFlags;
  std::optional<float> m_surfaceValue;

  std::optional<Color> m_color;

public:
  kdl_reflect_decl(
    BrushFaceAttributes,
    m_uvAttributes,
    m_surfaceContents,
    m_surfaceFlags,
    m_surfaceValue,
    m_color);

  const UvAttributes& uvAttributes() const;

  bool hasSurfaceAttributes() const;
  const std::optional<int>& surfaceContents() const;
  const std::optional<int>& surfaceFlags() const;
  const std::optional<float>& surfaceValue() const;

  bool hasColor() const;
  const std::optional<Color>& color() const;

  bool valid() const;

  bool setUvAttributes(const UvAttributes& uvAttributes);
  bool setSurfaceContents(const std::optional<int>& surfaceContents);
  bool setSurfaceFlags(const std::optional<int>& surfaceFlags);
  bool setSurfaceValue(const std::optional<float>& surfaceValue);
  bool setColor(const std::optional<Color>& color);
};

} // namespace tb::mdl
