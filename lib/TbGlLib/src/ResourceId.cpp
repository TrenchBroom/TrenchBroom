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

#include "gl/ResourceId.h"

#include "kd/reflection_impl.h"

namespace tb::gl
{
namespace
{
size_t currentId = 0;
}

kdl_reflect_impl(ResourceId);

ResourceId::ResourceId()
  : m_id{++currentId}
{
}

} // namespace tb::gl

std::size_t std::hash<tb::gl::ResourceId>::operator()(
  const tb::gl::ResourceId& resourceId) const noexcept
{
  return std::hash<size_t>{}(resourceId.m_id);
}
