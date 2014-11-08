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

#include "Menu.h"

#include "PreferenceManager.h"
#include "View/CommandIds.h"

#include <wx/menu.h>
#include <wx/menuitem.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        MenuItem::MenuItem(const Type type, MenuItemParent* parent) :
        m_type(type),
        m_parent(parent) {}
        
        MenuItem::~MenuItem() {}
        
        MenuItem::Type MenuItem::type() const {
            return m_type;
        }
        
        const MenuItemParent* MenuItem::parent() const {
            return m_parent;
        }

        const MenuAction* MenuItem::findAction(const int id) const {
            return NULL;
        }

        void MenuItem::resetShortcutsToDefaults() {
            doResetShortcutsToDefaults();
        }

        void MenuItem::doResetShortcutsToDefaults() {}

        MenuItemWithCaption::MenuItemWithCaption(const Type type, MenuItemParent* parent) :
        MenuItem(type, parent) {}
        
        MenuItemWithCaption::~MenuItemWithCaption() {}
        
        ActionMenuItem::ActionMenuItem(const Type type, MenuItemParent* parent, const int id, const int context, const String& text, const KeyboardShortcut& defaultShortcut, const bool modifiable) :
        MenuItemWithCaption(type, parent),
        m_action(id, context, text, path(text), defaultShortcut, modifiable) {
            assert(type == Type_Action || type == Type_Check);
        }
        
        ActionMenuItem::~ActionMenuItem() {}

        int ActionMenuItem::id() const {
            return m_action.id();
        }

        const String& ActionMenuItem::text() const {
            return m_action.name();
        }

        wxString ActionMenuItem::menuText() const {
            return m_action.menuItemString();
        }
        
        MenuAction& ActionMenuItem::action() {
            return m_action;
        }

        const MenuAction* ActionMenuItem::findAction(const int i_id) const {
            if (i_id == id())
                return &m_action;
            return NULL;
        }
        
        void ActionMenuItem::doResetShortcutsToDefaults() {
            m_action.resetShortcut();
        }

        IO::Path ActionMenuItem::path(const String& text) const {
            IO::Path path(text);
            
            const MenuItemParent* p = parent();
            while (p != NULL) {
                if (!p->text().empty())
                    path = IO::Path(p->text()) + path;
                p = p->parent();
            }

            return path;
        }

        int MenuItemParent::id() const {
            return m_id;
        }

        const String& MenuItemParent::text() const {
            return m_text;
        }
        
        const MenuAction* MenuItemParent::findAction(const int id) const {
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const MenuItem::Ptr item = *it;
                const MenuAction* action = item->findAction(id);
                if (action != NULL)
                    return action;
            }
            return NULL;
        }

        const MenuItemParent::List& MenuItemParent::items() const {
            return m_items;
        }
        
        MenuItemParent::List& MenuItemParent::items() {
            return m_items;
        }
        
        void MenuItemParent::addItem(MenuItem::Ptr item) {
            m_items.push_back(item);
        }
        
        MenuItemParent::MenuItemParent(const Type type, MenuItemParent* parent, const int id, const String& text) :
        MenuItemWithCaption(type, parent),
        m_id(id),
        m_text(text) {}

        MenuItemParent::~MenuItemParent() {}

        void MenuItemParent::doResetShortcutsToDefaults() {
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const MenuItem::Ptr item = *it;
                item->resetShortcutsToDefaults();
            }
        }

        Menu::Menu(MenuItemParent* parent, const int id, const String& text) :
        MenuItemParent(Type_Menu, parent, id, text) {}
        
        Menu::Menu(const String& text) :
        MenuItemParent(Type_Menu, NULL, wxID_ANY, text) {}

        Menu::~Menu() {}

        MenuItem::Ptr Menu::addModifiableActionItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut) {
            return addActionItem(id, context, text, defaultShortcut, true);
        }
        
        MenuItem::Ptr Menu::addUnmodifiableActionItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut) {
            return addActionItem(id, context, text, defaultShortcut, false);
        }
        
        MenuItem::Ptr Menu::addModifiableCheckItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut) {
            return addCheckItem(id, context, text, defaultShortcut, true);
        }
        
        MenuItem::Ptr Menu::addUnmodifiableCheckItem(int id, int context, const String& text, const KeyboardShortcut& defaultShortcut) {
            return addCheckItem(id, context, text, defaultShortcut, false);
        }

        void Menu::addSeparator() {
            MenuItem::Ptr item(new MenuItem(MenuItem::Type_Separator, this));
            addItem(item);
        }
        
        Menu& Menu::addMenu(const int id, const String& text) {
            Menu* menu = new Menu(this, id, text);
            addItem(Menu::Ptr(menu));
            return *menu;
        }
        
        MenuItem::Ptr Menu::addActionItem(const int id, const int context, const String& text, const KeyboardShortcut& defaultShortcut, const bool modifiable) {
            MenuItem::Ptr item = MenuItem::Ptr(new ActionMenuItem(MenuItem::Type_Action, this, id, context, text, defaultShortcut, modifiable));
            addItem(item);
            return item;
        }
        
        MenuItem::Ptr Menu::addCheckItem(const int id, const int context, const String& text, const KeyboardShortcut& defaultShortcut, const bool modifiable) {
            MenuItem::Ptr item = MenuItem::Ptr(new ActionMenuItem(MenuItem::Type_Check, this, id, context, text, defaultShortcut, modifiable));
            addItem(item);
            return item;
        }
    }
}
