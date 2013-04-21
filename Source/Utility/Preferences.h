/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Preferences__
#define __TrenchBroom__Preferences__

#include "Controller/Input.h"
#include "Utility/Color.h"
#include "Utility/MessageException.h"
#include "Utility/SharedPointer.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"
#include "View/CommandIds.h"
#include "View/KeyboardShortcut.h"

#include <wx/config.h>
#include <wx/confbase.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <map>
#include <vector>

using namespace TrenchBroom::Math;
using namespace TrenchBroom::Controller::ModifierKeys;
using namespace TrenchBroom::Controller::MouseButtons;
using namespace TrenchBroom::View::CommandIds;

namespace TrenchBroom {
    namespace Preferences {
        using View::KeyboardShortcut;

        class PreferenceManager;

        template <typename T>
        class Converter {
        public:
            wxString toWxString(const T& value) const {
                return "";
            }

            T fromWxString(const wxString& string) const {
                return T();
            }
        };

        template <>
        class Converter<bool> {
        public:
            wxString toWxString(const bool& value) const {
                wxString string;
                string << (value ? 1 : 0);
                return string;
            }

            bool fromWxString(const wxString& string) const {
                long longValue;
                if (string.ToLong(&longValue))
                    return longValue != 0L;
                return false;
            }
        };

        template <>
        class Converter<int> {
        public:
            wxString toWxString(const int& value) const {
                wxString string;
                string << value;
                return string;
            }

            int fromWxString(const wxString& string) const {
                long longValue;
                if (string.ToLong(&longValue) && longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max())
                    return static_cast<int>(longValue);
                return 0;
            }
        };

        template <>
        class Converter<float> {
        public:
            wxString toWxString(const float& value) const {
                wxString string;
                string << value;
                return string;
            }

            float fromWxString(const wxString& string) const {
                double doubleValue;
                if (string.ToDouble(&doubleValue) && doubleValue >= std::numeric_limits<float>::min() && doubleValue <= std::numeric_limits<float>::max())
                    return static_cast<float>(doubleValue);
                return 0.0f;
            }
        };

        template <>
        class Converter<String> {
        public:
            wxString toWxString(const String& value) const {
                return wxString(value);
            }

            String fromWxString(const wxString& string) const {
                return string.ToStdString();
            }
        };

        template <>
        class Converter<Color> {
        public:
            wxString toWxString(const Color& value) const {
                return wxString(value.asString());
            }

            Color fromWxString(const wxString& string) const {
                return Color(string.ToStdString());
            }
        };

        template<>
        class Converter<KeyboardShortcut> {
        public:
            wxString toWxString(const KeyboardShortcut& value) const {
                return wxString(value.asString());
            }

            KeyboardShortcut fromWxString(const wxString& string) const {
                return KeyboardShortcut(string.ToStdString());
            }
        };

        class ValueHolderBase {
        };

        template <typename T>
        class ValueHolder : public ValueHolderBase {
        private:
            T m_value;
        public:
            ValueHolder(T value) :
            m_value(value) {}

            inline const T& value() const {
                return m_value;
            }
        };

        class PreferenceBase {
        public:
            virtual ~PreferenceBase() {}

            virtual void load(wxConfigBase* config) const = 0;
            virtual void save(wxConfigBase* config) const = 0;
            virtual void setValue(const ValueHolderBase* valueHolder) const = 0;
        };

        template <typename T>
        class Preference : public PreferenceBase {
        protected:
            friend class PreferenceManager;

            Converter<T> m_converter;
            String m_name;
            mutable T m_value;
            mutable bool m_initialized;
            mutable bool m_modified;

            inline void setValue(const T& value) const {
                m_modified = true;
                m_value = value;
            }

            inline bool initialized() const {
                return m_initialized;
            }

            inline void load(wxConfigBase* config) const {
                wxString string;
                if (config->Read(m_name, &string))
                    m_value = m_converter.fromWxString(string);
                m_initialized = true;
            }

            inline void save(wxConfigBase* config) const {
                if (m_modified) {
                    wxString string = m_converter.toWxString(m_value);
                    bool success = config->Write(m_name, string);
                    assert(success);
                    m_modified = false;
                }
            }
        public:
            Preference(const String& name, const T& defaultValue) :
            m_name(name),
            m_value(defaultValue),
            m_initialized(false),
            m_modified(false) {
                m_modified = m_initialized;
            }

