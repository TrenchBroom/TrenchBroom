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

#include <QKeySequence>

#include "Macros.h"
#include "ui/ActionContext.h"

#include "kdl/overload.h"
#include "kdl/path_hash.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tb::mdl
{
class EntityDefinition;
class SmartTag;
} // namespace tb::mdl

namespace tb::ui
{
class MapDocument;
class MapFrame;
class MapViewBase;

class ActionExecutionContext
{
private:
  ActionContext::Type m_actionContext;
  MapFrame* m_frame;
  MapViewBase* m_mapView;

public:
  ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView);

  bool hasDocument() const;
  bool hasActionContext(ActionContext::Type actionContext) const;

  MapFrame* frame();
  const MapFrame* frame() const;

  MapViewBase* view();
  const MapViewBase* view() const;

  MapDocument* document();
  const MapDocument* document() const;
};

using ExecuteFn = std::function<void(ActionExecutionContext&)>;
using EnabledFn = std::function<bool(const ActionExecutionContext&)>;
using CheckedFn = std::function<bool(const ActionExecutionContext&)>;

class Action
{
private:
  QString m_label;
  std::filesystem::path m_preferencePath;
  ActionContext::Type m_actionContext;
  QKeySequence m_defaultShortcut;

  ExecuteFn m_execute;
  EnabledFn m_enabled;
  std::optional<CheckedFn> m_checked;

  std::optional<std::filesystem::path> m_iconPath;
  std::optional<QString> m_statusTip;

public:
  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    QKeySequence defaultShortcut,
    ExecuteFn execute,
    EnabledFn enabled,
    std::optional<CheckedFn> checked,
    std::optional<std::filesystem::path> iconPath = std::nullopt,
    std::optional<QString> statusTip = std::nullopt);

  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    QKeySequence defaultShortcut,
    ExecuteFn execute,
    EnabledFn enabled,
    std::optional<std::filesystem::path> iconPath = std::nullopt,
    std::optional<QString> statusTip = std::nullopt);

  Action(
    std::filesystem::path preferencePath,
    QString label,
    ActionContext::Type actionContext,
    ExecuteFn execute,
    EnabledFn enabled);

  const QString& label() const;
  const std::filesystem::path& preferencePath() const;
  ActionContext::Type actionContext() const;
  QKeySequence keySequence() const;
  void setKeySequence(const QKeySequence& keySequence) const;
  void resetKeySequence() const;

  void execute(ActionExecutionContext& context) const;
  bool enabled(const ActionExecutionContext& context) const;
  bool checkable() const;
  bool checked(const ActionExecutionContext& context) const;

  const std::optional<std::filesystem::path>& iconPath() const;

  const std::optional<QString>& statusTip() const;

  deleteCopy(Action);

  // cannot be noexcept because it will call QKeySequence's copy constructor
  Action(Action&& other) = default;
  Action& operator=(Action&& other) = default;
};

enum class MenuEntryType
{
  RecentDocuments,
  Undo,
  Redo,
  Cut,
  Copy,
  Paste,
  PasteAtOriginalPosition,
  None
};

struct MenuSeparator
{
};

struct MenuAction
{
  const Action& action;
  MenuEntryType entryType;
};

struct Menu;

using MenuEntry = std::variant<MenuSeparator, MenuAction, Menu>;

struct Menu
{
  std::string name;
  MenuEntryType entryType;
  std::vector<MenuEntry> entries;

  void addSeparator();
  const Action& addItem(
    const Action& action, MenuEntryType entryType = MenuEntryType::None);
  Menu& addMenu(std::string name, MenuEntryType entryType = MenuEntryType::None);

  template <typename Visitor>
  void visitEntries(const Visitor& visitor) const
  {
    for (const auto& entry : entries)
    {
      std::visit(
        kdl::overload(
          [&](const MenuSeparator& separatorItem) { visitor(separatorItem); },
          [&](const MenuAction& actionItem) { visitor(actionItem); },
          [&](const Menu& menu) { visitor(visitor, menu); }),
        entry);
    }
  }
};

using ActionVisitor = std::function<void(const Action&)>;

class ActionManager
{
private:
  /**
   * All actions which are used either in a menu, a tool bar or as a shortcut.
   * Indexed by preference path.
   */
  std::unordered_map<std::filesystem::path, Action, kdl::path_hash> m_actions;

  /**
   * The main menu for the map editing window.
   * These will hold pointers to the actions in m_actions.
   */
  std::vector<Menu> m_mainMenu;

  /**
   * The toolbar for the map editing window. Stored as a menu to allow for separators.
   * These will hold pointers to the actions in m_actions.
   */
  Menu m_toolBar = Menu{"Toolbar", MenuEntryType::None, {}};

private:
  ActionManager();

public:
  deleteCopyAndMove(ActionManager);

  static const ActionManager& instance();

  /**
   * Note, unlike createAction(), these are not registered / owned by the ActionManager.
   */
  std::vector<Action> createTagActions(const std::vector<mdl::SmartTag>& tags) const;
  /**
   * Note, unlike createAction(), these are not registered / owned by the ActionManager.
   */
  std::vector<Action> createEntityDefinitionActions(
    const std::vector<mdl::EntityDefinition*>& entityDefinitions) const;

  template <typename MenuVisitor>
  void visitMainMenu(const MenuVisitor& visitor) const
  {
    for (const auto& menu : m_mainMenu)
    {
      visitor(visitor, menu);
    }
  }

  template <typename MenuVisitor>
  void visitToolBar(const MenuVisitor& visitor) const
  {
    m_toolBar.visitEntries(visitor);
  }

  /**
   * Visits actions not used in the menu or toolbar.
   */
  void visitMapViewActions(const ActionVisitor& visitor) const;

  const std::unordered_map<std::filesystem::path, Action, kdl::path_hash>& actionsMap()
    const;

  void resetAllKeySequences() const;

private:
  void initialize();
  void createViewActions();

  void createMenu();
  void createFileMenu();
  void createEditMenu();
  void createViewMenu();
  void createRunMenu();
  void createDebugMenu();
  void createHelpMenu();

  Menu& createMainMenu(std::string name);

  void createToolbar();
  const Action& existingAction(const std::filesystem::path& preferencePath) const;

  const Action& addAction(Action action);
};

std::vector<size_t> findConflicts(const std::vector<const Action*>& actions);

} // namespace tb::ui
