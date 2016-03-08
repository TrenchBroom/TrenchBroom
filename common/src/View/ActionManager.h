/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_ActionManager
#define TrenchBroom_ActionManager

#include "View/ActionContext.h"
#include "View/ViewShortcut.h"

class wxAcceleratorTable;
class wxMenu;
class wxMenuBar;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class MenuBar;
        class ActionMenuItem;
        class KeyboardShortcutEntry;
        
        class ActionManager {
        public:
            typedef std::vector<KeyboardShortcutEntry*> ShortcutEntryList;
        private:
            typedef std::vector<wxAcceleratorEntry> AcceleratorEntryList;

            MenuBar* m_menuBar;
            ViewShortcut::List m_viewShortcuts;
        public:
            static ActionManager& instance();
            ~ActionManager();

            
            static wxMenu* findRecentDocumentsMenu(const wxMenuBar* menuBar);
            const ActionMenuItem* findMenuItem(int id) const;
            
            void getShortcutEntries(ShortcutEntryList& entries);
            String getJSTable();
        private:
            void getKeysJSTable(StringStream& str);
            void getMenuJSTable(StringStream& str);
            void getActionJSTable(StringStream& str);
        public:
            wxMenuBar* createMenuBar(bool withShortcuts) const;
            bool isMenuShortcutPreference(const IO::Path& path) const;

            wxAcceleratorTable createViewAcceleratorTable(ActionContext context, ActionView view) const;
        private:
            void addViewActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const;
            void addMenuActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const;
        public:
            void resetShortcutsToDefaults();
        private:
            ActionManager();
            ActionManager(const ActionManager&);
            ActionManager& operator=(const ActionManager&);
            
            void createMenuBar();
            void createViewShortcuts();
            void createViewShortcut(const KeyboardShortcut& shortcut, int context, const Action& action2D, const Action& action3D);
            void createViewShortcut(const KeyboardShortcut& shortcut, int context, const Action& action);
        };
    }
}

#endif /* defined(TrenchBroom_ActionManager) */
