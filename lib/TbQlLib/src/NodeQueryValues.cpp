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
#include "mdl/Map.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/Tag.h"
#include "mdl/TagManager.h"
#include "ql/EntityNodeLazyMap.h"

#include "kd/ranges/to.h"

namespace tb::ql
{

el::Value layerNameValue(const mdl::Node& node)
{
  const auto* layer = mdl::findContainingLayer(node);
  return layer ? el::Value{layer->layer().name()} : el::Value::Undefined;
}

el::Value groupNameValue(const mdl::Node& node)
{
  const auto* group = mdl::findContainingGroup(node);
  return group ? el::Value{group->group().name()} : el::Value::Undefined;
}

el::Value tagsValue(const mdl::Map& map, const mdl::Taggable& taggable)
{
  return el::Value{
    map.tagManager().smartTags()
    | std::views::filter([&](const auto& smartTag) { return taggable.hasTag(smartTag); })
    | std::views::transform(
      [](const auto& smartTag) { return el::Value{smartTag.name()}; })
    | kdl::ranges::to<el::ArrayType>()};
}

el::Value ownerEntityValue(const mdl::EntityNodeBase* owner)
{
  return owner ? makeEntityLazyMap(owner->entity()) : el::Value::Undefined;
}

} // namespace tb::ql