            void setValue(const ValueHolderBase* valueHolder) const {
                const ValueHolder<T>* actualValueHolder = static_cast<const ValueHolder<T>*>(valueHolder);
                setValue(actualValueHolder->value());
            }

            inline const String& name() const {
                return m_name;
            }

            inline const T& value() const {
                return m_value;
            }
        };

        extern const Preference<float>  CameraLookSpeed;
        extern const Preference<float>  CameraPanSpeed;
        extern const Preference<float>  CameraMoveSpeed;
        extern const Preference<bool>   CameraLookInvertX;
        extern const Preference<bool>   CameraLookInvertY;
        extern const Preference<bool>   CameraPanInvertX;
        extern const Preference<bool>   CameraPanInvertY;
        extern const Preference<bool>   CameraEnableAltMove;
        extern const Preference<bool>   CameraMoveInCursorDir;
        extern const Preference<float>  HandleRadius;
        extern const Preference<float>  MaximumHandleDistance;
        extern const Preference<float>  HandleScalingFactor;
        extern const Preference<float>  MaximumNearFaceDistance;
        extern const Preference<float>  CameraFieldOfVision;
        extern const Preference<float>  CameraNearPlane;
        extern const Preference<float>  CameraFarPlane;

        extern const Preference<float>  InfoOverlayFadeDistance;
        extern const Preference<float>  SelectedInfoOverlayFadeDistance;
        extern const Preference<int>    RendererFontSize;
        extern const Preference<float>  RendererBrightness;
        extern const Preference<float>  GridAlpha;
        extern const Preference<bool>   GridCheckerboard;

        extern const Preference<Color>  EntityRotationDecoratorColor;

        extern const Preference<Color>  XColor;
        extern const Preference<Color>  YColor;
        extern const Preference<Color>  ZColor;
        extern const Preference<Color>  BackgroundColor;

        extern const Preference<Color>  GuideColor;
        extern const Preference<Color>  HoveredGuideColor;

        extern const Preference<Color>  EntityLinkColor;
        extern const Preference<Color>  OccludedEntityLinkColor;
        extern const Preference<Color>  SelectedEntityLinkColor;
        extern const Preference<Color>  OccludedSelectedEntityLinkColor;

        extern const Preference<Color>  EntityKillLinkColor;
        extern const Preference<Color>  OccludedEntityKillLinkColor;
        extern const Preference<Color>  SelectedEntityKillLinkColor;
        extern const Preference<Color>  OccludedSelectedEntityKillLinkColor;

        extern const Preference<Color>  FaceColor;
        extern const Preference<Color>  SelectedFaceColor;
        extern const Preference<Color>  LockedFaceColor;
        extern const Preference<Color>  ClippedFaceColor;

        extern const Preference<Color>  EdgeColor;
        extern const Preference<Color>  SelectedEdgeColor;
        extern const Preference<Color>  OccludedSelectedEdgeColor;
        extern const Preference<Color>  LockedEdgeColor;
        extern const Preference<Color>  ClippedEdgeColor;
        extern const Preference<Color>  OccludedClippedEdgeColor;

        extern const Preference<Color>  SelectedEntityColor;
        extern const Preference<Color>  EntityBoundsColor;
        extern const Preference<Color>  SelectedEntityBoundsColor;
        extern const Preference<Color>  OccludedSelectedEntityBoundsColor;
        extern const Preference<Color>  LockedEntityColor;
        extern const Preference<Color>  LockedEntityBoundsColor;
        extern const Preference<Color>  EntityBoundsWireframeColor;

        extern const Preference<Color>  SelectionGuideColor;
        extern const Preference<Color>  OccludedSelectionGuideColor;

        extern const Preference<Color>  InfoOverlayTextColor;
        extern const Preference<Color>  InfoOverlayBackgroundColor;
        extern const Preference<Color>  OccludedInfoOverlayTextColor;
        extern const Preference<Color>  OccludedInfoOverlayBackgroundColor;
        extern const Preference<Color>  SelectedInfoOverlayTextColor;
        extern const Preference<Color>  SelectedInfoOverlayBackgroundColor;
        extern const Preference<Color>  OccludedSelectedInfoOverlayTextColor;
        extern const Preference<Color>  OccludedSelectedInfoOverlayBackgroundColor;
        extern const Preference<Color>  LockedInfoOverlayTextColor;
        extern const Preference<Color>  LockedInfoOverlayBackgroundColor;

