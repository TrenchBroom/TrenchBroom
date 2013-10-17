/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Menu.h"

#include "PreferenceManager.h"
#include "View/CommandIds.h"

#include <wx/menu.h>
#include <wx/menuitem.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        MenuItem::MenuItem(const MenuItemType type, MenuItemParent* parent) :
        m_type(type),
        m_parent(parent) {}
        
        MenuItem::~MenuItem() {}
        
        MenuItem::MenuItemType MenuItem::type() const {
            return m_type;
        }
        
        const MenuItemParent* MenuItem::parent() const {
            return m_parent;
        }

        const KeyboardShortcut* MenuItem::shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
            return NULL;
        }

        TextMenuItem::TextMenuItem(const MenuItemType type, MenuItemParent* parent) :
        MenuItem(type, parent) {}
        
        TextMenuItem::~TextMenuItem() {}
        
        String ShortcutMenuItem::path() const {
            StringList components;
            components.push_back(text());
            const MenuItemParent* p = parent();
            while (p != NULL) {
                if (!p->text().empty())
                    components.push_back(p->text());
                p = p->parent();
            }
            components.push_back("Menu");
            std::reverse(components.begin(), components.end());
            return StringUtils::join(components, "/");
        }
        
        ShortcutMenuItem::ShortcutMenuItem(MenuItemType type, const KeyboardShortcut& shortcut, MenuItemParent* parent) :
        TextMenuItem(type, parent),
        m_shortcut(shortcut),
        m_preference(path(), m_shortcut) {
            assert(type == MITAction || type == MITCheck);
            PreferenceManager& prefs = PreferenceManager::instance();
            const String p = path();
            Preference<KeyboardShortcut> preference(p, m_shortcut);
            prefs.getKeyboardShortcut(preference);
        }
        
        ShortcutMenuItem::~ShortcutMenuItem() {}

        const String& ShortcutMenuItem::text() const {
            return m_shortcut.text();
        }

        const String ShortcutMenuItem::longText() const {
            StringList components;
            components.push_back(shortcut().text());
            const MenuItemParent* p = parent();
            while (p != NULL) {
                if (!p->text().empty())
                    components.push_back(p->text());
                p = p->parent();
            }
            std::reverse(components.begin(), components.end());
            return StringUtils::join(components, " > ");
        }
        
        const KeyboardShortcut& ShortcutMenuItem::shortcut() const {
            return m_shortcut;
        }
        
        void ShortcutMenuItem::setShortcut(const KeyboardShortcut& shortcut) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            const String p = path();
            prefs.setKeyboardShortcut(m_preference, shortcut);
            m_shortcut = m_preference.value();
        }
        
        const KeyboardShortcut* ShortcutMenuItem::shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
            if (m_shortcut.matches(key, modifierKey1, modifierKey2, modifierKey3))
                return &m_shortcut;
            return NULL;
        }
        
        const MenuItemParent::List& MenuItemParent::items() const {
            return m_items;
        }
        
        void MenuItemParent::addItem(MenuItem::Ptr item) {
            m_items.push_back(item);
        }
        
        const String& MenuItemParent::text() const {
            return m_text;
        }
        
        int MenuItemParent::menuId() const {
            return m_menuId;
        }

        const KeyboardShortcut* MenuItemParent::shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
            List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const MenuItem& item = **it;
                const KeyboardShortcut* shortcut = item.shortcutByKeys(key, modifierKey1, modifierKey2, modifierKey3);
                if (shortcut != NULL)
                    return shortcut;
            }
            return NULL;
        }
        
        MenuItemParent::MenuItemParent(MenuItemType type, const String& text, MenuItemParent* parent, int menuId) :
        TextMenuItem(type, parent),
        m_text(text),
        m_menuId(menuId) {}

        MenuItemParent::~MenuItemParent() {}

        MultiMenuSelector::~MultiMenuSelector() {}

        const Menu* NullMenuSelector::select(const MultiMenu& multiMenu) const {
            return NULL;
        }

        MultiMenu::MultiMenu(const String& text, MenuItemParent* parent, int menuId) :
        MenuItemParent(MITMultiMenu, text, parent, menuId) {
            assert(parent != NULL);
        }

        Menu& MultiMenu::addMenu(const String& text, const int menuId) {
            Menu* menu = new Menu(text, this, menuId);
            addItem(MenuItem::Ptr(menu));
            return *menu;
        }
        
        const Menu* MultiMenu::menuById(const int menuId) const {
            const MenuItem::List& myItems = items();
            List::const_iterator it, end;
            for (it = myItems.begin(), end = myItems.end(); it != end; ++it) {
                const Menu* menu = static_cast<Menu*>(it->get());
                if (menu->menuId() == menuId)
                    return menu;
            }
            return NULL;
        }
        
        const Menu* MultiMenu::selectMenu(const MultiMenuSelector& selector) const {
            return selector.select(*this);
        }

        const String FileMenu = "File";
        const String EditMenu = "Edit";
        const String ViewMenu = "View";

        Menu::Menu(const String& text, MenuItemParent* parent, const int menuId) :
        MenuItemParent(MITMenu, text, parent, menuId) {}
        
        Menu::~Menu() {}

        MenuItem::Ptr Menu::addActionItem(const KeyboardShortcut& shortcut) {
            MenuItem::Ptr item = MenuItem::Ptr(new ShortcutMenuItem(MenuItem::MITAction, shortcut, this));
            addItem(item);
            return item;
        }
        
        MenuItem::Ptr Menu::addCheckItem(const KeyboardShortcut& shortcut) {
            MenuItem::Ptr item = MenuItem::Ptr(new ShortcutMenuItem(MenuItem::MITCheck, shortcut, this));
            addItem(item);
            return item;
        }
        
        void Menu::addSeparator() {
            MenuItem* item = new MenuItem(MenuItem::MITSeparator, this);
            addItem(MenuItem::Ptr(item));
        }
        
        Menu& Menu::addMenu(const String& text, int menuId) {
            Menu* menu = new Menu(text, this, menuId);
            addItem(Menu::Ptr(menu));
            return *menu;
        }
        
        MultiMenu& Menu::addMultiMenu(const String& text, int menuId) {
            MultiMenu* menu = new MultiMenu(text, this, menuId);
            addItem(MultiMenu::Ptr(menu));
            return *menu;
        }

        wxMenuBar* Menu::createMenuBar(const MultiMenuSelector& selector, const bool showModifiers) {
            wxMenu* fileMenu = createMenu(FileMenu, selector, showModifiers);
            wxMenu* editMenu = createMenu(EditMenu, selector, showModifiers);
            wxMenu* viewMenu = createMenu(ViewMenu, selector, showModifiers);

            wxMenu* helpMenu = new wxMenu();
            helpMenu->Append(CommandIds::Menu::HelpShowHelp, wxT("TrenchBroom Help"));

#ifdef __APPLE__
            // these won't show up in the app menu if we don't add them here
            fileMenu->Append(wxID_ABOUT, wxT("About"));
            fileMenu->Append(wxID_PREFERENCES, wxT("Preferences...\tCtrl-,"));
            fileMenu->Append(wxID_EXIT, wxT("Exit"));
#endif
            
            wxMenuBar* menuBar = new wxMenuBar();
            menuBar->Append(fileMenu, wxT("File"));
            menuBar->Append(editMenu, wxT("Edit"));
            menuBar->Append(viewMenu, wxT("View"));
            menuBar->Append(helpMenu, wxT("Help"));
            return menuBar;
        }

        wxMenu* Menu::findRecentDocumentsMenu(const wxMenuBar* menuBar) {
            const size_t fileMenuIndex = menuBar->FindMenu("File");
            const wxMenu* fileMenu = menuBar->GetMenu(fileMenuIndex);
            if (fileMenu == NULL)
                return NULL;
            const wxMenuItem* recentDocumentsItem = fileMenu->FindItem(CommandIds::Menu::FileOpenRecent);
            if (recentDocumentsItem == NULL)
                return NULL;
            return recentDocumentsItem->GetSubMenu();
        }
        
        const KeyboardShortcut& Menu::undoShortcut() {
            static const KeyboardShortcut shortcut(wxID_UNDO, WXK_CONTROL, 'Z', KeyboardShortcut::SCAny, "Undo");
            return shortcut;
        }
        
        const KeyboardShortcut& Menu::redoShortcut() {
            static const KeyboardShortcut shortcut(wxID_REDO, WXK_CONTROL, WXK_SHIFT, 'Z', KeyboardShortcut::SCAny, "Redo");
            return shortcut;
        }

        
        wxMenu* Menu::createMenu(const String& name, const MultiMenuSelector& selector, const bool showModifiers) {
            const Menu& menu = getMenu(name);
            return createMenu(menu, selector, showModifiers);
        }

        wxMenu* Menu::createMenu(const Menu& menu, const MultiMenuSelector& selector, const bool showModifiers) {
            wxMenu* result = new wxMenu();
            
            const Menu::List& items = menu.items();
            Menu::List::const_iterator it, end;
            for (it = items.begin(), end = items.end(); it != end; ++it) {
                const MenuItem& item = **it;
                switch (item.type()) {
                    case MenuItem::MITAction: {
                        const ShortcutMenuItem& shortcutItem = static_cast<const ShortcutMenuItem&>(item);
                        const KeyboardShortcut& shortcut = shortcutItem.shortcut();
                        if (showModifiers || shortcut.alwaysShowModifier())
                            result->Append(shortcut.commandId(), shortcut.menuText());
                        else
                            result->Append(shortcut.commandId(), shortcut.text());
                        break;
                    }
                    case MenuItem::MITCheck: {
                        const ShortcutMenuItem& shortcutItem = static_cast<const ShortcutMenuItem&>(item);
                        const KeyboardShortcut& shortcut = shortcutItem.shortcut();
                        if (showModifiers || shortcut.alwaysShowModifier())
                            result->AppendCheckItem(shortcut.commandId(), shortcut.menuText());
                        else
                            result->AppendCheckItem(shortcut.commandId(), shortcut.text());
                        break;
                    }
                    case MenuItem::MITMenu: {
                        const Menu& subMenu = static_cast<const Menu&>(item);
                        wxMenuItem* wxSubMenuItem = new wxMenuItem(result, subMenu.menuId(), subMenu.text());
                        wxSubMenuItem->SetSubMenu(createMenu(subMenu, selector, showModifiers));
                        result->Append(wxSubMenuItem);
                        break;
                    }
                    case MenuItem::MITMultiMenu: {
                        const MultiMenu& multiMenu = static_cast<const MultiMenu&>(item);
                        const Menu* multiMenuItem = multiMenu.selectMenu(selector);
                        if (multiMenuItem != NULL) {
                            wxMenuItem* wxSubMenuItem = new wxMenuItem(result, multiMenu.menuId(), multiMenu.text());
                            wxSubMenuItem->SetSubMenu(createMenu(*multiMenuItem, selector, showModifiers));
                            result->Append(wxSubMenuItem);
                        } else {
                            result->Append(multiMenu.menuId(), multiMenu.text());
                        }
                        break;
                    }
                    case MenuItem::MITSeparator: {
                        result->AppendSeparator();
                        break;
                    }
                }
            }
            
            return result;
        }

        const Menu& Menu::getMenu(const String& name) {
            static const Menu::MenuMap menus = buildMenus();
            MenuMap::const_iterator it = menus.find(name);
            assert(it != menus.end());
            return static_cast<const Menu&>(*(it->second.get()));
        }
        
        const Menu::MenuMap Menu::buildMenus() {
            Menu::MenuMap menus;
            
            Menu* fileMenu = new Menu("File");
            menus[FileMenu] = Menu::Ptr(fileMenu);
            
            fileMenu->addActionItem(KeyboardShortcut(wxID_NEW, WXK_CONTROL, 'N', KeyboardShortcut::SCAny, "New"));
            fileMenu->addSeparator();
            fileMenu->addActionItem(KeyboardShortcut(wxID_OPEN, WXK_CONTROL, 'O', KeyboardShortcut::SCAny, "Open..."));
            fileMenu->addMenu("Open Recent", CommandIds::Menu::FileOpenRecent);
            fileMenu->addSeparator();
            fileMenu->addActionItem(KeyboardShortcut(wxID_SAVE, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save"));
            fileMenu->addActionItem(KeyboardShortcut(wxID_SAVEAS, WXK_SHIFT, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save as..."));
            fileMenu->addSeparator();
            fileMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::FileLoadPointFile, KeyboardShortcut::SCAny, "Load Point File..."));
            fileMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::FileUnloadPointFile, KeyboardShortcut::SCAny, "Unload Point File"));
            fileMenu->addSeparator();
            fileMenu->addActionItem(KeyboardShortcut(wxID_CLOSE, WXK_CONTROL, 'W', KeyboardShortcut::SCAny, "Close"));
            
            Menu* editMenu = new Menu("Edit");
            menus[EditMenu] = Menu::Ptr(editMenu);
            
            editMenu->addActionItem(undoShortcut());
            editMenu->addActionItem(redoShortcut());
            editMenu->addSeparator();
            editMenu->addActionItem(KeyboardShortcut(wxID_CUT, WXK_CONTROL, 'X', KeyboardShortcut::SCAny, "Cut"));
            editMenu->addActionItem(KeyboardShortcut(wxID_COPY, WXK_CONTROL, 'C', KeyboardShortcut::SCAny, "Copy"));
            editMenu->addActionItem(KeyboardShortcut(wxID_PASTE, WXK_CONTROL, 'V', KeyboardShortcut::SCAny, "Paste"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditPasteAtOriginalPosition, WXK_CONTROL, WXK_SHIFT, 'V', KeyboardShortcut::SCAny, "Paste at Original Position"));
#ifdef __APPLE__
            editMenu->addActionItem(KeyboardShortcut(wxID_DELETE, WXK_BACK, KeyboardShortcut::SCAny, "Delete"));
#else
            editMenu->addActionItem(KeyboardShortcut(wxID_DELETE, WXK_DELETE, KeyboardShortcut::SCAny, "Delete"));
#endif
            editMenu->addSeparator();
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectAll, WXK_CONTROL, 'A', KeyboardShortcut::SCAny, "Select All"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectSiblings, WXK_CONTROL, WXK_ALT, 'A', KeyboardShortcut::SCAny, "Select Siblings"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectTouching, WXK_CONTROL, 'T', KeyboardShortcut::SCAny, "Select Touching"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectContained, WXK_CONTROL, WXK_ALT, 'T', KeyboardShortcut::SCAny, "Select Contained"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectByFilePosition, KeyboardShortcut::SCAny, "Select by Line Number"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditSelectNone, WXK_CONTROL, WXK_SHIFT, 'A', KeyboardShortcut::SCAny, "Select None"));
            editMenu->addSeparator();
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditHideSelected, WXK_CONTROL, 'H', KeyboardShortcut::SCAny, "Hide Selected"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditHideUnselected, WXK_CONTROL, WXK_ALT, 'H', KeyboardShortcut::SCAny, "Hide Unselected"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditUnhideAll, WXK_CONTROL, WXK_SHIFT, 'H', KeyboardShortcut::SCAny, "Unhide All"));
            editMenu->addSeparator();
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditLockSelected, WXK_CONTROL, 'L', KeyboardShortcut::SCAny, "Lock Selected"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditLockUnselected, WXK_CONTROL, WXK_ALT, 'L', KeyboardShortcut::SCAny, "Lock Unselected"));
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditUnlockAll, WXK_CONTROL, WXK_SHIFT, 'L', KeyboardShortcut::SCAny, "Unlock All"));
            editMenu->addSeparator();
            
            Menu& toolMenu = editMenu->addMenu("Tools");
            toolMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::EditToggleClipTool, 'C', KeyboardShortcut::SCAny, "Clip Tool"));
            toolMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::EditToggleVertexTool, 'V', KeyboardShortcut::SCAny, "Vertex Tool"));
            toolMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::EditToggleRotateObjectsTool, 'R', KeyboardShortcut::SCAny, "Rotate Tool"));
            
            MultiMenu& actionMenu = editMenu->addMultiMenu("Actions", CommandIds::Menu::EditActions);
            
            Menu& faceActionMenu = actionMenu.addMenu("Faces", CommandIds::Menu::EditFaceActions);
