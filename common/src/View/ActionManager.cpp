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

#include "ActionManager.h"

#include "Preferences.h"
#include "IO/Path.h"
#include "View/Action.h"
#include "View/CommandIds.h"
#include "View/Menu.h"
#include "View/Action.h"
#include "View/ViewShortcut.h"
#include "View/wxKeyStrings.h"

#include <wx/accel.h>
#include <wx/menu.h>
#include <wx/menuitem.h>

namespace TrenchBroom {
    namespace View {
        ActionManager& ActionManager::instance() {
            static ActionManager instance;
            return instance;
        }
        
        ActionManager::~ActionManager() {
            delete m_menuBar;
        }

        wxMenu* ActionManager::findRecentDocumentsMenu(const wxMenuBar* menuBar) {
            const size_t fileMenuIndex = static_cast<size_t>(menuBar->FindMenu("File"));
            const wxMenu* fileMenu = menuBar->GetMenu(fileMenuIndex);
            if (fileMenu == NULL)
                return NULL;
            const wxMenuItem* recentDocumentsItem = fileMenu->FindItem(CommandIds::Menu::FileOpenRecent);
            if (recentDocumentsItem == NULL)
                return NULL;
            return recentDocumentsItem->GetSubMenu();
        }
        
        const ActionMenuItem* ActionManager::findMenuItem(const int id) const {
            return m_menuBar->findActionMenuItem(id);
        }
        
        void ActionManager::getShortcutEntries(ShortcutEntryList& entries) {
            m_menuBar->getShortcutEntries(entries);
            
            ViewShortcut::List::iterator it, end;
            for (it = m_viewShortcuts.begin(), end = m_viewShortcuts.end(); it != end; ++it) {
                ViewShortcut& shortcut = *it;
                entries.push_back(&shortcut);
            }
        }

        String ActionManager::getJSTable() {
            StringStream str;
            getKeysJSTable(str);
            getMenuJSTable(str);
            getActionJSTable(str);
            return str.str();
        }

        void ActionManager::getKeysJSTable(StringStream& str) {
            str << "var keys = { mac: {}, windows: {}, linux: {} };" << std::endl;
            wxKeyStringsWindows().appendJS("windows", str);
            wxKeyStringsMac().appendJS("mac", str);
            wxKeyStringsLinux().appendJS("linux", str);
            str << std::endl;
        }
        
        void ActionManager::getMenuJSTable(StringStream& str) {
            str << "var menu = {};" << std::endl;
            
            ShortcutEntryList entries;
            m_menuBar->getShortcutEntries(entries);
            ShortcutEntryList::const_iterator it, end;
            for (it = entries.begin(), end = entries.end(); it != end; ++it) {
                const KeyboardShortcutEntry& entry = **it;
                str << "menu[\"" << entry.preferencePath().asString() << "\"] = " << entry.asJsonString() << ";" << std::endl;;
            }
        }

        void printActionPreference(StringStream& str, const Preference<KeyboardShortcut>& pref);
        void printActionPreference(StringStream& str, const Preference<KeyboardShortcut>& pref) {
            str << "actions[\"" << pref.path().asString() << "\"] = " << pref.value().asJsonString() << ";" << std::endl;
        }
        
        void ActionManager::getActionJSTable(StringStream& str) {
            str << "var actions = {};" << std::endl;
            
            ViewShortcut::List::iterator it, end;
            for (it = m_viewShortcuts.begin(), end = m_viewShortcuts.end(); it != end; ++it) {
                ViewShortcut& entry = *it;
                str << "actions[\"" << entry.preferencePath().asString() << "\"] = " << entry.asJsonString() << ";" << std::endl;
            }
            
            printActionPreference(str, Preferences::CameraFlyForward);
            printActionPreference(str, Preferences::CameraFlyBackward);
            printActionPreference(str, Preferences::CameraFlyLeft);
            printActionPreference(str, Preferences::CameraFlyRight);
        }

        wxMenuBar* ActionManager::createMenuBar(const bool withShortcuts) const {
            return m_menuBar->createMenuBar(withShortcuts);
        }

        bool ActionManager::isMenuShortcutPreference(const IO::Path& path) const {
            return !path.isEmpty() && path.firstComponent().asString() == "Menu";
        }
        
        wxAcceleratorTable ActionManager::createViewAcceleratorTable(const ActionContext context, const ActionView view) const {
            AcceleratorEntryList tableEntries;
            addViewActions(context, view, tableEntries);
            addMenuActions(context, view, tableEntries);
            return wxAcceleratorTable(static_cast<int>(tableEntries.size()), &tableEntries.front());
        }