        extern const Preference<Color>  VertexHandleColor;
        extern const Preference<Color>  OccludedVertexHandleColor;
        extern const Preference<Color>  SelectedVertexHandleColor;
        extern const Preference<Color>  OccludedSelectedVertexHandleColor;

        extern const Preference<Color>  SplitHandleColor;
        extern const Preference<Color>  OccludedSplitHandleColor;
        extern const Preference<Color>  SelectedSplitHandleColor;
        extern const Preference<Color>  OccludedSelectedSplitHandleColor;

        extern const Preference<Color>  EdgeHandleColor;
        extern const Preference<Color>  OccludedEdgeHandleColor;
        extern const Preference<Color>  SelectedEdgeHandleColor;
        extern const Preference<Color>  OccludedSelectedEdgeHandleColor;

        extern const Preference<Color>  FaceHandleColor;
        extern const Preference<Color>  OccludedFaceHandleColor;
        extern const Preference<Color>  SelectedFaceHandleColor;
        extern const Preference<Color>  OccludedSelectedFaceHandleColor;

        extern const Preference<Color>  ClipHandleColor;
        extern const Preference<Color>  OccludedClipHandleColor;
        extern const Preference<Color>  SelectedClipHandleColor;
        extern const Preference<Color>  ClipPlaneColor;

        extern const Preference<Color>  ResizeBrushFaceColor;
        extern const Preference<Color>  OccludedResizeBrushFaceColor;

        extern const Preference<Color>  BrowserTextColor;
        extern const Preference<Color>  SelectedTextureColor;
        extern const Preference<Color>  UsedTextureColor;
        extern const Preference<Color>  OverriddenTextureColor;
        extern const Preference<Color>  BrowserGroupBackgroundColor;
        extern const Preference<int>    TextureBrowserFontSize;
        extern const Preference<int>    EntityBrowserFontSize;
        extern const Preference<float>  TextureBrowserIconSize;

        extern const Preference<String> QuakePath;
        extern const Preference<String> RendererFontName;
        extern const Preference<int>    RendererInstancingMode;
        extern const int                RendererInstancingModeAutodetect;
        extern const int                RendererInstancingModeForceOn;
        extern const int                RendererInstancingModeForceOff;

        extern const String FileMenu;
        extern const String EditMenu;
        extern const String ViewMenu;

        class MenuItemParent;

        class MenuItem {
        public:
            typedef enum {
                MITSeparator,
                MITAction,
                MITCheck,
                MITMenu,
                MITMultiMenu
            } MenuItemType;

            typedef std::tr1::shared_ptr<MenuItem> Ptr;
            typedef std::vector<Ptr> List;
        private:
            MenuItemType m_type;
            MenuItemParent* m_parent;
        public:
            MenuItem(MenuItemType type, MenuItemParent* parent) :
            m_type(type),
            m_parent(parent) {}

            virtual ~MenuItem() {}

            inline MenuItemType type() const {
                return m_type;
            }

            inline const MenuItemParent* parent() const {
                return m_parent;
            }
        };

        class TextMenuItem : public MenuItem {
        public:
            TextMenuItem(MenuItemType type, MenuItemParent* parent) :
            MenuItem(type, parent) {}

            virtual ~TextMenuItem() {}

            virtual const String& text() const = 0;
        };

        class ShortcutMenuItem : public TextMenuItem {
        private:
            mutable KeyboardShortcut m_shortcut;

            String path() const;
        public:
            ShortcutMenuItem(MenuItemType type, const KeyboardShortcut& shortcut, MenuItemParent* parent);
            virtual ~ShortcutMenuItem() {}

            inline const String& text() const {
                return m_shortcut.text();
            }

            const String longText() const;
            const KeyboardShortcut& shortcut() const;
            void setShortcut(const KeyboardShortcut& shortcut) const;
        };

        class Menu;

