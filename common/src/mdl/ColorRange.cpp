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

#include "ColorRange.h"

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h" // IWYU pragma: keep
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

namespace tb::mdl
{

ColorRange::Type detectColorRange(const std::string& str)
{
  return Color::parse(str) | kdl::transform([](const auto& color) {
           return color.isFloat() ? ColorRange::Float : ColorRange::Byte;
         })
         | kdl::value_or(ColorRange::Unset);
}

ColorRange::Type detectColorRange(
  const std::string& propertyKey, const std::vector<EntityNodeBase*>& nodes)
{
  auto result = ColorRange::Unset;
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](const EntityNodeBase* entityNode) {
        if (const auto* value = entityNode->entity().property(propertyKey))
        {
          const auto range = detectColorRange(*value);
          if (result == ColorRange::Unset)
          {
            result = range;
          }
          else if (result != range)
          {
            result = ColorRange::Mixed;
          }
        }
      },
      [](const LayerNode*) {},
      [](const GroupNode*) {},
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
  }
  return result;
}

} // namespace tb::mdl
