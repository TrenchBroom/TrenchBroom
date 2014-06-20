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

#include "View/CommandIds.h"
#include "View/Menu.h"

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
            delete m_menu;
            m_menu = NULL;
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
        
        const Action* ActionManager::findMenuAction(const int id) const {
            return m_menu->findAction(id);
        }
        
        wxMenuBar* ActionManager::createMenuBar() const {
            wxMenuBar* menuBar = new wxMenuBar();
            
            const MenuItem::List& menus = m_menu->items();
            MenuItem::List::const_iterator it, end;
            for (it = menus.begin(), end = menus.end(); it != end; ++it) {
                const MenuItem::Ptr item = *it;
                const Menu* menu = static_cast<const Menu*>(item.get());
                menuBar->Append(createMenu(*menu), menu->text());
            }
            
            return menuBar;
        }

        Menu& ActionManager::getMenu() {
            return *m_menu;
        }

        wxAcceleratorTable ActionManager::createMapViewAcceleratorTable(const Action::Context context) const {
            typedef std::vector<wxAcceleratorEntry> EntryList;
            
            EntryList tableEntries;
            
            Action::List::const_iterator it, end;
            for (it = m_mapViewActions.begin(), end = m_mapViewActions.end(); it != end; ++it) {
                const Action& action = *it;
                if (action.appliesToContext(context))
                    tableEntries.push_back(action.acceleratorEntry());
            }
            
            return wxAcceleratorTable(static_cast<int>(tableEntries.size()), &tableEntries.front());
        }

        wxMenu* ActionManager::createMenu(const Menu& menu) const {
            wxMenu* result = new wxMenu();
            
            const Menu::List& items = menu.items();
            Menu::List::const_iterator it, end;
            for (it = items.begin(), end = items.end(); it != end; ++it) {
                const MenuItem& item = **it;
                switch (item.type()) {
                    case MenuItem::Type_Action: {
                        const ActionMenuItem& actionItem = static_cast<const ActionMenuItem&>(item);
                        result->Append(actionItem.id(), actionItem.menuText());
                        break;
                    }
                    case MenuItem::Type_Check: {
                        const ActionMenuItem& actionItem = static_cast<const ActionMenuItem&>(item);
                        result->AppendCheckItem(actionItem.id(), actionItem.menuText());
                        break;
                    }
                    case MenuItem::Type_Menu: {
                        const Menu& subMenu = static_cast<const Menu&>(item);
                        wxMenuItem* wxSubMenuItem = new wxMenuItem(result, subMenu.id(), subMenu.text());
                        wxSubMenuItem->SetSubMenu(createMenu(subMenu));
                        result->Append(wxSubMenuItem);
                        break;
                    }
                    case MenuItem::Type_Separator: {
                        result->AppendSeparator();
                        break;
                    }
                }
            }
            
            return result;
        }

        ActionManager::ActionManager() {
            createMapViewActions();
            createMenu();
        }
        
        void ActionManager::createMapViewActions() {
            createMapViewAction(CommandIds::Actions::ToggleClipTool, Action::Context_Any, "Clip Tool", KeyboardShortcut('C'));
            createMapViewAction(CommandIds::Actions::ToggleClipSide, Action::Context_ClipTool, "Toggle Clip Side", KeyboardShortcut(WXK_RETURN, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::PerformClip, Action::Context_ClipTool, "Perform Clip", KeyboardShortcut(WXK_RETURN));
#ifdef __APPLE__
            createMapViewAction(CommandIds::Actions::DeleteLastClipPoint, Action::Context_ClipTool, "Delete Last Clip Point", KeyboardShortcut(WXK_BACK));
#else
            createMapViewAction(CommandIds::Actions::DeleteLastClipPoint, Action::Context_ClipTool, "Delete Last Clip Point", KeyboardShortcut(WXK_DELETE));
#endif
            
            createMapViewAction(CommandIds::Actions::MoveObjectsForward, Action::Context_ObjectSelection, "Move Forward", KeyboardShortcut(WXK_UP));
            createMapViewAction(CommandIds::Actions::MoveObjectsBackward, Action::Context_ObjectSelection, "Move Backward", KeyboardShortcut(WXK_DOWN));
            createMapViewAction(CommandIds::Actions::MoveObjectsLeft, Action::Context_ObjectSelection, "Move Left", KeyboardShortcut(WXK_LEFT));
            createMapViewAction(CommandIds::Actions::MoveObjectsRight, Action::Context_ObjectSelection, "Move Right", KeyboardShortcut(WXK_RIGHT));
            createMapViewAction(CommandIds::Actions::MoveObjectsUp, Action::Context_ObjectSelection, "Move Up", KeyboardShortcut(WXK_PAGEUP));
            createMapViewAction(CommandIds::Actions::MoveObjectsDown, Action::Context_ObjectSelection, "Move Down", KeyboardShortcut(WXK_PAGEDOWN));
            
            createMapViewAction(CommandIds::Actions::DuplicateObjectsForward, Action::Context_ObjectSelection, "Duplicate & Move Forward", KeyboardShortcut(WXK_UP, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::DuplicateObjectsBackward, Action::Context_ObjectSelection, "Duplicate & Move Backward", KeyboardShortcut(WXK_DOWN, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::DuplicateObjectsLeft, Action::Context_ObjectSelection, "Duplicate & Move Left", KeyboardShortcut(WXK_LEFT, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::DuplicateObjectsRight, Action::Context_ObjectSelection, "Duplicate & Move Right", KeyboardShortcut(WXK_RIGHT, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::DuplicateObjectsUp, Action::Context_ObjectSelection, "Duplicate & Move Up", KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL));
            createMapViewAction(CommandIds::Actions::DuplicateObjectsDown, Action::Context_ObjectSelection, "Duplicate & Move Down", KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL));

            /*
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsForward, Action::Context_ObjectSelection, "Move Forward", KeyboardShortcut(WXK_UP));
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsBackward, Action::Context_ObjectSelection, "Move Backward", KeyboardShortcut(WXK_DOWN));
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsLeft, Action::Context_ObjectSelection, "Move Left", KeyboardShortcut(WXK_LEFT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsRight, Action::Context_ObjectSelection, "Move Right", KeyboardShortcut(WXK_RIGHT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsUp, Action::Context_ObjectSelection, "Move Up", KeyboardShortcut(WXK_PAGEUP));
             objectActionMenu.addActionItem(CommandIds::Menu::EditMoveObjectsDown, Action::Context_ObjectSelection, "Move Down", KeyboardShortcut(WXK_PAGEDOWN));
             */
            
        }

        void ActionManager::createMapViewAction(const int id, const int context, const String& name, const KeyboardShortcut& defaultShortcut) {
            m_mapViewActions.push_back(Action(id, context, name, IO::Path("Actions/Map View") + IO::Path(name), defaultShortcut, true));
        }

        void ActionManager::createMenu() {
            m_menu = new Menu("Menu");
            
            Menu& fileMenu = m_menu->addMenu(wxID_ANY, "File");
            fileMenu.addUnmodifiableActionItem(wxID_NEW, Action::Context_Any, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_OPEN, Action::Context_Any, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            fileMenu.addMenu(CommandIds::Menu::FileOpenRecent, "Open Recent");
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_SAVE, Action::Context_Any, "Save", KeyboardShortcut('S', WXK_CONTROL));
            fileMenu.addUnmodifiableActionItem(wxID_SAVEAS, Action::Context_Any, "Save as...", KeyboardShortcut('S', WXK_SHIFT, WXK_CONTROL));
            fileMenu.addSeparator();
            fileMenu.addModifiableActionItem(CommandIds::Menu::FileLoadPointFile, Action::Context_Any, "Load Point File");
            fileMenu.addModifiableActionItem(CommandIds::Menu::FileUnloadPointFile, Action::Context_Any, "Unload Point File");
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_CLOSE, Action::Context_Any, "Close", KeyboardShortcut('W', WXK_CONTROL));
            
            Menu& editMenu = m_menu->addMenu(wxID_ANY, "Edit");
            editMenu.addUnmodifiableActionItem(wxID_UNDO, Action::Context_Any, "Undo", KeyboardShortcut('Z', WXK_CONTROL));
            editMenu.addUnmodifiableActionItem(wxID_REDO, Action::Context_Any, "Redo", KeyboardShortcut('Z', WXK_CONTROL, WXK_SHIFT));
            editMenu.addSeparator();
            editMenu.addUnmodifiableActionItem(wxID_CUT, Action::Context_Any, "Cut", KeyboardShortcut('X', WXK_CONTROL));
            editMenu.addUnmodifiableActionItem(wxID_COPY, Action::Context_Any, "Copy", KeyboardShortcut('C', WXK_CONTROL));
            editMenu.addUnmodifiableActionItem(wxID_PASTE, Action::Context_Any, "Paste", KeyboardShortcut('V', WXK_CONTROL));
            editMenu.addUnmodifiableActionItem(CommandIds::Menu::EditPasteAtOriginalPosition, Action::Context_Any, "Paste at Original Position", KeyboardShortcut('V', WXK_CONTROL, WXK_SHIFT));
            
            /*
#ifdef __APPLE__
            editMenu.addModifiableActionItem(wxID_DELETE, Action::Context_ObjectSelection, "Delete", KeyboardShortcut(WXK_BACK));
#else
            editMenu.addModifiableActionItem(wxID_DELETE, Action::Context_ObjectSelection, "Delete", KeyboardShortcut(WXK_DELETE));
#endif
             */
            
            editMenu.addSeparator();
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectAll, Action::Context_Any, "Select All", KeyboardShortcut('A', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectSiblings, Action::Context_Any, "Select Siblings", KeyboardShortcut('A', WXK_CONTROL, WXK_ALT));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectTouching, Action::Context_Any, "Select Touching", KeyboardShortcut('T', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectContained, Action::Context_Any, "Select Contained", KeyboardShortcut('T', WXK_CONTROL, WXK_ALT));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectByFilePosition, Action::Context_Any, "Select by Line Number");
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectNone, Action::Context_Any, "Select None", KeyboardShortcut('A', WXK_CONTROL, WXK_SHIFT));
            editMenu.addSeparator();
            /*
             editMenu.addModifiableActionItem(CommandIds::Menu::EditHideSelected, Action::Context_Any, "Hide Selected", KeyboardShortcut('H', WXK_CONTROL));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditHideUnselected, Action::Context_Any, "Hide Unselected", KeyboardShortcut('H', WXK_CONTROL, WXK_ALT));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditUnhideAll, Action::Context_Any, "Unhide All", KeyboardShortcut('H', WXK_CONTROL, WXK_SHIFT));
             editMenu.addSeparator();
             editMenu.addModifiableActionItem(CommandIds::Menu::EditLockSelected, Action::Context_Any, "Lock Selected", KeyboardShortcut('L', WXK_CONTROL));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditLockUnselected, Action::Context_Any, "Lock Unselected", KeyboardShortcut('L', WXK_CONTROL, WXK_ALT));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditUnlockAll, Action::Context_Any, "Unlock All", KeyboardShortcut('L', WXK_CONTROL, WXK_SHIFT));
             editMenu.addSeparator();
             */
            
            /*
             MultiMenu& actionMenu = editMenu.addMultiMenu(CommandIds::Menu::EditActions, "Actions");
             
             Menu& faceActionMenu = actionMenu.addMenu(CommandIds::Menu::EditFaceActions, "Faces");
             #ifdef __linux__ // unmodified cursor keys are not allowed as a menu accelerator on GTK
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesUp, Action::Context_FaceSelection, "Move Up", KeyboardShortcut(WXK_UP, WXK_SHIFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesDown, Action::Context_FaceSelection, "Move Down", KeyboardShortcut(WXK_DOWN, WXK_SHIFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesLeft, Action::Context_FaceSelection, "Move Left", KeyboardShortcut(WXK_LEFT, WXK_SHIFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesRight, Action::Context_FaceSelection, "Move Right", KeyboardShortcut(WXK_RIGHT, WXK_SHIFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCW, Action::Context_FaceSelection, "Rotate Clockwise by 15", KeyboardShortcut(WXK_PAGEUP, WXK_SHIFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCW, Action::Context_FaceSelection, "Rotate Counter-clockwise by 15", KeyboardShortcut(WXK_PAGEDOWN, WXK_SHIFT));
             #else
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesUp, Action::Context_FaceSelection, "Move Up", KeyboardShortcut(WXK_UP));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesDown, Action::Context_FaceSelection, "Move Down", KeyboardShortcut(WXK_DOWN));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesLeft, Action::Context_FaceSelection, "Move Left", KeyboardShortcut(WXK_LEFT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesRight, Action::Context_FaceSelection, "Move Right", KeyboardShortcut(WXK_RIGHT));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCW, Action::Context_FaceSelection, "Rotate Clockwise by 15", KeyboardShortcut(WXK_PAGEUP));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCCW, Action::Context_FaceSelection, "Rotate Counter-clockwise by 15", KeyboardShortcut(WXK_PAGEDOWN));
             #endif
             faceActionMenu.addSeparator();
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesUpFine, Action::Context_FaceSelection, "Move Up by 1", KeyboardShortcut(WXK_UP, WXK_CONTROL));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesDownFine, Action::Context_FaceSelection, "Move Down by 1", KeyboardShortcut(WXK_DOWN, WXK_CONTROL));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesLeftFine, Action::Context_FaceSelection, "Move Left by 1", KeyboardShortcut(WXK_LEFT, WXK_CONTROL));
             faceActionMenu.addActionItem(CommandIds::Menu::EditMoveTexturesRightFine, Action::Context_FaceSelection, "Move Right by 1", KeyboardShortcut(WXK_RIGHT, WXK_CONTROL));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCWFine, Action::Context_FaceSelection, "Rotate Clockwise by 1", KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL));
             faceActionMenu.addActionItem(CommandIds::Menu::EditRotateTexturesCCWFine, Action::Context_FaceSelection, "Rotate Counter-clockwise by 1", KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL));
             
             Menu& objectActionMenu = actionMenu.addMenu(CommandIds::Menu::EditObjectActions, "Objects");
             objectActionMenu.addActionItem(CommandIds::Menu::EditRollObjectsCW, Action::Context_ObjectSelection, "Rotate Clockwise by 90", KeyboardShortcut(WXK_UP, WXK_ALT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditRollObjectsCCW, Action::Context_ObjectSelection, "Rotate Counter-clockwise by 90", KeyboardShortcut(WXK_DOWN, WXK_ALT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditYawObjectsCW, Action::Context_ObjectSelection, "Rotate Left by 90", KeyboardShortcut(WXK_LEFT, WXK_ALT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditYawObjectsCCW, Action::Context_ObjectSelection, "Rotate Right by 90", KeyboardShortcut(WXK_RIGHT, WXK_ALT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditPitchObjectsCW, Action::Context_ObjectSelection, "Rotate Up by 90", KeyboardShortcut(WXK_PAGEUP, WXK_ALT));
             objectActionMenu.addActionItem(CommandIds::Menu::EditPitchObjectsCCW, Action::Context_ObjectSelection, "Rotate Down by 90", KeyboardShortcut(WXK_PAGEDOWN, WXK_ALT));
             objectActionMenu.addSeparator();
             objectActionMenu.addActionItem(CommandIds::Menu::EditFlipObjectsHorizontally, Action::Context_ObjectSelection, "Flip Horizontally", KeyboardShortcut('F', WXK_CONTROL));
             objectActionMenu.addActionItem(CommandIds::Menu::EditFlipObjectsVertically, Action::Context_ObjectSelection, "Flip Vertically", KeyboardShortcut('F', WXK_CONTROL, WXK_ALT));
             objectActionMenu.addSeparator();
             #ifdef __linux__ // tab is not allowed as a menu accelerator on GTK
             MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(CommandIds::Menu::EditToggleMovementRestriction, Action::Context_ObjectSelection | Action::Context_VertexTool, "Toggle Movement Axis", KeyboardShortcut('X'));
             #else
             MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(CommandIds::Menu::EditToggleMovementRestriction, Action::Context_ObjectSelection | Action::Context_VertexTool, "Toggle Movement Axis", KeyboardShortcut(WXK_TAB));
             #endif
             
             Menu& vertexActionMenu = actionMenu.addMenu(CommandIds::Menu::EditVertexActions, "Vertices");
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesForward, Action::Context_VertexTool, "Move Forward", KeyboardShortcut(WXK_UP));
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesBackward, Action::Context_VertexTool, "Move Backward", KeyboardShortcut(WXK_DOWN));
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesLeft, Action::Context_VertexTool, "Move Left", KeyboardShortcut(WXK_LEFT));
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesRight, Action::Context_VertexTool, "Move Right", KeyboardShortcut(WXK_RIGHT));
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesUp, Action::Context_VertexTool, "Move Up", KeyboardShortcut(WXK_PAGEUP));
             vertexActionMenu.addActionItem(CommandIds::Menu::EditMoveVerticesDown, Action::Context_VertexTool, "Move Down", KeyboardShortcut(WXK_PAGEDOWN));
             vertexActionMenu.addSeparator();
             vertexActionMenu.addActionItem(CommandIds::Menu::EditSnapVertices, Action::Context_VertexTool, "Snap Vertices to Grid");
             vertexActionMenu.addSeparator();
             vertexActionMenu.addItem(toggleAxisItem);
             
             Menu& clipActionMenu = actionMenu.addMenu(CommandIds::Menu::EditClipActions, "Clip Tool");
             clipActionMenu.addActionItem(CommandIds::Menu::EditToggleClipSide, Action::Context_ClipTool, "Toggle Clip Side", KeyboardShortcut(WXK_RETURN, WXK_CONTROL));
             clipActionMenu.addActionItem(CommandIds::Menu::EditPerformClip, Action::Context_ClipTool, "Perform Clip", KeyboardShortcut(WXK_RETURN));
             #ifdef __APPLE__
             clipActionMenu.addActionItem(CommandIds::Menu::EditDeleteLastClipPoint, Action::Context_ClipTool, "Delete Last Clip Point", KeyboardShortcut(WXK_BACK));
             #else
             clipActionMenu.addActionItem(CommandIds::Menu::EditDeleteLastClipPoint, Action::Context_ClipTool, "Delete Last Clip Point", KeyboardShortcut(WXK_DELETE));
             #endif
             
             editMenu.addSeparator();
             */
            
            editMenu.addModifiableCheckItem(CommandIds::Menu::EditToggleTextureLock, Action::Context_Any, "Texture Lock");
            
            Menu& viewMenu = m_menu->addMenu(wxID_ANY, "View");
            Menu& gridMenu = viewMenu.addMenu(wxID_ANY, "Grid");
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleShowGrid, Action::Context_Any, "Show Grid", KeyboardShortcut('G', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleSnapToGrid, Action::Context_Any, "Snap to Grid", KeyboardShortcut('G', WXK_CONTROL, WXK_SHIFT));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewIncGridSize, Action::Context_Any, "Increase Grid Size", KeyboardShortcut('+', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewDecGridSize, Action::Context_Any, "Decrease Grid Size", KeyboardShortcut('-', WXK_CONTROL));
            gridMenu.addSeparator();
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize1, Action::Context_Any, "Set Grid Size 1", KeyboardShortcut('1', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize2, Action::Context_Any, "Set Grid Size 2", KeyboardShortcut('2', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize4, Action::Context_Any, "Set Grid Size 4", KeyboardShortcut('3', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize8, Action::Context_Any, "Set Grid Size 8", KeyboardShortcut('4', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize16, Action::Context_Any, "Set Grid Size 16", KeyboardShortcut('5', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize32, Action::Context_Any, "Set Grid Size 32", KeyboardShortcut('6', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize64, Action::Context_Any, "Set Grid Size 64", KeyboardShortcut('7', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize128, Action::Context_Any, "Set Grid Size 128", KeyboardShortcut('8', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize256, Action::Context_Any, "Set Grid Size 256", KeyboardShortcut('9', WXK_CONTROL));
            
            Menu& cameraMenu = viewMenu.addMenu(wxID_ANY, "Camera");
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToNextPoint, Action::Context_Any, "Move to Next Point", KeyboardShortcut('+', WXK_SHIFT));
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPreviousPoint, Action::Context_Any, "Move to Previous Point", KeyboardShortcut('-', WXK_SHIFT));
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewCenterCameraOnSelection, Action::Context_Any, "Center on Selection", KeyboardShortcut('C', WXK_CONTROL, WXK_SHIFT));
            
            /*
             cameraMenu.addSeparator();
             cameraMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleCameraFlyMode, Action::Context_Any, "Fly Mode", KeyboardShortcut('F'));
             */
            
            viewMenu.addSeparator();
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToMapInspector, Action::Context_Any, "Switch to Map Inspector", KeyboardShortcut('1', WXK_SHIFT));
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToEntityInspector, Action::Context_Any, "Switch to Entity Inspector", KeyboardShortcut('2', WXK_SHIFT));
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToFaceInspector, Action::Context_Any, "Switch to Face Inspector", KeyboardShortcut('3', WXK_SHIFT));
            
            Menu& helpMenu = m_menu->addMenu(wxID_ANY, "Help");
            helpMenu.addUnmodifiableActionItem(CommandIds::Menu::HelpShowHelp, Action::Context_Any, "TrenchBroom Help");
            
#ifdef __APPLE__
            // these won't show up in the app menu if we don't add them here
            fileMenu.addUnmodifiableActionItem(wxID_ABOUT, Action::Context_Any, "About TrenchBroom");
            fileMenu.addUnmodifiableActionItem(wxID_PREFERENCES, Action::Context_Any, "Preferences...\tCtrl-,");
            fileMenu.addUnmodifiableActionItem(wxID_EXIT, Action::Context_Any, "Exit");
#else
            viewMenu.addSeparator();
            viewMenu.addUnmodifiableActionItem(wxID_PREFERENCES, Action::Context_Any, "Preferences...");
            
            helpMenu.addSeparator();
            helpMenu.addUnmodifiableActionItem(wxID_ABOUT, Action::Context_Any, "About TrenchBroom");
#endif
        }
    }
}
