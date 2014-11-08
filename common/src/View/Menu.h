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

#ifndef __TrenchBroom__Menu__
#define __TrenchBroom__Menu__

#include "Preference.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "View/MenuAction.h"

#include <vector>
#include <map>

class wxMenu;
class wxMenuBar;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        class MenuItemParent;
        
        class MenuItem {
        public:
            typedef enum {
                Type_Separator,
                Type_Action,
                Type_Check,
                Type_Menu
            } Type;
            
            typedef std::tr1::shared_ptr<MenuItem> Ptr;
            typedef std::vector<Ptr> List;
        private:
            Type m_type;
            MenuItemParent* m_parent;
        public:
            MenuItem(const Type type, MenuItemParent* parent);
            virtual ~MenuItem();
            
            Type type() const;
            const MenuItemParent* parent() const;
            
            virtual const MenuAction* findAction(int id) const;

            void resetShortcutsToDefaults();
        private:
            virtual void doResetShortcutsToDefaults();
        };

        class MenuItemWithCaption : public MenuItem {
        public:
            MenuItemWithCaption(Type type, MenuItemParent* parent);
            virtual ~MenuItemWithCaption();

            virtual int id() const = 0;
            virtual const String& text() const = 0;
        };
        
        class ActionMenuItem : public MenuItemWithCaption {
        private:
            mutable MenuAction m_action;
        public:
            ActionMenuItem(Type type, MenuItemParent* parent, int id, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);
            virtual ~ActionMenuItem();
            
            int id() const;
            const String& text() const;
            wxString menuText() const;

            MenuAction& action();

            const MenuAction* findAction(int id) const;
        private:
            void doResetShortcutsToDefaults();
            IO::Path path(const String& text) const;
        };
        
        class Menu;
        
        class MenuItemParent : public MenuItemWithCaption {
        private:
            int m_id;
            String m_text;
            List m_items;
        public:
            int id() const;
            const String& text() const;

            const MenuAction* findAction(int id) const;

            const List& items() const;
            List& items();

            void addItem(MenuItem::Ptr item);
        protected:
            MenuItemParent(Type type, MenuItemParent* parent, int id, const String& text);
            virtual ~MenuItemParent();
        private:
            void doResetShortcutsToDefaults();
        };
        
        class Menu : public MenuItemParent {
        public:
            Menu(MenuItemParent* parent, int id, const String& text);
            Menu(const String& text);
            virtual ~Menu();

            MenuItem::Ptr addModifiableActionItem(int id, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem::Ptr addUnmodifiableActionItem(int id, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            
            MenuItem::Ptr addModifiableCheckItem(int id, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem::Ptr addUnmodifiableCheckItem(int id, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);

            void addSeparator();
            Menu& addMenu(int id, const String& text);
        private:
            MenuItem::Ptr addActionItem(int id, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);
            MenuItem::Ptr addCheckItem(int id, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);
        };
    }
}

#endif /* defined(__TrenchBroom__Menu__) */