#ifdef __linux__ // unmodified cursor keys are not allowed as a menu accelerator on GTK
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesUp, WXK_SHIFT, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesDown, WXK_SHIFT, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesLeft, WXK_SHIFT, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesRight, WXK_SHIFT, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_SHIFT, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_SHIFT, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
#else
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesUp, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesDown, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesLeft, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesRight, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
#endif
            faceActionMenu.addSeparator();
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesUp, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCTextures, "Move Up by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesDown, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveTexturesRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 1"));
            faceActionMenu.addSeparator();
            faceActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditPrintFilePositions, KeyboardShortcut::SCTextures, "Print Line Numbers"));
            
            Menu& objectActionMenu = actionMenu.addMenu("Objects", CommandIds::Menu::EditObjectActions);
#ifdef __linux__ // unmodified cursor keys are not allowed as a menu accelerator on GTK
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsForward, WXK_SHIFT, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsBackward, WXK_SHIFT, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsLeft, WXK_SHIFT, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsRight, WXK_SHIFT, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsUp, WXK_SHIFT, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsDown, WXK_SHIFT, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
#else
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsForward, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsBackward, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsLeft, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsRight, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsUp, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveObjectsDown, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
#endif
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsForward, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCObjects, "Duplicate & Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsBackward, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCObjects, "Duplicate & Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCObjects, "Duplicate & Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsUp, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Duplicate & Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjectsDown, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Down"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDuplicateObjects, WXK_CONTROL, 'D', KeyboardShortcut::SCObjects, "Duplicate"));
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRollObjectsCW, WXK_ALT, WXK_UP, KeyboardShortcut::SCObjects, "Rotate Clockwise by 90"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditRollObjectsCCW, WXK_ALT, WXK_DOWN, KeyboardShortcut::SCObjects, "Rotate Counter-clockwise by 90"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditYawObjectsCW, WXK_ALT, WXK_LEFT, KeyboardShortcut::SCObjects, "Rotate Left by 90"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditYawObjectsCCW, WXK_ALT, WXK_RIGHT, KeyboardShortcut::SCObjects, "Rotate Right by 90"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditPitchObjectsCW, WXK_ALT, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Rotate Up by 90"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditPitchObjectsCCW, WXK_ALT, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Rotate Down by 90"));
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditFlipObjectsHorizontally, WXK_CONTROL, 'F', KeyboardShortcut::SCObjects, "Flip Horizontally"));
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditFlipObjectsVertically, WXK_CONTROL, WXK_ALT, 'F', KeyboardShortcut::SCObjects, "Flip Vertically"));
            objectActionMenu.addSeparator();
            MenuItem::Ptr snapVerticesItem = objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditSnapVertices, KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Snap Vertices"));
            objectActionMenu.addSeparator();
