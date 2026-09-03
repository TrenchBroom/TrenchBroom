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

#include "mdl/NodeVariableStore.h"

#include "el/Value.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Node.h"
#include "mdl/Object.h"
#include "mdl/PatchNode.h"
#include "mdl/QueryVariableValues.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/vector_utils.h"

namespace tb::mdl
{
namespace
{

el::Value materialsArrayValue(const BrushNode& brushNode)
{
  auto materialNames = std::vector<std::string>{};
  for (const auto& face : brushNode.brush().faces())
  {
    materialNames.push_back(face.materialName());
  }
  materialNames = kdl::vec_sort_and_remove_duplicates(std::move(materialNames));

  auto result = el::ArrayType{};
  result.reserve(materialNames.size());
  for (auto& materialName : materialNames)
  {
    result.emplace_back(el::Value{std::move(materialName)});
  }
  return el::Value{std::move(result)};
}

el::Value materialsArrayValue(const PatchNode& patchNode)
{
  return el::Value{el::ArrayType{el::Value{patchNode.patch().materialName()}}};
}

bool isLinked(Map& map, const Object& object)
{
  return collectGroupsWithLinkId({&map.worldNode()}, object.linkId()).size() > 1u;
}

el::Value typeNameValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) { return el::Value{"world"}; },
    [](const LayerNode&) { return el::Value{"layer"}; },
    [](const GroupNode&) { return el::Value{"group"}; },
    [](const EntityNode&) { return el::Value{"entity"}; },
    [](const BrushNode&) { return el::Value{"brush"}; },
    [](const PatchNode&) { return el::Value{"patch"}; }));
}

el::Value nameFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode& layerNode) -> el::Value { return el::Value{layerNode.name()}; },
    [](const GroupNode& groupNode) -> el::Value { return el::Value{groupNode.name()}; },
    [](const EntityNode& entityNode) -> el::Value {
      return el::Value{entityNode.entity().classname()};
    },
    [](const BrushNode&) -> el::Value { return el::Value::Undefined; },
    [](const PatchNode&) -> el::Value { return el::Value::Undefined; }));
}

el::Value classnameFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode&) -> el::Value { return el::Value::Undefined; },
    [](const EntityNode& entityNode) -> el::Value {
      return el::Value{entityNode.entity().classname()};
    },
    [](const BrushNode&) -> el::Value { return el::Value::Undefined; },
    [](const PatchNode&) -> el::Value { return el::Value::Undefined; }));
}

el::Value propertiesFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode&) -> el::Value { return el::Value::Undefined; },
    [](const EntityNode& entityNode) -> el::Value {
      return el::Value{propertiesMapValue(entityNode.entity())};
    },
    [](const BrushNode&) -> el::Value { return el::Value::Undefined; },
    [](const PatchNode&) -> el::Value { return el::Value::Undefined; }));
}

el::Value entityFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode&) -> el::Value { return el::Value::Undefined; },
    [](const EntityNode& entityNode) -> el::Value {
      return entityMapValue(entityNode.entity());
    },
    [](const BrushNode& brushNode) -> el::Value {
      return entityMapValue(brushNode.entity()->entity());
    },
    [](const PatchNode& patchNode) -> el::Value {
      return entityMapValue(patchNode.entity()->entity());
    }));
}

el::Value materialsFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode&) -> el::Value { return el::Value::Undefined; },
    [](const EntityNode&) -> el::Value { return el::Value::Undefined; },
    [](const BrushNode& brushNode) -> el::Value {
      return materialsArrayValue(brushNode);
    },
    [](const PatchNode& patchNode) -> el::Value {
      return materialsArrayValue(patchNode);
    }));
}

el::Value tagsFieldValue(Map& map, const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode&) -> el::Value { return el::Value::Undefined; },
    [&](const EntityNode& entityNode) -> el::Value {
      return tagsArrayValue(map, entityNode);
    },
    [&](const BrushNode& brushNode) -> el::Value {
      return tagsArrayValue(map, brushNode);
    },
    [&](const PatchNode& patchNode) -> el::Value {
      return tagsArrayValue(map, patchNode);
    }));
}

