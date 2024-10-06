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
#include "asset/ColorRange.h"
#include "mdl/BrushNode.h"
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

asset::ColorRange::Type detectColorRange(
  const std::string& propertyKey, const std::vector<EntityNodeBase*>& nodes)
{
  auto result = asset::ColorRange::Unset;
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](const EntityNodeBase* entityNode) {
        if (const auto* value = entityNode->entity().property(propertyKey))
        {
          const auto range = asset::detectColorRange(*value);
          if (result == asset::ColorRange::Unset)
          {
            result = range;
          }
          else if (result != range)
          {
            result = asset::ColorRange::Mixed;
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
  const std::string& str, const asset::ColorRange::Type colorRange)
{
  const auto color = parseEntityColor(str);
  return entityColorAsString(color, colorRange);
}

Color parseEntityColor(const std::string& str)
{
  const auto components = kdl::str_split(str, " ");
  const auto range = asset::detectColorRange(components);
  assert(range != asset::ColorRange::Mixed);

  int r = 0, g = 0, b = 0;
  if (range == asset::ColorRange::Byte)
  {
    r = std::atoi(components[0].c_str());
    g = std::atoi(components[1].c_str());
    b = std::atoi(components[2].c_str());
  }
  else if (range == asset::ColorRange::Float)
  {
    r = static_cast<int>(std::atof(components[0].c_str()) * 255.0);
    g = static_cast<int>(std::atof(components[1].c_str()) * 255.0);
    b = static_cast<int>(std::atof(components[2].c_str()) * 255.0);
  }

  return {r, g, b};
}

std::string entityColorAsString(
  const Color& color, const asset::ColorRange::Type colorRange)
{
  std::stringstream result;
  if (colorRange == asset::ColorRange::Byte)
  {
    result << int(color.r() * 255.0f) << " " << int(color.g() * 255.0f) << " "
           << int(color.b() * 255.0f);
  }
  else if (colorRange == asset::ColorRange::Float)
  {
    result << float(color.r()) << " " << float(color.g()) << " " << float(color.b());
  }
  return result.str();
}

} // namespace tb::mdl
