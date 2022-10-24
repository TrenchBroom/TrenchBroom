/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityProperties.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <vecmath/vec_io.h> // for Color stream output operator

#include <kdl/overload.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <fmt/format.h>

#include <string>

namespace TrenchBroom
{
namespace IO
{
NodeSerializer::NodeSerializer()
  : m_entityNo(0)
  , m_brushNo(0)
  , m_exporting(false)
{
}

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

void NodeSerializer::beginFile(const std::vector<const Model::Node*>& rootNodes)
{
  m_entityNo = 0;
  m_brushNo = 0;
  doBeginFile(rootNodes);
}

void NodeSerializer::endFile()
{
  doEndFile();
}

/**
 * Writes the worldspawn entity.
 */
void NodeSerializer::defaultLayer(const Model::WorldNode& world)
{
  auto worldEntity = world.entity();

  // Transfer the color, locked state, and hidden state from the default layer Layer
  // object to worldspawn
  const Model::LayerNode* defaultLayerNode = world.defaultLayer();
  const Model::Layer& defaultLayer = defaultLayerNode->layer();
  const auto& entityPropertyConfig = world.entityPropertyConfig();
  if (defaultLayer.color())
  {
    worldEntity.addOrUpdateProperty(
      entityPropertyConfig,
      Model::EntityPropertyKeys::LayerColor,
      kdl::str_to_string(*defaultLayer.color()));
  }
  else
  {
    worldEntity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerColor);
  }

  if (defaultLayerNode->lockState() == Model::LockState::Locked)
  {
    worldEntity.addOrUpdateProperty(
      entityPropertyConfig,
      Model::EntityPropertyKeys::LayerLocked,
      Model::EntityPropertyValues::LayerLockedValue);
  }
  else
  {
    worldEntity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerLocked);
  }

  if (defaultLayerNode->hidden())
  {
    worldEntity.addOrUpdateProperty(
      entityPropertyConfig,
      Model::EntityPropertyKeys::LayerHidden,
      Model::EntityPropertyValues::LayerHiddenValue);
  }
  else
  {
    worldEntity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerHidden);
  }

  if (defaultLayer.omitFromExport())
  {
    worldEntity.addOrUpdateProperty(
      entityPropertyConfig,
      Model::EntityPropertyKeys::LayerOmitFromExport,
      Model::EntityPropertyValues::LayerOmitFromExportValue);
  }
  else
  {
    worldEntity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerOmitFromExport);
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

void NodeSerializer::customLayer(const Model::LayerNode* layer)
{
  if (!(m_exporting && layer->layer().omitFromExport()))
  {
    entity(layer, layerProperties(layer), {}, layer);
  }
}

void NodeSerializer::group(
  const Model::GroupNode* group,
  const std::vector<Model::EntityProperty>& extraProperties)
{
  entity(group, groupProperties(group), extraProperties, group);
}

void NodeSerializer::entity(
  const Model::Node* node,
  const std::vector<Model::EntityProperty>& properties,
  const std::vector<Model::EntityProperty>& extraProperties,
  const Model::Node* brushParent)
{
  beginEntity(node, properties, extraProperties);

  brushParent->visitChildren(kdl::overload(
    [](const Model::WorldNode*) {},
    [](const Model::LayerNode*) {},
    [](const Model::GroupNode*) {},
    [](const Model::EntityNode*) {},
    [&](const Model::BrushNode* b) { brush(b); },
    [&](const Model::PatchNode* p) { patch(p); }));

  endEntity(node);
}

void NodeSerializer::entity(
  const Model::Node* node,
  const std::vector<Model::EntityProperty>& properties,
  const std::vector<Model::EntityProperty>& extraProperties,
  const std::vector<Model::BrushNode*>& entityBrushes)
{
  beginEntity(node, properties, extraProperties);
  brushes(entityBrushes);
  endEntity(node);
}

void NodeSerializer::beginEntity(
  const Model::Node* node,
  const std::vector<Model::EntityProperty>& properties,
  const std::vector<Model::EntityProperty>& extraAttributes)
{
  beginEntity(node);
  entityProperties(properties);
  entityProperties(extraAttributes);
}

void NodeSerializer::beginEntity(const Model::Node* node)
{
  m_brushNo = 0;
  doBeginEntity(node);
}

void NodeSerializer::endEntity(const Model::Node* node)
{
  doEndEntity(node);
  ++m_entityNo;
}

void NodeSerializer::entityProperties(
  const std::vector<Model::EntityProperty>& properties)
{
  for (const auto& property : properties)
  {
    entityProperty(property);
  }
}

