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

#include "vecmath/util.h"

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

        bool ActionExecutionContext::hasActionContext(const int actionContext) const {
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

        Action::Action(const String& name, const int actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const IO::Path& iconPath) :
        m_name(name),
        m_actionContext(actionContext),
        m_preference(IO::Path("Actions") + IO::Path(m_name), defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_iconPath(iconPath) {}

        Action::Action(const String& name, const int actionContext, const KeyboardShortcut& defaultShortcut,
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
            return context.hasActionContext(m_actionContext) && m_enabled(context);
        }

        bool Action::checkable() const {
            return m_checked.has_value();
        }

        bool Action::checked(ActionExecutionContext& context) const {
            assert(checkable());
            return (m_checked.value_or([](auto& c) { return false; }))(context);
        }

        bool Action::hasIcon() const {
            return m_iconPath.has_value();
        }

        const IO::Path& Action::iconPath() const {
            assert(hasIcon());
            return m_iconPath.value_or(IO::Path::EmptyPath);
        }

        MenuVisitor::~MenuVisitor() = default;

        MenuEntry::MenuEntry(const MenuEntryType entryType) :
        m_entryType(entryType) {}

        MenuEntry::~MenuEntry() = default;

        MenuEntryType MenuEntry::entryType() const {
            return m_entryType;
        }

        MenuSeparatorItem::MenuSeparatorItem() :
        MenuEntry(MenuEntryType::Menu_None) {}

        void MenuSeparatorItem::accept(MenuVisitor& menuVisitor) const {
            menuVisitor.visit(*this);
        }

        MenuActionItem::MenuActionItem(const Action* action, const MenuEntryType entryType) :
        MenuEntry(entryType),
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

        Menu::Menu(const String& name, const MenuEntryType entryType) :
        MenuEntry(entryType),
        m_name(name) {}

        const String& Menu::name() const {
            return m_name;
        }

        Menu& Menu::addMenu(const String& name, const MenuEntryType entryType) {
            m_entries.emplace_back(std::make_unique<Menu>(name, entryType));
            return *static_cast<Menu*>(m_entries.back().get());
        }

        void Menu::addSeparator() {
            m_entries.emplace_back(std::make_unique<MenuSeparatorItem>());
        }

        MenuActionItem& Menu::addItem(const Action* action, const MenuEntryType entryType) {
            m_entries.emplace_back(std::make_unique<MenuActionItem>(action, entryType));
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

        void ActionManager::visitToolBarActions(MenuVisitor& visitor) const {
            if (m_toolBar != nullptr) {
                m_toolBar->accept(visitor);
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
                    app.newDocument();
                },
                [](ActionExecutionContext& context) { return true; });
            const auto* openFile = createAction("Open Document...", ActionContext_Any, QKeySequence(QKeySequence::Open),
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.openDocument();
                },
                [](ActionExecutionContext& context) { return true; });

            const auto* saveFile = createAction("Save Document", ActionContext_Any, QKeySequence(QKeySequence::Save),
                [](ActionExecutionContext& context) {
                    context.frame()->saveDocument();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* saveFileAs = createAction("Save Document as...", ActionContext_Any, QKeySequence(QKeySequence::SaveAs),
                [](ActionExecutionContext& context) {
                    context.frame()->saveDocumentAs();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* exportWavefrontObj = createAction("Wavefront OBJ...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->exportDocumentAsObj();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            const auto* loadPointFile = createAction("Load Point File...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->loadPointFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* reloadPointFile = createAction("Reload Point File", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->reloadPointFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument() && context.frame()->canReloadPointFile(); });
            const auto* unloadPointFile = createAction("Unload Point File", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->unloadPointFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument() && context.frame()->canUnloadPointFile(); });
            const auto* loadPortalFile = createAction("Load Portal File...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->loadPortalFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* reloadPortalFile = createAction("Reload Portal File", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->reloadPortalFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument() && context.frame()->canReloadPortalFile(); });
            const auto* unloadPortalFile = createAction("Unload Portal File", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->unloadPortalFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument() && context.frame()->canUnloadPortalFile(); });

            const auto* reloadTextureCollections = createAction("Reload Texture Collections", ActionContext_Any, QKeySequence(Qt::Key_F5),
                [](ActionExecutionContext& context) {
                    context.frame()->reloadTextureCollections();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* reloadEntityDefinitions = createAction("Reload Entity Definitions", ActionContext_Any, QKeySequence(Qt::Key_F6),
                [](ActionExecutionContext& context) {
                    context.frame()->reloadEntityDefinitions();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            const auto* closeDocument = createAction("Close Document", ActionContext_Any, QKeySequence(QKeySequence::Close),
                [](ActionExecutionContext& context) {
                    context.frame()->closeDocument();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            const auto* undo = createAction("Undo", ActionContext_Any, QKeySequence(QKeySequence::Undo),
                [](ActionExecutionContext& context) {
                    context.frame()->undo();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUndo();
                });
            const auto* redo = createAction("Redo", ActionContext_Any, QKeySequence(QKeySequence::Redo),
                [](ActionExecutionContext& context) {
                    context.frame()->redo();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canRedo();
                });

            const auto* repeat = createAction("Repeat Last Commands", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_R),
                [](ActionExecutionContext& context) {
                    context.frame()->repeatLastCommands();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                });
            const auto* clearRepeat = createAction("Repeat Last Commands", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_R),
                [](ActionExecutionContext& context) {
                    context.frame()->clearRepeatableCommands();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->hasRepeatableCommands();
                });

            const auto* cut = createAction("Cut", ActionContext_Any, QKeySequence(QKeySequence::Cut),
                [](ActionExecutionContext& context) {
                    context.frame()->cutSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canCopySelection();
                });
            const auto* copy = createAction("Copy", ActionContext_Any, QKeySequence(QKeySequence::Copy),
                [](ActionExecutionContext& context) {
                    context.frame()->copySelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canCopySelection();
                });
            const auto* paste = createAction("Paste", ActionContext_Any, QKeySequence(QKeySequence::Paste),
                [](ActionExecutionContext& context) {
                    context.frame()->pasteAtCursorPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canPaste();
                });
            const auto* pasteAtOriginalPosition = createAction("Paste at Original Position", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_V),
                [](ActionExecutionContext& context) {
                    context.frame()->pasteAtOriginalPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canPaste();
                });
            const auto* duplicate = createAction("Duplicate", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_D),
                [](ActionExecutionContext& context) {
                    context.frame()->duplicateSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDuplicateSelectino();
                });
            const auto* delete_ = createAction("Delete", ActionContext_Any, QKeySequence(
#ifdef __APPLE__
                Qt::Key_Backspace
#else
                QKeySequence::Delete
#endif
                ),
                [](ActionExecutionContext& context) {
                    context.frame()->deleteSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDeleteSelection();
                });

            const auto* selectAll = createAction("Select All", ActionContext_Any, QKeySequence(QKeySequence::SelectAll),
                [](ActionExecutionContext& context) {
                    context.frame()->selectAll();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelect();
                });
            const auto* selectSiblings = createAction("Select Siblings", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_B),
                [](ActionExecutionContext& context) {
                    context.frame()->selectSiblings();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectSiblings();
                });
            const auto* selectTouching = createAction("Select Touching", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_T),
                [](ActionExecutionContext& context) {
                    context.frame()->selectTouching();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectByBrush();
                });
            const auto* selectInside = createAction("Select Inside", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_E),
                [](ActionExecutionContext& context) {
                    context.frame()->selectInside();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectByBrush();
                });
            const auto* selectTall = createAction("Select Tall", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E),
                [](ActionExecutionContext& context) {
                    context.frame()->selectTall();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectTall();
                });
            const auto* selectByLineNo = createAction("Select by Line Number...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->selectByLineNumber();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelect();
                });
            const auto* selectNone = createAction("Select None", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A),
                [](ActionExecutionContext& context) {
                    context.frame()->OnEditSelectNone();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDeselect();
                });

            const auto* moveObjectsForward = createAction("Move Objects Forward", ActionContext_NodeSelection, QKeySequence(Qt::Key_Up),
                [](ActionExecutionContext& context) {
                    context.view()->moveObjects(vm::direction::forward);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* moveObjectsBackward = createAction("Move Objects Backward", ActionContext_NodeSelection, QKeySequence(Qt::Key_Down),
                [](ActionExecutionContext& context) {
                    context.view()->moveObjects(vm::direction::backward);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* moveObjectsLeft = createAction("Move Objects Left", ActionContext_NodeSelection, QKeySequence(Qt::Key_Left),
                [](ActionExecutionContext& context) {
                     context.view()->moveObjects(vm::direction::left);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* moveObjectsRight = createAction("Move Objects Right", ActionContext_NodeSelection, QKeySequence(Qt::Key_Right),
                [](ActionExecutionContext& context) {
                     context.view()->moveObjects(vm::direction::right);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* moveObjectsUp = createAction("Move Objects Up", ActionContext_NodeSelection, QKeySequence(Qt::Key_PageUp),
                [](ActionExecutionContext& context) {
                     context.view()->moveObjects(vm::direction::up);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* moveObjectsDown = createAction("Move Objects Down", ActionContext_NodeSelection, QKeySequence(Qt::Key_PageDown),
                [](ActionExecutionContext& context) {
                     context.view()->moveObjects(vm::direction::down);
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            m_mapViewActions.push_back(moveObjectsForward);
            m_mapViewActions.push_back(moveObjectsBackward);
            m_mapViewActions.push_back(moveObjectsLeft);
            m_mapViewActions.push_back(moveObjectsRight);
            m_mapViewActions.push_back(moveObjectsUp);
            m_mapViewActions.push_back(moveObjectsDown);

            auto& fileMenu = createMainMenu("File");
            auto& editMenu = createMainMenu("Edit");
            auto& viewMenu = createMainMenu("View");
            auto& helpMenu = createMainMenu("Help");

            fileMenu.addItem(newFile);
            fileMenu.addSeparator();
            fileMenu.addItem(openFile);
            fileMenu.addMenu("Open Recent", MenuEntryType::Menu_RecentDocuments);
            fileMenu.addSeparator();
            fileMenu.addItem(saveFile);
            fileMenu.addItem(saveFileAs);

            auto& exportMenu = fileMenu.addMenu("Export");
            exportMenu.addItem(exportWavefrontObj);

            fileMenu.addSeparator();
            fileMenu.addItem(loadPointFile);
            fileMenu.addItem(reloadPointFile);
            fileMenu.addItem(unloadPointFile);
            fileMenu.addSeparator();
            fileMenu.addItem(loadPortalFile);
            fileMenu.addItem(reloadPortalFile);
            fileMenu.addItem(unloadPortalFile);
            fileMenu.addSeparator();
            fileMenu.addItem(reloadTextureCollections);
            fileMenu.addItem(reloadEntityDefinitions);
            fileMenu.addSeparator();
            fileMenu.addItem(closeDocument);

            editMenu.addItem(undo, MenuEntryType::Menu_Undo);
            editMenu.addItem(redo, MenuEntryType::Menu_Redo);
            editMenu.addSeparator();
            editMenu.addItem(repeat);
            editMenu.addItem(clearRepeat);
            editMenu.addSeparator();
            editMenu.addItem(cut, MenuEntryType::Menu_Cut);
            editMenu.addItem(copy, MenuEntryType::Menu_Copy);
            editMenu.addItem(paste, MenuEntryType::Menu_Paste);
            editMenu.addItem(pasteAtOriginalPosition, MenuEntryType::Menu_PasteAtOriginalPosition);
            editMenu.addItem(duplicate);
            editMenu.addItem(delete_);
            editMenu.addSeparator();
            editMenu.addItem(selectAll);
            editMenu.addItem(selectSiblings);
            editMenu.addItem(selectTouching);
            editMenu.addItem(selectInside);
            editMenu.addItem(selectTall);
            editMenu.addItem(selectByLineNo);
            editMenu.addItem(selectNone);
            editMenu.addSeparator();
        }

        const Action* ActionManager::createAction(const String& name, const int actionContext,
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

        const Action* ActionManager::createAction(const String& name, const int actionContext,
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
            auto menu = std::make_unique<Menu>(name, MenuEntryType::Menu_None);
            auto* result = menu.get();
            m_mainMenu.emplace_back(std::move(menu));
            return *result;
        }
    }
}
