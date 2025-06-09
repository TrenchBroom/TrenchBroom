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

#include "MapDocumentCommandFacade.h"

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionFileSpec.h" // IWYU pragma: keep
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/GroupNode.h"
#include "mdl/Issue.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/CommandProcessor.h"
#include "ui/SelectionChange.h"
#include "ui/UndoableCommand.h"

#include "kdl/map_utils.h"
#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tb::ui
{
namespace
{

std::vector<mdl::Node*> collectOldChildren(
  const std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>&
    nodes)
{
  auto result = std::vector<mdl::Node*>{};
  for (auto& [parent, newChildren] : nodes)
  {
    result = kdl::vec_concat(std::move(result), parent->children());
  }
  return result;
}

} // namespace

std::shared_ptr<MapDocument> MapDocumentCommandFacade::newMapDocument(
  kdl::task_manager& taskManager)
{
  // can't use std::make_shared here because the constructor is private
  return std::shared_ptr<MapDocument>{new MapDocumentCommandFacade{taskManager}};
}

MapDocumentCommandFacade::MapDocumentCommandFacade(kdl::task_manager& taskManager)
  : MapDocument{taskManager}
  , m_commandProcessor{std::make_unique<CommandProcessor>(*this)}
{
  connectObservers();
}

MapDocumentCommandFacade::~MapDocumentCommandFacade() = default;

void MapDocumentCommandFacade::performAddNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes)
{
  const auto parents = collectNodesAndAncestors(kdl::map_keys(nodes));
  auto notifyParents =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, parents};

  auto addedNodes = std::vector<mdl::Node*>{};
  for (const auto& [parent, children] : nodes)
  {
    parent->addChildren(children);
    addedNodes = kdl::vec_concat(std::move(addedNodes), children);
  }

  setHasPendingChanges(mdl::collectGroups(addedNodes), false);
  setEntityDefinitions(addedNodes);
  setEntityModels(addedNodes);
  setMaterials(addedNodes);

  nodesWereAddedNotifier(addedNodes);
}

void MapDocumentCommandFacade::performRemoveNodes(
  const std::map<mdl::Node*, std::vector<mdl::Node*>>& nodes)
{
  const auto parents = collectNodesAndAncestors(kdl::map_keys(nodes));
  auto notifyParents =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, parents};

  const auto allChildren = kdl::vec_flatten(kdl::map_values(nodes));
  auto notifyChildren = NotifyBeforeAndAfter{
    nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allChildren};

  for (const auto& [parent, children] : nodes)
  {
    parent->removeChildren(std::begin(children), std::end(children));
  }

  unsetEntityModels(allChildren);
  unsetEntityDefinitions(allChildren);
  unsetMaterials(allChildren);
}

std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>
MapDocumentCommandFacade::performReplaceChildren(
  std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>> nodes)
{
  if (nodes.empty())
  {
    return {};
  }

  const auto parents = collectNodesAndAncestors(kdl::map_keys(nodes));
  auto notifyParents =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, parents};

  const auto allOldChildren = collectOldChildren(nodes);
  auto notifyChildren = NotifyBeforeAndAfter{
    nodesWillBeRemovedNotifier, nodesWereRemovedNotifier, allOldChildren};

  auto result =
    std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>{};
  auto allNewChildren = std::vector<mdl::Node*>{};

  for (auto& [parent, newChildren] : nodes)
  {
    allNewChildren = kdl::vec_concat(
      std::move(allNewChildren),
      kdl::vec_transform(newChildren, [](auto& child) { return child.get(); }));

    auto oldChildren = parent->replaceChildren(std::move(newChildren));

    result.emplace_back(parent, std::move(oldChildren));
  }

  unsetEntityModels(allOldChildren);
  unsetEntityDefinitions(allOldChildren);
  unsetMaterials(allOldChildren);

  setEntityDefinitions(allNewChildren);
  setEntityModels(allNewChildren);
  setMaterials(allNewChildren);

  nodesWereAddedNotifier(allNewChildren);

  return result;
}

