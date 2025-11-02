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

#include "Color.h"
#include "Notifier.h"
#include "NotifierConnection.h"
#include "mdl/PointTrace.h"
#include "ui/Actions.h"
#include "ui/CachingLogger.h"

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

namespace tb::mdl
{
enum class MapFormat;

class Game;
class Map;
class Node;
class PickResult;
} // namespace tb::mdl

namespace tb::ui
{
class ViewEffectsService;
class AsyncTaskRunner;

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

class MapDocument : public CachingLogger
{
public:
  static const vm::bbox3d DefaultWorldBounds;
  static const std::string DefaultDocumentName;

  Notifier<> pointFileWasLoadedNotifier;
  Notifier<> pointFileWasUnloadedNotifier;

  Notifier<> portalFileWasLoadedNotifier;
  Notifier<> portalFileWasUnloadedNotifier;

private:
  std::unique_ptr<mdl::Map> m_map;

  std::optional<PointFile> m_pointFile;
  std::optional<PortalFile> m_portalFile;

  std::vector<Action> m_tagActions;
  std::vector<Action> m_entityDefinitionActions;

  ViewEffectsService* m_viewEffectsService = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit MapDocument(kdl::task_manager& taskManager);
  ~MapDocument() override;

public: // accessors and such
  mdl::Map& map();
  const mdl::Map& map() const;

  Logger& logger();

  void setViewEffectsService(ViewEffectsService* viewEffectsService);

public: // tag and entity definition actions
  template <typename ActionVisitor>
  void visitTagActions(const ActionVisitor& visitor) const
  {
    visitActions(visitor, m_tagActions);
  }

  template <typename ActionVisitor>
  void visitEntityDefinitionActions(const ActionVisitor& visitor) const
  {
    visitActions(visitor, m_entityDefinitionActions);
  }

private: // tag and entity definition actions
  template <typename ActionVisitor>
  void visitActions(
    const ActionVisitor& visitor, const std::vector<Action>& actions) const
  {
    for (const auto& action : actions)
    {
      visitor(action);
    }
  }

  void createTagActions();
  void clearTagActions();
  void createEntityDefinitionActions();

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
  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);
  void mapWasCleared(mdl::Map& map);
  void entityDefinitionsDidChange();
};

} // namespace tb::ui
