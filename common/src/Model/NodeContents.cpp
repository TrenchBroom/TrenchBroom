/*
 Copyright (C) 2020 Kristian Duske

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

#include "NodeContents.h"

#include "Model/BrushFace.h"

#include <kdl/overload.h>

namespace TrenchBroom
{
namespace Model
{
NodeContents::NodeContents(
  std::variant<Layer, Group, Entity, Brush, BezierPatch> contents)
  : m_contents(std::move(contents))
{
  std::visit(
    kdl::overload(
      [](Layer&) {},
      [](Group&) {},
      [](Entity& entity) { entity.unsetEntityDefinitionAndModel(); },
      [](Brush& brush) {
        for (auto& face : brush.faces())
        {
          face.setTexture(nullptr);
        }
      },
      [](BezierPatch&) {}),
    m_contents);
}

const std::variant<Layer, Group, Entity, Brush, BezierPatch>& NodeContents::get() const
{
  return m_contents;
}

std::variant<Layer, Group, Entity, Brush, BezierPatch>& NodeContents::get()
{
  return m_contents;
}
} // namespace Model
} // namespace TrenchBroom
