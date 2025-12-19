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

#include "ui/MapDocument.h"

#include "LoggingHub.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "fs/DiskIO.h"
#include "io/LoadMaterialCollections.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/WorldReader.h"
#include "mdl/Autosaver.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/GameInfo.h"
#include "mdl/Grid.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/MaterialManager.h"
#include "mdl/PortalFile.h"
#include "mdl/PushSelection.h"
#include "mdl/TagManager.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "render/MapRenderer.h"
#include "ui/ActionManager.h"
#include "ui/ViewEffectsService.h"

#include "kd/contracts.h"
#include "kd/result.h"
#include "kd/task_manager.h"

#include "vm/polygon.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <vector>


namespace tb::ui
{

const vm::bbox3d MapDocument::DefaultWorldBounds(-32768.0, 32768.0);

MapDocument::MapDocument(kdl::task_manager& taskManager)
  : m_taskManager{&taskManager}
  , m_loggingHub{std::make_unique<LoggingHub>()}
{
  connectObservers();
}

MapDocument::MapDocument(MapDocument&&) noexcept = default;
MapDocument& MapDocument::operator=(MapDocument&&) noexcept = default;

Result<std::unique_ptr<MapDocument>> MapDocument::createDocument(
  mdl::MapFormat mapFormat,
  const mdl::GameInfo& gameInfo,
  const vm::bbox3d& worldBounds,
  kdl::task_manager& taskManager)
{
  auto document = std::make_unique<MapDocument>(taskManager);
  return document->create(mapFormat, gameInfo, worldBounds)
         | kdl::transform([&]() { return std::move(document); });
}

Result<std::unique_ptr<MapDocument>> MapDocument::loadDocument(
  std::filesystem::path path,
  mdl::MapFormat mapFormat,
  const mdl::GameInfo& gameInfo,
  const vm::bbox3d& worldBounds,
  kdl::task_manager& taskManager)
{
  auto document = std::make_unique<MapDocument>(taskManager);
  return document->load(std::move(path), mapFormat, gameInfo, worldBounds)
         | kdl::transform([&]() { return std::move(document); });
}

MapDocument::~MapDocument()
{
  if (isPointFileLoaded())
  {
    unloadPointFile();
  }
  if (isPortalFileLoaded())
  {
    unloadPortalFile();
  }
}

Result<void> MapDocument::create(
  mdl::MapFormat mapFormat, const mdl::GameInfo& gameInfo, const vm::bbox3d& worldBounds)
{
  return mdl::Map::createMap(mapFormat, gameInfo, worldBounds, *m_taskManager, logger())
         | kdl::transform([&](auto map) {
             setMap(std::move(map));
             documentWasLoadedNotifier();
           });
}

Result<void> MapDocument::load(
  std::filesystem::path path,
  mdl::MapFormat mapFormat,
  const mdl::GameInfo& gameInfo,
  const vm::bbox3d& worldBounds)
{
  return mdl::Map::loadMap(
           path, mapFormat, gameInfo, worldBounds, *m_taskManager, logger())
         | kdl::transform([&](auto map) {
             setMap(std::move(map));
             documentWasLoadedNotifier();
           });
}

Result<void> MapDocument::reload()
{
  return m_map->reload() | kdl::transform([&](auto map) {
           setMap(std::move(map));
           documentWasLoadedNotifier();
         });
}

void MapDocument::triggerAutosave()
{
  m_autosaver->triggerAutosave();
}

void MapDocument::setMap(std::unique_ptr<mdl::Map> map)
{
  m_map = std::move(map);
  m_mapRenderer = std::make_unique<render::MapRenderer>(*m_map);
  m_autosaver = std::make_unique<mdl::Autosaver>(*m_map);

  updateMapFromPreferences();
  connectMapObservers();
}

void MapDocument::updateMapFromPreferences()
{
  m_map->editorContext().setShowPointEntities(pref(Preferences::ShowPointEntities));
  m_map->editorContext().setShowBrushes(pref(Preferences::ShowBrushes));
}

mdl::Map& MapDocument::map()
{
  return *m_map;
}

const mdl::Map& MapDocument::map() const
{
  return *m_map;
}

render::MapRenderer& MapDocument::mapRenderer()
{
  return *m_mapRenderer;
}

const render::MapRenderer& MapDocument::mapRenderer() const
{
  return *m_mapRenderer;
}

Logger& MapDocument::logger()
{
  return *m_loggingHub;
}

void MapDocument::setTargetLogger(Logger* targetLogger)
{
  m_loggingHub->setTargetLogger(targetLogger);
}

mdl::PointTrace* MapDocument::pointTrace()
{
  return m_pointFile ? &m_pointFile->trace : nullptr;
}

const std::vector<vm::polygon3f>* MapDocument::portals() const
{
  return m_portalFile ? &m_portalFile->portals : nullptr;
}

void MapDocument::setViewEffectsService(ViewEffectsService* viewEffectsService)
{
  m_viewEffectsService = viewEffectsService;
}

void MapDocument::createTagActions()
{
  const auto& actionManager = ActionManager::instance();
  m_tagActions = actionManager.createTagActions(m_map->tagManager().smartTags());
}

void MapDocument::clearTagActions()
{
  m_tagActions.clear();
}

void MapDocument::createEntityDefinitionActions()
{
  const auto& actionManager = ActionManager::instance();
  m_entityDefinitionActions = actionManager.createEntityDefinitionActions(
    m_map->entityDefinitionManager().definitions());
}

void MapDocument::clearEntityDefinitionActions()
{
  m_entityDefinitionActions.clear();
}

void MapDocument::loadPointFile(std::filesystem::path path)
{
  static_assert(
    !std::is_reference_v<decltype(path)>,
    "path must be passed by value because reloadPointFile() passes m_pointFilePath");

  if (isPointFileLoaded())
  {
    unloadPointFile();
  }

  fs::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPointFile(stream) | kdl::transform([&](auto trace) {
             logger().info() << "Loaded point file " << path;
             m_pointFile = PointFile{std::move(trace), std::move(path)};
             pointFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    logger().error() << "Couldn't load portal file " << path << ": " << e.msg;
    m_pointFile = {};
  });
}

bool MapDocument::isPointFileLoaded() const
{
  return m_pointFile != std::nullopt;
}

bool MapDocument::canReloadPointFile() const
{
  return isPointFileLoaded();
}

void MapDocument::reloadPointFile()
{
  contract_pre(isPointFileLoaded());

  loadPointFile(m_pointFile->path);
}

void MapDocument::unloadPointFile()
{
  contract_pre(isPointFileLoaded());

  m_pointFile = std::nullopt;

  logger().info() << "Unloaded point file";
  pointFileWasUnloadedNotifier();
}

void MapDocument::loadPortalFile(std::filesystem::path path)
{
  static_assert(
    !std::is_reference_v<decltype(path)>,
    "path must be passed by value because reloadPortalFile() passes m_portalFilePath");

  if (!mdl::canLoadPortalFile(path))
  {
    return;
  }

  if (isPortalFileLoaded())
  {
    unloadPortalFile();
  }

  fs::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPortalFile(stream) | kdl::transform([&](auto portalFile) {
             logger().info() << "Loaded portal file " << path;
             m_portalFile = {std::move(portalFile), std::move(path)};
             portalFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    logger().error() << "Couldn't load portal file " << path << ": " << e.msg;
    m_portalFile = std::nullopt;
  });
}

bool MapDocument::isPortalFileLoaded() const
{
  return m_portalFile != std::nullopt;
}

bool MapDocument::canReloadPortalFile() const
{
  return m_portalFile && mdl::canLoadPortalFile(m_portalFile->path);
}

void MapDocument::reloadPortalFile()
{
  contract_pre(isPortalFileLoaded());

  loadPortalFile(m_portalFile->path);
}

void MapDocument::unloadPortalFile()
{
  contract_pre(isPortalFileLoaded());

  m_portalFile = std::nullopt;

  logger().info() << "Unloaded portal file";
  portalFileWasUnloadedNotifier();
}

void MapDocument::connectObservers()
{
  m_notifierConnection +=
    documentWasLoadedNotifier.connect(this, &MapDocument::documentWasLoaded);

  m_notifierConnection +=
    transactionDoneNotifier.connect(this, &MapDocument::transactionDone);
  m_notifierConnection +=
    transactionUndoneNotifier.connect(this, &MapDocument::transactionUndone);

  m_notifierConnection += entityDefinitionsDidChangeNotifier.connect(
    this, &MapDocument::entityDefinitionsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapDocument::preferenceDidChange);
}

void MapDocument::connectMapObservers()
{
  contract_assert(m_map != nullptr);

  m_notifierConnection += m_map->mapWasSavedNotifier.connect(documentWasSavedNotifier);
  m_notifierConnection +=
    m_map->modificationStateDidChangeNotifier.connect(modificationStateDidChangeNotifier);
  m_notifierConnection +=
    m_map->editorContextDidChangeNotifier.connect(editorContextDidChangeNotifier);
  m_notifierConnection +=
    m_map->currentLayerDidChangeNotifier.connect(currentLayerDidChangeNotifier);
  m_notifierConnection += m_map->currentMaterialNameDidChangeNotifier.connect(
    currentMaterialNameDidChangeNotifier);
  m_notifierConnection +=
    m_map->selectionWillChangeNotifier.connect(selectionWillChangeNotifier);
  m_notifierConnection +=
    m_map->selectionDidChangeNotifier.connect(selectionDidChangeNotifier);
  m_notifierConnection += m_map->nodesWereAddedNotifier.connect(nodesWereAddedNotifier);
  m_notifierConnection +=
    m_map->nodesWillBeRemovedNotifier.connect(nodesWillBeRemovedNotifier);
  m_notifierConnection +=
    m_map->nodesWereRemovedNotifier.connect(nodesWereRemovedNotifier);
  m_notifierConnection += m_map->nodesWillChangeNotifier.connect(nodesWillChangeNotifier);
  m_notifierConnection += m_map->nodesDidChangeNotifier.connect(nodesDidChangeNotifier);
  m_notifierConnection +=
    m_map->nodeLockingDidChangeNotifier.connect(nodeLockingDidChangeNotifier);
  m_notifierConnection += m_map->groupWasOpenedNotifier.connect(groupWasOpenedNotifier);
  m_notifierConnection += m_map->groupWasClosedNotifier.connect(groupWasClosedNotifier);
  m_notifierConnection +=
    m_map->resourcesWereProcessedNotifier.connect(resourcesWereProcessedNotifier);
  m_notifierConnection += m_map->materialCollectionsWillChangeNotifier.connect(
    materialCollectionsWillChangeNotifier);
  m_notifierConnection += m_map->materialCollectionsDidChangeNotifier.connect(
    materialCollectionsDidChangeNotifier);
  m_notifierConnection += m_map->materialUsageCountsDidChangeNotifier.connect(
    materialUsageCountsDidChangeNotifier);
  m_notifierConnection += m_map->entityDefinitionsWillChangeNotifier.connect(
    entityDefinitionsWillChangeNotifier);
  m_notifierConnection +=
    m_map->entityDefinitionsDidChangeNotifier.connect(entityDefinitionsDidChangeNotifier);
  m_notifierConnection += m_map->modsWillChangeNotifier.connect(modsWillChangeNotifier);
  m_notifierConnection += m_map->modsDidChangeNotifier.connect(modsDidChangeNotifier);

  auto& grid = m_map->grid();
  m_notifierConnection += grid.gridDidChangeNotifier.connect(gridDidChangeNotifier);

  auto& commandProcessor = m_map->commandProcessor();
  m_notifierConnection += commandProcessor.commandDoNotifier.connect(commandDoNotifier);
  m_notifierConnection +=
    commandProcessor.commandDoneNotifier.connect(commandDoneNotifier);
  m_notifierConnection +=
    commandProcessor.commandDoFailedNotifier.connect(commandDoFailedNotifier);
  m_notifierConnection +=
    commandProcessor.commandUndoNotifier.connect(commandUndoNotifier);
  m_notifierConnection +=
    commandProcessor.commandUndoneNotifier.connect(commandUndoneNotifier);
  m_notifierConnection +=
    commandProcessor.commandUndoFailedNotifier.connect(commandUndoFailedNotifier);
  m_notifierConnection +=
    commandProcessor.transactionDoneNotifier.connect(transactionDoneNotifier);
  m_notifierConnection +=
    commandProcessor.transactionUndoneNotifier.connect(transactionUndoneNotifier);
}

void MapDocument::transactionDone(const std::string&, const bool observable)
{
  if (observable)
  {
    documentDidChangeNotifier();
  }
}

void MapDocument::transactionUndone(const std::string&, const bool observable)
{
  if (observable)
  {
    documentDidChangeNotifier();
  }
}

void MapDocument::documentWasLoaded()
{
  m_mapRenderer = std::make_unique<render::MapRenderer>(*m_map);

  createTagActions();
  createEntityDefinitionActions();
}

void MapDocument::entityDefinitionsDidChange()
{
  createEntityDefinitionActions();
}

void MapDocument::preferenceDidChange(const std::filesystem::path&)
{
  updateMapFromPreferences();
}

} // namespace tb::ui
