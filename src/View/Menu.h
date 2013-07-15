/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Menu__
#define __TrenchBroom__Menu__

#include "Preference.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

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
                MITSeparator,
                MITAction,
                MITCheck,
                MITMenu,
                MITMultiMenu
            } MenuItemType;
            
            typedef std::tr1::shared_ptr<MenuItem> Ptr;
            typedef std::vector<Ptr> List;
        private:
            MenuItemType m_type;
            MenuItemParent* m_parent;
        public:
            MenuItem(const MenuItemType type, MenuItemParent* parent);
            virtual ~MenuItem();
            
            MenuItemType type() const;
            const MenuItemParent* parent() const;
            virtual const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
        };

        class TextMenuItem : public MenuItem {
        public:
            TextMenuItem(const MenuItemType type, MenuItemParent* parent);
            virtual ~TextMenuItem();
            virtual const String& text() const = 0;
        };
        
        class ShortcutMenuItem : public TextMenuItem {
        private:
            mutable KeyboardShortcut m_shortcut;
            mutable Preference<KeyboardShortcut> m_preference;
            
            String path() const;
        public:
            ShortcutMenuItem(MenuItemType type, const KeyboardShortcut& shortcut, MenuItemParent* parent);
            virtual ~ShortcutMenuItem();
            
            inline const String& text() const;
            const String longText() const;
            const KeyboardShortcut& shortcut() const;
            void setShortcut(const KeyboardShortcut& shortcut) const;
            const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
        };
        
        class Menu;
        
        class MenuItemParent : public TextMenuItem {
        private:
            String m_text;
            int m_menuId;
            List m_items;
        public:
            const List& items() const;
            void addItem(MenuItem::Ptr item);
            const String& text() const;
            int menuId() const;
            const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
        protected:
            MenuItemParent(MenuItemType type, const String& text, MenuItemParent* parent, int menuId);
            virtual ~MenuItemParent();
        };
        
        class MultiMenu;
        
        class MultiMenuSelector {
        public:
            virtual ~MultiMenuSelector();
            virtual const Menu* select(const MultiMenu& multiMenu) const = 0;
        };
        
        class NullMenuSelector : public MultiMenuSelector {
        public:
            const Menu* select(const MultiMenu& multiMenu) const;
        };

        class MultiMenu : public MenuItemParent {
        public:
            MultiMenu(const String& text, MenuItemParent* parent, const int menuId);
            Menu& addMenu(const String& text, const int menuId);
            const Menu* menuById(const int menuId) const;
            const Menu* selectMenu(const MultiMenuSelector& selector) const;
        };
        
        extern const String FileMenu;
        extern const String EditMenu;
        extern const String ViewMenu;

        class Menu : public MenuItemParent {
        public:
            typedef std::map<String, Ptr> MenuMap;
        public:
            Menu(const String& text, MenuItemParent* parent = NULL, int menuId = wxID_ANY);
            virtual ~Menu();
            
            MenuItem::Ptr addActionItem(const KeyboardShortcut& shortcut);
            MenuItem::Ptr addCheckItem(const KeyboardShortcut& shortcut);
            void addSeparator();
            Menu& addMenu(const String& text, int menuId = wxID_ANY);
            MultiMenu& addMultiMenu(const String& text, int menuId);
            
            static wxMenuBar* createMenuBar(const MultiMenuSelector& selector, const bool showModifiers);
            static wxMenu* findRecentDocumentsMenu(const wxMenuBar* menuBar);
            static wxMenu* createMenu(const String& name, const MultiMenuSelector& selector, const bool showModifiers);
        private:
            static wxMenu* createMenu(const Menu& menu, const MultiMenuSelector& selector, const bool showModifiers);
            static const Menu& getMenu(const String& name);
            static const MenuMap buildMenus();
        };
    }
}

#endif /* defined(__TrenchBroom__Menu__) */
