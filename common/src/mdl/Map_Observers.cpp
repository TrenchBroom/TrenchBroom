/*
 Copyright (C) 2025 Kristian Duske

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

#include "Logger.h"
#include "Map.h"
#include "PreferenceManager.h"
#include "mdl/Command.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EditorContext.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/ModelUtils.h"
#include "mdl/RepeatStack.h"
#include "mdl/UndoableCommand.h"

namespace tb::mdl
{

void Map::connectObservers()
{
  m_notifierConnection += nodesWereAddedNotifier.connect(this, &Map::nodesWereAdded);
  m_notifierConnection += nodesWereRemovedNotifier.connect(this, &Map::nodesWereRemoved);
  m_notifierConnection += nodesDidChangeNotifier.connect(this, &Map::nodesDidChange);

  m_notifierConnection +=
    selectionDidChangeNotifier.connect(this, &Map::selectionDidChange);
  m_notifierConnection +=
    selectionWillChangeNotifier.connect(this, &Map::selectionWillChange);

  m_notifierConnection += materialCollectionsWillChangeNotifier.connect(
    this, &Map::materialCollectionsWillChange);
  m_notifierConnection += materialCollectionsDidChangeNotifier.connect(
    this, &Map::materialCollectionsDidChange);

  m_notifierConnection +=
    entityDefinitionsWillChangeNotifier.connect(this, &Map::entityDefinitionsWillChange);
  m_notifierConnection +=
    entityDefinitionsDidChangeNotifier.connect(this, &Map::entityDefinitionsDidChange);

  m_notifierConnection += modsWillChangeNotifier.connect(this, &Map::modsWillChange);
  m_notifierConnection += modsDidChangeNotifier.connect(this, &Map::modsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &Map::preferenceDidChange);
  m_notifierConnection += m_editorContext->editorContextDidChangeNotifier.connect(
    editorContextDidChangeNotifier);
  m_notifierConnection += commandDoneNotifier.connect(this, &Map::commandDone);
  m_notifierConnection += commandUndoneNotifier.connect(this, &Map::commandUndone);
  m_notifierConnection += transactionDoneNotifier.connect(this, &Map::transactionDone);
  m_notifierConnection +=
    transactionUndoneNotifier.connect(this, &Map::transactionUndone);

  // tag management
  m_notifierConnection += mapWasCreatedNotifier.connect(this, &Map::mapWasCreated);
  m_notifierConnection += mapWasLoadedNotifier.connect(this, &Map::mapWasLoaded);
  m_notifierConnection += nodesWereAddedNotifier.connect(this, &Map::initializeNodeTags);
  m_notifierConnection += nodesWillBeRemovedNotifier.connect(this, &Map::clearNodeTags);
  m_notifierConnection += nodesDidChangeNotifier.connect(this, &Map::updateNodeTags);
  m_notifierConnection += brushFacesDidChangeNotifier.connect(this, &Map::updateFaceTags);
  m_notifierConnection += modsDidChangeNotifier.connect(this, &Map::updateAllFaceTags);
  m_notifierConnection += resourcesWereProcessedNotifier.connect(
    this, &Map::updateFaceTagsAfterResourcesWhereProcessed);

  // command processing
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

void Map::mapWasCreated(Map&)
{
  initializeAllNodeTags();
}

void Map::mapWasLoaded(Map&)
{
  initializeAllNodeTags();
}

void Map::nodesWereAdded(const std::vector<Node*>& nodes)
{
  setHasPendingChanges(collectGroups(nodes), false);
  setEntityDefinitions(nodes);
  setEntityModels(nodes);
  setMaterials(nodes);

  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::nodesWereRemoved(const std::vector<Node*>& nodes)
{
  unsetEntityModels(nodes);
  unsetEntityDefinitions(nodes);
  unsetMaterials(nodes);

  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::nodesDidChange(const std::vector<Node*>& nodes)
{
  setEntityDefinitions(nodes);
  setEntityModels(nodes);
  setMaterials(nodes);

  m_cachedSelectionBounds = std::nullopt;
}

void Map::selectionWillChange()
{
  if (const auto currentSelectionBounds = selectionBounds())
  {
    m_lastSelectionBounds = currentSelectionBounds;
  }
}

void Map::selectionDidChange(const SelectionChange&)
{
  m_repeatStack->clearOnNextPush();
  m_cachedSelection = std::nullopt;
  m_cachedSelectionBounds = std::nullopt;
}

void Map::materialCollectionsWillChange()
{
  unsetMaterials();
}

void Map::materialCollectionsDidChange()
{
  loadMaterials();
  setMaterials();
  updateAllFaceTags();
}

void Map::entityDefinitionsWillChange()
{
  clearEntityDefinitions();
  clearEntityModels();
}

void Map::entityDefinitionsDidChange()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
}

void Map::modsWillChange()
{
  unsetEntityModels();
  unsetEntityDefinitions();
  clearEntityModels();
}

void Map::modsDidChange()
{
  updateGameSearchPaths();
  setEntityDefinitions();
  setEntityModels();
}

void Map::preferenceDidChange(const std::filesystem::path& path)
{
  if (m_game && m_game->isGamePathPreference(path))
  {
    const auto& gameFactory = GameFactory::instance();
    const auto newGamePath = gameFactory.gamePath(m_game->config().name);
    m_game->setGamePath(newGamePath, m_logger);

    clearEntityModels();
    setEntityModels();

    reloadMaterials();
    setMaterials();
  }
}

void Map::commandDone(Command& command)
{
  m_logger.debug() << "Command '" << command.name() << "' executed";
}

void Map::commandUndone(UndoableCommand& command)
{
  m_logger.debug() << "Command '" << command.name() << "' undone";
}

void Map::transactionDone(const std::string& name)
{
  m_logger.debug() << "Transaction '" << name << "' executed";
}

void Map::transactionUndone(const std::string& name)
{
  m_logger.debug() << "Transaction '" << name << "' undone";
}

} // namespace tb::mdl