        void ActionManager::addViewActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const {
            ViewShortcut::List::const_iterator it, end;
            for (it = m_viewShortcuts.begin(), end = m_viewShortcuts.end(); it != end; ++it) {
                const ViewShortcut& shortcut = *it;
                if (shortcut.appliesToContext(context))
                    accelerators.push_back(shortcut.acceleratorEntry(view));
            }
        }
        
        void ActionManager::addMenuActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const {
            ShortcutEntryList menuShortcuts;
            m_menuBar->getShortcutEntries(menuShortcuts);
            ShortcutEntryList::const_iterator it, end;
            for (it = menuShortcuts.begin(), end = menuShortcuts.end(); it != end; ++it) {
                const KeyboardShortcutEntry* entry = *it;
                if (entry->appliesToContext(context))
                    accelerators.push_back(entry->acceleratorEntry(view));
            }
        }

        void ActionManager::resetShortcutsToDefaults() {
            m_menuBar->resetShortcuts();

            ViewShortcut::List::iterator it, end;
            for (it = m_viewShortcuts.begin(), end = m_viewShortcuts.end(); it != end; ++it) {
                ViewShortcut& shortcut = *it;
                shortcut.resetShortcut();
            }
        }

        ActionManager::ActionManager() :
        m_menuBar(NULL) {
            createMenuBar();
            createViewShortcuts();
        }

