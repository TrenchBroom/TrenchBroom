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

#include "SwapNodeContentsCommand.h"

#include "mdl/Game.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"
#include "ui/MapDocument.h"

#include "kdl/range_to_vector.h"
#include "kdl/vector_utils.h"

#include <ranges>

namespace tb::mdl
{
namespace
{

auto notifySpecialWorldProperties(
  const mdl::Game& game,
  const std::vector<std::pair<mdl::Node*, mdl::NodeContents>>& nodesToSwap)
{
  for (const auto& [node, contents] : nodesToSwap)
  {
    if (const auto* worldNode = dynamic_cast<const mdl::WorldNode*>(node))
    {
      const auto& oldEntity = worldNode->entity();
      const auto& newEntity = std::get<mdl::Entity>(contents.get());

      const auto* oldWads = oldEntity.property(mdl::EntityPropertyKeys::Wad);
      const auto* newWads = newEntity.property(mdl::EntityPropertyKeys::Wad);

      const bool notifyWadsChange =
        (oldWads == nullptr) != (newWads == nullptr)
        || (oldWads != nullptr && newWads != nullptr && *oldWads != *newWads);

      const auto oldEntityDefinitionSpec = game.extractEntityDefinitionFile(oldEntity);
      const auto newEntityDefinitionSpec = game.extractEntityDefinitionFile(newEntity);
      const bool notifyEntityDefinitionsChange =
        oldEntityDefinitionSpec != newEntityDefinitionSpec;

      const auto oldMods = game.extractEnabledMods(oldEntity);
      const auto newMods = game.extractEnabledMods(newEntity);
      const bool notifyModsChange = oldMods != newMods;

      return std::tuple{
        notifyWadsChange, notifyEntityDefinitionsChange, notifyModsChange};
    }
  }

  return std::tuple{false, false, false};
}

void doSwapNodeContents(
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>>& nodesToSwap,
  ui::MapDocument& document)
{
  const auto nodes = nodesToSwap
                     | std::views::transform([](const auto& pair) { return pair.first; })
                     | kdl::to_vector;
  const auto parents = collectAncestors(nodes);
  const auto descendants = collectDescendants(nodes);

  auto notifyNodes = NotifyBeforeAndAfter{
    document.nodesWillChangeNotifier, document.nodesDidChangeNotifier, nodes};
  auto notifyParents = NotifyBeforeAndAfter{
    document.nodesWillChangeNotifier, document.nodesDidChangeNotifier, parents};
  auto notifyDescendants = NotifyBeforeAndAfter{
    document.nodesWillChangeNotifier, document.nodesDidChangeNotifier, descendants};

  const auto [notifyWadsChange, notifyEntityDefinitionsChange, notifyModsChange] =
    notifySpecialWorldProperties(*document.game(), nodesToSwap);
  auto notifyWads = NotifyBeforeAndAfter{
    notifyWadsChange,
    document.materialCollectionsWillChangeNotifier,
    document.materialCollectionsDidChangeNotifier};
  auto notifyEntityDefinitions = NotifyBeforeAndAfter{
    notifyEntityDefinitionsChange,
    document.entityDefinitionsWillChangeNotifier,
    document.entityDefinitionsDidChangeNotifier};
  auto notifyMods = NotifyBeforeAndAfter{
    notifyModsChange, document.modsWillChangeNotifier, document.modsDidChangeNotifier};

  for (auto& pair : nodesToSwap)
  {
    auto* node = pair.first;
    auto& contents = pair.second.get();

    pair.second = node->accept(kdl::overload(
      [&](mdl::WorldNode* worldNode) {
        return mdl::NodeContents{
          worldNode->setEntity(std::get<mdl::Entity>(std::move(contents)))};
      },
      [&](mdl::LayerNode* layerNode) {
        return mdl::NodeContents(
          layerNode->setLayer(std::get<mdl::Layer>(std::move(contents))));
      },
      [&](mdl::GroupNode* groupNode) {
        return mdl::NodeContents{
          groupNode->setGroup(std::get<mdl::Group>(std::move(contents)))};
      },
      [&](mdl::EntityNode* entityNode) {
        return mdl::NodeContents{
          entityNode->setEntity(std::get<mdl::Entity>(std::move(contents)))};
      },
      [&](mdl::BrushNode* brushNode) {
        return mdl::NodeContents{
          brushNode->setBrush(std::get<mdl::Brush>(std::move(contents)))};
      },
      [&](mdl::PatchNode* patchNode) {
        return mdl::NodeContents{
          patchNode->setPatch(std::get<mdl::BezierPatch>(std::move(contents)))};
      }));
  }
}

} // namespace

SwapNodeContentsCommand::SwapNodeContentsCommand(
  std::string name, std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes)
  : UpdateLinkedGroupsCommandBase{std::move(name), true}
  , m_nodes{std::move(nodes)}
{
}

SwapNodeContentsCommand::~SwapNodeContentsCommand() = default;

std::unique_ptr<CommandResult> SwapNodeContentsCommand::doPerformDo(
  ui::MapDocument& document)
{
  doSwapNodeContents(m_nodes, document);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SwapNodeContentsCommand::doPerformUndo(
  ui::MapDocument& document)
{
  doSwapNodeContents(m_nodes, document);
  return std::make_unique<CommandResult>(true);
}

bool SwapNodeContentsCommand::doCollateWith(UndoableCommand& command)
{
  if (auto* other = dynamic_cast<SwapNodeContentsCommand*>(&command))
  {
    auto myNodes =
      kdl::vec_transform(m_nodes, [](const auto& pair) { return pair.first; });
    auto theirNodes =
      kdl::vec_transform(other->m_nodes, [](const auto& pair) { return pair.first; });

    kdl::vec_sort(myNodes);
    kdl::vec_sort(theirNodes);

    return myNodes == theirNodes;
  }

  return false;
}

} // namespace tb::mdl
