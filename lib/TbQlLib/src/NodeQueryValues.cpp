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

#include "ql/NodeQueryValues.h"

#include "mdl/EntityNodeBase.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Tag.h"
#include "mdl/TagManager.h"
#include "mdl/WorldNode.h"
#include "ql/EntityNodeBinding.h"

#include <algorithm>

namespace tb::ql
{

el::Value layerNameValue(const mdl::Node& node)
{
  // findContainingLayer has no const overload, but it never mutates node -- same
  // assumption ModelUtils.cpp itself relies on for findContainingGroup(const Node*)
  const auto* layer = mdl::findContainingLayer(const_cast<mdl::Node*>(&node));
  return layer ? el::Value{layer->layer().name()} : el::Value::Undefined;
}

el::Value groupNameValue(const mdl::Node& node)
{
  const auto* group = mdl::findContainingGroup(&node);
  return group ? el::Value{group->group().name()} : el::Value::Undefined;
}

bool isLinked(const mdl::Map& map, const mdl::Node& node)
{
  const auto linkIds = mdl::collectParentLinkedGroupIds(node);
  return std::ranges::any_of(linkIds, [&](const auto& linkId) {
    return mdl::collectGroupsWithLinkId(
             {const_cast<mdl::WorldNode*>(&map.worldNode())}, linkId)
             .size()
           > 1;
  });
}

el::Value tagsValue(const mdl::Map& map, const mdl::Taggable& taggable)
{
  auto names = el::ArrayType{};
  for (const auto& tag : map.tagManager().smartTags())
  {
    if (taggable.hasTag(tag))
    {
      names.push_back(el::Value{tag.name()});
    }
  }
  return el::Value{std::move(names)};
}

el::Value ownerEntityValue(const mdl::EntityNodeBase* owner)
{
  return owner ? makeEntityBinding(owner->entity()) : el::Value::Undefined;
}

} // namespace tb::ql
