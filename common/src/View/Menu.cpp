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
#include "View/ActionContext.h"
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
        
        void MenuItem::appendToMenu(wxMenu* menu, const bool withShortcuts) const {
            doAppendToMenu(menu, withShortcuts);
        }
        
        void MenuItem::appendToMenu(wxMenuBar* menu, const bool withShortcuts) const {
            doAppendToMenu(menu, withShortcuts);
        }
        
        const ActionMenuItem* MenuItem::findActionMenuItem(const int id) const {
            return doFindActionMenuItem(id);
        }
        
        void MenuItem::getShortcutEntries(KeyboardShortcutEntry::List& entries) {
            doGetShortcutEntries(entries);
        }

        void MenuItem::resetShortcuts() {
            doResetShortcuts();
        }

        void MenuItem::doAppendToMenu(wxMenuBar* menu, const bool withShortcuts) const {}

        const ActionMenuItem* MenuItem::doFindActionMenuItem(const int id) const {
            return NULL;
        }

        void MenuItem::doGetShortcutEntries(KeyboardShortcutEntry::List& entries) {}

        void MenuItem::doResetShortcuts() {}

        SeparatorItem::SeparatorItem(MenuItemParent* parent) :
        MenuItem(Type_Separator, parent) {}
        
        void SeparatorItem::doAppendToMenu(wxMenu* menu, const bool withShortcuts) const {
            menu->AppendSeparator();
        }

        LabeledMenuItem::LabeledMenuItem(const Type type, MenuItemParent* parent) :
        MenuItem(type, parent) {}
        
        LabeledMenuItem::~LabeledMenuItem() {}
        
        int LabeledMenuItem::id() const {
            return doGetId();
        }
        
        const String& LabeledMenuItem::label() const {
            return doGetLabel();
        }

        ActionMenuItem::ActionMenuItem(const Type type, MenuItemParent* parent, const int id, const String& label, const KeyboardShortcut& defaultShortcut, const bool modifiable) :
        LabeledMenuItem(type, parent),
        m_action(id, label, modifiable),
        m_preference(IO::Path("Menu") + path(label), defaultShortcut) {
            assert(type == Type_Action || type == Type_Check);
        }
        
        ActionMenuItem::~ActionMenuItem() {}

        wxString ActionMenuItem::menuString(const wxString& suffix, const bool withShortcuts) const {
            wxString caption;
            caption << label();
            if (!suffix.empty())
                caption << " " << suffix;
            if (!m_action.modifiable() || withShortcuts)
                return shortcut().shortcutMenuItemString(caption);
            else
                return caption;
        }

        IO::Path ActionMenuItem::path(const String& label) const {
            IO::Path path(label);
            
            const MenuItemParent* p = parent();
            while (p != NULL) {
                if (!p->label().empty())
                    path = IO::Path(p->label()) + path;
                p = p->parent();
            }

            return path;
        }

        void ActionMenuItem::doAppendToMenu(wxMenu* menu, const bool withShortcuts) const {
            if (type() == Type_Action)
                menu->Append(id(), menuString("", withShortcuts));
            else
                menu->AppendCheckItem(id(), menuString("", withShortcuts));
        }
        
        const ActionMenuItem* ActionMenuItem::doFindActionMenuItem(int id) const {
            if (id == m_action.id())
                return this;
            return NULL;
        }
        
        void ActionMenuItem::doGetShortcutEntries(KeyboardShortcutEntry::List& entries) {
            entries.push_back(this);
        }

        void ActionMenuItem::doResetShortcuts() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(m_preference);
        }
        
        int ActionMenuItem::doGetId() const {
            return m_action.id();
        }
        
        const String& ActionMenuItem::doGetLabel() const {
            return m_action.name();
        }

        int ActionMenuItem::doGetActionContext() const {
            return ActionContext_Any;
        }

        bool ActionMenuItem::doGetModifiable() const {
            return m_action.modifiable();
        }

        wxString ActionMenuItem::doGetActionDescription() const {
            return m_preference.path().asString(" > ");
        }
        
        wxString ActionMenuItem::doGetJsonString() const {
            const IO::Path menuPath = path(label());
            
            wxString str;
            str << "{ path: [\"" << menuPath.asString("\", \"") << "\"], shortcut: " << shortcut().asJsonString() << " }";
            return str;
        }

        const Preference<KeyboardShortcut>& ActionMenuItem::doGetPreference() const {
            return m_preference;
        }

        const KeyboardShortcut& ActionMenuItem::doGetShortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.get(m_preference);
        }
        
        void ActionMenuItem::doUpdateShortcut(const KeyboardShortcut& shortcut) {
            assert(m_action.modifiable());
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(m_preference, shortcut);
        }

        wxAcceleratorEntry ActionMenuItem::doGetAcceleratorEntry(const ActionView view) const {
            return shortcut().acceleratorEntry(id());
        }

        MenuItemParent::MenuItemParent(const Type type, MenuItemParent* parent, const int id, const String& label) :
        LabeledMenuItem(type, parent),
        m_id(id),
        m_label(label) {}
        
        MenuItemParent::~MenuItemParent() {
            VectorUtils::clearAndDelete(m_items);
        }
        
        void MenuItemParent::addItem(MenuItem* item) {
            m_items.push_back(item);
        }
        
        const MenuItemParent::List& MenuItemParent::items() const {
            return m_items;
        }
        
        MenuItemParent::List& MenuItemParent::items() {
            return m_items;
        }
        
        void MenuItemParent::doAppendToMenu(wxMenu* menu, const bool withShortcuts) const {
            wxMenu* subMenu = buildMenu(withShortcuts);

            wxMenuItem* subMenuItem = new wxMenuItem(subMenu, id(), label());
            subMenuItem->SetSubMenu(subMenu);
            menu->Append(subMenuItem);
        }
        
        void MenuItemParent::doAppendToMenu(wxMenuBar* menu, const bool withShortcuts) const {
            wxMenu* subMenu = buildMenu(withShortcuts);
            menu->Append(subMenu, label());
        }

        wxMenu* MenuItemParent::buildMenu(const bool withShortcuts) const {
            wxMenu* subMenu = new wxMenu();
            
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const MenuItem* item = *it;
                item->appendToMenu(subMenu, withShortcuts);
            }
            
            return subMenu;
        }

        const ActionMenuItem* MenuItemParent::doFindActionMenuItem(int id) const {
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const MenuItem* item = *it;
                const ActionMenuItem* foundItem = item->findActionMenuItem(id);
                if (foundItem != NULL)
                    return foundItem;
            }
            return NULL;
        }
        
        void MenuItemParent::doGetShortcutEntries(KeyboardShortcutEntry::List& entries) {
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                MenuItem* item = *it;
                item->getShortcutEntries(entries);
            }
        }

        void MenuItemParent::doResetShortcuts() {
            MenuItem::List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                MenuItem* item = *it;
                item->resetShortcuts();
            }
        }
        
        int MenuItemParent::doGetId() const {
            return m_id;
        }

        const String& MenuItemParent::doGetLabel() const {
            return m_label;
        }
        
        Menu::Menu(MenuItemParent* parent, const int id, const String& label) :
        MenuItemParent(Type_Menu, parent, id, label) {}
        
        Menu::Menu(const String& label) :
        MenuItemParent(Type_Menu, NULL, wxID_ANY, label) {}

        Menu::~Menu() {}

        MenuItem* Menu::addModifiableActionItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut) {
            return addActionItem(id, label, defaultShortcut, true);
        }
        
        MenuItem* Menu::addUnmodifiableActionItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut) {
            return addActionItem(id, label, defaultShortcut, false);
        }
        
        MenuItem* Menu::addModifiableCheckItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut) {
            return addCheckItem(id, label, defaultShortcut, true);
        }
        
        MenuItem* Menu::addUnmodifiableCheckItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut) {
            return addCheckItem(id, label, defaultShortcut, false);
        }

        void Menu::addSeparator() {
            MenuItem* item(new SeparatorItem(this));
            addItem(item);
        }
        
        Menu* Menu::addMenu(const String& label) {
            return addMenu(wxID_ANY, label);
        }
        
        Menu* Menu::addMenu(int id, const String& label) {
            Menu* menu = new Menu(this, id, label);
            addItem(menu);
            return menu;
        }
        
        MenuItem* Menu::addActionItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut, const bool modifiable) {
            MenuItem* item = new ActionMenuItem(MenuItem::Type_Action, this, id, label, defaultShortcut, modifiable);
            addItem(item);
            return item;
        }
        
        MenuItem* Menu::addCheckItem(const int id, const String& label, const KeyboardShortcut& defaultShortcut, const bool modifiable) {
            MenuItem* item = new ActionMenuItem(MenuItem::Type_Check, this, id, label, defaultShortcut, modifiable);
            addItem(item);
            return item;
        }

        MenuBar::MenuBar() {}
        MenuBar::~MenuBar() {
            VectorUtils::clearAndDelete(m_menus);
        }
        
        const ActionMenuItem* MenuBar::findActionMenuItem(int id) const {
            MenuList::const_iterator it, end;
            for (it = m_menus.begin(), end = m_menus.end(); it != end; ++it) {
                const Menu* menu = *it;
                const ActionMenuItem* item = menu->findActionMenuItem(id);
                if (item != NULL)
                    return item;
            }
            return NULL;
        }

        void MenuBar::resetShortcuts() {
            MenuList::const_iterator it, end;
            for (it = m_menus.begin(), end = m_menus.end(); it != end; ++it) {
                Menu* menu = *it;
                menu->resetShortcuts();
            }
        }

        Menu* MenuBar::addMenu(const String& label) {
            Menu* menu = new Menu(label);
            m_menus.push_back(menu);
            return menu;
        }
        
        wxMenuBar* MenuBar::createMenuBar(const bool withShortcuts) {
            wxMenuBar* menuBar = new wxMenuBar();
            MenuList::const_iterator it, end;
            for (it = m_menus.begin(), end = m_menus.end(); it != end; ++it) {
                const Menu* menu = *it;
                menu->appendToMenu(menuBar, withShortcuts);
            }
            return menuBar;
        }

        void MenuBar::getShortcutEntries(KeyboardShortcutEntry::List& entries) const {
            MenuList::const_iterator it, end;
            for (it = m_menus.begin(), end = m_menus.end(); it != end; ++it) {
                Menu* menu = *it;
                menu->getShortcutEntries(entries);
            }
        }
    }
}
