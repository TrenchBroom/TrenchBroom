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
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "Model/EditorContext.h"
#include "Model/EntityAttributes.h"
#include "Model/Tag.h"
#include "View/Grid.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapViewBase.h"

#include "vecmath/util.h"

#include <QKeySequence>
#include <QMessageBox>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        // ActionExecutionContext

        ActionExecutionContext::ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView) :
        m_actionContext(mapView != nullptr ? mapView->actionContext() : ActionContext::Any), // cache here for performance reasons
        m_frame(mapFrame),
        m_mapView(mapView) {
            if (m_frame != nullptr) {
                assert(m_mapView != nullptr);
            }
        }

        bool ActionExecutionContext::hasDocument() const {
            return m_frame != nullptr;
        }

        bool ActionExecutionContext::hasActionContext(const ActionContext::Type actionContext) const {
            if (actionContext == ActionContext::Any || m_actionContext == ActionContext::Any) {
                return true;
            }

            if (hasDocument()) {
                return actionContextMatches(m_actionContext, actionContext);
            }
            return false;
        }

        MapFrame* ActionExecutionContext::frame() {
            assert(hasDocument());
            return m_frame;
        }

        MapViewBase* ActionExecutionContext::view() {
            assert(hasDocument());
            assert(m_mapView != nullptr);
            return m_mapView;
        }

        MapDocument* ActionExecutionContext::document() {
            assert(hasDocument());
            return m_frame->document().get();
        }

        // Action

        Action::Action(const String& name, const ActionContext::Type actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const IO::Path& iconPath) :
        m_name(name),
        m_preferencePath(IO::Path("Action") + IO::Path(m_name)),
        m_actionContext(actionContext),
        m_defaultShortcut(defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_checkable(false),
        m_iconPath(iconPath) {}

        Action::Action(const String& name, const ActionContext::Type actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const Action::CheckedFn& checked,
            const IO::Path& iconPath) :
        m_name(name),
        m_preferencePath(IO::Path("Action") + IO::Path(m_name)),
        m_actionContext(actionContext),
        m_defaultShortcut(defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_checkable(true),
        m_checked(checked),
        m_iconPath(iconPath) {}

        const String& Action::name() const {
            return m_name;
        }

        ActionContext::Type Action::actionContext() const {
            return m_actionContext;
        }

        QKeySequence Action::keySequence() const {
            auto& prefs = PreferenceManager::instance();
            const auto& pref = prefs.dynamicPreference(m_preferencePath, KeyboardShortcut(m_defaultShortcut));
            return prefs.get(pref).keySequence();
        }

        void Action::setKeySequence(const QKeySequence& keySequence) const {
            auto& prefs = PreferenceManager::instance();
            auto& pref = prefs.dynamicPreference(m_preferencePath, KeyboardShortcut(m_defaultShortcut));
            prefs.set(pref, KeyboardShortcut(keySequence));
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
            return m_checkable;
        }

        bool Action::checked(ActionExecutionContext& context) const {
            assert(checkable());
            return m_checked(context);
        }

        bool Action::hasIcon() const {
            return !m_iconPath.isEmpty();
        }

        const IO::Path& Action::iconPath() const {
            assert(hasIcon());
            return m_iconPath;
        }

        // MenuVisitor

        MenuVisitor::~MenuVisitor() = default;

        // MenuEntry

        MenuEntry::MenuEntry(const MenuEntryType entryType) :
        m_entryType(entryType) {}

        MenuEntry::~MenuEntry() = default;

        MenuEntryType MenuEntry::entryType() const {
            return m_entryType;
        }

        // MenuSeparatorItem

        MenuSeparatorItem::MenuSeparatorItem() :
        MenuEntry(MenuEntryType::Menu_None) {}

        void MenuSeparatorItem::accept(MenuVisitor& menuVisitor) const {
            menuVisitor.visit(*this);
        }

        // MenuActionItem

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

        // Menu

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

        void Menu::visitEntries(MenuVisitor& visitor) const {
            for (const auto& entry : m_entries) {
                entry->accept(visitor);
            }
        }

        // ActionManager

        ActionManager::ActionManager() {
            initialize();
        }

        const ActionManager& ActionManager::instance() {
            static const auto instance = ActionManager();
            return instance;
        }

        std::list<Action> ActionManager::createTagActions(const std::list<Model::SmartTag>& tags) const {
            std::list<Action> result;

            const auto actionContext = ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool;
            for (const auto& tag : tags) {
                result.emplace_back("Tags/Toggle/" + tag.name(), actionContext, KeyboardShortcut(),
                    [&tag](ActionExecutionContext& context) {
                        context.view()->toggleTagVisible(tag);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); },
                    IO::Path());
                if (tag.canEnable()) {
                    result.emplace_back("Tags/Enable/" + tag.name(), actionContext, KeyboardShortcut(),
                        [&tag](ActionExecutionContext& context) {
                            context.view()->enableTag(tag);
                        },
                        [](ActionExecutionContext& context) { return context.hasDocument(); },
                        IO::Path());
                }
                if (tag.canDisable()) {
                    result.emplace_back("Tags/Disable/" + tag.name(), actionContext, KeyboardShortcut(),
                        [&tag](ActionExecutionContext& context) {
                            context.view()->disableTag(tag);
                        },
                        [](ActionExecutionContext& context) { return context.hasDocument(); },
                        IO::Path());
                }
            }

            return result;
        }

        std::list<Action> ActionManager::createEntityDefinitionActions(const std::vector<Assets::EntityDefinition*>& entityDefinitions) const {
            std::list<Action> result;

            const auto actionContext = ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyTool;
            for (const auto* definition : entityDefinitions) {
                result.emplace_back("Entity Definitions/Toggle/" + definition->name(), actionContext, KeyboardShortcut(),
                    [definition](ActionExecutionContext& context) {
                        context.view()->toggleEntityDefinitionVisible(definition);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); },
                    IO::Path());
                if (definition->name() != Model::AttributeValues::WorldspawnClassname) {
                    result.emplace_back("Entity Definitions/Create/" + definition->name(), actionContext, KeyboardShortcut(),
                        [definition](ActionExecutionContext& context) {
                            context.view()->createEntity(definition);
                        },
                        [](ActionExecutionContext& context) { return context.hasDocument(); },
                        IO::Path());
                }
            }

            return result;
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
            createViewActions();
            createMenu();
            createToolbar();
        }

        void ActionManager::createViewActions() {
            /* ========== Editing Actions ========== */
            /* ========== Tool Specific Actions ========== */
            m_mapViewActions.push_back(
                createAction("Create Brush", ActionContext::View3D | ActionContext::CreateComplexBrushTool, QKeySequence(Qt::Key_Return),
                    [](ActionExecutionContext& context) {
                        context.view()->createComplexBrush();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->createComplexBrushToolActive();
                    }));
            m_mapViewActions.push_back(
                createAction("Toggle Clip Side", ActionContext::AnyView | ActionContext::ClipTool, QKeySequence(Qt::CTRL + Qt::Key_Return),
                    [](ActionExecutionContext& context) {
                        context.view()->toggleClipSide();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->clipToolActive();
                    }));
            m_mapViewActions.push_back(
                createAction("Perform Clip", ActionContext::AnyView | ActionContext::ClipTool, QKeySequence(Qt::Key_Return),
                    [](ActionExecutionContext& context) {
                        context.view()->performClip();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->clipToolActive();
                    }));

            /* ========== Translation ========== */
            // applies to objects, vertices, handles (e.g. rotation center)
            m_mapViewActions.push_back(
                createAction("Move Forward", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_Up),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::forward);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Backward", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_Down),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::backward);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Left", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_Left),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::left);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Right", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_Right),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::right);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Up", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_PageUp),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::up);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Down", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool | ActionContext::RotateTool, QKeySequence(Qt::Key_PageDown),
                    [](ActionExecutionContext& context) {
                        context.view()->move(vm::direction::down);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== Duplication ========== */
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Forward", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_Up),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::forward);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Backward", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_Down),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::backward);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Left", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_Left),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::left);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Right", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_Right),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::right);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Up", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_PageUp),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::up);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Duplicate and Move Down", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_PageDown),
                    [](ActionExecutionContext& context) {
                        context.view()->duplicateAndMoveObjects(vm::direction::down);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== Rotation ========== */
            // applies to objects, vertices, handles (e.g. rotation center)
            m_mapViewActions.push_back(
                createAction("Roll Clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_Up),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::roll, true);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Roll Counter-clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_Down),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::roll, false);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Yaw Clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_Left),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::yaw, true);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Yaw Counter-clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_Right),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::yaw, false);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Pitch Clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_PageUp),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::pitch, true);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Pitch Counter-clockwise", ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool, QKeySequence(Qt::ALT + Qt::Key_PageDown),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateObjects(vm::rotation_axis::pitch, false);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== Flip ========== */
            m_mapViewActions.push_back(
                createAction("Flip Horizontally", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::Key_F),
                    [](ActionExecutionContext& context) {
                        context.view()->flipObjects(vm::direction::left);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument() && context.view()->canFlipObjects(); },
                    IO::Path("FlipHorizontally.png")));
            m_mapViewActions.push_back(
                createAction("Flip Vertically", ActionContext::AnyView | ActionContext::NodeSelection, QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F),
                    [](ActionExecutionContext& context) {
                        context.view()->flipObjects(vm::direction::up);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument() && context.view()->canFlipObjects(); },
                    IO::Path("FlipVertically.png")));

            /* ========== Texturing ========== */
            m_mapViewActions.push_back(
                createAction("Move Textures Up", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_Up),
                    [](ActionExecutionContext& context) {
                        context.view()->moveTextures(vm::direction::up);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Textures Down", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_Down),
                    [](ActionExecutionContext& context) {
                        context.view()->moveTextures(vm::direction::down);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Textures Left", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_Left),
                    [](ActionExecutionContext& context) {
                        context.view()->moveTextures(vm::direction::left);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Move Textures Right", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_Right),
                    [](ActionExecutionContext& context) {
                        context.view()->moveTextures(vm::direction::right);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Rotate Textures Clockwise", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_PageUp),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateTextures(true);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(
                createAction("Rotate Textures Counter-clockwise", ActionContext::View3D | ActionContext::FaceSelection, QKeySequence(Qt::Key_PageDown),
                    [](ActionExecutionContext& context) {
                        context.view()->rotateTextures(false);
                    },
                    [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== Tag Actions ========== */
            m_mapViewActions.push_back(createAction("Make Structural", ActionContext::NodeSelection, QKeySequence(Qt::ALT + Qt::Key_S),
                [](ActionExecutionContext& context) { context.view()->makeStructural(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== View / Filter Actions ========== */
            m_mapViewActions.push_back(createAction("Toggle Show Entity Classnames", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowEntityClassnames(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Group Bounds", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowGroupBounds(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Brush Entity Bounds", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowBrushEntityBounds(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Point Entity Bounds", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowPointEntityBounds(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Point Entities", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowPointEntities(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Point Entity Models", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowPointEntityModels(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Brushes", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowBrushes(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Show Textures", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->showTextures(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Hide Textures", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->hideTextures(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Hide Faces", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->hideFaces(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Shade Faces", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShadeFaces(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Fog", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowFog(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Toggle Show Edges", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->toggleShowEdges(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Show All Entity Links", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->showAllEntityLinks(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Show Transitively Selected Entity Links", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->showTransitivelySelectedEntityLinks(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Show Directly Selected Entity Links", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->showDirectlySelectedEntityLinks(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Hide All Entity Links", ActionContext::Any, QKeySequence(),
                [](ActionExecutionContext& context) { context.view()->hideAllEntityLinks(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));


            /* ========== Misc Actions ========== */
            m_mapViewActions.push_back(createAction("Cycle View", ActionContext::Any, QKeySequence(Qt::Key_Space),
                [](ActionExecutionContext& context) { context.view()->cycleMapView(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Reset Camera Zoom", ActionContext::View3D, QKeySequence(Qt::SHIFT + Qt::Key_Escape),
                [](ActionExecutionContext& context) { context.view()->resetCameraZoom(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Cancel", ActionContext::Any, QKeySequence(Qt::Key_Escape),
                [](ActionExecutionContext& context) { context.view()->cancel(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            m_mapViewActions.push_back(createAction("Deactivate Current Tool", ActionContext::AnyView | ActionContext::AnyTool, QKeySequence(Qt::CTRL + Qt::Key_Escape),
                [](ActionExecutionContext& context) { context.view()->deactivateTool(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); },
                IO::Path("NoTool.png")));
        }

        void ActionManager::createMenu() {
            createFileMenu();
            createEditMenu();
            createViewMenu();
            createRunMenu();
            createDebugMenu();
            createHelpMenu();
        }

        void ActionManager::createFileMenu() {
            auto& fileMenu = createMainMenu("File");
            fileMenu.addItem(createMenuAction("New Document", QKeySequence::New,
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.newDocument();
                },
                [](ActionExecutionContext& context) { return true; }));
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Open Document...", QKeySequence::Open,
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.openDocument();
                },
                [](ActionExecutionContext& context) { return true; }));
            fileMenu.addMenu("Open Recent", MenuEntryType::Menu_RecentDocuments);
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Save Document", QKeySequence::Save,
                [](ActionExecutionContext& context) {
                    context.frame()->saveDocument();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            fileMenu.addItem(createMenuAction("Save Document as...", QKeySequence::SaveAs,
                [](ActionExecutionContext& context) {
                    context.frame()->saveDocumentAs();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));

            auto& exportMenu = fileMenu.addMenu("Export");
            exportMenu.addItem(createMenuAction("Wavefront OBJ...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->exportDocumentAsObj();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));

            /* ========== File Menu (Associated Resources) ========== */
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Load Point File...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->loadPointFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            fileMenu.addItem(createMenuAction("Reload Point File", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->reloadPointFile();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canReloadPointFile();
                }));
            fileMenu.addItem(createMenuAction("Unload Point File", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->unloadPointFile();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUnloadPointFile();
                }));
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Load Portal File...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->loadPortalFile();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            fileMenu.addItem(createMenuAction("Reload Portal File", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->reloadPortalFile();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canReloadPortalFile();
                }));
            fileMenu.addItem(createMenuAction("Unload Portal File", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->unloadPortalFile();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUnloadPortalFile();
                }));
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Reload Texture Collections", Qt::Key_F5,
                [](ActionExecutionContext& context) {
                    context.frame()->reloadTextureCollections();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            fileMenu.addItem(createMenuAction("Reload Entity Definitions", Qt::Key_F6,
                [](ActionExecutionContext& context) {
                    context.frame()->reloadEntityDefinitions();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
            fileMenu.addSeparator();
            fileMenu.addItem(createMenuAction("Close Document", QKeySequence::Close,
                [](ActionExecutionContext& context) {
                    context.frame()->closeDocument();
                },
                [](ActionExecutionContext& context) { return context.hasDocument(); }));
        }

        void ActionManager::createEditMenu() {/* ========== Edit Menu ========== */
            auto& editMenu = createMainMenu("Edit");
            editMenu.addItem(createMenuAction("Undo", QKeySequence::Undo,
                [](ActionExecutionContext& context) {
                    context.frame()->undo();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUndo();
                }), MenuEntryType::Menu_Undo);
            editMenu.addItem(createMenuAction("Redo", QKeySequence::Redo,
                [](ActionExecutionContext& context) {
                    context.frame()->redo();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canRedo();
                }), MenuEntryType::Menu_Redo);
            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Repeat Last Commands", Qt::CTRL + Qt::Key_R,
                [](ActionExecutionContext& context) {
                    context.frame()->repeatLastCommands();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            editMenu.addItem(createMenuAction("Clear Repeatable Commands", Qt::CTRL + Qt::SHIFT + Qt::Key_R,
                [](ActionExecutionContext& context) {
                    context.frame()->clearRepeatableCommands();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->hasRepeatableCommands();
                }));
            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Cut", QKeySequence::Cut,
                [](ActionExecutionContext& context) {
                    context.frame()->cutSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canCopySelection();
                }), MenuEntryType::Menu_Cut);
            editMenu.addItem(createMenuAction("Copy", QKeySequence::Copy,
                [](ActionExecutionContext& context) {
                    context.frame()->copySelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canCopySelection();
                }), MenuEntryType::Menu_Copy);
            editMenu.addItem(createMenuAction("Paste", QKeySequence::Paste,
                [](ActionExecutionContext& context) {
                    context.frame()->pasteAtCursorPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canPaste();
                }), MenuEntryType::Menu_Paste);
            editMenu.addItem(createMenuAction("Paste at Original Position", Qt::CTRL + Qt::ALT + Qt::Key_V,
                [](ActionExecutionContext& context) {
                    context.frame()->pasteAtOriginalPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canPaste();
                }), MenuEntryType::Menu_PasteAtOriginalPosition);
            editMenu.addItem(createMenuAction("Duplicate", Qt::CTRL + Qt::Key_D,
                [](ActionExecutionContext& context) {
                    context.frame()->duplicateSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDuplicateSelectino();
                },
                IO::Path("DuplicateObjects.png")));
            editMenu.addItem(createAction("Delete", ActionContext::Any, QKeySequence(
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
                }));
            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Select All", QKeySequence::SelectAll,
                [](ActionExecutionContext& context) {
                    context.frame()->selectAll();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelect();
                }));
            editMenu.addItem(createMenuAction("Select Siblings", Qt::CTRL + Qt::Key_B,
                [](ActionExecutionContext& context) {
                    context.frame()->selectSiblings();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectSiblings();
                }));
            editMenu.addItem(createMenuAction("Select Touching", Qt::CTRL + Qt::Key_T,
                [](ActionExecutionContext& context) {
                    context.frame()->selectTouching();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectByBrush();
                }));
            editMenu.addItem(createMenuAction("Select Inside", Qt::CTRL + Qt::Key_E,
                [](ActionExecutionContext& context) {
                    context.frame()->selectInside();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelectByBrush();
                }));
            editMenu.addItem(
                createAction("Select Tall", ActionContext::Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E),
                    [](ActionExecutionContext& context) {
                        context.frame()->selectTall();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->canSelectTall();
                    }));
            editMenu.addItem(createMenuAction("Select by Line Number...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->selectByLineNumber();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSelect();
                }));
            editMenu.addItem(
                createAction("Select None", ActionContext::Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A),
                    [](ActionExecutionContext& context) {
                        context.frame()->selectNone();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->canDeselect();
                    }));
            editMenu.addSeparator();
            editMenu.addItem(
                createAction("Group Selected Objects", ActionContext::Any, QKeySequence(Qt::CTRL + Qt::Key_G),
                    [](ActionExecutionContext& context) {
                        context.frame()->groupSelectedObjects();
                    },
                    [](ActionExecutionContext& context) {
                        return context.hasDocument() && context.frame()->canGroupSelectedObjects();
                    }));
            editMenu.addItem(createMenuAction("Ungroup Selected Objects", Qt::CTRL + Qt::SHIFT + Qt::Key_G,
                [](ActionExecutionContext& context) {
                    context.frame()->ungroupSelectedObjects();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUngroupSelectedObjects();
                }));
            editMenu.addSeparator();

            auto& toolMenu = editMenu.addMenu("Tools");
            toolMenu.addItem(createMenuAction("Brush Tool", Qt::Key_B,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleCreateComplexBrushTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleCreateComplexBrushTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->createComplexBrushToolActive();
                },
                IO::Path("BrushTool.png")));
            toolMenu.addItem(createMenuAction("Clip Tool", Qt::Key_C,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleClipTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleClipTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->clipToolActive();
                },
                IO::Path("ClipTool.png")));
            toolMenu.addItem(createMenuAction("Rotate Tool", Qt::Key_R,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleRotateObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleRotateObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->rotateObjectsToolActive();
                },
                IO::Path("RotateTool.png")));
            toolMenu.addItem(createMenuAction("Scale Tool", Qt::Key_T,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleScaleObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleScaleObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->scaleObjectsToolActive();
                },
                IO::Path("ScaleTool.png")));
            toolMenu.addItem(createMenuAction("Shear Tool", Qt::Key_G,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleShearObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleShearObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->shearObjectsToolActive();
                },
                IO::Path("ShearTool.png")));
            toolMenu.addItem(createMenuAction("Vertex Tool", Qt::Key_V,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleVertexTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleVertexTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->vertexToolActive();
                },
                IO::Path("VertexTool.png")));
            toolMenu.addItem(createMenuAction("Edge Tool", Qt::Key_E,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleEdgeTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleEdgeTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->edgeToolActive();
                },
                IO::Path("EdgeTool.png")));
            toolMenu.addItem(createMenuAction("Face Tool", Qt::Key_F,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleFaceTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleFaceTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->faceToolActive();
                },
                IO::Path("FaceTool.png")));

            auto& csgMenu = editMenu.addMenu("CSG");
            csgMenu.addItem(createMenuAction("Convex Merge", Qt::CTRL + Qt::Key_J,
                [](ActionExecutionContext& context) {
                    context.frame()->csgConvexMerge();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgConvexMerge();
                }));
            csgMenu.addItem(createMenuAction("Subtract", Qt::CTRL + Qt::Key_K,
                [](ActionExecutionContext& context) {
                    context.frame()->csgSubtract();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgSubtract();
                }));
            csgMenu.addItem(createMenuAction("Hollow", Qt::CTRL + Qt::SHIFT + Qt::Key_K,
                [](ActionExecutionContext& context) {
                    context.frame()->csgHollow();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgHollow();
                }));
            csgMenu.addItem(createMenuAction("Intersect", Qt::CTRL + Qt::Key_L,
                [](ActionExecutionContext& context) {
                    context.frame()->csgIntersect();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgIntersect();
                }));

            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Snap Vertices to Integer", Qt::CTRL + Qt::SHIFT + Qt::Key_V,
                [](ActionExecutionContext& context) {
                    context.frame()->snapVerticesToInteger();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSnapVertices();
                }));
            editMenu.addItem(createMenuAction("Snap Vertices to Grid", Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_V,
                [](ActionExecutionContext& context) {
                    context.frame()->snapVerticesToGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSnapVertices();
                }));
            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Texture Lock", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleTextureLock();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return pref(Preferences::TextureLock);
                },
                IO::Path("TextureLock.png")));
            editMenu.addItem(createMenuAction("UV Lock", Qt::Key_U,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleUVLock();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return pref(Preferences::UVLock);
                },
                IO::Path("UVLock.png")));
            editMenu.addSeparator();
            editMenu.addItem(createMenuAction("Replace Texture...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->replaceTexture();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
        }

        void ActionManager::createViewMenu() {
            auto& viewMenu = createMainMenu("View");
            auto& gridMenu = viewMenu.addMenu("Grid");
            gridMenu.addItem(createMenuAction("Show Grid", Qt::Key_0,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleShowGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            gridMenu.addItem(createMenuAction("Snap to Grid", Qt::ALT + Qt::Key_0,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleSnapToGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            gridMenu.addItem(createMenuAction("Increase Grid Size", Qt::Key_Plus,
                [](ActionExecutionContext& context) {
                    context.frame()->incGridSize();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canIncGridSize();
                }));
            gridMenu.addItem(createMenuAction("Decrease Grid Size", Qt::Key_Minus,
                [](ActionExecutionContext& context) {
                    context.frame()->decGridSize();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDecGridSize();
                }));
            gridMenu.addSeparator();
            gridMenu.addItem(createMenuAction("Set Grid Size 0.125", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-3);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -3;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 0.25", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-2);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -2;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 0.5", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-1);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -1;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 1", Qt::Key_1,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(0);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 0;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 2", Qt::Key_2,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(1);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 1;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 4", Qt::Key_3,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(2);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 2;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 8", Qt::Key_4,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(3);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 3;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 16", Qt::Key_5,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(4);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 4;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 32", Qt::Key_6,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(5);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 5;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 64", Qt::Key_7,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(6);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 6;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 128", Qt::Key_8,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(7);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 7;
                }));
            gridMenu.addItem(createMenuAction("Set Grid Size 256", Qt::Key_9,
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(8);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 8;
                }));

            auto& cameraMenu = viewMenu.addMenu("Camera");
            cameraMenu.addItem(createMenuAction("Move Camera to Next Point", Qt::Key_Period,
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToNextPoint();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canMoveCameraToNextPoint();
                }));
            cameraMenu.addItem(createMenuAction("Move Camera to Previous Point", Qt::Key_Comma,
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToPreviousPoint();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canMoveCameraToPreviousPoint();
                }));
            cameraMenu.addItem(createMenuAction("Focus Camera on Selection", Qt::CTRL + Qt::Key_U,
                [](ActionExecutionContext& context) {
                    context.frame()->focusCameraOnSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canFocusCamera();
                }));
            cameraMenu.addItem(createMenuAction("Move Camera to...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));

            viewMenu.addSeparator();
            viewMenu.addItem(createMenuAction("Isolate Selection", Qt::CTRL + Qt::Key_I,
                [](ActionExecutionContext& context) {
                    context.frame()->isolateSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canIsolateSelection();
                }));
            viewMenu.addItem(createMenuAction("Hide Selection", Qt::CTRL + Qt::ALT + Qt::Key_I,
                [](ActionExecutionContext& context) {
                    context.frame()->hideSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canHideSelection();
                }));
            viewMenu.addItem(createMenuAction("Show All", Qt::CTRL + Qt::SHIFT + Qt::Key_I,
                [](ActionExecutionContext& context) {
                    context.frame()->showAll();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            viewMenu.addSeparator();
            viewMenu.addItem(createMenuAction("Show Map Inspector", Qt::CTRL + Qt::Key_1,
                [](ActionExecutionContext& context) {
                    context.frame()->switchToInspectorPage(Inspector::InspectorPage_Map);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            viewMenu.addItem(createMenuAction("Show Entity Inspector", Qt::CTRL + Qt::Key_2,
                [](ActionExecutionContext& context) {
                    context.frame()->switchToInspectorPage(Inspector::InspectorPage_Entity);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            viewMenu.addItem(createMenuAction("Show Face Inspector", Qt::CTRL + Qt::Key_3,
                [](ActionExecutionContext& context) {
                    context.frame()->switchToInspectorPage(Inspector::InspectorPage_Face);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            viewMenu.addSeparator();
            viewMenu.addItem(createMenuAction("Toggle Info Panel", Qt::CTRL + Qt::Key_4,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleInfoPanel();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->infoPanelVisible();
                }));
            viewMenu.addItem(createMenuAction("Toggle Inspector", Qt::CTRL + Qt::Key_5,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleInspector();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->inspectorVisible();
                }));
            viewMenu.addItem(createMenuAction("Maximize Current View", Qt::CTRL + Qt::Key_Space,
                [](ActionExecutionContext& context) {
                    context.frame()->toggleMaximizeCurrentView();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->currentViewMaximized();
                }));
            viewMenu.addSeparator();
            viewMenu.addItem(createMenuAction("Preferences...", QKeySequence::Preferences,
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.showPreferences();
                },
                [](ActionExecutionContext& context) {
                    return true;
                }));
        }

        void ActionManager::createRunMenu() {
            auto& runMenu = createMainMenu("Run");
            runMenu.addItem(createMenuAction("Compile Map...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->showCompileDialog();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            runMenu.addItem(createMenuAction("Launch Engine...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->showLaunchEngineDialog();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
        }

        void ActionManager::createDebugMenu() {
#ifndef NDEBUG
            auto& debugMenu = createMainMenu("Debug");
            debugMenu.addItem(createMenuAction("Print Vertices to Console", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugPrintVertices();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Create Brush...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugCreateBrush();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Create Cube...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugCreateCube();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Clip Brush...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugClipBrush();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Copy Javascript Shortcut Map", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugCopyJSShortcutMap();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Crash...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugCrash();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Throw Exception During Command", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugThrowExceptionDuringCommand();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
            debugMenu.addItem(createMenuAction("Show Crash Report Dialog...", 0,
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.debugShowCrashReportDialog();
                },
                [](ActionExecutionContext& context) {
                    return true;
                }));
            debugMenu.addItem(createMenuAction("Set Window Size...", 0,
                [](ActionExecutionContext& context) {
                    context.frame()->debugSetWindowSize();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                }));
#endif
        }

        void ActionManager::createHelpMenu() {
            auto& helpMenu = createMainMenu("Help");
            helpMenu.addItem(
                createAction("TrenchBroom Manual", ActionContext::Any, QKeySequence(QKeySequence::HelpContents),
                    [](ActionExecutionContext& context) {
                        auto& app = TrenchBroomApp::instance();
                        app.showManual();
                    },
                    [](ActionExecutionContext& context) {
                        return true;
                    }));
            helpMenu.addItem(createMenuAction("About TrenchBroom", 0,
                [](ActionExecutionContext& context) {
                    auto& app = TrenchBroomApp::instance();
                    app.showAboutDialog();
                },
                [](ActionExecutionContext& context) {
                    return true;
                }));
        }

        void ActionManager::createToolbar() {
            m_toolBar = std::make_unique<Menu>("Toolbar", MenuEntryType::Menu_None);
            m_toolBar->addItem(existingAction("Deactivate Current Tool"));
            m_toolBar->addItem(existingAction("Brush Tool"));
            m_toolBar->addItem(existingAction("Clip Tool"));
            m_toolBar->addItem(existingAction("Vertex Tool"));
            m_toolBar->addItem(existingAction("Edge Tool"));
            m_toolBar->addItem(existingAction("Face Tool"));
            m_toolBar->addItem(existingAction("Rotate Tool"));
            m_toolBar->addItem(existingAction("Scale Tool"));
            m_toolBar->addItem(existingAction("Shear Tool"));
            m_toolBar->addSeparator();
            m_toolBar->addItem(existingAction("Duplicate"));
            m_toolBar->addItem(existingAction("Flip Horizontally"));
            m_toolBar->addItem(existingAction("Flip Vertically"));
            m_toolBar->addSeparator();
            m_toolBar->addItem(existingAction("Texture Lock"));
            m_toolBar->addItem(existingAction("UV Lock"));
            m_toolBar->addSeparator();
        }

        const Action* ActionManager::createMenuAction(const String& name, const int key, const Action::ExecuteFn& execute,
                                                      const Action::EnabledFn& enabled,
                                                      const IO::Path& iconPath) {
            return createAction(name, ActionContext::Any, QKeySequence(key), execute, enabled, iconPath);
        }

        const Action* ActionManager::createMenuAction(const String& name, const int key, const Action::ExecuteFn& execute,
                                                      const Action::EnabledFn& enabled,
                                                      const Action::CheckedFn& checked,
                                                      const IO::Path& iconPath) {
            return createAction(name, ActionContext::Any, QKeySequence(key), execute, enabled, checked, iconPath);
        }

        const Action* ActionManager::createMenuAction(const String& name, const QKeySequence::StandardKey key,
                                                      const Action::ExecuteFn& execute,
                                                      const Action::EnabledFn& enabled,
                                                      const IO::Path& iconPath) {
            return createAction(name, ActionContext::Any, QKeySequence(key), execute, enabled, iconPath);
        }

        const Action* ActionManager::createMenuAction(const String& name, const QKeySequence::StandardKey key,
                                                      const Action::ExecuteFn& execute,
                                                      const Action::EnabledFn& enabled,
                                                      const Action::CheckedFn& checked,
                                                      const IO::Path& iconPath) {
            return createAction(name, ActionContext::Any, QKeySequence(key), execute, enabled, checked, iconPath);
        }

        const Action* ActionManager::createAction(const String& name, const int actionContext,
            const QKeySequence& defaultShortcut, const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
            const IO::Path& iconPath) {
            auto action = std::make_unique<Action>(
                name,
                actionContext,
                KeyboardShortcut(defaultShortcut),
                execute,
                enabled,
                iconPath);

            auto [it, didInsert] = m_actions.insert({name, std::move(action)});
            ensure(didInsert, "duplicate action name");
            return it->second.get();
        }

        const Action* ActionManager::createAction(const String& name, const int actionContext,
            const QKeySequence& defaultShortcut, const Action::ExecuteFn& execute, const Action::EnabledFn& enabled,
            const Action::CheckedFn& checked, const IO::Path& iconPath) {
            auto action = std::make_unique<Action>(
                name,
                actionContext,
                KeyboardShortcut(defaultShortcut),
                execute,
                enabled,
                checked,
                iconPath);

            auto [it, didInsert] = m_actions.insert({name, std::move(action)});
            ensure(didInsert, "duplicate action name");
            return it->second.get();
        }

        Menu& ActionManager::createMainMenu(const String& name) {
            auto menu = std::make_unique<Menu>(name, MenuEntryType::Menu_None);
            auto* result = menu.get();
            m_mainMenu.emplace_back(std::move(menu));
            return *result;
        }

        const Action* ActionManager::existingAction(const String& name) const {
            auto it = m_actions.find(name);
            ensure(it != m_actions.end(), "couldn't find action");
            return it->second.get();
        }
    }
}
