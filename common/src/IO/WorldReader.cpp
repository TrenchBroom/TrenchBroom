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

#include "WorldReader.h"

#include "Color.h"
#include "Error.h"
#include "IO/ParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/LayerNode.h"
#include "Model/LinkedGroupUtils.h"
#include "Model/LockState.h"
#include "Model/ModelUtils.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"

#include "kdl/grouped_range.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"
#include <kdl/string_utils.h>
#include <kdl/vector_set.h>

#include <fmt/format.h>

#include <cassert>
#include <sstream>
#include <string>

namespace TrenchBroom::IO
{
namespace
{
std::string formatParserExceptions(
  const std::vector<std::tuple<Model::MapFormat, std::string>>& parserExceptions)
{
  auto result = std::stringstream{};
  for (const auto& [mapFormat, message] : parserExceptions)
  {
    result << "Error parsing as " << Model::formatName(mapFormat) << ": " << message
           << "\n";
  }
  return result.str();
}
} // namespace

WorldReaderException::WorldReaderException() = default;

WorldReaderException::WorldReaderException(
  const std::vector<std::tuple<Model::MapFormat, std::string>>& parserExceptions)
  : Exception{formatParserExceptions(parserExceptions)}
{
}

WorldReader::WorldReader(
  std::string_view str,
  const Model::MapFormat sourceAndTargetMapFormat,
  const Model::EntityPropertyConfig& entityPropertyConfig)
  : MapReader{std::move(str), sourceAndTargetMapFormat, sourceAndTargetMapFormat, entityPropertyConfig, {}}
  , m_worldNode{std::make_unique<Model::WorldNode>(
      entityPropertyConfig, Model::Entity{}, sourceAndTargetMapFormat)}
{
  m_worldNode->disableNodeTreeUpdates();
}

std::unique_ptr<Model::WorldNode> WorldReader::tryRead(
  std::string_view str,
  const std::vector<Model::MapFormat>& mapFormatsToTry,
  const vm::bbox3& worldBounds,
  const Model::EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status)
{
  auto parserExceptions = std::vector<std::tuple<Model::MapFormat, std::string>>{};

  for (const auto mapFormat : mapFormatsToTry)
  {
    if (mapFormat == Model::MapFormat::Unknown)
    {
      continue;
    }

    try
    {
      auto reader = WorldReader{str, mapFormat, entityPropertyConfig};
      return reader.read(worldBounds, status);
    }
    catch (const ParserException& e)
    {
      parserExceptions.emplace_back(mapFormat, std::string{e.what()});
    }
  }

  if (!parserExceptions.empty())
  {
    // No format parsed successfully. Just throw the parse error from the last one.
    throw WorldReaderException{parserExceptions};
  }
  // mapFormatsToTry was empty or all elements were Model::MapFormat::Unknown
  throw WorldReaderException{
    {{Model::MapFormat::Unknown, "No valid formats to parse as"}}};
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
void sanitizeLayerSortIndicies(Model::WorldNode& worldNode, ParserStatus& /* status */)
{
  auto customLayers = worldNode.customLayers();
  Model::LayerNode::sortLayers(customLayers);

  // Gather the layers whose sort indices are invalid. Visit them in the current sorted
  // order.
  auto invalidLayers = std::vector<Model::LayerNode*>{};
  auto validLayers = std::vector<Model::LayerNode*>{};
  auto usedIndices = kdl::vector_set<int>{};
  for (auto* layerNode : customLayers)
  {
    // Check for a totally invalid index
    const auto sortIndex = layerNode->layer().sortIndex();
    if (sortIndex < 0 || sortIndex == Model::Layer::invalidSortIndex())
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

  assert(invalidLayers.size() + validLayers.size() == customLayers.size());

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

void setLinkIds(Model::WorldNode& worldNode, ParserStatus& status)
{
  const auto errors = Model::initializeLinkIds({&worldNode});
  for (const auto& error : errors)
  {
    status.error("Could not restore linked groups: " + error.msg);
  }
}

} // namespace

std::unique_ptr<Model::WorldNode> WorldReader::read(
  const vm::bbox3& worldBounds, ParserStatus& status)
{
  readEntities(worldBounds, status);
  sanitizeLayerSortIndicies(*m_worldNode, status);
  setLinkIds(*m_worldNode, status);
  m_worldNode->rebuildNodeTree();
  m_worldNode->enableNodeTreeUpdates();
  return std::move(m_worldNode);
}

Model::Node* WorldReader::onWorldNode(
  std::unique_ptr<Model::WorldNode> worldNode, ParserStatus&)
{
  // we transfer the properties and the configuration of the default layer, but don't use
  // the given node
  m_worldNode->setEntity(worldNode->entity());

  auto* myDefaultLayerNode = m_worldNode->defaultLayer();
  const auto* theirDefaultLayerNode = worldNode->defaultLayer();
  myDefaultLayerNode->setLayer(theirDefaultLayerNode->layer());
  myDefaultLayerNode->setLockState(theirDefaultLayerNode->lockState());
  myDefaultLayerNode->setVisibilityState(theirDefaultLayerNode->visibilityState());

  return myDefaultLayerNode;
}

void WorldReader::onLayerNode(std::unique_ptr<Model::Node> layerNode, ParserStatus&)
{
  m_worldNode->addChild(layerNode.release());
}

void WorldReader::onNode(
  Model::Node* parentNode, std::unique_ptr<Model::Node> node, ParserStatus&)
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

} // namespace TrenchBroom::IO
