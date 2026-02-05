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

#pragma once

#include "Notifier.h"
#include "NotifierConnection.h"
#include "gl/ResourceId.h"
#include "mdl/PointTrace.h"
#include "ui/Action.h"

#include "vm/bbox.h"
#include "vm/polygon.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kdl
{
class task_manager;
} // namespace kdl

namespace tb
{
class Logger;
class LoggingHub;

namespace gl
{
class ResourceManager;
}

namespace mdl
{
enum class MapFormat;

class Autosaver;
class Command;
class Map;
class Node;
class PickResult;
class UndoableCommand;

struct EnvironmentConfig;
struct GameInfo;
struct SelectionChange;
} // namespace mdl

namespace render
{
class MapRenderer;
}

namespace ui
{
class ActionManager;
class AsyncTaskRunner;
class CachingLogger;
class ViewEffectsService;

struct PointFile
{
  mdl::PointTrace trace;
  std::filesystem::path path;
};

struct PortalFile
{
  std::vector<vm::polygon3f> portals;
  std::filesystem::path path;
};

class MapDocument
{
public:
  static const vm::bbox3d DefaultWorldBounds;

  Notifier<> documentWasLoadedNotifier;
  Notifier<> documentWasSavedNotifier;
  Notifier<> documentDidChangeNotifier;

  Notifier<> modificationStateDidChangeNotifier;

  Notifier<> editorContextDidChangeNotifier;
  Notifier<> currentLayerDidChangeNotifier;
  Notifier<> currentMaterialNameDidChangeNotifier;

  Notifier<> selectionWillChangeNotifier;
  Notifier<const mdl::SelectionChange&> selectionDidChangeNotifier;

  Notifier<const std::vector<mdl::Node*>&> nodesWereAddedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWillBeRemovedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWereRemovedNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesWillChangeNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodesDidChangeNotifier;

  Notifier<const std::vector<mdl::Node*>&> nodeVisibilityDidChangeNotifier;
  Notifier<const std::vector<mdl::Node*>&> nodeLockingDidChangeNotifier;

  Notifier<> groupWasOpenedNotifier;
  Notifier<> groupWasClosedNotifier;

  Notifier<const std::vector<gl::ResourceId>&> resourcesWereProcessedNotifier;

  Notifier<> materialCollectionsWillChangeNotifier;
  Notifier<> materialCollectionsDidChangeNotifier;

  Notifier<> materialUsageCountsDidChangeNotifier;

  Notifier<> entityDefinitionsWillChangeNotifier;
  Notifier<> entityDefinitionsDidChangeNotifier;

  Notifier<> modsWillChangeNotifier;
  Notifier<> modsDidChangeNotifier;

  Notifier<> gridDidChangeNotifier;

  Notifier<mdl::Command&> commandDoNotifier;
  Notifier<mdl::Command&> commandDoneNotifier;
  Notifier<mdl::Command&> commandDoFailedNotifier;
  Notifier<mdl::UndoableCommand&> commandUndoNotifier;
  Notifier<mdl::UndoableCommand&> commandUndoneNotifier;
  Notifier<mdl::UndoableCommand&> commandUndoFailedNotifier;
  Notifier<const std::string&, bool, bool> transactionDoneNotifier;
  Notifier<const std::string&, bool, bool> transactionUndoneNotifier;

  Notifier<> pointFileWasLoadedNotifier;
  Notifier<> pointFileWasUnloadedNotifier;

  Notifier<> portalFileWasLoadedNotifier;
  Notifier<> portalFileWasUnloadedNotifier;

private:
  // pointer so that MapDocument can be moveable
  kdl::task_manager* m_taskManager;
  gl::ResourceManager* m_resourceManager;
  std::unique_ptr<LoggingHub> m_loggingHub;

  std::unique_ptr<mdl::Map> m_map;
  std::unique_ptr<mdl::Autosaver> m_autosaver;

  std::optional<PointFile> m_pointFile;
  std::optional<PortalFile> m_portalFile;

  std::optional<std::vector<Action>> m_cachedTagActions;
  std::optional<std::vector<Action>> m_cachedEntityDefinitionActions;

  ViewEffectsService* m_viewEffectsService = nullptr;

  std::unique_ptr<render::MapRenderer> m_mapRenderer;

  NotifierConnection m_notifierConnection;

public:
  explicit MapDocument(
    kdl::task_manager& taskManager, gl::ResourceManager& resourceManager);

  MapDocument(MapDocument&&) noexcept;
  MapDocument& operator=(MapDocument&&) noexcept;

  static Result<std::unique_ptr<MapDocument>> createDocument(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    kdl::task_manager& taskManager,
    gl::ResourceManager& resourceManager);

  static Result<std::unique_ptr<MapDocument>> loadDocument(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::filesystem::path path,
    kdl::task_manager& taskManager,
    gl::ResourceManager& resourceManager);

  ~MapDocument();

  Result<void> create(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds);

  Result<void> load(
    const mdl::EnvironmentConfig& environmentConfig,
    const mdl::GameInfo& gameInfo,
    mdl::MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    std::filesystem::path path);

  Result<void> reload();

  void triggerAutosave();

private:
  void setMap(std::unique_ptr<mdl::Map>);

  void updateMapFromPreferences();

public: // accessors and such
  mdl::Map& map();
  const mdl::Map& map() const;

  render::MapRenderer& mapRenderer();
  const render::MapRenderer& mapRenderer() const;

  Logger& logger();
  void setTargetLogger(Logger* parentLogger);

  void setViewEffectsService(ViewEffectsService* viewEffectsService);

public: // tag and entity definition actions
  template <typename ActionVisitor>
  void visitTagActions(const ActionManager& actionManager, ActionVisitor&& visitor)
  {
    for (auto& action : cacheTagActions(actionManager))
    {
      visitor(action);
    }
  }

  template <typename ActionVisitor>
  void visitEntityDefinitionActions(
    const ActionManager& actionManager, ActionVisitor&& visitor)
  {
    for (auto& action : cacheEntityDefinitionActions(actionManager))
    {
      visitor(action);
    }
  }

private: // tag and entity definition actions
  std::vector<Action>& cacheTagActions(const ActionManager& actionManager);
  void clearTagActions();

  std::vector<Action>& cacheEntityDefinitionActions(const ActionManager& actionManager);
  void clearEntityDefinitionActions();

public: // point file management
  mdl::PointTrace* pointTrace();
  void loadPointFile(std::filesystem::path path);
  bool isPointFileLoaded() const;
  bool canReloadPointFile() const;
  void reloadPointFile();
  void unloadPointFile();

public: // portal file management
  const std::vector<vm::polygon3f>* portals() const;
  void loadPortalFile(std::filesystem::path path);
  bool isPortalFileLoaded() const;
  bool canReloadPortalFile() const;
  void reloadPortalFile();
  void unloadPortalFile();

private: // observers
  void connectObservers();
  void connectMapObservers();

  void transactionDone(const std::string& name, bool observable, bool isModification);
  void transactionUndone(const std::string& name, bool observable, bool isModification);
  void documentWasLoaded();
  void documentWasCleared();
  void entityDefinitionsDidChange();
  void preferenceDidChange(const std::filesystem::path& preferencePath);
};

} // namespace ui
} // namespace tb
