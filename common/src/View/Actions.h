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

#ifndef TRENCHBROOM_ACTIONS_H
#define TRENCHBROOM_ACTIONS_H

#include "Preference.h"
#include "StringUtils.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class KeyboardShortcut;
        class MapDocument;
        class MapFrame;

        class ActionExecutionContext {
        private:
            MapFrame* m_frame;
        public:
            explicit ActionExecutionContext(MapFrame* mapFrame);

            bool hasDocument() const;
            MapFrame* frame();
            MapDocument* document();
        };

        class Action {
        public:
            using ExecuteFn = std::function<void(ActionExecutionContext& context)>;
            using EnabledFn = std::function<bool(ActionExecutionContext& context)>;
            using CheckedFn = std::function<bool(ActionExecutionContext& context)>;
        private:
            String m_name;
            Preference<KeyboardShortcut> m_preference;
            ExecuteFn m_execute;
            EnabledFn m_enabled;
            std::optional<CheckedFn> m_checked;
            std::optional<IO::Path> m_iconPath;
        public:
            Action(String&& name, KeyboardShortcut&& defaultShortcut,
                ExecuteFn&& execute, EnabledFn&& enabled, IO::Path&& iconPath = IO::Path());
            Action(String&& name, KeyboardShortcut&& defaultShortcut,
                ExecuteFn&& execute, EnabledFn&& enabled, CheckedFn&& checked, IO::Path&& iconPath = IO::Path());

            const String& name() const;

            void execute(ActionExecutionContext& context) const;
            bool enabled(ActionExecutionContext& context) const;
            bool checkable() const;
            bool checked(ActionExecutionContext& context) const;

            bool hasIcon() const;
            const IO::Path& iconPath() const;

            deleteCopyAndMove(Action)
        };

        class Menu;
        class MenuSeparatorItem;
        class MenuActionItem;

        class MenuVisitor {
        public:
            virtual ~MenuVisitor();

            virtual void visit(const Menu& menu) = 0;
            virtual void visit(const MenuSeparatorItem& item) = 0;
            virtual void visit(const MenuActionItem& item) = 0;
        };

        class MenuEntry {
        public:
            MenuEntry();
            virtual ~MenuEntry();
            virtual void accept(MenuVisitor& visitor) const = 0;

            deleteCopyAndMove(MenuEntry)
        };

        class MenuSeparatorItem : public MenuEntry {
        public:
            MenuSeparatorItem();
            void accept(MenuVisitor& visitor) const override;

            deleteCopyAndMove(MenuSeparatorItem)
        };

        class MenuActionItem : public MenuEntry {
        private:
            const Action* m_action;
        public:
            MenuActionItem(const Action* action);

            const String& name() const;
            const Action& action() const;

            void accept(MenuVisitor& visitor) const override;

            deleteCopyAndMove(MenuActionItem)
        };

        class Menu : public MenuEntry {
        private:
            String m_name;
            std::vector<std::unique_ptr<MenuEntry>> m_entries;
        public:
            explicit Menu(String&& name);

            const String& name() const;

            Menu& addMenu(String name);
            void addSeparator();
            MenuActionItem& addItem(const Action* action);

            void accept(MenuVisitor& visitor) const override;
            void visitEntries(MenuVisitor& visitor) const;

            deleteCopyAndMove(Menu)
        };

        class ActionManager {
        private:
            /**
             * All actions which are used either in a menu, a tool bar or as a shortcut.
             */
            std::vector<std::unique_ptr<const Action>> m_actions;
            std::vector<std::unique_ptr<const Menu>> m_mainMenu;
            std::vector<const Action*> m_toolBar;
            std::vector<const Action*> m_shortcuts;
        private:
            ActionManager();
        public:
            static const ActionManager& instance();

            void visitMainMenu(MenuVisitor& visitor) const;
        private:
            void initialize();
            const Action* createAction(String&& name, QKeySequence&& defaultShortcut,
                                       Action::ExecuteFn&& execute, Action::EnabledFn&& enabled, IO::Path&& iconPath = IO::Path());
            const Action* createAction(String&& name, QKeySequence&& defaultShortcut,
                                       Action::ExecuteFn&& execute, Action::EnabledFn&& enabled, Action::CheckedFn&& checked, IO::Path&& iconPath = IO::Path());

            Menu& createMainMenu(String&& name);

            deleteCopyAndMove(ActionManager)
        };
    }
}

#endif //TRENCHBROOM_ACTIONS_H
