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

#include "Actions.h"

#include "TrenchBroomApp.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"

#include <QKeySequence>
#include <QMessageBox>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ActionExecutionContext::ActionExecutionContext(MapFrame* mapFrame) :
        m_frame(mapFrame) {}

        bool ActionExecutionContext::hasDocument() const {
            return m_frame != nullptr;
        }

        MapFrame* ActionExecutionContext::frame() {
            return m_frame;
        }

        MapDocument* ActionExecutionContext::document() {
            assert(hasDocument());
            return m_frame->document().get();
        }

        Action::Action(String&& name, KeyboardShortcut&& defaultShortcut, Action::ExecuteFn&& execute,
                       Action::EnabledFn&& enabled, IO::Path&& iconPath) :
        m_name(std::move(name)),
        m_preference(IO::Path("Actions") + IO::Path(m_name), std::move(defaultShortcut)),
        m_execute(std::move(execute)),
        m_enabled(std::move(enabled)),
        m_iconPath(std::move(iconPath)) {}

        Action::Action(String&& name, KeyboardShortcut&& defaultShortcut, Action::ExecuteFn&& execute,
                       Action::EnabledFn&& enabled, Action::CheckedFn&& checked, IO::Path&& iconPath) :
        m_name(std::move(name)),
        m_preference(IO::Path("Actions") + IO::Path(m_name), std::move(defaultShortcut)),
        m_execute(std::move(execute)),
        m_enabled(std::move(enabled)),
        m_checked(std::move(checked)),
        m_iconPath(std::move(iconPath)) {}

        const String& Action::name() const {
            return m_name;
        }

        void Action::execute(ActionExecutionContext& context) const {
            m_execute(context);
        }

        bool Action::enabled(ActionExecutionContext& context) const {
            return m_enabled(context);
        }

        bool Action::checkable() const {
            return m_checked.has_value();
        }

        bool Action::checked(ActionExecutionContext& context) const {
            assert(checkable());
            return (m_checked.value())(context);
        }

        bool Action::hasIcon() const {
            return m_iconPath.has_value();
        }

        const IO::Path& Action::iconPath() const {
            assert(hasIcon());
            return m_iconPath.value();
        }

        MenuVisitor::~MenuVisitor() = default;

        MenuEntry::MenuEntry() = default;

        MenuEntry::~MenuEntry() = default;

        MenuSeparatorItem::MenuSeparatorItem() = default;

        void MenuSeparatorItem::accept(MenuVisitor& menuVisitor) const {
            menuVisitor.visit(*this);
        }

        MenuActionItem::MenuActionItem(const Action* action) :
        m_action(action) {}

        const String& MenuActionItem::name() const {
            return m_action->name();
        }

        const Action& MenuActionItem::action() const {
            return *m_action;
        }

        void MenuActionItem::accept(MenuVisitor& menuVisitor) const {
            menuVisitor.visit(*this);
        }

        Menu::Menu(String&& name) :
        m_name(std::move(name)) {}

        const String& Menu::name() const {
            return m_name;
        }

        Menu& Menu::addMenu(String name) {
            m_entries.emplace_back(std::make_unique<Menu>(std::move(name)));
            return *static_cast<Menu*>(m_entries.back().get());
        }

        void Menu::addSeparator() {
            m_entries.emplace_back(std::make_unique<MenuSeparatorItem>());
        }

        MenuActionItem& Menu::addItem(const Action* action) {
            m_entries.emplace_back(std::make_unique<MenuActionItem>(action));
            return *static_cast<MenuActionItem*>(m_entries.back().get());
        }

        void Menu::accept(MenuVisitor& visitor) const {
            visitor.visit(*this);
        }
#


        void Menu::visitEntries(MenuVisitor& visitor) const {
            for (const auto& entry : m_entries) {
                entry->accept(visitor);
            }
        }

        ActionManager::ActionManager() {
            initialize();
        }

        const ActionManager& ActionManager::instance() {
            static const auto instance = ActionManager();
            return instance;
        }

        void ActionManager::visitMainMenu(MenuVisitor& visitor) const {
            for (const auto& menu : m_mainMenu) {
                menu->accept(visitor);
            }
        }

        void ActionManager::initialize() {
            const auto* newFile = createAction("New Document", QKeySequence(QKeySequence::New),
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.OnFileNew();
                },
                [](ActionExecutionContext& context) { return true; });
            const auto* openFile = createAction("Open Document...", QKeySequence(QKeySequence::Open),
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.OnFileOpen();
                },
                [](ActionExecutionContext& context) { return true; });
            const auto* saveFile = createAction("Save Document", QKeySequence(QKeySequence::Save),
                [](ActionExecutionContext& context) {
                    assert(context.hasDocument());
                    context.frame()->OnFileSave();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* saveFileAs = createAction("Save Document as...", QKeySequence(QKeySequence::SaveAs),
                [](ActionExecutionContext& context) {
                    assert(context.hasDocument());
                    context.frame()->OnFileSaveAs();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            auto& fileMenu = createMainMenu("File");
            auto& editMenu = createMainMenu("Edit");
            auto& viewMenu = createMainMenu("View");
            auto& helpMenu = createMainMenu("Help");

            fileMenu.addItem(newFile);
            fileMenu.addSeparator();
            fileMenu.addItem(openFile);
            fileMenu.addSeparator();
            fileMenu.addItem(saveFile);
            fileMenu.addItem(saveFileAs);
        }

        const Action* ActionManager::createAction(String&& name, QKeySequence&& defaultShortcut,
                                                  Action::ExecuteFn&& execute, Action::EnabledFn&& enabled,
                                                  IO::Path&& iconPath) {
            m_actions.emplace_back(std::make_unique<Action>(
                std::move(name),
                KeyboardShortcut(defaultShortcut),
                std::move(execute),
                std::move(enabled),
                std::move(iconPath)));
            return m_actions.back().get();
        }

        const Action* ActionManager::createAction(String&& name, QKeySequence&& defaultShortcut,
                                                  Action::ExecuteFn&& execute, Action::EnabledFn&& enabled,
                                                  Action::CheckedFn&& checked, IO::Path&& iconPath) {
            m_actions.emplace_back(std::make_unique<Action>(
                std::move(name),
                KeyboardShortcut(defaultShortcut),
                std::move(execute),
                std::move(enabled),
                std::move(checked),
                std::move(iconPath)));
            return m_actions.back().get();
        }

        Menu& ActionManager::createMainMenu(String&& name) {
            auto menu = std::make_unique<Menu>(std::move(name));
            auto* result = menu.get();
            m_mainMenu.emplace_back(std::move(menu));
            return *result;
        }
    }
}
