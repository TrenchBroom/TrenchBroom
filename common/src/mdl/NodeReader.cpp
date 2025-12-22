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

#include "NodeReader.h"

#include "ParserStatus.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/WorldNode.h"

#include "kd/vector_utils.h"

#include <string>
#include <vector>

namespace tb::mdl
{

NodeReader::NodeReader(
  const std::string_view str,
  const MapFormat sourceMapFormat,
  const MapFormat targetMapFormat,
  const EntityPropertyConfig& entityPropertyConfig)
  : MapReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig}
{
}

Result<std::vector<Node*>> NodeReader::read(
  const std::string& str,
  const MapFormat preferredMapFormat,
  const vm::bbox3d& worldBounds,
  const EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status,
  kdl::task_manager& taskManager)
{
  // Try preferred format first
  for (const auto compatibleMapFormat : compatibleFormats(preferredMapFormat))
  {
    if (
      auto result = readAsFormat(
        compatibleMapFormat,
        preferredMapFormat,
        str,
        worldBounds,
        entityPropertyConfig,
        status,
        taskManager))
    {
      return std::move(result) | kdl::transform([&](auto nodes) {
               for (const auto& error : initializeLinkIds(nodes))
               {
                 status.error("Could not restore linked groups: " + error.msg);
               }
               return nodes;
             });
    }
  }

  // All formats failed
  return Error{"Could not parse map data"};
}

/**
 * Attempts to parse the string as one or more entities (in the given source format),
 * and if that fails, as one or more brushes.
 *
 * Does not throw upon parsing failure, but instead logs the failure to `status` and
 * returns {}.
 *
 * @returns the parsed nodes; caller is responsible for freeing them.
 */
Result<std::vector<Node*>> NodeReader::readAsFormat(
  const MapFormat sourceMapFormat,
  const MapFormat targetMapFormat,
  const std::string& str,
  const vm::bbox3d& worldBounds,
  const EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status,
  kdl::task_manager& taskManager)
{
  auto entityReader =
    NodeReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig};
  return entityReader.readEntities(worldBounds, status, taskManager)
         | kdl::transform([&]() {
             status.info(
               "Parsed successfully as " + formatName(sourceMapFormat)
               + " entities");
             return entityReader.m_nodes;
           })
         | kdl::or_else([&](const auto& entityReaderError) {
             status.info(
               "Couldn't parse as " + formatName(sourceMapFormat)
               + " entities: " + entityReaderError.msg);
             kdl::vec_clear_and_delete(entityReader.m_nodes);

             auto brushReader =
               NodeReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig};
             return brushReader.readBrushes(worldBounds, status, taskManager)
                    | kdl::transform([&]() {
                        status.info(
                          "Parsed successfully as " + formatName(sourceMapFormat)
                          + " brushes");
                        return brushReader.m_nodes;
                      })
                    | kdl::if_error([&](const auto& brushReaderError) {
                        status.info(
                          "Couldn't parse as " + formatName(sourceMapFormat)
                          + " brushes: " + brushReaderError.msg);
                        kdl::vec_clear_and_delete(brushReader.m_nodes);
                      });
           });
}

Node* NodeReader::onWorldNode(std::unique_ptr<WorldNode>, ParserStatus&)
{
  // we create a fake layer node instead of using a proper world node
  // layers can contain any node we might parse
  auto* layerNode = new LayerNode{Layer{""}};
  m_nodes.insert(std::begin(m_nodes), layerNode);
  return layerNode;
}

void NodeReader::onLayerNode(std::unique_ptr<Node> layerNode, ParserStatus&)
{
  m_nodes.push_back(layerNode.release());
}

void NodeReader::onNode(
  Node* parentNode, std::unique_ptr<Node> node, ParserStatus&)
{
  if (parentNode != nullptr)
  {
    parentNode->addChild(node.release());
  }
  else
  {
    m_nodes.push_back(node.release());
  }
}

} // namespace tb::mdl
