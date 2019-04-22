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
#include "View/Grid.h"
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
        m_checkable(false),
        m_iconPath(iconPath) {}

        Action::Action(const String& name, const int actionContext, const KeyboardShortcut& defaultShortcut,
            const Action::ExecuteFn& execute, const Action::EnabledFn& enabled, const Action::CheckedFn& checked,
            const IO::Path& iconPath) :
        m_name(name),
        m_actionContext(actionContext),
        m_preference(IO::Path("Actions") + IO::Path(m_name), defaultShortcut),
        m_execute(execute),
        m_enabled(enabled),
        m_checkable(true),
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
            /* ========== File Menu ========== */
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

            /* ========== Edit Menu ========== */
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
                    context.frame()->selectNone();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDeselect();
                });

            const auto* groupSelection = createAction("Group Selected Objects", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_G),
                [](ActionExecutionContext& context) {
                    context.frame()->groupSelectedObjects();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canGroup();
                });
            const auto* ungroupSelection = createAction("Ungroup Selected Objects", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G),
                [](ActionExecutionContext& context) {
                    context.frame()->ungroupSelectedObjects();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canUngroup();
                });

            /* ========== Edit / Tools Menu ========== */
            const auto* toggleBrushTool = createAction("Brush Tool", ActionContext_Any, QKeySequence(Qt::Key_B),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleCreateComplexBrushTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleCreateComplexBrushTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->createComplexBrushToolActive();
                });
            const auto* toggleClipTool = createAction("Clip Tool", ActionContext_Any, QKeySequence(Qt::Key_C),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleClipTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleClipTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->clipToolActive();
                });
            const auto* toggleRotateTool = createAction("Rotate Tool", ActionContext_Any, QKeySequence(Qt::Key_R),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleRotateObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleRotateObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->rotateObjectsToolActive();
                });
            const auto* toggleScaleTool = createAction("Scale Tool", ActionContext_Any, QKeySequence(Qt::Key_T),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleScaleObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleScaleObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->scaleObjectsToolActive();
                });
            const auto* toggleShearTool = createAction("Shear Tool", ActionContext_Any, QKeySequence(Qt::Key_G),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleShearObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleShearObjectsTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->shearObjectsToolActive();
                });
            const auto* toggleVertexTool = createAction("Vertex Tool", ActionContext_Any, QKeySequence(Qt::Key_V),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleVertexTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleVertexTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->vertexToolActive();
                });
            const auto* toggleEdgeTool = createAction("Edge Tool", ActionContext_Any, QKeySequence(Qt::Key_E),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleEdgeTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleEdgeTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->edgeToolActive();
                });
            const auto* toggleFaceTool = createAction("Face Tool", ActionContext_Any, QKeySequence(Qt::Key_F),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleFaceTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canToggleFaceTool();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->faceToolActive();
                });

            /* ========== Edit / CSG Menu ========== */
            const auto* csgConvexMerge = createAction("Convex Merge", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_J),
                [](ActionExecutionContext& context) {
                    context.frame()->csgConvexMerge();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgConvexMerge();
                });
            const auto* csgSubtract = createAction("Subtract", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_K),
                [](ActionExecutionContext& context) {
                    context.frame()->csgSubtract();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgSubtract();
                });
            const auto* csgHollow = createAction("Hollow", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_K),
                [](ActionExecutionContext& context) {
                    context.frame()->csgHollow();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgHollow();
                });
            const auto* csgIntersect = createAction("Intersect", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_L),
                [](ActionExecutionContext& context) {
                    context.frame()->csgIntersect();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDoCsgIntersect();
                });

            /* ========== Edit Menu Continued ========== */
            const auto* snapVerticesToInteger = createAction("Snap Vertices to Integer", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V),
                [](ActionExecutionContext& context) {
                    context.frame()->snapVerticesToInteger();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSnapVertices();
                });
            const auto* snapVerticesToGrid = createAction("Snap Vertices to Grid", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_V),
                [](ActionExecutionContext& context) {
                    context.frame()->snapVerticesToGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canSnapVertices();
                });

            const auto* toggleTextureLock = createAction("Texture Lock", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleTextureLock();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return pref(Preferences::TextureLock);
                });
            const auto* toggleUVLock = createAction("UV Lock", ActionContext_Any, QKeySequence(Qt::Key_U),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleUVLock();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return pref(Preferences::UVLock);
                });
            const auto* replaceTexture = createAction("Replace Texture...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->replaceTexture();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                });

            /* ========== View Menu ========== */
            /* ========== View / Grid Menu ========== */
            const auto* toggleShowGrid = createAction("Show Grid", ActionContext_Any, QKeySequence(Qt::Key_0),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleShowGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                });
            const auto* toggleSnapToGrid = createAction("Snap to Grid", ActionContext_Any, QKeySequence(Qt::ALT + Qt::Key_0),
                [](ActionExecutionContext& context) {
                    context.frame()->toggleSnapToGrid();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                });
            const auto* incGridSize = createAction("Increase Grid Size", ActionContext_Any, QKeySequence(Qt::Key_Plus),
                [](ActionExecutionContext& context) {
                    context.frame()->incGridSize();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canIncGridSize();
                });
            const auto* decGridSize = createAction("Decrease Grid Size", ActionContext_Any, QKeySequence(Qt::Key_Minus),
                [](ActionExecutionContext& context) {
                    context.frame()->decGridSize();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canDecGridSize();
                });

            const auto* setGridSize0125 = createAction("Set Grid Size 0.125", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-3);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -3;
                });
            const auto* setGridSize025 = createAction("Set Grid Size 0.25", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-2);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -2;
                });
            const auto* setGridSize05 = createAction("Set Grid Size 0.5", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(-1);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == -1;
                });
            const auto* setGridSize1 = createAction("Set Grid Size 1", ActionContext_Any, QKeySequence(Qt::Key_1),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(0);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 0;
                });
            const auto* setGridSize2 = createAction("Set Grid Size 2", ActionContext_Any, QKeySequence(Qt::Key_2),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(1);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 1;
                });
            const auto* setGridSize4 = createAction("Set Grid Size 4", ActionContext_Any, QKeySequence(Qt::Key_3),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(2);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 2;
                });
            const auto* setGridSize8 = createAction("Set Grid Size 8", ActionContext_Any, QKeySequence(Qt::Key_4),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(3);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 3;
                });
            const auto* setGridSize16 = createAction("Set Grid Size 16", ActionContext_Any, QKeySequence(Qt::Key_5),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(4);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 4;
                });
            const auto* setGridSize32 = createAction("Set Grid Size 32", ActionContext_Any, QKeySequence(Qt::Key_6),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(5);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 5;
                });
            const auto* setGridSize64 = createAction("Set Grid Size 64", ActionContext_Any, QKeySequence(Qt::Key_7),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(6);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 6;
                });
            const auto* setGridSize128 = createAction("Set Grid Size 128", ActionContext_Any, QKeySequence(Qt::Key_8),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(7);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 7;
                });
            const auto* setGridSize256 = createAction("Set Grid Size 256", ActionContext_Any, QKeySequence(Qt::Key_9),
                [](ActionExecutionContext& context) {
                    context.frame()->setGridSize(8);
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.document()->grid().size() == 8;
                });

            /* ========== View / Camera Menu ========== */
            const auto* moveCameraToNextPoint = createAction("Move Camera to Next Point", ActionContext_Any, QKeySequence(Qt::Key_Period),
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToNextPoint();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canMoveCameraToNextPoint();
                });
            const auto* moveCameraToPreviousPoint = createAction("Move Camera to Previous Point", ActionContext_Any, QKeySequence(Qt::Key_Comma),
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToPreviousPoint();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canMoveCameraToPreviousPoint();
                });
            const auto* focusCamera = createAction("Focus Camera on Selection", ActionContext_Any, QKeySequence(Qt::CTRL + Qt::Key_U),
                [](ActionExecutionContext& context) {
                    context.frame()->focusCameraOnSelection();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument() && context.frame()->canFocusCamera();
                });
            const auto* moveCameraToPoint = createAction("Move Camera to...", ActionContext_Any, QKeySequence(),
                [](ActionExecutionContext& context) {
                    context.frame()->moveCameraToPosition();
                },
                [](ActionExecutionContext& context) {
                    return context.hasDocument();
                });

            /* ========== View Menu Continued ========== */

            /* ========== Editing Actions ========== */
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

            const auto* cancel = createAction("Cancel", ActionContext_Any, QKeySequence(Qt::Key_Escape),
                [](ActionExecutionContext& context) { context.view()->OnCancel(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); });
            const auto* deactivateCurrentTool = createAction("Deactivate Current Tool", ActionContext_AnyTool, QKeySequence(Qt::CTRL + Qt::Key_Escape),
                [](ActionExecutionContext& context) { context.view()->OnDeactivateTool(); },
                [](ActionExecutionContext& context) { return context.hasDocument(); });

            m_mapViewActions.push_back(moveObjectsForward);
            m_mapViewActions.push_back(moveObjectsBackward);
            m_mapViewActions.push_back(moveObjectsLeft);
            m_mapViewActions.push_back(moveObjectsRight);
            m_mapViewActions.push_back(moveObjectsUp);
            m_mapViewActions.push_back(moveObjectsDown);
            m_mapViewActions.push_back(cancel);
            m_mapViewActions.push_back(deactivateCurrentTool);

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
            editMenu.addItem(groupSelection);
            editMenu.addItem(ungroupSelection);
            editMenu.addSeparator();

            auto& toolMenu = editMenu.addMenu("Tools");
            toolMenu.addItem(toggleBrushTool);
            toolMenu.addItem(toggleClipTool);
            toolMenu.addItem(toggleRotateTool);
            toolMenu.addItem(toggleScaleTool);
            toolMenu.addItem(toggleShearTool);
            toolMenu.addItem(toggleVertexTool);
            toolMenu.addItem(toggleEdgeTool);
            toolMenu.addItem(toggleFaceTool);

            auto& csgMenu = editMenu.addMenu("CSG");
            csgMenu.addItem(csgConvexMerge);
            csgMenu.addItem(csgSubtract);
            csgMenu.addItem(csgHollow);
            csgMenu.addItem(csgIntersect);

            editMenu.addSeparator();
            editMenu.addItem(snapVerticesToInteger);
            editMenu.addItem(snapVerticesToGrid);
            editMenu.addSeparator();
            editMenu.addItem(toggleTextureLock);
            editMenu.addItem(toggleUVLock);
            editMenu.addSeparator();
            editMenu.addItem(replaceTexture);

            auto& gridMenu = viewMenu.addMenu("Grid");
            gridMenu.addItem(toggleShowGrid);
            gridMenu.addItem(toggleSnapToGrid);
            gridMenu.addItem(incGridSize);
            gridMenu.addItem(decGridSize);
            gridMenu.addSeparator();
            gridMenu.addItem(setGridSize0125);
            gridMenu.addItem(setGridSize025);
            gridMenu.addItem(setGridSize05);
            gridMenu.addItem(setGridSize1);
            gridMenu.addItem(setGridSize2);
            gridMenu.addItem(setGridSize4);
            gridMenu.addItem(setGridSize8);
            gridMenu.addItem(setGridSize16);
            gridMenu.addItem(setGridSize32);
            gridMenu.addItem(setGridSize64);
            gridMenu.addItem(setGridSize128);
            gridMenu.addItem(setGridSize256);

            auto& cameraMenu = viewMenu.addMenu("Camera");
            cameraMenu.addItem(moveCameraToNextPoint);
            cameraMenu.addItem(moveCameraToPreviousPoint);
            cameraMenu.addItem(focusCamera);
            cameraMenu.addItem(moveCameraToPoint);

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
