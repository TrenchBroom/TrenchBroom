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
#include "IO/ParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"

#include <kdl/string_utils.h>
#include <kdl/vector_set.h>

#include <cassert>
#include <sstream>
#include <string>

namespace TrenchBroom
{
namespace IO
{
// WorldReaderException

static std::string formatParserExceptions(
  const std::vector<std::tuple<Model::MapFormat, std::string>>& parserExceptions)
{
  std::stringstream result;
  for (const auto& [mapFormat, message] : parserExceptions)
  {
    result << "Error parsing as " << Model::formatName(mapFormat) << ": " << message
           << "\n";
  }
  return result.str();
}

WorldReaderException::WorldReaderException()
  : Exception()
{
}
WorldReaderException::WorldReaderException(
  const std::vector<std::tuple<Model::MapFormat, std::string>>& parserExceptions)
  : Exception{formatParserExceptions(parserExceptions)}
{
}

// WorldReader

WorldReader::WorldReader(
  std::string_view str,
  const Model::MapFormat sourceAndTargetMapFormat,
  const Model::EntityPropertyConfig& entityPropertyConfig)
  : MapReader(
    std::move(str),
    sourceAndTargetMapFormat,
    sourceAndTargetMapFormat,
    entityPropertyConfig)
  , m_world(std::make_unique<Model::WorldNode>(
      entityPropertyConfig, Model::Entity{}, sourceAndTargetMapFormat))
{
  m_world->disableNodeTreeUpdates();
}

std::unique_ptr<Model::WorldNode> WorldReader::tryRead(
  std::string_view str,
  const std::vector<Model::MapFormat>& mapFormatsToTry,
  const vm::bbox3& worldBounds,
  const Model::EntityPropertyConfig& entityPropertyConfig,
  ParserStatus& status)
{
  std::vector<std::tuple<Model::MapFormat, std::string>> parserExceptions;

  for (const auto mapFormat : mapFormatsToTry)
  {
    if (mapFormat == Model::MapFormat::Unknown)
    {
      continue;
    }

    try
    {
      WorldReader reader{str, mapFormat, entityPropertyConfig};
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
    throw WorldReaderException(parserExceptions);
  }
  else
  {
    // mapFormatsToTry was empty or all elements were Model::MapFormat::Unknown
    throw WorldReaderException(
      {{Model::MapFormat::Unknown, "No valid formats to parse as"}});
  }
}

std::unique_ptr<Model::WorldNode> WorldReader::read(
  const vm::bbox3& worldBounds, ParserStatus& status)
{
  readEntities(worldBounds, status);
  sanitizeLayerSortIndicies(status);
  m_world->rebuildNodeTree();
  m_world->enableNodeTreeUpdates();
  return std::move(m_world);
}

/**
 * Sanitizes the sort indices of custom layers:
 * Ensures there are no duplicates or sort indices less than 0.
 *
 * This will be a no-op on a well-formed map file.
 * If the map was saved without layer indices, the file order is used.
 */
void WorldReader::sanitizeLayerSortIndicies(ParserStatus& /* status */)
{
  std::vector<Model::LayerNode*> customLayers = m_world->customLayers();
  Model::LayerNode::sortLayers(customLayers);

  // Gather the layers whose sort indices are invalid. Visit them in the current sorted
  // order.
  std::vector<Model::LayerNode*> invalidLayers;
  std::vector<Model::LayerNode*> validLayers;
  kdl::vector_set<int> usedIndices;
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
    const bool wasInserted = usedIndices.insert(sortIndex).second;
    if (!wasInserted)
    {
      invalidLayers.push_back(layerNode);
      continue;
    }

    validLayers.push_back(layerNode);
  }

  assert(invalidLayers.size() + validLayers.size() == customLayers.size());

  // Renumber the invalid layers
  int nextValidLayerIndex =
    (validLayers.empty() ? 0 : (validLayers.back()->layer().sortIndex() + 1));
  for (auto* layerNode : invalidLayers)
  {
    auto layer = layerNode->layer();
    layer.setSortIndex(nextValidLayerIndex++);
    layerNode->setLayer(std::move(layer));
  }
}

Model::Node* WorldReader::onWorldNode(
  std::unique_ptr<Model::WorldNode> worldNode, ParserStatus&)
{
  // we transfer the properties and the configuration of the default layer, but don't use
  // the given node
  m_world->setEntity(worldNode->entity());

  auto* myDefaultLayerNode = m_world->defaultLayer();
  const auto* theirDefaultLayerNode = worldNode->defaultLayer();
  myDefaultLayerNode->setLayer(theirDefaultLayerNode->layer());
  myDefaultLayerNode->setLockState(theirDefaultLayerNode->lockState());
  myDefaultLayerNode->setVisibilityState(theirDefaultLayerNode->visibilityState());

  return myDefaultLayerNode;
}

void WorldReader::onLayerNode(std::unique_ptr<Model::Node> layerNode, ParserStatus&)
{
  m_world->addChild(layerNode.release());
}

void WorldReader::onNode(
  Model::Node* parentNode, std::unique_ptr<Model::Node> node, ParserStatus&)
{
  if (parentNode != nullptr)
  {
    parentNode->addChild(node.release());
  }
  else
  {
    m_world->defaultLayer()->addChild(node.release());
  }
}
} // namespace IO
} // namespace TrenchBroom
