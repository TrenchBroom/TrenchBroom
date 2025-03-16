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

#include "io/ParserException.h"
#include "io/ParserStatus.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/WorldNode.h"

#include "kdl/vector_utils.h"

#include <string>
#include <vector>

namespace tb::io
{

NodeReader::NodeReader(
  const std::string_view str,
  const mdl::MapFormat sourceMapFormat,
  const mdl::MapFormat targetMapFormat,
  const mdl::EntityPropertyConfig& entityPropertyConfig)
  : MapReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig}
{
}

std::vector<mdl::Node*> NodeReader::read(
  const std::string& str,
  const mdl::MapFormat preferredMapFormat,
  const vm::bbox3d& worldBounds,
  const mdl::EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status,
  kdl::task_manager& taskManager)
{
  // Try preferred format first
  for (const auto compatibleMapFormat : mdl::compatibleFormats(preferredMapFormat))
  {
    if (auto result = readAsFormat(
          compatibleMapFormat,
          preferredMapFormat,
          str,
          worldBounds,
          entityPropertyConfig,
          status,
          taskManager);
        !result.empty())
    {
      for (const auto& error : mdl::initializeLinkIds(result))
      {
        status.error("Could not restore linked groups: " + error.msg);
      }
      return result;
    }
  }

  // All formats failed
  return {};
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
std::vector<mdl::Node*> NodeReader::readAsFormat(
  const mdl::MapFormat sourceMapFormat,
  const mdl::MapFormat targetMapFormat,
  const std::string& str,
  const vm::bbox3d& worldBounds,
  const mdl::EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status,
  kdl::task_manager& taskManager)
{
  {
    auto reader = NodeReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig};
    try
    {
      reader.readEntities(worldBounds, status, taskManager);
      status.info(
        "Parsed successfully as " + mdl::formatName(sourceMapFormat) + " entities");
      return reader.m_nodes;
    }
    catch (const ParserException& e)
    {
      status.info(
        "Couldn't parse as " + mdl::formatName(sourceMapFormat)
        + " entities: " + e.what());
      kdl::vec_clear_and_delete(reader.m_nodes);
    }
  }

  {
    auto reader = NodeReader{str, sourceMapFormat, targetMapFormat, entityPropertyConfig};
    try
    {
      reader.readBrushes(worldBounds, status, taskManager);
      status.info(
        "Parsed successfully as " + mdl::formatName(sourceMapFormat) + " brushes");
      return reader.m_nodes;
    }
    catch (const ParserException& e)
    {
      status.info(
        "Couldn't parse as " + mdl::formatName(sourceMapFormat)
        + " brushes: " + e.what());
      kdl::vec_clear_and_delete(reader.m_nodes);
    }
  }
  return {};
}

mdl::Node* NodeReader::onWorldNode(std::unique_ptr<mdl::WorldNode>, ParserStatus&)
{
  // we create a fake layer node instead of using a proper world node
  // layers can contain any node we might parse
  auto* layerNode = new mdl::LayerNode{mdl::Layer{""}};
  m_nodes.insert(std::begin(m_nodes), layerNode);
  return layerNode;
}

void NodeReader::onLayerNode(std::unique_ptr<mdl::Node> layerNode, ParserStatus&)
{
  m_nodes.push_back(layerNode.release());
}

void NodeReader::onNode(
  mdl::Node* parentNode, std::unique_ptr<mdl::Node> node, ParserStatus&)
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

} // namespace tb::io
