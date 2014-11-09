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

#ifndef __TrenchBroom__ActionManager__
#define __TrenchBroom__ActionManager__

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
            MenuBar* m_menuBar;
            ViewShortcut::List m_viewShortcuts;
        public:
            static ActionManager& instance();
            ~ActionManager();

            
            static wxMenu* findRecentDocumentsMenu(const wxMenuBar* menuBar);
            const ActionMenuItem* findMenuItem(int id) const;
            
            void getShortcutEntries(ShortcutEntryList& entries);

            wxMenuBar* createMenuBar() const;
            bool isMenuShortcutPreference(const IO::Path& path) const;

            wxAcceleratorTable createViewAcceleratorTable(ActionContext context, ActionView view) const;
            
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

#endif /* defined(__TrenchBroom__ActionManager__) */