#ifdef __linux__ // tab is not allowed as a menu accelerator on GTK
            MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditToggleMovementRestriction, 'X', KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Toggle Movement Axis"));
#else
            MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditToggleMovementRestriction, WXK_TAB, KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Toggle Movement Axis"));
#endif
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditPrintFilePositions, KeyboardShortcut::SCObjects | KeyboardShortcut::SCTextures, "Print Line Numbers"));
            
            Menu& vertexActionMenu = actionMenu.addMenu("Vertices", CommandIds::Menu::EditVertexActions);
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesForward, WXK_UP, KeyboardShortcut::SCVertexTool, "Move Forward"));
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesBackward, WXK_DOWN, KeyboardShortcut::SCVertexTool, "Move Backward"));
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesLeft, WXK_LEFT, KeyboardShortcut::SCVertexTool, "Move Left"));
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesRight, WXK_RIGHT, KeyboardShortcut::SCVertexTool, "Move Right"));
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesUp, WXK_PAGEUP, KeyboardShortcut::SCVertexTool, "Move Up"));
            vertexActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditMoveVerticesDown, WXK_PAGEDOWN, KeyboardShortcut::SCVertexTool, "Move Down"));
            vertexActionMenu.addSeparator();
            vertexActionMenu.addItem(snapVerticesItem);
            vertexActionMenu.addSeparator();
            vertexActionMenu.addItem(toggleAxisItem);
            
            Menu& clipActionMenu = actionMenu.addMenu("Clip Tool", CommandIds::Menu::EditClipActions);
            clipActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditToggleClipSide, WXK_CONTROL, WXK_RETURN, KeyboardShortcut::SCClipTool, "Toggle Clip Side"));
            clipActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditPerformClip, WXK_RETURN, KeyboardShortcut::SCClipTool, "Perform Clip"));
