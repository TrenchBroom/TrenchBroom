/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_Menu
#define TrenchBroom_Menu

#include "Preference.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "View/Action.h"
#include "View/KeyboardShortcutEntry.h"

#include <map>
#include <memory>
#include <vector>

class wxMenu;
class wxMenuBar;

namespace TrenchBroom {
    namespace View {
        class ActionMenuItem;
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

            using List = std::vector<MenuItem*>;
        private:
            Type m_type;
            MenuItemParent* m_parent;
        public:
            MenuItem(const Type type, MenuItemParent* parent);
            virtual ~MenuItem();

            Type type() const;
            const MenuItemParent* parent() const;

            void appendToMenu(wxMenu* menu, bool withShortcuts) const;
            void appendToMenu(wxMenuBar* menu, bool withShortcuts) const;
            const ActionMenuItem* findActionMenuItem(int id) const;
            void getShortcutEntries(KeyboardShortcutEntry::List& entries);

            void resetShortcuts();
        private:
            virtual void doAppendToMenu(wxMenu* menu, bool withShortcuts) const = 0;
            virtual void doAppendToMenu(wxMenuBar* menu, bool withShortcuts) const;
            virtual const ActionMenuItem* doFindActionMenuItem(int id) const;
            virtual void doGetShortcutEntries(KeyboardShortcutEntry::List& entries);
            virtual void doResetShortcuts();
        };

        class SeparatorItem : public MenuItem {
        public:
            SeparatorItem(MenuItemParent* parent);
        private:
            void doAppendToMenu(wxMenu* menu, bool withShortcuts) const override;
        };

        class LabeledMenuItem : public MenuItem {
        public:
            LabeledMenuItem(Type type, MenuItemParent* parent);
            virtual ~LabeledMenuItem();
        public:
            int id() const;
            const String& label() const;
        private:
            virtual int doGetId() const = 0;
            virtual const String& doGetLabel() const = 0;
        };

        class ActionMenuItem : public LabeledMenuItem {
        public:
            class ActionKeyboardShortcutEntry : public KeyboardShortcutEntry {
            private:
                ActionMenuItem& m_menuItem;
            public:
                explicit ActionKeyboardShortcutEntry(ActionMenuItem& menuItem);
            private: // implement KeyboardShortcutEntry interface
                int doGetActionContext() const override;
                bool doGetModifiable() const override;
                wxString doGetActionDescription() const override;
                wxString doGetJsonString() const override;
                const Preference<KeyboardShortcut>& doGetPreference() const override;
                Preference<KeyboardShortcut>& doGetPreference() override;
                wxAcceleratorEntry doGetAcceleratorEntry(ActionView view) const override;
            };
        private:
            mutable Action m_action;
            mutable Preference<KeyboardShortcut> m_preference;
        public:
            ActionMenuItem(Type type, MenuItemParent* parent, int id, const String& label, const KeyboardShortcut& defaultShortcut, bool modifiable);
            ~ActionMenuItem() override;

            wxString menuString(const wxString& suffix, bool withShortcuts) const;
        private:
            IO::Path path(const String& text) const;
        private: // implement LabeledMenuItem interface
            void doAppendToMenu(wxMenu* menu, bool withShortcuts) const override;
            const ActionMenuItem* doFindActionMenuItem(int id) const override;
            void doGetShortcutEntries(KeyboardShortcutEntry::List& entries) override;
            void doResetShortcuts() override;

            int doGetId() const override;
            const String& doGetLabel() const override;
        };

        class Menu;

        class MenuItemParent : public LabeledMenuItem {
        private:
            int m_id;
            String m_label;
            List m_items;
        protected:
            MenuItemParent(Type type, MenuItemParent* parent, int id, const String& label);
        public:
            ~MenuItemParent() override;

            void addItem(MenuItem* item);
            const List& items() const;
            List& items();
        private:
            void doAppendToMenu(wxMenu* menu, bool withShortcuts) const override;
            void doAppendToMenu(wxMenuBar* menu, bool withShortcuts) const override;
            wxMenu* buildMenu(bool withShortcuts) const;

            const ActionMenuItem* doFindActionMenuItem(int id) const override;
            void doGetShortcutEntries(KeyboardShortcutEntry::List& entries) override;
            void doResetShortcuts() override;

            int doGetId() const override;
            const String& doGetLabel() const override;
        };

        class Menu : public MenuItemParent {
        public:
            Menu(MenuItemParent* parent, int id, const String& label);
            Menu(const String& label);
            virtual ~Menu();

            MenuItem* addModifiableActionItem(int id, const String& label, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem* addUnmodifiableActionItem(int id, const String& label, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);

            MenuItem* addModifiableCheckItem(int id, const String& label, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);
            MenuItem* addUnmodifiableCheckItem(int id, const String& label, const KeyboardShortcut& defaultShortcut = KeyboardShortcut::Empty);

            void addSeparator();
            Menu* addMenu(const String& label);
            Menu* addMenu(int id, const String& label);
        private:
            MenuItem* addActionItem(int id, const String& label, const KeyboardShortcut& defaultShortcut, bool modifiable);
            MenuItem* addCheckItem(int id, const String& label, const KeyboardShortcut& defaultShortcut, bool modifiable);
        };

        class MenuBar {
        private:
            using MenuList = std::vector<Menu*>;
            MenuList m_menus;
        public:
            MenuBar();
            ~MenuBar();

            const ActionMenuItem* findActionMenuItem(int id) const;
            void resetShortcuts();

            Menu* addMenu(const String& label);
            wxMenuBar* createMenuBar(bool withShortcuts);

            void getShortcutEntries(KeyboardShortcutEntry::List& entries) const;
        };
    }
}

#endif /* defined(TrenchBroom_Menu) */
