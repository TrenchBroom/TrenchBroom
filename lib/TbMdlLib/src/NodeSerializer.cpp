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

#include "mdl/NodeSerializer.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LockState.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <fmt/format.h>

#include <string>

namespace tb::mdl
{

NodeSerializer::~NodeSerializer() = default;

NodeSerializer::ObjectNo NodeSerializer::entityNo() const
{
  return m_entityNo;
}

NodeSerializer::ObjectNo NodeSerializer::brushNo() const
{
  return m_brushNo;
}

bool NodeSerializer::exporting() const
{
  return m_exporting;
}

void NodeSerializer::setExporting(const bool exporting)
{
  m_exporting = exporting;
}

void NodeSerializer::beginFile(
  const std::vector<const Node*>& rootNodes, kdl::task_manager& taskManager)
{
  m_entityNo = 0;
  m_brushNo = 0;
  doBeginFile(rootNodes, taskManager);
}

void NodeSerializer::endFile()
{
  doEndFile();
}

/**
 * Writes the worldspawn entity.
 */
void NodeSerializer::defaultLayer(const WorldNode& world)
{
  auto worldEntity = world.entity();

  // Transfer the color, locked state, and hidden state from the default layer Layer
  // object to worldspawn
  const auto* defaultLayerNode = world.defaultLayer();
  const auto& defaultLayer = defaultLayerNode->layer();
  if (const auto color = defaultLayer.color())
  {
    worldEntity.addOrUpdateProperty(EntityPropertyKeys::TbLayerColor, color->toString());
  }
  else
  {
    worldEntity.removeProperty(EntityPropertyKeys::TbLayerColor);
  }

  if (defaultLayerNode->lockState() == LockState::Locked)
  {
    worldEntity.addOrUpdateProperty(
      EntityPropertyKeys::TbLayerLocked, EntityPropertyValues::LayerLockedValue);
  }
  else
  {
    worldEntity.removeProperty(EntityPropertyKeys::TbLayerLocked);
  }

  if (defaultLayerNode->hidden())
  {
    worldEntity.addOrUpdateProperty(
      EntityPropertyKeys::TbLayerHidden, EntityPropertyValues::LayerHiddenValue);
  }
  else
  {
    worldEntity.removeProperty(EntityPropertyKeys::TbLayerHidden);
  }

  if (defaultLayer.omitFromExport())
  {
    worldEntity.addOrUpdateProperty(
      EntityPropertyKeys::TbLayerOmitFromExport,
      EntityPropertyValues::LayerOmitFromExportValue);
  }
  else
  {
    worldEntity.removeProperty(EntityPropertyKeys::TbLayerOmitFromExport);
  }

  if (m_exporting && defaultLayer.omitFromExport())
  {
    beginEntity(&world, worldEntity.properties(), {});
    endEntity(&world);
  }
  else
  {
    entity(&world, worldEntity.properties(), {}, world.defaultLayer());
  }
}

void NodeSerializer::customLayer(const LayerNode* layer)
{
  if (!(m_exporting && layer->layer().omitFromExport()))
  {
    entity(layer, layerProperties(layer), {}, layer);
  }
}

void NodeSerializer::group(
  const GroupNode* group, const std::vector<EntityProperty>& extraProperties)
{
  entity(group, groupProperties(group), extraProperties, group);
}

void NodeSerializer::entity(
  const Node* node,
  const std::vector<EntityProperty>& properties,
  const std::vector<EntityProperty>& extraProperties,
  const Node* brushParent)
{
  beginEntity(node, properties, extraProperties);

  brushParent->visitChildren(kdl::overload(
    [](const WorldNode*) {},
    [](const LayerNode*) {},
    [](const GroupNode*) {},
    [](const EntityNode*) {},
    [&](const BrushNode* b) { brush(b); },
    [&](const PatchNode* p) { patch(p); }));

  endEntity(node);
}

void NodeSerializer::entity(
  const Node* node,
  const std::vector<EntityProperty>& properties,
  const std::vector<EntityProperty>& extraProperties,
  const std::vector<BrushNode*>& entityBrushes)
{
  beginEntity(node, properties, extraProperties);
  brushes(entityBrushes);
  endEntity(node);
}

void NodeSerializer::beginEntity(
  const Node* node,
  const std::vector<EntityProperty>& properties,
  const std::vector<EntityProperty>& extraAttributes)
{
  beginEntity(node);
  entityProperties(properties);
  entityProperties(extraAttributes);
}

void NodeSerializer::beginEntity(const Node* node)
{
  m_brushNo = 0;
  doBeginEntity(node);
}

void NodeSerializer::endEntity(const Node* node)
{
  doEndEntity(node);
  ++m_entityNo;
}

void NodeSerializer::entityProperties(const std::vector<EntityProperty>& properties)
{
  for (const auto& property : properties)
  {
    entityProperty(property);
  }
}

void NodeSerializer::entityProperty(const EntityProperty& property)
{
  doEntityProperty(property);
}

void NodeSerializer::brushes(const std::vector<BrushNode*>& brushNodes)
{
  for (auto* brush : brushNodes)
  {
    this->brush(brush);
  }
}

void NodeSerializer::brush(const BrushNode* brushNode)
{
  doBrush(brushNode);
  ++m_brushNo;
}

void NodeSerializer::patch(const PatchNode* patchNode)
{
  doPatch(patchNode);
  ++m_brushNo;
}

void NodeSerializer::brushFaces(const std::vector<BrushFace>& faces)
{
  for (const auto& face : faces)
  {
    brushFace(face);
  }
}

void NodeSerializer::brushFace(const BrushFace& face)
{
  doBrushFace(face);
}

std::vector<EntityProperty> NodeSerializer::parentProperties(const Node* node)
{
  if (node == nullptr)
  {
    return std::vector<EntityProperty>{};
  }

  auto properties = std::vector<EntityProperty>{};
  node->accept(kdl::overload(
    [](const WorldNode*) {},
    [&](const LayerNode* layerNode) {
      properties.emplace_back(
        EntityPropertyKeys::TbLayer, kdl::str_to_string(*layerNode->persistentId()));
    },
    [&](const GroupNode* groupNode) {
      properties.emplace_back(
        EntityPropertyKeys::TbGroup, kdl::str_to_string(*groupNode->persistentId()));
    },
    [](const EntityNode*) {},
    [](const BrushNode*) {},
    [](const PatchNode*) {}));

  return properties;
}

std::vector<EntityProperty> NodeSerializer::layerProperties(const LayerNode* layerNode)
{
  std::vector<EntityProperty> result = {
    {EntityPropertyKeys::Classname, EntityPropertyValues::LayerClassname},
    {EntityPropertyKeys::TbGroupType, EntityPropertyValues::GroupTypeLayer},
    {EntityPropertyKeys::TbLayerName, layerNode->name()},
    {EntityPropertyKeys::TbLayerId, kdl::str_to_string(*layerNode->persistentId())},
  };

  const auto& layer = layerNode->layer();
  if (layer.hasSortIndex())
  {
    result.emplace_back(
      EntityPropertyKeys::TbLayerSortIndex, kdl::str_to_string(layer.sortIndex()));
  }
  if (layerNode->lockState() == LockState::Locked)
  {
    result.emplace_back(
      EntityPropertyKeys::TbLayerLocked, EntityPropertyValues::LayerLockedValue);
  }
  if (layerNode->hidden())
  {
    result.emplace_back(
      EntityPropertyKeys::TbLayerHidden, EntityPropertyValues::LayerHiddenValue);
  }
  if (layer.omitFromExport())
  {
    result.emplace_back(
      EntityPropertyKeys::TbLayerOmitFromExport,
      EntityPropertyValues::LayerOmitFromExportValue);
  }
  return result;
}

std::vector<EntityProperty> NodeSerializer::groupProperties(const GroupNode* groupNode)
{
  auto result = std::vector<EntityProperty>{
    {EntityPropertyKeys::Classname, EntityPropertyValues::GroupClassname},
    {EntityPropertyKeys::TbGroupType, EntityPropertyValues::GroupTypeGroup},
    {EntityPropertyKeys::TbGroupName, groupNode->name()},
    {EntityPropertyKeys::TbGroupId, kdl::str_to_string(*groupNode->persistentId())},
  };

  const auto& linkId = groupNode->linkId();
  result.emplace_back(EntityPropertyKeys::TbLinkId, kdl::str_to_string(linkId));

  // write transformation matrix in column major format
  const auto& transformation = groupNode->group().transformation();
  if (transformation != vm::mat4x4d::identity())
  {
    const auto transformationStr = fmt::format(
      "{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
      transformation[0][0],
      transformation[1][0],
      transformation[2][0],
      transformation[3][0], // row 0
      transformation[0][1],
      transformation[1][1],
      transformation[2][1],
      transformation[3][1], // row 1
      transformation[0][2],
      transformation[1][2],
      transformation[2][2],
      transformation[3][2], // row 2
      transformation[0][3],
      transformation[1][3],
      transformation[2][3],
      transformation[3][3]); // row 3
    result.emplace_back(EntityPropertyKeys::TbGroupTransformation, transformationStr);
  }

  return result;
}

std::string NodeSerializer::escapeEntityProperties(const std::string& str) const
{
  // Remove a trailing unescaped backslash, as this will choke the parser.
  const auto l = str.size();
  if (l > 0 && str[l - 1] == '\\')
  {
    const auto p = str.find_last_not_of('\\');
    if ((l - p) % 2 == 0)
    {
      // Only remove a trailing backslash if there is an uneven number of trailing
      // backslashes.
      return kdl::str_escape_if_necessary(str.substr(0, l - 1), "\"");
    }
  }
  return kdl::str_escape_if_necessary(str, "\"");
}

} // namespace tb::mdl
