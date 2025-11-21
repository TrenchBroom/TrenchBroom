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

#include "io/DiskIO.h"
#include "io/LoadMaterialCollections.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/WorldReader.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/MaterialManager.h"
#include "mdl/PortalFile.h"
#include "mdl/PushSelection.h"
#include "mdl/TagManager.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "ui/Actions.h"
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
const std::string MapDocument::DefaultDocumentName("unnamed.map");

MapDocument::MapDocument(kdl::task_manager& taskManager)
  : m_map{std::make_unique<mdl::Map>(taskManager, *this)}
{
  connectObservers();
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

mdl::Map& MapDocument::map()
{
  return *m_map;
}

const mdl::Map& MapDocument::map() const
{
  return *m_map;
}


Logger& MapDocument::logger()
{
  return *this;
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

  io::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPointFile(stream) | kdl::transform([&](auto trace) {
             info() << "Loaded point file " << path;
             m_pointFile = PointFile{std::move(trace), std::move(path)};
             pointFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    error() << "Couldn't load portal file " << path << ": " << e.msg;
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

  info() << "Unloaded point file";
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

  io::Disk::withInputStream(path, [&](auto& stream) {
    return mdl::loadPortalFile(stream) | kdl::transform([&](auto portalFile) {
             info() << "Loaded portal file " << path;
             m_portalFile = {std::move(portalFile), std::move(path)};
             portalFileWasLoadedNotifier();
           });
  }) | kdl::transform_error([&](auto e) {
    error() << "Couldn't load portal file " << path << ": " << e.msg;
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

  info() << "Unloaded portal file";
  portalFileWasUnloadedNotifier();
}

void MapDocument::connectObservers()
{
  m_notifierConnection +=
    m_map->mapWasCreatedNotifier.connect(this, &MapDocument::mapWasCreated);
  m_notifierConnection +=
    m_map->mapWasLoadedNotifier.connect(this, &MapDocument::mapWasLoaded);
  m_notifierConnection +=
    m_map->mapWasClearedNotifier.connect(this, &MapDocument::mapWasCleared);
  m_notifierConnection += m_map->entityDefinitionsDidChangeNotifier.connect(
    this, &MapDocument::entityDefinitionsDidChange);
}

void MapDocument::mapWasCreated(mdl::Map&)
{
  createTagActions();
  createEntityDefinitionActions();
}

void MapDocument::mapWasLoaded(mdl::Map&)
{
  createTagActions();
  createEntityDefinitionActions();
}

void MapDocument::mapWasCleared(mdl::Map&)
{
  clearTagActions();
  clearEntityDefinitionActions();
}

void MapDocument::entityDefinitionsDidChange()
{
  createEntityDefinitionActions();
}

} // namespace tb::ui
