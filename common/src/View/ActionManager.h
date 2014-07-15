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

#include "View/Action.h"

class wxAcceleratorTable;
class wxMenu;
class wxMenuBar;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class Menu;
        
        class ActionManager {
        private:
            Action::List m_mapViewActions;
            Menu* m_menu;
        public:
            static ActionManager& instance();
            ~ActionManager();
            
            static wxMenu* findRecentDocumentsMenu(const wxMenuBar* menuBar);
            const Action* findMenuAction(int id) const;

            Menu& getMenu();
            wxMenuBar* createMenuBar() const;
            bool isMenuShortcutPreference(const IO::Path& path) const;
            
            Action::List& mapViewActions();
            wxAcceleratorTable createMapViewAcceleratorTable(Action::Context context) const;
        private:
            wxMenu* createMenu(const Menu& menu) const;
        private:
            ActionManager();
            
            void createMapViewActions();
            void createMapViewAction(int id, int context, const String& name, const KeyboardShortcut& defaultShortcut);
            void createMenu();
        };
    }
}

#endif /* defined(__TrenchBroom__ActionManager__) */
