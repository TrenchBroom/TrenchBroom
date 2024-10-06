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

#include "NodeWriter.h"

#include "io/MapFileSerializer.h"
#include "io/NodeSerializer.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/string_format.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <vector>

namespace tb::io
{
namespace
{

void doWriteNodes(
  NodeSerializer& serializer,
  const std::vector<mdl::Node*>& nodes,
  const mdl::Node* parent = nullptr)
{
  auto parentStack = std::vector<const mdl::Node*>{parent};
  const auto parentProperties = [&]() {
    assert(!parentStack.empty());
    return serializer.parentProperties(parentStack.back());
  };

  for (const auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [&](auto&& thisLambda, const mdl::GroupNode* group) {
        serializer.group(group, parentProperties());

        parentStack.push_back(group);
        group->visitChildren(thisLambda);
        parentStack.pop_back();
      },
      [&](const mdl::EntityNode* entityNode) {
        auto extraProperties = parentProperties();
        const auto& protectedProperties = entityNode->entity().protectedProperties();
        if (!protectedProperties.empty())
        {
          const auto escapedProperties = kdl::vec_transform(
            protectedProperties,
            [](const auto& key) { return kdl::str_escape(key, ";"); });
          extraProperties.emplace_back(
            mdl::EntityPropertyKeys::ProtectedEntityProperties,
            kdl::str_join(escapedProperties, ";"));
        }
        serializer.entity(
          entityNode, entityNode->entity().properties(), extraProperties, entityNode);
      },
      [](const mdl::BrushNode*) {},
      [](const mdl::PatchNode*) {}));
  }
}

} // namespace

NodeWriter::NodeWriter(const mdl::WorldNode& world, std::ostream& stream)
  : NodeWriter{world, MapFileSerializer::create(world.mapFormat(), stream)}
{
}

NodeWriter::NodeWriter(
  const mdl::WorldNode& world, std::unique_ptr<NodeSerializer> serializer)
  : m_world{world}
  , m_serializer{std::move(serializer)}
{
}

NodeWriter::~NodeWriter() = default;

void NodeWriter::setExporting(const bool exporting)
{
  m_serializer->setExporting(exporting);
}

void NodeWriter::writeMap()
{
  m_serializer->beginFile({&m_world});
  writeDefaultLayer();
  writeCustomLayers();
  m_serializer->endFile();
}

void NodeWriter::writeDefaultLayer()
{
  m_serializer->defaultLayer(m_world);

  if (!(m_serializer->exporting() && m_world.defaultLayer()->layer().omitFromExport()))
  {
    doWriteNodes(*m_serializer, m_world.defaultLayer()->children());
  }
}

void NodeWriter::writeCustomLayers()
{
  const std::vector<const mdl::LayerNode*> customLayers = m_world.customLayers();
  for (auto* layer : customLayers)
  {
    writeCustomLayer(layer);
  }
}

void NodeWriter::writeCustomLayer(const mdl::LayerNode* layerNode)
{
  if (!(m_serializer->exporting() && layerNode->layer().omitFromExport()))
  {
    m_serializer->customLayer(layerNode);
    doWriteNodes(*m_serializer, layerNode->children(), layerNode);
  }
}

void NodeWriter::writeNodes(const std::vector<mdl::Node*>& nodes)
{
  m_serializer->beginFile(kdl::vec_static_cast<const mdl::Node*>(nodes));

  // Assort nodes according to their type and, in case of brushes, whether they are entity
  // or world brushes.
  std::vector<mdl::Node*> groups;
  std::vector<mdl::Node*> entities;
  std::vector<mdl::BrushNode*> worldBrushes;
  EntityBrushesMap entityBrushes;

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) { groups.push_back(group); },
      [&](mdl::EntityNode* entity) { entities.push_back(entity); },
      [&](mdl::BrushNode* brush) {
        if (auto* entity = dynamic_cast<mdl::EntityNode*>(brush->parent()))
        {
          entityBrushes[entity].push_back(brush);
        }
        else
        {
          worldBrushes.push_back(brush);
        }
      },
      [](mdl::PatchNode*) {}));
  }

  writeWorldBrushes(worldBrushes);
  writeEntityBrushes(entityBrushes);

  doWriteNodes(*m_serializer, groups);
  doWriteNodes(*m_serializer, entities);

  m_serializer->endFile();
}

void NodeWriter::writeWorldBrushes(const std::vector<mdl::BrushNode*>& brushes)
{
  if (!brushes.empty())
  {
    m_serializer->entity(&m_world, m_world.entity().properties(), {}, brushes);
  }
}

void NodeWriter::writeEntityBrushes(const EntityBrushesMap& entityBrushes)
{
  for (const auto& [entityNode, brushes] : entityBrushes)
  {
    m_serializer->entity(entityNode, entityNode->entity().properties(), {}, brushes);
  }
}

void NodeWriter::writeBrushFaces(const std::vector<mdl::BrushFace>& faces)
{
  m_serializer->beginFile({});
  m_serializer->brushFaces(faces);
  m_serializer->endFile();
}

} // namespace tb::io
