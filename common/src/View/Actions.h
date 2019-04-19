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
#include "View/ActionContext.h"

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
        class MapViewBase;

        class ActionExecutionContext {
        private:
            MapFrame* m_frame;
            MapViewBase* m_mapView;
        public:
            ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView);

            bool hasDocument() const;
            bool hasActionContext(ActionContext actionContext) const;
            MapFrame* frame();
            MapViewBase* view();
            MapDocument* document();
        };

        class Action {
        public:
            using ExecuteFn = std::function<void(ActionExecutionContext& context)>;
            using EnabledFn = std::function<bool(ActionExecutionContext& context)>;
            using CheckedFn = std::function<bool(ActionExecutionContext& context)>;
        private:
            String m_name;
            ActionContext m_actionContext;
            Preference<KeyboardShortcut> m_preference;
            ExecuteFn m_execute;
            EnabledFn m_enabled;
            std::optional<CheckedFn> m_checked;
            std::optional<IO::Path> m_iconPath;
        public:
            Action(const String& name, ActionContext actionContext, const KeyboardShortcut& defaultShortcut,
                const ExecuteFn& execute, const EnabledFn& enabled, const IO::Path& iconPath);
            Action(const String& name, ActionContext actionContext, const KeyboardShortcut& defaultShortcut,
                const ExecuteFn& execute, const EnabledFn& enabled, const CheckedFn& checked, const IO::Path& iconPath);

            const String& name() const;

            QKeySequence keySequence() const;

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

        enum class MenuEntryType {
            Menu_RecentDocuments,
            Menu_Undo,
            Menu_Redo,
            Menu_Cut,
            Menu_Copy,
            Menu_Paste,
            Menu_PasteAtOriginalPosition,
            Menu_None
        };

        class MenuEntry {
        private:
            MenuEntryType m_entryType;
        public:
            explicit MenuEntry(MenuEntryType entryType);
            virtual ~MenuEntry();
            virtual void accept(MenuVisitor& visitor) const = 0;

            MenuEntryType entryType() const;

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
            MenuActionItem(const Action* action, MenuEntryType entryType);

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
            Menu(const String& name, MenuEntryType entryType);

            const String& name() const;

            Menu& addMenu(const String& name, MenuEntryType entryType = MenuEntryType::Menu_None);
            void addSeparator();
            MenuActionItem& addItem(const Action* action, MenuEntryType entryType = MenuEntryType::Menu_None);

            void accept(MenuVisitor& visitor) const override;
            void visitEntries(MenuVisitor& visitor) const;

            deleteCopyAndMove(Menu)
        };

        using ActionVisitor = std::function<void(const Action&)>;

        class ActionManager {
        private:
            /**
             * All actions which are used either in a menu, a tool bar or as a shortcut.
             */
            std::vector<std::unique_ptr<const Action>> m_actions;

            /**
             * The main menu for the map editing window.
             */
            std::vector<std::unique_ptr<const Menu>> m_mainMenu;

            /**
             * The toolbar for the map editing window. Stored as a menu to allow for separators.
             */
            std::unique_ptr<Menu> m_toolBar;

            /**
             * Actions which can be triggered by keyboard shortcuts in the map editing views.
             */
            std::vector<const Action*> m_mapViewActions;
        private:
            ActionManager();
        public:
            static const ActionManager& instance();

            void visitMainMenu(MenuVisitor& visitor) const;
            void visitToolBarActions(MenuVisitor& visitor) const;
            void visitMapViewActions(const ActionVisitor& visitor) const;
        private:
            void initialize();
            const Action* createAction(const String& name, ActionContext actionContext, const QKeySequence& defaultShortcut,
                                       const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
                                       const IO::Path& iconPath = IO::Path());
            const Action* createAction(const String& name, ActionContext actionContext, const QKeySequence& defaultShortcut,
                                       const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
                                       const Action::CheckedFn& checked, const IO::Path& iconPath = IO::Path());

            Menu& createMainMenu(const String& name);

            deleteCopyAndMove(ActionManager)
        };
    }
}

#endif //TRENCHBROOM_ACTIONS_H
