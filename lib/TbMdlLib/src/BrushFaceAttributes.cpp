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

#include "mdl/BrushFaceAttributes.h"

#include "kd/reflection_impl.h"

namespace tb::mdl
{

kdl_reflect_impl(BrushFaceAttributes);

const UvAttributes& BrushFaceAttributes::uvAttributes() const
{
  return m_uvAttributes;
}

const SurfaceAttributes& BrushFaceAttributes::surfaceAttributes() const
{
  return m_surfaceAttributes;
}

bool BrushFaceAttributes::hasSurfaceAttributes() const
{
  return m_surfaceAttributes.contents || m_surfaceAttributes.flags
         || m_surfaceAttributes.value;
}

bool BrushFaceAttributes::hasColor() const
{
  return m_surfaceAttributes.color.has_value();
}

bool BrushFaceAttributes::valid() const
{
  return m_uvAttributes.valid();
}

bool BrushFaceAttributes::setUvAttributes(const UvAttributes& uvAttributes)
{
  if (uvAttributes != m_uvAttributes)
  {
    m_uvAttributes = uvAttributes;
    return true;
  }
  return false;
}

bool BrushFaceAttributes::setSurfaceAttributes(const SurfaceAttributes& surfaceAttributes)
{
  if (surfaceAttributes != m_surfaceAttributes)
  {
    m_surfaceAttributes = surfaceAttributes;
    return true;
  }
  return false;
}

} // namespace tb::mdl