        void ActionManager::createMenuBar() {
            assert(m_menuBar == NULL);
            m_menuBar = new MenuBar();
            
            Menu* fileMenu = m_menuBar->addMenu("File");
            fileMenu->addUnmodifiableActionItem(wxID_NEW, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_OPEN, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            fileMenu->addMenu(CommandIds::Menu::FileOpenRecent, "Open Recent");
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_SAVE, "Save", KeyboardShortcut('S', WXK_CONTROL));
            fileMenu->addUnmodifiableActionItem(wxID_SAVEAS, "Save as...", KeyboardShortcut('S', WXK_SHIFT, WXK_CONTROL));
            fileMenu->addSeparator();
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileLoadPointFile, "Load Point File");
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileUnloadPointFile, "Unload Point File");
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_CLOSE, "Close", KeyboardShortcut('W', WXK_CONTROL));
            
            Menu* editMenu = m_menuBar->addMenu("Edit");
            editMenu->addUnmodifiableActionItem(wxID_UNDO, "Undo", KeyboardShortcut('Z', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_REDO, "Redo", KeyboardShortcut('Z', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditRepeat, "Repeat", KeyboardShortcut('R', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditClearRepeat, "Clear Repeatable Commands", KeyboardShortcut('R', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addUnmodifiableActionItem(wxID_CUT, "Cut", KeyboardShortcut('X', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_COPY, "Copy", KeyboardShortcut('C', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_PASTE, "Paste", KeyboardShortcut('V', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditPasteAtOriginalPosition, "Paste at Original Position", KeyboardShortcut('V', WXK_CONTROL, WXK_ALT));
            editMenu->addModifiableActionItem(wxID_DUPLICATE, "Duplicate", KeyboardShortcut('D', WXK_CONTROL));
#ifdef __APPLE__
            editMenu->addModifiableActionItem(wxID_DELETE, "Delete", KeyboardShortcut(WXK_BACK));
#else
            editMenu->addModifiableActionItem(wxID_DELETE, "Delete", KeyboardShortcut(WXK_DELETE));
#endif
            
            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectAll, "Select All", KeyboardShortcut('A', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectSiblings, "Select Siblings", KeyboardShortcut('B', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectTouching, "Select Touching", KeyboardShortcut('T', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectInside, "Select Inside", KeyboardShortcut('E', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectTall, "Select Tall", KeyboardShortcut('E', WXK_CONTROL, WXK_SHIFT));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectByFilePosition, "Select by Line Number");
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectNone, "Select None", KeyboardShortcut('A', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            
            editMenu->addModifiableActionItem(CommandIds::Menu::EditGroupSelection, "Group", KeyboardShortcut('G', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditUngroupSelection, "Ungroup", KeyboardShortcut('G', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            
            Menu* toolMenu = editMenu->addMenu("Tools");
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", KeyboardShortcut('B'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleClipTool, "Clip Tool", KeyboardShortcut('C'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", KeyboardShortcut('R'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", KeyboardShortcut('V'));
            
            Menu* csgMenu = editMenu->addMenu("CSG");
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgConvexMerge, "Convex Merge", KeyboardShortcut('+', WXK_CONTROL));
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgSubtract, "Subtract", KeyboardShortcut('-', WXK_CONTROL));
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgIntersect, "Intersect", KeyboardShortcut('#', WXK_CONTROL));
            
            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSnapVertices, "Snap Vertices", KeyboardShortcut('V', WXK_SHIFT, WXK_CONTROL));
            editMenu->addSeparator();
            editMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleTextureLock, "Texture Lock");
            editMenu->addModifiableActionItem(CommandIds::Menu::EditReplaceTexture, "Replace Texture...");

            
            Menu* viewMenu = m_menuBar->addMenu("View");
            Menu* gridMenu = viewMenu->addMenu("Grid");
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleShowGrid, "Show Grid", KeyboardShortcut('0'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleSnapToGrid, "Snap to Grid", KeyboardShortcut('0', WXK_ALT));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewIncGridSize, "Increase Grid Size", KeyboardShortcut('+'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewDecGridSize, "Decrease Grid Size", KeyboardShortcut('-'));
            gridMenu->addSeparator();
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize1, "Set Grid Size 1", KeyboardShortcut('1'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize2, "Set Grid Size 2", KeyboardShortcut('2'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize4, "Set Grid Size 4", KeyboardShortcut('3'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize8, "Set Grid Size 8", KeyboardShortcut('4'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize16, "Set Grid Size 16", KeyboardShortcut('5'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize32, "Set Grid Size 32", KeyboardShortcut('6'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize64, "Set Grid Size 64", KeyboardShortcut('7'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize128, "Set Grid Size 128", KeyboardShortcut('8'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize256, "Set Grid Size 256", KeyboardShortcut('9'));
            
            Menu* cameraMenu = viewMenu->addMenu("Camera");
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToNextPoint, "Move to Next Point", KeyboardShortcut('.'));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPreviousPoint, "Move to Previous Point", KeyboardShortcut(','));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewFocusCameraOnSelection, "Focus on Selection", KeyboardShortcut('U', WXK_CONTROL));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPosition, "Move Camera to...");
            cameraMenu->addSeparator();

            viewMenu->addSeparator();
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewIsolateSelection, "Isolate", KeyboardShortcut('I', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewHideSelection, "Hide", KeyboardShortcut('I', WXK_CONTROL, WXK_ALT));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewUnhideAll, "Show All", KeyboardShortcut('I', WXK_CONTROL, WXK_SHIFT));

            viewMenu->addSeparator();
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToMapInspector, "Switch to Map Inspector", KeyboardShortcut('1', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToEntityInspector, "Switch to Entity Inspector", KeyboardShortcut('2', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToFaceInspector, "Switch to Face Inspector", KeyboardShortcut('3', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleInfoPanel, "Toggle Info Panel", KeyboardShortcut('4', WXK_CONTROL));
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleInspector, "Toggle Inspector", KeyboardShortcut('5', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleMaximizeCurrentView, "Maximize Current View", KeyboardShortcut(WXK_SPACE, WXK_CONTROL));
            

#ifndef NDEBUG
            Menu* debugMenu = m_menuBar->addMenu("Debug");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugPrintVertices, "Print Vertices");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCreateBrush, "Create Brush...");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCopyJSShortcuts, "Copy Javascript Shortcut Map");
#endif
            
            Menu* helpMenu = m_menuBar->addMenu("Help");
            helpMenu->addUnmodifiableActionItem(wxID_HELP, "TrenchBroom Help");
            
#ifdef __APPLE__
            // these won't show up in the app menu if we don't add them here
            fileMenu->addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
            fileMenu->addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...", KeyboardShortcut(',', WXK_CONTROL));
            fileMenu->addUnmodifiableActionItem(wxID_EXIT, "Exit");
#else
            viewMenu->addSeparator();
            viewMenu->addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...");
            
            helpMenu->addSeparator();
            helpMenu->addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
#endif
        }

        void ActionManager::createViewShortcuts() {
            createViewShortcut(KeyboardShortcut(WXK_RETURN), ActionContext_CreateComplexBrushTool,
                               Action(View::CommandIds::Actions::Nothing, "", false),
                               Action(View::CommandIds::Actions::PerformCreateBrush, "Create brush", true));
            
            createViewShortcut(KeyboardShortcut(WXK_RETURN, WXK_CONTROL), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::ToggleClipSide, "Toggle clip side", true));
            createViewShortcut(KeyboardShortcut(WXK_RETURN), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::PerformClip, "Perform clip", true));
#ifdef __APPLE__
            createViewShortcut(KeyboardShortcut(WXK_BACK, WXK_CONTROL), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::RemoveLastClipPoint, "Delete last clip point", true));
#else
            createViewShortcut(KeyboardShortcut(WXK_DELETE, WXK_CONTROL), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::RemoveLastClipPoint, "Delete last clip point", true));
#endif

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesUp, "Move vertices up", true),
                               Action(View::CommandIds::Actions::MoveVerticesForward, "Move vertices forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesDown, "Move vertices down", true),
                               Action(View::CommandIds::Actions::MoveVerticesBackward, "Move vertices backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesLeft, "Move vertices left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesRight, "Move vertices right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesForward, "Move vertices forward", true),
                               Action(View::CommandIds::Actions::MoveVerticesUp, "Move vertices up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_VertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesBackward, "Move vertices backward", true),
                               Action(View::CommandIds::Actions::MoveVerticesDown, "Move vertices down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterUp, "Move rotation center up", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterForward, "Move rotation center forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterDown, "Move rotation center down", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterBackward, "Move rotation center backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterLeft, "Move rotation center left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterRight, "Move rotation center right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterForward, "Move rotation center forward", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterUp, "Move rotation center up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterBackward, "Move rotation center backward", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterDown, "Move rotation center down", true));

            createViewShortcut(KeyboardShortcut('F'), ActionContext_Any, Action(),
                               Action(View::CommandIds::Actions::ToggleFlyMode, "Toggle fly mode", true));

            createViewShortcut(KeyboardShortcut('<'), ActionContext_Any, Action(),
                               Action(View::CommandIds::Actions::ToggleMovementRestriction, "Toggle movement axis", true));
            createViewShortcut(KeyboardShortcut('X'), ActionContext_Any, Action(),
                               Action(View::CommandIds::Actions::SetMovementRestrictionX, "Set movement axis X", true));
            createViewShortcut(KeyboardShortcut('Y'), ActionContext_Any, Action(),
                               Action(View::CommandIds::Actions::SetMovementRestrictionY, "Set movement axis Y", true));
            createViewShortcut(KeyboardShortcut('Z'), ActionContext_Any, Action(),
                               Action(View::CommandIds::Actions::SetMovementRestrictionZ, "Set movement axis Z", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsUp, "Move objects up", true),
                               Action(View::CommandIds::Actions::MoveObjectsForward, "Move objects forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsDown, "Move objects down", true),
                               Action(View::CommandIds::Actions::MoveObjectsBackward, "Move objects backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsLeft, "Move objects left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsRight, "Move objects right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsForward, "Move objects forward", true),
                               Action(View::CommandIds::Actions::MoveObjectsUp, "Move objects up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsBackward, "Move objects backward", true),
                               Action(View::CommandIds::Actions::MoveObjectsDown, "Move objects down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::RollObjectsCW, "Roll objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::RollObjectsCCW, "Roll objects counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::YawObjectsCW, "Yaw objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::YawObjectsCCW, "Yaw objects counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::PitchObjectsCW, "Pitch objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::PitchObjectsCCW, "Pitch objects counter-clockwise", true));

            createViewShortcut(KeyboardShortcut('F', WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::FlipObjectsHorizontally, "Flip objects horizontally", true));
            createViewShortcut(KeyboardShortcut('F', WXK_CONTROL, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::FlipObjectsVertically, "Flip objects vertically", true));

            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsUp, "Duplicate and move objects up", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsForward, "Duplicate and move objects forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsDown, "Duplicate and move objects down", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsBackward, "Duplicate and move objects backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsLeft, "Duplicate and move objects left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsRight, "Duplicate and move objects right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsForward, "Duplicate and move objects forward", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsUp, "Duplicate and move objects up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsBackward, "Duplicate and move objects backward", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsDown, "Duplicate and move objects down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up", true));
            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right (coarse)", true));

            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise (coarse)", true));
            
            createViewShortcut(KeyboardShortcut(WXK_SPACE), ActionContext_Any,
                               Action(View::CommandIds::Actions::CycleMapViews, "Cycle map view", true));

            createViewShortcut(KeyboardShortcut(WXK_ESCAPE), ActionContext_Any,
                               Action(View::CommandIds::Actions::Cancel, "Cancel", true));
            createViewShortcut(KeyboardShortcut(WXK_ESCAPE, WXK_CONTROL), ActionContext_Any,
                               Action(View::CommandIds::Actions::DeactivateTool, "Deactivate current tool", true));
        }

        void ActionManager::createViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action2D, const Action& action3D) {
            m_viewShortcuts.push_back(ViewShortcut(shortcut, context, action2D, action3D));
        }
        
        void ActionManager::createViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action) {
            m_viewShortcuts.push_back(ViewShortcut(shortcut, context, action));
        }
    }
}
