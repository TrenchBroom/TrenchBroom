/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/NodeQueryValues.h"

#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Tag.h"
#include "mdl/WorldNode.h"

#include <algorithm>

namespace tb::mdl
{

el::Value layerNameValue(const Node& node)
{
  // findContainingLayer has no const overload, but it never mutates node -- same
  // assumption ModelUtils.cpp itself relies on for findContainingGroup(const Node*)
  const auto* layer = findContainingLayer(const_cast<Node*>(&node));
  return layer ? el::Value{layer->layer().name()} : el::Value::Undefined;
}

el::Value groupNameValue(const Node& node)
{
  const auto* group = findContainingGroup(&node);
  return group ? el::Value{group->group().name()} : el::Value::Undefined;
}

bool isLinked(const Map& map, const Node& node)
{
  const auto linkIds = collectParentLinkedGroupIds(node);
  return std::ranges::any_of(linkIds, [&](const auto& linkId) {
    return collectGroupsWithLinkId({const_cast<WorldNode*>(&map.worldNode())}, linkId)
             .size()
           > 1;
  });
}

el::Value tagsValue(const Map& map, const Taggable& taggable)
{
  auto names = el::ArrayType{};
  for (const auto& tag : map.smartTags())
  {
    if (taggable.hasTag(tag))
    {
      names.push_back(el::Value{tag.name()});
    }
  }
  return el::Value{std::move(names)};
}

} // namespace tb::mdl
