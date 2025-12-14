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
#include "mdl/EntityDefinition.h"
#include "mdl/Tag.h"
#include "ui/Action.h"
#include "ui/ActionMenu.h"

#include "kd/path_hash.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::ui
{

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

  static ActionManager& instance();

  /**
   * Note, unlike createAction(), these are not registered / owned by the ActionManager.
   */
  std::vector<Action> createTagActions(const std::vector<mdl::SmartTag>& tags) const;
  /**
   * Note, unlike createAction(), these are not registered / owned by the ActionManager.
   */
  std::vector<Action> createEntityDefinitionActions(
    const std::vector<mdl::EntityDefinition>& entityDefinitions) const;

  template <typename MenuVisitor>
  void visitMainMenu(MenuVisitor&& visitor) const
  {
    for (const auto& menu : m_mainMenu)
    {
      visitor(visitor, menu);
    }
  }

  template <typename MenuVisitor>
  void visitMainMenu(MenuVisitor&& visitor)
  {
    for (auto& menu : m_mainMenu)
    {
      visitor(visitor, menu);
    }
  }

  template <typename MenuVisitor>
  void visitToolBar(MenuVisitor&& visitor) const
  {
    m_toolBar.visitEntries(visitor);
  }

  template <typename MenuVisitor>
  void visitToolBar(MenuVisitor&& visitor)
  {
    m_toolBar.visitEntries(visitor);
  }

  /**
   * Visits actions not used in the menu or toolbar.
   */
  template <typename ActionVisitor>
  void visitMapViewActions(ActionVisitor&& visitor) const
  {
    for (const auto& [path, action] : m_actions)
    {
      unused(path);
      if (!action.isMenuAction())
      {
        visitor(action);
      }
    }
  }

  template <typename ActionVisitor>
  void visitMapViewActions(ActionVisitor&& visitor)
  {
    for (auto& [path, action] : m_actions)
    {
      unused(path);
      if (!action.isMenuAction())
      {
        visitor(action);
      }
    }
  }

  const std::unordered_map<std::filesystem::path, Action, kdl::path_hash>& actionsMap()
    const;

  void resetAllKeySequences() const;

private:
  void initialize();
  void createViewActions();

  void createMenu();
  void createFileMenu();
  void createEditMenu();
  void createSelectionMenu();
  void createGroupsMenu();
  void createToolsMenu();
  void createViewMenu();
  void createRunMenu();
  void createDebugMenu();
  void createHelpMenu();

  Menu& createMainMenu(std::string name);

  void createToolbar();
  Action& existingAction(const std::filesystem::path& preferencePath);

  Action& addAction(Action action);
};

} // namespace tb::ui