el::Value layerNameFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode& groupNode) -> el::Value { return layerNameValue(groupNode); },
    [](const EntityNode& entityNode) -> el::Value { return layerNameValue(entityNode); },
    [](const BrushNode& brushNode) -> el::Value { return layerNameValue(brushNode); },
    [](const PatchNode& patchNode) -> el::Value { return layerNameValue(patchNode); }));
}

el::Value groupNameFieldValue(const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value::Undefined; },
    [](const LayerNode&) -> el::Value { return el::Value::Undefined; },
    [](const GroupNode& groupNode) -> el::Value { return groupNameValue(groupNode); },
    [](const EntityNode& entityNode) -> el::Value { return groupNameValue(entityNode); },
    [](const BrushNode& brushNode) -> el::Value { return groupNameValue(brushNode); },
    [](const PatchNode& patchNode) -> el::Value { return groupNameValue(patchNode); }));
}

el::Value linkedFieldValue(Map& map, const Node& node)
{
  return node.accept(kdl::overload(
    [](const WorldNode&) -> el::Value { return el::Value{false}; },
    [](const LayerNode&) -> el::Value { return el::Value{false}; },
    [&](const GroupNode& groupNode) -> el::Value {
      return el::Value{isLinked(map, groupNode)};
    },
    [&](const EntityNode& entityNode) -> el::Value {
      return el::Value{isLinked(map, entityNode)};
    },
    [&](const BrushNode& brushNode) -> el::Value {
      return el::Value{isLinked(map, brushNode)};
    },
    [&](const PatchNode& patchNode) -> el::Value {
      return el::Value{isLinked(map, patchNode)};
    }));
}

} // namespace

NodeVariableStore::NodeVariableStore(Map& map, const Node& node)
  : m_map{map}
  , m_node{node}
{
}

el::VariableStore* NodeVariableStore::clone() const
{
  return new NodeVariableStore{m_map, m_node};
}

size_t NodeVariableStore::size() const
{
  return names().size();
}

el::Value NodeVariableStore::value(const std::string& name) const
{
  if (name == "type")
  {
    return typeNameValue(m_node);
  }
  if (name == "name")
  {
    return nameFieldValue(m_node);
  }
  if (name == "classname")
  {
    return classnameFieldValue(m_node);
  }
  if (name == "properties")
  {
    return propertiesFieldValue(m_node);
  }
  if (name == "entity")
  {
    return entityFieldValue(m_node);
  }
  if (name == "materials")
  {
    return materialsFieldValue(m_node);
  }
  if (name == "bounds")
  {
    return el::Value{m_node.logicalBounds()};
  }
  if (name == "center")
  {
    return el::Value{m_node.logicalBounds().center()};
  }
  if (name == "layerName")
  {
    return layerNameFieldValue(m_node);
  }
  if (name == "groupName")
  {
    return groupNameFieldValue(m_node);
  }
  if (name == "tags")
  {
    return tagsFieldValue(m_map, m_node);
  }
  if (name == "visible")
  {
    return el::Value{m_node.visible()};
  }
  if (name == "locked")
  {
    return el::Value{m_node.locked()};
  }
  if (name == "selected")
  {
    return el::Value{m_node.selected()};
  }
  if (name == "linked")
  {
    return linkedFieldValue(m_map, m_node);
  }
  return el::Value::Undefined;
}

std::vector<std::string> NodeVariableStore::names() const
{
  return {
    "type",
    "name",
    "classname",
    "properties",
    "entity",
    "materials",
    "bounds",
    "center",
    "layerName",
    "groupName",
    "tags",
    "visible",
    "locked",
    "selected",
    "linked",
  };
}

void NodeVariableStore::set(std::string, el::Value) {}

} // namespace tb::mdl
