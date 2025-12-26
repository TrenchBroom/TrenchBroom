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

#include "mdl/WorldReader.h"

#include "ParserStatus.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityProperties.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/ModelUtils.h"
#include "mdl/WorldNode.h"

#include "kd/contracts.h"
#include "kd/vector_set.h"

#include <fmt/format.h>

#include <sstream>
#include <string>

namespace tb::mdl
{
namespace
{

std::string formatParserErrors(
  const std::vector<std::tuple<MapFormat, std::string>>& errors)
{
  auto result = std::stringstream{};
  for (const auto& [mapFormat, message] : errors)
  {
    result << "Error parsing as " << formatName(mapFormat) << ": " << message << "\n";
  }
  return result.str();
}

} // namespace

WorldReader::WorldReader(
  std::string_view str,
  const MapFormat sourceAndTargetMapFormat,
  const EntityPropertyConfig& entityPropertyConfig)
  : MapReader{std::move(str), sourceAndTargetMapFormat, sourceAndTargetMapFormat, entityPropertyConfig}
  , m_worldNode{std::make_unique<WorldNode>(
      entityPropertyConfig, Entity{}, sourceAndTargetMapFormat)}
{
  m_worldNode->disableNodeTreeUpdates();
}

Result<std::unique_ptr<WorldNode>> WorldReader::tryRead(
  std::string_view str,
  const std::vector<MapFormat>& mapFormatsToTry,
  const vm::bbox3d& worldBounds,
  const EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status,
  kdl::task_manager& taskManager)
{
  auto parserErrors = std::vector<std::tuple<MapFormat, std::string>>{};

  for (const auto mapFormat : mapFormatsToTry)
  {
    if (mapFormat == MapFormat::Unknown)
    {
      continue;
    }

    auto reader = WorldReader{str, mapFormat, entityPropertyConfig};
    if (auto result = reader.read(worldBounds, status, taskManager))
    {
      return result;
    }
    else
    {
      std::visit(
        [&](const auto& e) { parserErrors.emplace_back(mapFormat, e.msg); },
        result.error());
    }
  }

  if (!parserErrors.empty())
  {
    // No format parsed successfully. Just return the parse error from the last one.
    return Error{formatParserErrors(parserErrors)};
  }

  // mapFormatsToTry was empty or all elements were MapFormat::Unknown
  return Error{"No valid formats to parse as"};
}

namespace
{

/**
 * Sanitizes the sort indices of custom layers:
 * Ensures there are no duplicates or sort indices less than 0.
 *
 * This will be a no-op on a well-formed map file.
 * If the map was saved without layer indices, the file order is used.
 */
void sanitizeLayerSortIndicies(WorldNode& worldNode, ParserStatus& /* status */)
{
  auto customLayers = worldNode.customLayers();
  LayerNode::sortLayers(customLayers);

  // Gather the layers whose sort indices are invalid. Visit them in the current sorted
  // order.
  auto invalidLayers = std::vector<LayerNode*>{};
  auto validLayers = std::vector<LayerNode*>{};
  auto usedIndices = kdl::vector_set<int>{};
  for (auto* layerNode : customLayers)
  {
    // Check for a totally invalid index
    const auto sortIndex = layerNode->layer().sortIndex();
    if (sortIndex < 0 || sortIndex == Layer::invalidSortIndex())
    {
      invalidLayers.push_back(layerNode);
      continue;
    }

    // Check for an index that has already been used
    if (!usedIndices.insert(sortIndex).second)
    {
      invalidLayers.push_back(layerNode);
      continue;
    }

    validLayers.push_back(layerNode);
  }

  contract_assert(invalidLayers.size() + validLayers.size() == customLayers.size());

  // Renumber the invalid layers
  auto nextValidLayerIndex =
    validLayers.empty() ? 0 : (validLayers.back()->layer().sortIndex() + 1);
  for (auto* layerNode : invalidLayers)
  {
    auto layer = layerNode->layer();
    layer.setSortIndex(nextValidLayerIndex++);
    layerNode->setLayer(std::move(layer));
  }
}

void setLinkIds(WorldNode& worldNode, ParserStatus& status)
{
  const auto errors = initializeLinkIds({&worldNode});
  for (const auto& error : errors)
  {
    status.error("Could not restore linked groups: " + error.msg);
  }
}

} // namespace

Result<std::unique_ptr<WorldNode>> WorldReader::read(
  const vm::bbox3d& worldBounds, ParserStatus& status, kdl::task_manager& taskManager)
{
  return readEntities(worldBounds, status, taskManager) | kdl::transform([&]() {
           sanitizeLayerSortIndicies(*m_worldNode, status);
           setLinkIds(*m_worldNode, status);
           m_worldNode->rebuildNodeTree();
           m_worldNode->enableNodeTreeUpdates();
           return std::move(m_worldNode);
         });
}

Node* WorldReader::onWorldNode(std::unique_ptr<WorldNode> worldNode, ParserStatus&)
{
  // we transfer the properties and the configuration of the default layer, but don't
  // use the given node
  m_worldNode->setEntity(worldNode->entity());

  auto* myDefaultLayerNode = m_worldNode->defaultLayer();
  const auto* theirDefaultLayerNode = worldNode->defaultLayer();
  myDefaultLayerNode->setLayer(theirDefaultLayerNode->layer());
  myDefaultLayerNode->setLockState(theirDefaultLayerNode->lockState());
  myDefaultLayerNode->setVisibilityState(theirDefaultLayerNode->visibilityState());

  return myDefaultLayerNode;
}

void WorldReader::onLayerNode(std::unique_ptr<Node> layerNode, ParserStatus&)
{
  m_worldNode->addChild(layerNode.release());
}

void WorldReader::onNode(Node* parentNode, std::unique_ptr<Node> node, ParserStatus&)
{
  if (parentNode)
  {
    parentNode->addChild(node.release());
  }
  else
  {
    m_worldNode->defaultLayer()->addChild(node.release());
  }
}

} // namespace tb::mdl
