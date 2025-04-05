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

#include "EntityColor.h"

#include "Color.h"
#include "mdl/BrushNode.h"
#include "mdl/ColorRange.h"
#include "mdl/EntityNode.h" // IWYU pragma: keep
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kdl/overload.h"
#include "kdl/string_utils.h"

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace tb::mdl
{

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

const std::string convertEntityColor(
  const std::string& str, const ColorRange::Type colorRange)
{
  const auto color = parseEntityColor(str);
  return entityColorAsString(color, colorRange);
}

Color parseEntityColor(const std::string& str)
{
  const auto components = kdl::str_split(str, " ");
  const auto range = detectColorRange(components);
  assert(range != ColorRange::Mixed);

  return range == ColorRange::Byte ? 
  Color{
      kdl::str_to_int(components[0]).value_or(0),
      kdl::str_to_int(components[1]).value_or(0),
      kdl::str_to_int(components[2]).value_or(0),
  } : Color{
    static_cast<int>(kdl::str_to_double(components[0]).value_or(0.0) * 255.0),
    static_cast<int>(kdl::str_to_double(components[1]).value_or(0.0) * 255.0),
    static_cast<int>(kdl::str_to_double(components[2]).value_or(0.0) * 255.0),
  };
}

std::string entityColorAsString(const Color& color, const ColorRange::Type colorRange)
{
  std::stringstream result;
  if (colorRange == ColorRange::Byte)
  {
    result << int(color.r() * 255.0f) << " " << int(color.g() * 255.0f) << " "
           << int(color.b() * 255.0f);
  }
  else if (colorRange == ColorRange::Float)
  {
    result << float(color.r()) << " " << float(color.g()) << " " << float(color.b());
  }
  return result.str();
}

} // namespace tb::mdl
