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

using namespace TrenchBroom::VecMath;
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
            typedef std::set<const PreferenceBase*> Set;

            virtual ~PreferenceBase() {}

            virtual void load(wxConfigBase* config) const = 0;
            virtual void save(wxConfigBase* config) const = 0;
            virtual void setValue(const ValueHolderBase* valueHolder) const = 0;
            
            inline const bool operator== (const PreferenceBase& other) const {
                return this == &other;
            }
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
        extern const Preference<bool>   CameraAltModeInvertAxis;
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

        extern const Preference<Color>  EntityRotationDecoratorFillColor;
        extern const Preference<Color>  EntityRotationDecoratorOutlineColor;

        extern const Preference<Color>  XColor;
        extern const Preference<Color>  YColor;
        extern const Preference<Color>  ZColor;
        extern const Preference<Color>  DisabledColor;
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
        extern const Preference<float>  TransparentFaceAlpha;

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

        extern const Preference<Color>  HandleHighlightColor;
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

        extern const Preference<KeyboardShortcut>   CameraMoveForward;
        extern const Preference<KeyboardShortcut>   CameraMoveBackward;
        extern const Preference<KeyboardShortcut>   CameraMoveLeft;
        extern const Preference<KeyboardShortcut>   CameraMoveRight;
        
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

            virtual const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
                return NULL;
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
            mutable Preference<KeyboardShortcut> m_preference;

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
            const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
        };

        class Menu;

        class MenuItemParent : public TextMenuItem {
        private:
            String m_text;
            int m_menuId;
            List m_items;
        protected:
            MenuItemParent(MenuItemType type, const String& text, MenuItemParent* parent, int menuId) :
            TextMenuItem(type, parent),
            m_text(text),
            m_menuId(menuId) {}
            
            virtual ~MenuItemParent() {}
        public:
            inline const List& items() const {
                return m_items;
            }
            
            inline void addItem(MenuItem::Ptr item) {
                m_items.push_back(item);
            }
            
            inline const String& text() const {
                return m_text;
            }

            inline int menuId() const {
                return m_menuId;
            }

            const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const;
        };

        class MultiMenu;

        class MultiMenuSelector {
        public:
            virtual ~MultiMenuSelector() {}
            virtual const Menu* select(const MultiMenu& multiMenu) const = 0;
        };

        class MultiMenu : public MenuItemParent {
        public:
            MultiMenu(const String& text, MenuItemParent* parent, int menuId) :
            MenuItemParent(MITMultiMenu, text, parent, menuId) {
                assert(parent != NULL);
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
        public:
            Menu(const String& text, MenuItemParent* parent = NULL, int menuId = wxID_ANY) :
            MenuItemParent(MITMenu, text, parent, menuId) {}

            virtual ~Menu() {}

            inline MenuItem::Ptr addActionItem(const KeyboardShortcut& shortcut) {
                MenuItem::Ptr item = MenuItem::Ptr(new ShortcutMenuItem(MenuItem::MITAction, shortcut, this));
                addItem(item);
                return item;
            }

            inline MenuItem::Ptr addCheckItem(const KeyboardShortcut& shortcut) {
                MenuItem::Ptr item = MenuItem::Ptr(new ShortcutMenuItem(MenuItem::MITCheck, shortcut, this));
                addItem(item);
                return item;
            }

            inline void addSeparator() {
                MenuItem* item = new MenuItem(MenuItem::MITSeparator, this);
                addItem(MenuItem::Ptr(item));
            }

            inline Menu& addMenu(const String& text, int menuId = wxID_ANY) {
                Menu* menu = new Menu(text, this, menuId);
                addItem(Menu::Ptr(menu));
                return *menu;
            }

            inline MultiMenu& addMultiMenu(const String& text, int menuId) {
                MultiMenu* menu = new MultiMenu(text, this, menuId);
                addItem(MultiMenu::Ptr(menu));
                return *menu;
            }
        };

        class PreferenceManager {
        private:
            typedef std::map<const PreferenceBase*, ValueHolderBase*> UnsavedPreferences;

            bool m_saveInstantly;
            UnsavedPreferences m_unsavedPreferences;

            PreferenceManager();

            void markAsUnsaved(const PreferenceBase* preference, ValueHolderBase* valueHolder);
            const Menu::MenuMap buildMenus() const;
        public:
            inline static PreferenceManager& preferences() {
                static PreferenceManager prefs;
                return prefs;
            }

            inline const Menu& getMenu(const String& name) const {
                static const Menu::MenuMap menus = buildMenus();
                Menu::MenuMap::const_iterator it = menus.find(name);
                assert(it != menus.end());
                return static_cast<const Menu&>(*(it->second.get()));
            }

            bool saveInstantly() const;
            PreferenceBase::Set saveChanges();
            PreferenceBase::Set discardChanges();

            bool getBool(const Preference<bool>& preference) const;
            void setBool(const Preference<bool>& preference, bool value);

            int getInt(const Preference<int>& preference) const;
            void setInt(const Preference<int>& preference, int value);

            float getFloat(const Preference<float>& preference) const;
            void setFloat(const Preference<float>& preference, float value);

            const String& getString(const Preference<String>& preference) const;
            void setString(const Preference<String>& preference, const String& value);

            const Color& getColor(const Preference<Color>& preference) const;
            void setColor(const Preference<Color>& preference, const Color& value);

            const KeyboardShortcut& getKeyboardShortcut(const Preference<KeyboardShortcut>& preference) const;
            void setKeyboardShortcut(const Preference<KeyboardShortcut>& preference, const KeyboardShortcut& value);

        };
    }
}

#endif /* defined(__TrenchBroom__Preferences__) */
