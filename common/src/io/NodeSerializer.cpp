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

#include "NodeSerializer.h"

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

namespace tb::io
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
  const std::vector<const mdl::Node*>& rootNodes, kdl::task_manager& taskManager)
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
void NodeSerializer::defaultLayer(const mdl::WorldNode& world)
{
  auto worldEntity = world.entity();

  // Transfer the color, locked state, and hidden state from the default layer Layer
  // object to worldspawn
  const auto* defaultLayerNode = world.defaultLayer();
  const auto& defaultLayer = defaultLayerNode->layer();
  if (const auto color = defaultLayer.color())
  {
    worldEntity.addOrUpdateProperty(
      mdl::EntityPropertyKeys::LayerColor, color->toString());
  }
  else
  {
    worldEntity.removeProperty(mdl::EntityPropertyKeys::LayerColor);
  }

  if (defaultLayerNode->lockState() == mdl::LockState::Locked)
  {
    worldEntity.addOrUpdateProperty(
      mdl::EntityPropertyKeys::LayerLocked, mdl::EntityPropertyValues::LayerLockedValue);
  }
  else
  {
    worldEntity.removeProperty(mdl::EntityPropertyKeys::LayerLocked);
  }

  if (defaultLayerNode->hidden())
  {
    worldEntity.addOrUpdateProperty(
      mdl::EntityPropertyKeys::LayerHidden, mdl::EntityPropertyValues::LayerHiddenValue);
  }
  else
  {
    worldEntity.removeProperty(mdl::EntityPropertyKeys::LayerHidden);
  }

  if (defaultLayer.omitFromExport())
  {
    worldEntity.addOrUpdateProperty(
      mdl::EntityPropertyKeys::LayerOmitFromExport,
      mdl::EntityPropertyValues::LayerOmitFromExportValue);
  }
  else
  {
    worldEntity.removeProperty(mdl::EntityPropertyKeys::LayerOmitFromExport);
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

void NodeSerializer::customLayer(const mdl::LayerNode* layer)
{
  if (!(m_exporting && layer->layer().omitFromExport()))
  {
    entity(layer, layerProperties(layer), {}, layer);
  }
}

void NodeSerializer::group(
  const mdl::GroupNode* group, const std::vector<mdl::EntityProperty>& extraProperties)
{
  entity(group, groupProperties(group), extraProperties, group);
}

void NodeSerializer::entity(
  const mdl::Node* node,
  const std::vector<mdl::EntityProperty>& properties,
  const std::vector<mdl::EntityProperty>& extraProperties,
  const mdl::Node* brushParent)
{
  beginEntity(node, properties, extraProperties);

  brushParent->visitChildren(kdl::overload(
    [](const mdl::WorldNode*) {},
    [](const mdl::LayerNode*) {},
    [](const mdl::GroupNode*) {},
    [](const mdl::EntityNode*) {},
    [&](const mdl::BrushNode* b) { brush(b); },
    [&](const mdl::PatchNode* p) { patch(p); }));

  endEntity(node);
}

void NodeSerializer::entity(
  const mdl::Node* node,
  const std::vector<mdl::EntityProperty>& properties,
  const std::vector<mdl::EntityProperty>& extraProperties,
  const std::vector<mdl::BrushNode*>& entityBrushes)
{
  beginEntity(node, properties, extraProperties);
  brushes(entityBrushes);
  endEntity(node);
}

void NodeSerializer::beginEntity(
  const mdl::Node* node,
  const std::vector<mdl::EntityProperty>& properties,
  const std::vector<mdl::EntityProperty>& extraAttributes)
{
  beginEntity(node);
  entityProperties(properties);
  entityProperties(extraAttributes);
}

void NodeSerializer::beginEntity(const mdl::Node* node)
{
  m_brushNo = 0;
  doBeginEntity(node);
}

void NodeSerializer::endEntity(const mdl::Node* node)
{
  doEndEntity(node);
  ++m_entityNo;
}

void NodeSerializer::entityProperties(const std::vector<mdl::EntityProperty>& properties)
{
  for (const auto& property : properties)
  {
    entityProperty(property);
  }
}

void NodeSerializer::entityProperty(const mdl::EntityProperty& property)
{
  doEntityProperty(property);
}

void NodeSerializer::brushes(const std::vector<mdl::BrushNode*>& brushNodes)
{
  for (auto* brush : brushNodes)
  {
    this->brush(brush);
  }
}

void NodeSerializer::brush(const mdl::BrushNode* brushNode)
{
  doBrush(brushNode);
  ++m_brushNo;
}

void NodeSerializer::patch(const mdl::PatchNode* patchNode)
{
  doPatch(patchNode);
  ++m_brushNo;
}

void NodeSerializer::brushFaces(const std::vector<mdl::BrushFace>& faces)
{
  for (const auto& face : faces)
  {
    brushFace(face);
  }
}

void NodeSerializer::brushFace(const mdl::BrushFace& face)
{
  doBrushFace(face);
}

std::vector<mdl::EntityProperty> NodeSerializer::parentProperties(const mdl::Node* node)
{
  if (node == nullptr)
  {
    return std::vector<mdl::EntityProperty>{};
  }

  auto properties = std::vector<mdl::EntityProperty>{};
  node->accept(kdl::overload(
    [](const mdl::WorldNode*) {},
    [&](const mdl::LayerNode* layerNode) {
      properties.emplace_back(
        mdl::EntityPropertyKeys::Layer, kdl::str_to_string(*layerNode->persistentId()));
    },
    [&](const mdl::GroupNode* groupNode) {
      properties.emplace_back(
        mdl::EntityPropertyKeys::Group, kdl::str_to_string(*groupNode->persistentId()));
    },
    [](const mdl::EntityNode*) {},
    [](const mdl::BrushNode*) {},
    [](const mdl::PatchNode*) {}));

  return properties;
}

std::vector<mdl::EntityProperty> NodeSerializer::layerProperties(
  const mdl::LayerNode* layerNode)
{
  std::vector<mdl::EntityProperty> result = {
    {mdl::EntityPropertyKeys::Classname, mdl::EntityPropertyValues::LayerClassname},
    {mdl::EntityPropertyKeys::GroupType, mdl::EntityPropertyValues::GroupTypeLayer},
    {mdl::EntityPropertyKeys::LayerName, layerNode->name()},
    {mdl::EntityPropertyKeys::LayerId, kdl::str_to_string(*layerNode->persistentId())},
  };

  const auto& layer = layerNode->layer();
  if (layer.hasSortIndex())
  {
    result.emplace_back(
      mdl::EntityPropertyKeys::LayerSortIndex, kdl::str_to_string(layer.sortIndex()));
  }
  if (layerNode->lockState() == mdl::LockState::Locked)
  {
    result.emplace_back(
      mdl::EntityPropertyKeys::LayerLocked, mdl::EntityPropertyValues::LayerLockedValue);
  }
  if (layerNode->hidden())
  {
    result.emplace_back(
      mdl::EntityPropertyKeys::LayerHidden, mdl::EntityPropertyValues::LayerHiddenValue);
  }
  if (layer.omitFromExport())
  {
    result.emplace_back(
      mdl::EntityPropertyKeys::LayerOmitFromExport,
      mdl::EntityPropertyValues::LayerOmitFromExportValue);
  }
  return result;
}

std::vector<mdl::EntityProperty> NodeSerializer::groupProperties(
  const mdl::GroupNode* groupNode)
{
  auto result = std::vector<mdl::EntityProperty>{
    {mdl::EntityPropertyKeys::Classname, mdl::EntityPropertyValues::GroupClassname},
    {mdl::EntityPropertyKeys::GroupType, mdl::EntityPropertyValues::GroupTypeGroup},
    {mdl::EntityPropertyKeys::GroupName, groupNode->name()},
    {mdl::EntityPropertyKeys::GroupId, kdl::str_to_string(*groupNode->persistentId())},
  };

  const auto& linkId = groupNode->linkId();
  result.emplace_back(mdl::EntityPropertyKeys::LinkId, kdl::str_to_string(linkId));

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
    result.emplace_back(mdl::EntityPropertyKeys::GroupTransformation, transformationStr);
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

} // namespace tb::io