        class MenuItemParent : public TextMenuItem {
        private:
            String m_text;
            int m_menuId;
        protected:
            MenuItemParent(MenuItemType type, const String& text, MenuItemParent* parent, int menuId) :
            TextMenuItem(type, parent),
            m_text(text),
            m_menuId(menuId) {}
        public:
            inline const String& text() const {
                return m_text;
            }

            inline int menuId() const {
                return m_menuId;
            }
        };

        class MultiMenu;

        class MultiMenuSelector {
        public:
            virtual ~MultiMenuSelector() {}
            virtual const Menu* select(const MultiMenu& multiMenu) const = 0;
        };

        class MultiMenu : public MenuItemParent {
        private:
            List m_items;
        public:
            MultiMenu(const String& text, MenuItemParent* parent, int menuId) :
            MenuItemParent(MITMultiMenu, text, parent, menuId) {
                assert(parent != NULL);
            }

            inline const List& items() const {
                return m_items;
            }

            Menu& addMenu(const String& text, const int menuId);

            const Menu* menuById(const int menuId) const;

            inline const Menu* selectMenu(const MultiMenuSelector& selector) const {
                return selector.select(*this);
            }
        };

        class Menu : public MenuItemParent {
        public:
            typedef std::map<String, Ptr> MenuMap;
        private:
            List m_items;
        public:
            Menu(const String& text, MenuItemParent* parent = NULL, int menuId = wxID_ANY) :
            MenuItemParent(MITMenu, text, parent, menuId) {}

            virtual ~Menu() {}

            inline const List& items() const {
                return m_items;
            }

            inline void addActionItem(const KeyboardShortcut& shortcut) {
                MenuItem* item = new ShortcutMenuItem(MenuItem::MITAction, shortcut, this);
                m_items.push_back(MenuItem::Ptr(item));
            }

            inline void addCheckItem(const KeyboardShortcut& shortcut) {
                MenuItem* item = new ShortcutMenuItem(MenuItem::MITCheck, shortcut, this);
                m_items.push_back(MenuItem::Ptr(item));
            }

            inline void addSeparator() {
                MenuItem* item = new MenuItem(MenuItem::MITSeparator, this);
                m_items.push_back(MenuItem::Ptr(item));
            }

            inline Menu& addMenu(const String& text, int menuId = wxID_ANY) {
                Menu* menu = new Menu(text, this, menuId);
                m_items.push_back(Menu::Ptr(menu));
                return *menu;
            }

            inline MultiMenu& addMultiMenu(const String& text, int menuId) {
                MultiMenu* menu = new MultiMenu(text, this, menuId);
                m_items.push_back(MultiMenu::Ptr(menu));
                return *menu;
            }
        };

        class PreferenceManager {
        private:
            typedef std::map<const PreferenceBase*, ValueHolderBase*> UnsavedPreferences;

            bool m_saveInstantly;
            UnsavedPreferences m_unsavedPreferences;

            PreferenceManager() {
#if defined __APPLE__
                m_saveInstantly = true;
#else
                m_saveInstantly = false;
#endif
            }

            void markAsUnsaved(const PreferenceBase* preference, ValueHolderBase* valueHolder) {
                UnsavedPreferences::iterator it = m_unsavedPreferences.find(preference);
                if (it == m_unsavedPreferences.end())
                    m_unsavedPreferences[preference] = valueHolder;
                else
                    delete valueHolder;
            }

        public:
            inline static PreferenceManager& preferences() {
                static PreferenceManager prefs;
                return prefs;
            }

            inline void save() {
                UnsavedPreferences::iterator it, end;
                for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                    it->first->save(wxConfig::Get());
                    delete it->second;
                }

                m_unsavedPreferences.clear();
            }

            inline void discardChanges() {
                UnsavedPreferences::iterator it, end;
                for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                    it->first->setValue(it->second);
                    delete it->second;
                }

                m_unsavedPreferences.clear();
            }

            inline bool getBool(const Preference<bool>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setBool(const Preference<bool>& preference, bool value) {
                bool previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<bool>(previousValue));
            }

            inline int getInt(const Preference<int>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setInt(const Preference<int>& preference, int value) {
                int previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<int>(previousValue));
            }

            inline float getFloat(const Preference<float>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setFloat(const Preference<float>& preference, float value) {
                float previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<float>(previousValue));
            }

