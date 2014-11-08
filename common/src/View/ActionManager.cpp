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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/Path.h"
#include "View/CommandIds.h"
#include "View/Menu.h"
#include "View/MenuAction.h"
#include "View/ViewShortcut.h"

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
        
        const MenuAction* ActionManager::findMenuAction(const int id) const {
            return m_menu->findAction(id);
        }
        
        Menu& ActionManager::getMenu() {
            return *m_menu;
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

        bool ActionManager::isMenuShortcutPreference(const IO::Path& path) const {
            return !path.isEmpty() && path.firstComponent().asString() == "Menu";
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
        
        wxAcceleratorTable ActionManager::createViewAcceleratorTable(const ActionContext context, const ActionView view) const {
            typedef std::vector<wxAcceleratorEntry> EntryList;
            EntryList tableEntries;
            
            ViewShortcut::List::const_iterator it, end;
            PreferenceManager& prefs = PreferenceManager::instance();
            const ViewShortcut::List& shortcuts = prefs.get(Preferences::ViewShortcuts);
            for (it = shortcuts.begin(), end = shortcuts.end(); it != end; ++it) {
                const ViewShortcut& shortcut = *it;
                if (shortcut.appliesToContext(context))
                    tableEntries.push_back(shortcut.acceleratorEntry(view));
            }
            
            return wxAcceleratorTable(static_cast<int>(tableEntries.size()), &tableEntries.front());
        }

        void ActionManager::resetShortcutsToDefaults() {
            m_menu->resetShortcutsToDefaults();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(Preferences::ViewShortcuts);
        }

        ActionManager::ActionManager() {
            createMenu();
        }

        void ActionManager::createMenu() {
            m_menu = new Menu("Menu");
            
            Menu& fileMenu = m_menu->addMenu(wxID_ANY, "File");
            fileMenu.addUnmodifiableActionItem(wxID_NEW, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_OPEN, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            fileMenu.addMenu(CommandIds::Menu::FileOpenRecent, "Open Recent");
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_SAVE, "Save", KeyboardShortcut('S', WXK_CONTROL));
            fileMenu.addUnmodifiableActionItem(wxID_SAVEAS, "Save as...", KeyboardShortcut('S', WXK_SHIFT, WXK_CONTROL));
            fileMenu.addSeparator();
            fileMenu.addModifiableActionItem(CommandIds::Menu::FileLoadPointFile, "Load Point File");
            fileMenu.addModifiableActionItem(CommandIds::Menu::FileUnloadPointFile, "Unload Point File");
            fileMenu.addSeparator();
            fileMenu.addUnmodifiableActionItem(wxID_CLOSE, "Close", KeyboardShortcut('W', WXK_CONTROL));
            
            Menu& editMenu = m_menu->addMenu(wxID_ANY, "Edit");
            editMenu.addModifiableActionItem(wxID_UNDO, "Undo", KeyboardShortcut('Z', WXK_CONTROL));
            editMenu.addModifiableActionItem(wxID_REDO, "Redo", KeyboardShortcut('Z', WXK_CONTROL, WXK_SHIFT));
            editMenu.addSeparator();
            editMenu.addModifiableActionItem(CommandIds::Menu::EditRepeat, "Repeat", KeyboardShortcut('R', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditClearRepeat, "Clear Repeatable Commands", KeyboardShortcut('R', WXK_CONTROL, WXK_SHIFT));
            editMenu.addSeparator();
            editMenu.addModifiableActionItem(wxID_CUT, "Cut", KeyboardShortcut('X', WXK_CONTROL));
            editMenu.addModifiableActionItem(wxID_COPY, "Copy", KeyboardShortcut('C', WXK_CONTROL));
            editMenu.addModifiableActionItem(wxID_PASTE, "Paste", KeyboardShortcut('V', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditPasteAtOriginalPosition, "Paste at Original Position", KeyboardShortcut('V', WXK_CONTROL, WXK_SHIFT));
            
            editMenu.addSeparator();
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectAll, "Select All", KeyboardShortcut('A', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectSiblings, "Select Siblings", KeyboardShortcut('A', WXK_CONTROL, WXK_ALT));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectTouching, "Select Touching", KeyboardShortcut('T', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectInside, "Select Inside", KeyboardShortcut('I', WXK_CONTROL));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectByFilePosition, "Select by Line Number");
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSelectNone, "Select None", KeyboardShortcut('A', WXK_CONTROL, WXK_SHIFT));
            editMenu.addSeparator();
            editMenu.addModifiableCheckItem(CommandIds::Menu::EditToggleTextureLock, "Texture Lock");
            editMenu.addSeparator();
            editMenu.addModifiableActionItem(CommandIds::Menu::EditSnapVertices, "Snap Vertices", KeyboardShortcut('V', WXK_SHIFT, WXK_ALT));
            editMenu.addModifiableActionItem(CommandIds::Menu::EditReplaceTexture, "Replace Texture...");

            /*
             editMenu.addModifiableActionItem(CommandIds::Menu::EditHideSelected, "Hide Selected", KeyboardShortcut('H', WXK_CONTROL));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditHideUnselected, "Hide Unselected", KeyboardShortcut('H', WXK_CONTROL, WXK_ALT));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditUnhideAll, "Unhide All", KeyboardShortcut('H', WXK_CONTROL, WXK_SHIFT));
             editMenu.addSeparator();
             editMenu.addModifiableActionItem(CommandIds::Menu::EditLockSelected, "Lock Selected", KeyboardShortcut('L', WXK_CONTROL));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditLockUnselected, "Lock Unselected", KeyboardShortcut('L', WXK_CONTROL, WXK_ALT));
             editMenu.addModifiableActionItem(CommandIds::Menu::EditUnlockAll, "Unlock All", KeyboardShortcut('L', WXK_CONTROL, WXK_SHIFT));
             editMenu.addSeparator();
             */
            
            Menu& viewMenu = m_menu->addMenu(wxID_ANY, "View");
            Menu& gridMenu = viewMenu.addMenu(wxID_ANY, "Grid");
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleShowGrid, "Show Grid", KeyboardShortcut('G', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleSnapToGrid, "Snap to Grid", KeyboardShortcut('G', WXK_CONTROL, WXK_SHIFT));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewIncGridSize, "Increase Grid Size", KeyboardShortcut('+', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewDecGridSize, "Decrease Grid Size", KeyboardShortcut('-', WXK_CONTROL));
            gridMenu.addSeparator();
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize1, "Set Grid Size 1", KeyboardShortcut('1', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize2, "Set Grid Size 2", KeyboardShortcut('2', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize4, "Set Grid Size 4", KeyboardShortcut('3', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize8, "Set Grid Size 8", KeyboardShortcut('4', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize16, "Set Grid Size 16", KeyboardShortcut('5', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize32, "Set Grid Size 32", KeyboardShortcut('6', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize64, "Set Grid Size 64", KeyboardShortcut('7', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize128, "Set Grid Size 128", KeyboardShortcut('8', WXK_CONTROL));
            gridMenu.addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize256, "Set Grid Size 256", KeyboardShortcut('9', WXK_CONTROL));
            
            Menu& cameraMenu = viewMenu.addMenu(wxID_ANY, "Camera");
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToNextPoint, "Move to Next Point", KeyboardShortcut('+', WXK_SHIFT, WXK_CONTROL));
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPreviousPoint, "Move to Previous Point", KeyboardShortcut('-', WXK_SHIFT, WXK_CONTROL));
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewCenterCameraOnSelection, "Center on Selection", KeyboardShortcut('C', WXK_CONTROL, WXK_SHIFT));
            cameraMenu.addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPosition, "Move Camera to...");
            
            /*
             cameraMenu.addSeparator();
             cameraMenu.addModifiableCheckItem(CommandIds::Menu::ViewToggleCameraFlyMode, "Fly Mode", KeyboardShortcut('F'));
             */
            
            viewMenu.addSeparator();
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToMapInspector, "Switch to Map Inspector", KeyboardShortcut('1', WXK_SHIFT, WXK_ALT));
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToEntityInspector, "Switch to Entity Inspector", KeyboardShortcut('2', WXK_SHIFT, WXK_ALT));
            viewMenu.addModifiableActionItem(CommandIds::Menu::ViewSwitchToFaceInspector, "Switch to Face Inspector", KeyboardShortcut('3', WXK_SHIFT, WXK_ALT));
            
            Menu& helpMenu = m_menu->addMenu(wxID_ANY, "Help");
            helpMenu.addUnmodifiableActionItem(CommandIds::Menu::HelpShowHelp, "TrenchBroom Help");
            
#ifdef __APPLE__
            // these won't show up in the app menu if we don't add them here
            fileMenu.addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
            fileMenu.addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...", KeyboardShortcut(',', WXK_CONTROL));
            fileMenu.addUnmodifiableActionItem(wxID_EXIT, "Exit");
#else
            viewMenu.addSeparator();
            viewMenu.addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...");
            
            helpMenu.addSeparator();
            helpMenu.addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
#endif
        }
    }
}