#ifdef __APPLE__
            clipActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDeleteLastClipPoint, WXK_BACK, KeyboardShortcut::SCClipTool, "Delete Last Clip Point"));
#else
            clipActionMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::EditDeleteLastClipPoint, WXK_DELETE, KeyboardShortcut::SCClipTool, "Delete Last Clip Point"));
#endif
            
            editMenu->addSeparator();
            editMenu->addCheckItem(KeyboardShortcut(CommandIds::Menu::EditToggleTextureLock, KeyboardShortcut::SCAny, "Texture Lock"));
#ifdef __linux__ // escape key is not allowed as a menu accelerator on GTK
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditNavigateUp, KeyboardShortcut::SCAny, "Navigate Up"));
#else
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditNavigateUp, WXK_ESCAPE, KeyboardShortcut::SCAny, "Navigate Up"));
#endif
            editMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::EditShowMapProperties, KeyboardShortcut::SCAny, "Map Properties"));
            
            Menu* viewMenu = new Menu("View");
            menus[ViewMenu] = Menu::Ptr(viewMenu);
            
            Menu& gridMenu = viewMenu->addMenu("Grid");
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewToggleShowGrid, WXK_CONTROL, 'G', KeyboardShortcut::SCAny, "Show Grid"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewToggleSnapToGrid, WXK_CONTROL, WXK_SHIFT, 'G', KeyboardShortcut::SCAny, "Snap to Grid"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewIncGridSize, WXK_CONTROL, '+', KeyboardShortcut::SCAny, "Increase Grid Size"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewDecGridSize, WXK_CONTROL, '-', KeyboardShortcut::SCAny, "Decrease Grid Size"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize1, WXK_CONTROL, '1', KeyboardShortcut::SCAny, "Set Grid Size 1"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize2, WXK_CONTROL, '2', KeyboardShortcut::SCAny, "Set Grid Size 2"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize4, WXK_CONTROL, '3', KeyboardShortcut::SCAny, "Set Grid Size 4"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize8, WXK_CONTROL, '4', KeyboardShortcut::SCAny, "Set Grid Size 8"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize16, WXK_CONTROL, '5', KeyboardShortcut::SCAny, "Set Grid Size 16"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize32, WXK_CONTROL, '6', KeyboardShortcut::SCAny, "Set Grid Size 32"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize64, WXK_CONTROL, '7', KeyboardShortcut::SCAny, "Set Grid Size 64"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize128, WXK_CONTROL, '8', KeyboardShortcut::SCAny, "Set Grid Size 128"));
            gridMenu.addCheckItem(KeyboardShortcut(CommandIds::Menu::ViewSetGridSize256, WXK_CONTROL, '9', KeyboardShortcut::SCAny, "Set Grid Size 256"));
            
            Menu& cameraMenu = viewMenu->addMenu("Camera");
            cameraMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::ViewMoveCameraToNextPoint, WXK_SHIFT, '+', KeyboardShortcut::SCAny, "Move to Next Point"));
            cameraMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::ViewMoveCameraToPreviousPoint, WXK_SHIFT, '-', KeyboardShortcut::SCAny, "Move to Previous Point"));
            cameraMenu.addActionItem(KeyboardShortcut(CommandIds::Menu::ViewCenterCameraOnSelection, WXK_CONTROL, WXK_SHIFT, 'C', KeyboardShortcut::SCAny, "Center on Selection"));
            
            viewMenu->addSeparator();
            viewMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::ViewSwitchToEntityTab, '1', KeyboardShortcut::SCAny, "Switch to Entity Inspector"));
            viewMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::ViewSwitchToFaceTab, '2', KeyboardShortcut::SCAny, "Switch to Face Inspector"));
            viewMenu->addActionItem(KeyboardShortcut(CommandIds::Menu::ViewSwitchToViewTab, '3', KeyboardShortcut::SCAny, "Switch to View Inspector"));
            return menus;
        }
    }
}