static auto notifySpecialWorldProperties(
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

void MapDocumentCommandFacade::performSwapNodeContents(
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>>& nodesToSwap)
{
  const auto nodes =
    kdl::vec_transform(nodesToSwap, [](const auto& pair) { return pair.first; });
  const auto parents = collectAncestors(nodes);
  const auto descendants = collectDescendants(nodes);

  auto notifyNodes =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, nodes};
  auto notifyParents =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, parents};
  auto notifyDescendants =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, descendants};

  const auto [notifyWadsChange, notifyEntityDefinitionsChange, notifyModsChange] =
    notifySpecialWorldProperties(*game(), nodesToSwap);
  auto notifyWads = NotifyBeforeAndAfter{
    notifyWadsChange,
    materialCollectionsWillChangeNotifier,
    materialCollectionsDidChangeNotifier};
  auto notifyEntityDefinitions = NotifyBeforeAndAfter{
    notifyEntityDefinitionsChange,
    entityDefinitionsWillChangeNotifier,
    entityDefinitionsDidChangeNotifier};
  auto notifyMods =
    NotifyBeforeAndAfter{notifyModsChange, modsWillChangeNotifier, modsDidChangeNotifier};

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

  if (!notifyEntityDefinitionsChange && !notifyModsChange)
  {
    setEntityDefinitions(nodes);
    setEntityModels(nodes);
  }
  if (!notifyWadsChange)
  {
    setMaterials(nodes);
  }
}

void MapDocumentCommandFacade::incModificationCount(const size_t delta)
{
  m_modificationCount += delta;
  documentModificationStateDidChangeNotifier();
}

void MapDocumentCommandFacade::decModificationCount(const size_t delta)
{
  assert(m_modificationCount >= delta);
  m_modificationCount -= delta;
  documentModificationStateDidChangeNotifier();
}

void MapDocumentCommandFacade::connectObservers()
{
  m_notifierConnection +=
    m_commandProcessor->commandDoNotifier.connect(commandDoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoneNotifier.connect(commandDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandDoFailedNotifier.connect(commandDoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoNotifier.connect(commandUndoNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoneNotifier.connect(commandUndoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->commandUndoFailedNotifier.connect(commandUndoFailedNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionDoneNotifier.connect(transactionDoneNotifier);
  m_notifierConnection +=
    m_commandProcessor->transactionUndoneNotifier.connect(transactionUndoneNotifier);
}

bool MapDocumentCommandFacade::isCurrentDocumentStateObservable() const
{
  return m_commandProcessor->isCurrentDocumentStateObservable();
}

bool MapDocumentCommandFacade::doCanUndoCommand() const
{
  return m_commandProcessor->canUndo();
}

bool MapDocumentCommandFacade::doCanRedoCommand() const
{
  return m_commandProcessor->canRedo();
}

const std::string& MapDocumentCommandFacade::doGetUndoCommandName() const
{
  return m_commandProcessor->undoCommandName();
}

const std::string& MapDocumentCommandFacade::doGetRedoCommandName() const
{
  return m_commandProcessor->redoCommandName();
}

void MapDocumentCommandFacade::doUndoCommand()
{
  m_commandProcessor->undo();
}

void MapDocumentCommandFacade::doRedoCommand()
{
  m_commandProcessor->redo();
}

void MapDocumentCommandFacade::doClearCommandProcessor()
{
  m_commandProcessor->clear();
}

void MapDocumentCommandFacade::doStartTransaction(
  std::string name, const TransactionScope scope)
{
  m_commandProcessor->startTransaction(std::move(name), scope);
}

void MapDocumentCommandFacade::doCommitTransaction()
{
  m_commandProcessor->commitTransaction();
}

void MapDocumentCommandFacade::doRollbackTransaction()
{
  m_commandProcessor->rollbackTransaction();
}

std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecute(
  std::unique_ptr<Command> command)
{
  return m_commandProcessor->execute(std::move(command));
}

std::unique_ptr<CommandResult> MapDocumentCommandFacade::doExecuteAndStore(
  std::unique_ptr<UndoableCommand> command)
{
  return m_commandProcessor->executeAndStore(std::move(command));
}

} // namespace tb::ui