            inline const String& getString(const Preference<String>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setString(const Preference<String>& preference, const String& value) {
                String previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<String>(previousValue));
            }

            inline const Color& getColor(const Preference<Color>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setColor(const Preference<Color>& preference, const Color& value) {
                Color previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<Color>(previousValue));
            }

            inline const KeyboardShortcut& getKeyboardShortcut(const Preference<KeyboardShortcut>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());

                return preference.value();
            }

            inline void setKeyboardShortcut(const Preference<KeyboardShortcut>& preference, const KeyboardShortcut& value) {
                KeyboardShortcut previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<KeyboardShortcut>(previousValue));
            }

            inline const Menu::MenuMap buildMenus() const {
                Menu::MenuMap menus;

                Menu* fileMenu = new Menu("File");
                menus[FileMenu] = Menu::Ptr(fileMenu);

                fileMenu->addActionItem(KeyboardShortcut(wxID_NEW, WXK_CONTROL, 'N', KeyboardShortcut::SCAny, "New"));
                fileMenu->addSeparator();
                fileMenu->addActionItem(KeyboardShortcut(wxID_OPEN, WXK_CONTROL, 'O', KeyboardShortcut::SCAny, "Open..."));
                fileMenu->addMenu("Open Recent", View::CommandIds::Menu::FileOpenRecent);
                fileMenu->addSeparator();
                fileMenu->addActionItem(KeyboardShortcut(wxID_SAVE, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save"));
                fileMenu->addActionItem(KeyboardShortcut(wxID_SAVEAS, WXK_SHIFT, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save as..."));
                fileMenu->addSeparator();
                fileMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::FileLoadPointFile, KeyboardShortcut::SCAny, "Load Point File..."));
                fileMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::FileUnloadPointFile, KeyboardShortcut::SCAny, "Unload Point File"));
                fileMenu->addSeparator();
                fileMenu->addActionItem(KeyboardShortcut(wxID_CLOSE, WXK_CONTROL, 'W', KeyboardShortcut::SCAny, "Close"));

                Menu* editMenu = new Menu("Edit");
                menus[EditMenu] = Menu::Ptr(editMenu);

                editMenu->addActionItem(KeyboardShortcut(wxID_UNDO, WXK_CONTROL, 'Z', KeyboardShortcut::SCAny, "Undo"));
                editMenu->addActionItem(KeyboardShortcut(wxID_REDO, WXK_CONTROL, WXK_SHIFT, 'Z', KeyboardShortcut::SCAny, "Redo"));
                editMenu->addSeparator();
                editMenu->addActionItem(KeyboardShortcut(wxID_CUT, WXK_CONTROL, 'X', KeyboardShortcut::SCAny, "Cut"));
                editMenu->addActionItem(KeyboardShortcut(wxID_COPY, WXK_CONTROL, 'C', KeyboardShortcut::SCAny, "Copy"));
                editMenu->addActionItem(KeyboardShortcut(wxID_PASTE, WXK_CONTROL, 'V', KeyboardShortcut::SCAny, "Paste"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPasteAtOriginalPosition, WXK_CONTROL, WXK_SHIFT, 'V', KeyboardShortcut::SCAny, "Paste at Original Position"));
                editMenu->addActionItem(KeyboardShortcut(wxID_DELETE, WXK_BACK, KeyboardShortcut::SCAny, "Delete"));
                editMenu->addSeparator();
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSelectAll, WXK_CONTROL, 'A', KeyboardShortcut::SCAny, "Select All"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSelectSiblings, WXK_CONTROL, WXK_ALT, 'A', KeyboardShortcut::SCAny, "Select Siblings"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSelectTouching, WXK_CONTROL, 'T', KeyboardShortcut::SCAny, "Select Touching"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSelectByFilePosition, KeyboardShortcut::SCAny, "Select by Line Number"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSelectNone, WXK_CONTROL, WXK_SHIFT, 'A', KeyboardShortcut::SCAny, "Select None"));
                editMenu->addSeparator();
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditHideSelected, WXK_CONTROL, 'H', KeyboardShortcut::SCAny, "Hide Selected"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditHideUnselected, WXK_CONTROL, WXK_ALT, 'H', KeyboardShortcut::SCAny, "Hide Unselected"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditUnhideAll, WXK_CONTROL, WXK_SHIFT, 'H', KeyboardShortcut::SCAny, "Unhide All"));
                editMenu->addSeparator();
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditLockSelected, WXK_CONTROL, 'L', KeyboardShortcut::SCAny, "Lock Selected"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditLockUnselected, WXK_CONTROL, WXK_ALT, 'L', KeyboardShortcut::SCAny, "Lock Unselected"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditUnlockAll, WXK_CONTROL, WXK_SHIFT, 'L', KeyboardShortcut::SCAny, "Unlock All"));
                editMenu->addSeparator();

                Menu& toolMenu = editMenu->addMenu("Tools");
                toolMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleClipTool, 'C', KeyboardShortcut::SCAny, "Clip Tool"));
                toolMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleVertexTool, 'V', KeyboardShortcut::SCAny, "Vertex Tool"));
                toolMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleRotateObjectsTool, 'R', KeyboardShortcut::SCAny, "Rotate Tool"));

                MultiMenu& actionMenu = editMenu->addMultiMenu("Actions", View::CommandIds::Menu::EditActions);

                Menu& faceActionMenu = actionMenu.addMenu("Faces", View::CommandIds::Menu::EditFaceActions);
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesUp, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesDown, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesLeft, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesRight, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
                faceActionMenu.addSeparator();
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesUp, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCTextures, "Move Up by 1"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesDown, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down by 1"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left by 1"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right by 1"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 1"));
                faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 1"));

                Menu& objectActionMenu = actionMenu.addMenu("Objects", View::CommandIds::Menu::EditObjectActions);
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsForward, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsBackward, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsLeft, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsRight, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsUp, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsDown, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
                objectActionMenu.addSeparator();
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsForward, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCObjects, "Duplicate & Move Forward"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsBackward, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Backward"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCObjects, "Duplicate & Move Left"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCObjects, "Duplicate & Move Right"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsUp, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Duplicate & Move Up"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsDown, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Down"));
                objectActionMenu.addSeparator();
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRollObjectsCW, WXK_ALT, WXK_UP, KeyboardShortcut::SCObjects, "Rotate Clockwise by 90"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRollObjectsCCW, WXK_ALT, WXK_DOWN, KeyboardShortcut::SCObjects, "Rotate Counter-clockwise by 90"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditYawObjectsCW, WXK_ALT, WXK_LEFT, KeyboardShortcut::SCObjects, "Rotate Left by 90"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditYawObjectsCCW, WXK_ALT, WXK_RIGHT, KeyboardShortcut::SCObjects, "Rotate Right by 90"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPitchObjectsCW, WXK_ALT, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Rotate Up by 90"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPitchObjectsCCW, WXK_ALT, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Rotate Down by 90"));
                objectActionMenu.addSeparator();
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditFlipObjectsHorizontally, WXK_CONTROL, 'F', KeyboardShortcut::SCObjects, "Flip Horizontally"));
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditFlipObjectsVertically, WXK_CONTROL, WXK_ALT, 'F', KeyboardShortcut::SCObjects, "Flip Vertically"));
                objectActionMenu.addSeparator();
                objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjects, WXK_CONTROL, 'D', KeyboardShortcut::SCObjects, "Duplicate"));

                Menu& vertexActionMenu = actionMenu.addMenu("Vertices", View::CommandIds::Menu::EditVertexActions);
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesForward, WXK_UP, KeyboardShortcut::SCVertexTool, "Move Forward"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesBackward, WXK_DOWN, KeyboardShortcut::SCVertexTool, "Move Backward"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesLeft, WXK_LEFT, KeyboardShortcut::SCVertexTool, "Move Left"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesRight, WXK_RIGHT, KeyboardShortcut::SCVertexTool, "Move Right"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesUp, WXK_PAGEUP, KeyboardShortcut::SCVertexTool, "Move Up"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesDown, WXK_PAGEDOWN, KeyboardShortcut::SCVertexTool, "Move Down"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditCorrectVertices, KeyboardShortcut::SCVertexTool, "Correct Vertices"));
                vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSnapVertices, KeyboardShortcut::SCVertexTool, "Snap Vertices"));

                Menu& clipActionMenu = actionMenu.addMenu("Clip Tool", View::CommandIds::Menu::EditClipActions);
                clipActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleClipSide, WXK_TAB, KeyboardShortcut::SCClipTool, "Toggle Clip Side"));
                clipActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPerformClip, WXK_RETURN, KeyboardShortcut::SCClipTool, "Perform Clip"));

                editMenu->addSeparator();
                editMenu->addCheckItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleTextureLock, KeyboardShortcut::SCAny, "Texture Lock"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditNavigateUp, WXK_ESCAPE, KeyboardShortcut::SCAny, "Navigate Up"));
                editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditShowMapProperties, KeyboardShortcut::SCAny, "Map Properties"));

                Menu* viewMenu = new Menu("View");
                menus[ViewMenu] = Menu::Ptr(viewMenu);

                Menu& gridMenu = viewMenu->addMenu("Grid");
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewToggleShowGrid, WXK_CONTROL, 'G', KeyboardShortcut::SCAny, "Show Grid"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewToggleSnapToGrid, WXK_CONTROL, WXK_SHIFT, 'G', KeyboardShortcut::SCAny, "Snap to Grid"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewIncGridSize, WXK_CONTROL, '+', KeyboardShortcut::SCAny, "Increase Grid Size"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewDecGridSize, WXK_CONTROL, '-', KeyboardShortcut::SCAny, "Decrease Grid Size"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize1, WXK_CONTROL, '1', KeyboardShortcut::SCAny, "Set Grid Size 1"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize2, WXK_CONTROL, '2', KeyboardShortcut::SCAny, "Set Grid Size 2"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize4, WXK_CONTROL, '3', KeyboardShortcut::SCAny, "Set Grid Size 4"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize8, WXK_CONTROL, '4', KeyboardShortcut::SCAny, "Set Grid Size 8"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize16, WXK_CONTROL, '5', KeyboardShortcut::SCAny, "Set Grid Size 16"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize32, WXK_CONTROL, '6', KeyboardShortcut::SCAny, "Set Grid Size 32"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize64, WXK_CONTROL, '7', KeyboardShortcut::SCAny, "Set Grid Size 64"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize128, WXK_CONTROL, '8', KeyboardShortcut::SCAny, "Set Grid Size 128"));
                gridMenu.addCheckItem(KeyboardShortcut(View::CommandIds::Menu::ViewSetGridSize256, WXK_CONTROL, '9', KeyboardShortcut::SCAny, "Set Grid Size 256"));

                Menu& cameraMenu = viewMenu->addMenu("Camera");
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraForward, 'W', KeyboardShortcut::SCAny, "Move Forward"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraBackward, 'S', KeyboardShortcut::SCAny, "Move Backward"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraLeft, 'A', KeyboardShortcut::SCAny, "Move Left"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraRight, 'D', KeyboardShortcut::SCAny, "Move Right"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraUp, WXK_SHIFT, 'W', KeyboardShortcut::SCAny, "Move Up"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraDown, WXK_SHIFT, 'S', KeyboardShortcut::SCAny, "Move Down"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraToNextPoint, WXK_SHIFT, '+', KeyboardShortcut::SCAny, "Move to Next Point"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraToPreviousPoint, WXK_SHIFT, '-', KeyboardShortcut::SCAny, "Move to Previous Point"));
                cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewCenterCameraOnSelection, WXK_CONTROL, WXK_SHIFT, 'C', KeyboardShortcut::SCAny, "Center on Selection"));

                viewMenu->addSeparator();
                viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToEntityTab, '1', KeyboardShortcut::SCAny, "Switch to Entity Inspector"));
                viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToFaceTab, '2', KeyboardShortcut::SCAny, "Switch to Face Inspector"));
                viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToViewTab, '3', KeyboardShortcut::SCAny, "Switch to View Inspector"));
                return menus;
            }

            inline const Menu& getMenu(const String& name) const {
                static const Menu::MenuMap menus = buildMenus();
                Menu::MenuMap::const_iterator it = menus.find(name);
                assert(it != menus.end());
                return static_cast<const Menu&>(*(it->second.get()));
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Preferences__) */
