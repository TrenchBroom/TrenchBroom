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

#include "mdl/NodeWriter.h"

#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFileSerializer.h"
#include "mdl/Node.h"
#include "mdl/NodeSerializer.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include <vector>

namespace tb::mdl
{
namespace
{

void doWriteNodes(
  NodeSerializer& serializer,
  const std::vector<Node*>& nodes,
  const Node* parent = nullptr)
{
  auto parentStack = std::vector<const Node*>{parent};
  const auto parentProperties = [&]() {
    contract_pre(!parentStack.empty());

    return serializer.parentProperties(parentStack.back());
  };

  for (const auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](auto&& thisLambda, const GroupNode& groupNode) {
        serializer.group(groupNode, parentProperties());

        parentStack.push_back(&groupNode);
        groupNode.visitChildren(thisLambda);
        parentStack.pop_back();
      },
      [&](const EntityNode& entityNode) {
        auto extraProperties = parentProperties();
        const auto& protectedProperties = entityNode.entity().protectedProperties();
        if (!protectedProperties.empty())
        {
          const auto escapedProperties = protectedProperties
                                         | std::views::transform([](const auto& key) {
                                             return kdl::str_escape(key, ";");
                                           })
                                         | kdl::ranges::to<std::vector>();
          extraProperties.emplace_back(
            EntityPropertyKeys::TbProtectedEntityProperties,
            kdl::str_join(escapedProperties, ";"));
        }
        serializer.entity(
          entityNode, entityNode.entity().properties(), extraProperties, entityNode);
      },
      [](const BrushNode&) {},
      [](const PatchNode&) {}));
  }
}

} // namespace

NodeWriter::NodeWriter(const WorldNode& world, std::ostream& stream)
  : NodeWriter{world, MapFileSerializer::create(world.mapFormat(), stream)}
{
}

NodeWriter::NodeWriter(const WorldNode& world, std::unique_ptr<NodeSerializer> serializer)
  : m_world{world}
  , m_serializer{std::move(serializer)}
{
}

NodeWriter::~NodeWriter() = default;

void NodeWriter::setExporting(const bool exporting)
{
  m_serializer->setExporting(exporting);
}

void NodeWriter::setStripTbProperties(const bool stripTbProperties)
{
  m_serializer->setStripTbProperties(stripTbProperties);
}

void NodeWriter::setStripEntityPattern(std::optional<std::string> stripEntityPattern)
{
  m_serializer->setStripEntityPattern(std::move(stripEntityPattern));
}

void NodeWriter::setEntityToAdd(std::optional<Entity> entityToAdd)
{
  m_serializer->setEntityToAdd(std::move(entityToAdd));
}

void NodeWriter::writeMap(kdl::task_manager& taskManager)
{
  m_serializer->beginFile({&m_world}, taskManager);
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
  for (auto* layerNode : m_world.customLayers())
  {
    writeCustomLayer(*layerNode);
  }
}

void NodeWriter::writeCustomLayer(const LayerNode& layerNode)
{
  if (!(m_serializer->exporting() && layerNode.layer().omitFromExport()))
  {
    m_serializer->customLayer(layerNode);
    doWriteNodes(*m_serializer, layerNode.children(), &layerNode);
  }
}

void NodeWriter::writeNodes(
  const std::vector<Node*>& nodes, kdl::task_manager& taskManager)
{
  m_serializer->beginFile(kdl::vec_static_cast<const Node*>(nodes), taskManager);

  // Assort nodes according to their type and, in case of brushes and patches, whether
  // they are entity or world children. Brushes and patches belonging to the same parent
  // are kept in the relative order in which they appear in `nodes` so that round-tripping
  // a mixed selection through copy/paste does not reorder an entity's children.
  std::vector<Node*> groups;
  std::vector<Node*> entities;
  std::vector<Node*> worldChildren;
  EntityChildNodesMap entityChildNodes;

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode&) {},
      [](LayerNode&) {},
      [&](GroupNode& groupNode) { groups.push_back(&groupNode); },
      [&](EntityNode& entityNode) { entities.push_back(&entityNode); },
      [&](BrushNode& brushNode) {
        if (auto* entityNode = dynamic_cast<EntityNode*>(brushNode.parent()))
        {
          entityChildNodes[entityNode].push_back(&brushNode);
        }
        else
        {
          worldChildren.push_back(&brushNode);
        }
      },
      [&](PatchNode& patchNode) {
        if (auto* entityNode = dynamic_cast<EntityNode*>(patchNode.parent()))
        {
          entityChildNodes[entityNode].push_back(&patchNode);
        }
        else
        {
          worldChildren.push_back(&patchNode);
        }
      }));
  }

  writeWorldNode(worldChildren);
  writeEntityNodes(entityChildNodes);

  doWriteNodes(*m_serializer, groups);
  doWriteNodes(*m_serializer, entities);

  m_serializer->endFile();
}

void NodeWriter::writeWorldNode(const std::vector<Node*>& children)
{
  if (!children.empty())
  {
    m_serializer->entity(m_world, m_world.entity().properties(), {}, children);
  }
}

void NodeWriter::writeEntityNodes(const EntityChildNodesMap& entityChildNodes)
{
  for (const auto& [entityNode, children] : entityChildNodes)
  {
    m_serializer->entity(*entityNode, entityNode->entity().properties(), {}, children);
  }
}

void NodeWriter::writeBrushFaces(
  const std::vector<BrushFace>& faces, kdl::task_manager& taskManager)
{
  m_serializer->beginFile({}, taskManager);
  m_serializer->brushFaces(faces);
  m_serializer->endFile();
}

} // namespace tb::mdl
