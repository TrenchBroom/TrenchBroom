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

#include "Preference.h"
#include "PreferenceManager.h"
#include "TrenchBroomApp.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapViewBase.h"

#include <QKeySequence>
#include <QMessageBox>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ActionExecutionContext::ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView) :
        m_frame(mapFrame),
        m_mapView(mapView) {}

        bool ActionExecutionContext::hasDocument() const {
            return m_frame != nullptr;
        }

        bool ActionExecutionContext::hasActionContext(const ActionContext actionContext) const {
            return actionContext == ActionContext_Any || (hasDocument() && (m_mapView->actionContext() & actionContext) != 0);
        }

        MapFrame* ActionExecutionContext::frame() {
            assert(hasDocument());
            return m_frame;
        }

        MapViewBase* ActionExecutionContext::view() {
            assert(hasDocument());
            return m_mapView;
        }

        MapDocument* ActionExecutionContext::document() {
            assert(hasDocument());
            return m_frame->document().get();
        }

        Action::Action(const String& name, const ActionContext actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const IO::Path& iconPath) :
        m_name(name),
        m_actionContext(actionContext),
        m_preference(IO::Path("Actions") + IO::Path(m_name), defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_iconPath(iconPath) {}

        Action::Action(const String& name, const ActionContext actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const Action::CheckedFn& checked,
            const IO::Path& iconPath) :
        m_name(name),
        m_actionContext(actionContext),
        m_preference(IO::Path("Actions") + IO::Path(m_name), defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_checked(checked),
        m_iconPath(iconPath) {}

        const String& Action::name() const {
            return m_name;
        }

        QKeySequence Action::keySequence() const {
            return pref(m_preference).keySequence();
        }

        void Action::execute(ActionExecutionContext& context) const {
            if (enabled(context)) {
                m_execute(context);
            }
        }

        bool Action::enabled(ActionExecutionContext& context) const {
            return context.hasActionContext(m_actionContext), m_enabled(context);
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

        Menu::Menu(const String& name) :
        m_name(name) {}

        const String& Menu::name() const {
            return m_name;
        }

        Menu& Menu::addMenu(String name) {
            m_entries.emplace_back(std::make_unique<Menu>(name));
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

        void ActionManager::visitMapViewActions(const ActionVisitor& visitor) const {
            for (const auto* action : m_mapViewActions) {
                visitor(*action);
            }
        }

        void ActionManager::initialize() {
            const auto* newFile = createAction("New Document", ActionContext_Any, QKeySequence(QKeySequence::New),
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.OnFileNew();
                },
                [](ActionExecutionContext& context) { return true; });
            const auto* openFile = createAction("Open Document...", ActionContext_Any, QKeySequence(QKeySequence::Open),
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.OnFileOpen();
                },
                [](ActionExecutionContext& context) { return true; });
            const auto* saveFile = createAction("Save Document", ActionContext_Any, QKeySequence(QKeySequence::Save),
                [](ActionExecutionContext& context) {
                    context.frame()->OnFileSave();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* saveFileAs = createAction("Save Document as...", ActionContext_Any, QKeySequence(QKeySequence::SaveAs),
                [](ActionExecutionContext& context) {
                    context.frame()->OnFileSaveAs();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            const auto* moveObjectsForward = createAction("Move Objects Forward", ActionContext_NodeSelection, QKeySequence(Qt::Key_Up),
                [](ActionExecutionContext& context) {
                    context.view()->OnMoveObjectsForward();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            m_mapViewActions.push_back(moveObjectsForward);

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

        const Action* ActionManager::createAction(const String& name, const ActionContext actionContext,
            const QKeySequence& defaultShortcut, const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
            const IO::Path& iconPath) {
            m_actions.emplace_back(std::make_unique<Action>(
                name,
                actionContext,
                KeyboardShortcut(defaultShortcut),
                execute,
                enabled,
                iconPath));
            return m_actions.back().get();
        }

        const Action* ActionManager::createAction(const String& name, const ActionContext actionContext,
            const QKeySequence& defaultShortcut, const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
            const Action::CheckedFn& checked, const IO::Path& iconPath) {
            m_actions.emplace_back(std::make_unique<Action>(
                name,
                actionContext,
                KeyboardShortcut(defaultShortcut),
                execute,
                enabled,
                checked,
                iconPath));
            return m_actions.back().get();
        }

        Menu& ActionManager::createMainMenu(const String& name) {
            auto menu = std::make_unique<Menu>(name);
            auto* result = menu.get();
            m_mainMenu.emplace_back(std::move(menu));
            return *result;
        }
    }
}
