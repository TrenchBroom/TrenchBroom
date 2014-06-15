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
#include "View/Action.h"

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
                Type_Menu,
                Type_MultiMenu
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
            
            virtual const Action* findAction(int id) const;
            // virtual const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
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
            mutable Action m_action;
        public:
            ActionMenuItem(Type type, MenuItemParent* parent, int id, int context, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);
            virtual ~ActionMenuItem();
            
            int id() const;
            const String& text() const;
            wxString menuText() const;

            Action& action();

            const Action* findAction(int id) const;
        private:
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

            const Action* findAction(int id) const;

            const List& items() const;
            List& items();

            void addItem(MenuItem::Ptr item);
        protected:
            MenuItemParent(Type type, MenuItemParent* parent, int id, const String& text);
            virtual ~MenuItemParent();
        };
        
        class MultiMenu;
        
        class MultiMenuSelector {
        public:
            virtual ~MultiMenuSelector();
            virtual const Menu* select(const MultiMenu& multiMenu) const = 0;
        };
        class Menu;
        
        class NullMenuSelector : public MultiMenuSelector {
        public:
            const Menu* select(const MultiMenu& multiMenu) const;
        };
        
        class MultiMenu : public MenuItemParent {
        public:
            MultiMenu(MenuItemParent* parent, int id, const String& text);
            Menu& addMenu(int id, const String& text);
            const Menu* menuById(int id) const;
            const Menu* selectMenu(const MultiMenuSelector& selector) const;
        };

        
        class Menu : public MenuItemParent {
        public:
            Menu(MenuItemParent* parent, int id, const String& text);
            Menu(const String& text);
            virtual ~Menu();

            MenuItem::Ptr addModifiableActionItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem::Ptr addUnmodifiableActionItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            
            MenuItem::Ptr addModifiableCheckItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem::Ptr addUnmodifiableCheckItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);

            void addSeparator();
            Menu& addMenu(int id, const String& text);
            MultiMenu& addMultiMenu(int id, const String& text);
            
            static wxMenuBar* createMenuBar(const MultiMenuSelector& selector);
            static wxMenu* findRecentDocumentsMenu(const wxMenuBar* menuBar);

            static const Action* findMenuAction(int id);

            static Menu& getMenu();
        private:
            MenuItem::Ptr addActionItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);
            MenuItem::Ptr addCheckItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut, bool modifiable);

            static wxMenu* createMenu(const Menu& menu, const MultiMenuSelector& selector);
            static Menu* buildMenu();
            
        };
    }
}

#endif /* defined(__TrenchBroom__Menu__) */