void NodeSerializer::entityProperty(const Model::EntityProperty& property)
{
  doEntityProperty(property);
}

void NodeSerializer::brushes(const std::vector<Model::BrushNode*>& brushNodes)
{
  for (auto* brush : brushNodes)
  {
    this->brush(brush);
  }
}

void NodeSerializer::brush(const Model::BrushNode* brushNode)
{
  doBrush(brushNode);
  ++m_brushNo;
}

void NodeSerializer::patch(const Model::PatchNode* patchNode)
{
  doPatch(patchNode);
  ++m_brushNo;
}

void NodeSerializer::brushFaces(const std::vector<Model::BrushFace>& faces)
{
  for (const auto& face : faces)
  {
    brushFace(face);
  }
}

void NodeSerializer::brushFace(const Model::BrushFace& face)
{
  doBrushFace(face);
}

std::vector<Model::EntityProperty> NodeSerializer::parentProperties(
  const Model::Node* node)
{
  if (node == nullptr)
  {
    return std::vector<Model::EntityProperty>{};
  }

  auto properties = std::vector<Model::EntityProperty>{};
  node->accept(kdl::overload(
    [](const Model::WorldNode*) {},
    [&](const Model::LayerNode* layerNode) {
      properties.push_back(Model::EntityProperty(
        Model::EntityPropertyKeys::Layer,
        kdl::str_to_string(*layerNode->persistentId())));
    },
    [&](const Model::GroupNode* groupNode) {
      properties.push_back(Model::EntityProperty(
        Model::EntityPropertyKeys::Group,
        kdl::str_to_string(*groupNode->persistentId())));
    },
    [](const Model::EntityNode*) {},
    [](const Model::BrushNode*) {},
    [](const Model::PatchNode*) {}));

  return properties;
}

std::vector<Model::EntityProperty> NodeSerializer::layerProperties(
  const Model::LayerNode* layerNode)
{
  std::vector<Model::EntityProperty> result = {
    Model::EntityProperty(
      Model::EntityPropertyKeys::Classname, Model::EntityPropertyValues::LayerClassname),
    Model::EntityProperty(
      Model::EntityPropertyKeys::GroupType, Model::EntityPropertyValues::GroupTypeLayer),
    Model::EntityProperty(Model::EntityPropertyKeys::LayerName, layerNode->name()),
    Model::EntityProperty(
      Model::EntityPropertyKeys::LayerId, kdl::str_to_string(*layerNode->persistentId())),
  };

  const auto& layer = layerNode->layer();
  if (layer.hasSortIndex())
  {
    result.push_back(Model::EntityProperty(
      Model::EntityPropertyKeys::LayerSortIndex, kdl::str_to_string(layer.sortIndex())));
  }
  if (layerNode->lockState() == Model::LockState::Locked)
  {
    result.push_back(Model::EntityProperty(
      Model::EntityPropertyKeys::LayerLocked,
      Model::EntityPropertyValues::LayerLockedValue));
  }
  if (layerNode->hidden())
  {
    result.push_back(Model::EntityProperty(
      Model::EntityPropertyKeys::LayerHidden,
      Model::EntityPropertyValues::LayerHiddenValue));
  }
  if (layer.omitFromExport())
  {
    result.push_back(Model::EntityProperty(
      Model::EntityPropertyKeys::LayerOmitFromExport,
      Model::EntityPropertyValues::LayerOmitFromExportValue));
  }
  return result;
}

std::vector<Model::EntityProperty> NodeSerializer::groupProperties(
  const Model::GroupNode* groupNode)
{
  auto result = std::vector<Model::EntityProperty>{
    Model::EntityProperty(
      Model::EntityPropertyKeys::Classname, Model::EntityPropertyValues::GroupClassname),
    Model::EntityProperty(
      Model::EntityPropertyKeys::GroupType, Model::EntityPropertyValues::GroupTypeGroup),
    Model::EntityProperty(Model::EntityPropertyKeys::GroupName, groupNode->name()),
    Model::EntityProperty(
      Model::EntityPropertyKeys::GroupId, kdl::str_to_string(*groupNode->persistentId())),
  };

  if (const auto linkedGroupId = groupNode->group().linkedGroupId())
  {
    result.emplace_back(
      Model::EntityPropertyKeys::LinkedGroupId, kdl::str_to_string(*linkedGroupId));

    // write transformation matrix in column major format
    const auto& transformation = groupNode->group().transformation();
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
    result.emplace_back(
      Model::EntityPropertyKeys::GroupTransformation, transformationStr);
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
} // namespace IO
} // namespace TrenchBroom
