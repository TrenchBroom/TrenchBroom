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

#include "Preferences.h"

namespace TrenchBroom {
    namespace Preferences {

        const Preference<float> CameraLookSpeed = Preference<float>(                            "Controls/Camera/Look speed",                                   0.5f);
        const Preference<float> CameraPanSpeed = Preference<float>(                             "Controls/Camera/Pan speed",                                    0.5f);
        const Preference<bool>  CameraLookInvertX = Preference<bool>(                           "Controls/Camera/Look X inverted",                              false);
        const Preference<bool>  CameraLookInvertY = Preference<bool>(                           "Controls/Camera/Look Y inverted",                              false);
        const Preference<bool>  CameraPanInvertX = Preference<bool>(                            "Controls/Camera/Pan X inverted",                               false);
        const Preference<bool>  CameraPanInvertY = Preference<bool>(                            "Controls/Camera/Pan Y inverted",                               false);
        const Preference<bool>  CameraEnableAltMove = Preference<bool>(                         "Controls/Camera/Enable Alt to move",                           false);
        const Preference<bool>  CameraMoveInCursorDir = Preference<bool>(                       "Controls/Camera/Move camera towards cursor",                   false);
        const Preference<float> HandleRadius = Preference<float>(                               "Controls/Vertex handle radius",                                3.0f);
        const Preference<float> MaximumHandleDistance = Preference<float>(                      "Controls/Maximum handle distance",                             1000.0f);
        const Preference<float> HandleScalingFactor = Preference<float>(                        "Controls/Handle scaling factor",                               1.0f / 300.0f);
        const Preference<float> MaximumNearFaceDistance = Preference<float>(                    "Controls/Maximum near face distance",                          8.0f);
        const Preference<float> CameraFieldOfVision = Preference<float>(                        "Renderer/Camera field of vision",                              90.0f);
        const Preference<float> CameraNearPlane = Preference<float>(                            "Renderer/Camera near plane",                                   1.0f);
        const Preference<float> CameraFarPlane = Preference<float>(                             "Renderer/Camera far plane",                                    8192.0f);

        const Preference<float> InfoOverlayFadeDistance = Preference<float>(                    "Renderer/Info overlay fade distance",                          400.0f);
        const Preference<float> SelectedInfoOverlayFadeDistance = Preference<float>(            "Renderer/Selected info overlay fade distance",                 400.0f);
        const Preference<int>   RendererFontSize = Preference<int>(                             "Renderer/Font size",                                           13);
        const Preference<float> RendererBrightness = Preference<float>(                         "Renderer/Brightness",                                          1.0f);
        const Preference<float> GridAlpha = Preference<float>(                                  "Renderer/Grid Alpha",                                          0.25f);
        const Preference<bool>  GridCheckerboard = Preference<bool>(                            "Renderer/Grid Checkerboard",                                   false);

        const Preference<Color> EntityRotationDecoratorFillColor = Preference<Color>(           "Renderer/Colors/Decorators/Entity rotation fill color",        Color(1.0f,  0.0f,  0.0f,  0.3f ));
        const Preference<Color> EntityRotationDecoratorOutlineColor = Preference<Color>(        "Renderer/Colors/Decorators/Entity rotation outline color",     Color(1.0f,  1.0f,  1.0f,  0.7f ));

        const Preference<Color> XColor = Preference<Color>(                                     "Renderer/Colors/X",                                            Color(0xFF, 0x3D, 0x00));
        const Preference<Color> YColor = Preference<Color>(                                     "Renderer/Colors/Y",                                            Color(0x4B, 0x95, 0x00));
        const Preference<Color> ZColor = Preference<Color>(                                     "Renderer/Colors/Z",                                            Color(0x10, 0x9C, 0xFF));
        const Preference<Color> DisabledColor = Preference<Color>(                              "Renderer/Colors/Disabled",                                     Color(0xAA, 0xAA, 0xAA));
        const Preference<Color> BackgroundColor = Preference<Color>(                            "Renderer/Colors/Background",                                   Color(0.0f,  0.0f,  0.0f,  1.0f ));

        const Preference<Color> GuideColor = Preference<Color>(                                 "Renderer/Colors/Guide",                                        Color(1.0f,  0.0f,  0.0f,  0.3f ));
        const Preference<Color> HoveredGuideColor = Preference<Color>(                          "Renderer/Colors/Hovered guide",                                Color(1.0f,  0.0f,  0.0f,  0.7f ));

        const Preference<Color> EntityLinkColor = Preference<Color>(                            "Renderer/Colors/Entity link",                                  Color(0.1f,  0.3f,  0.6f,  1.0f ));
        const Preference<Color> OccludedEntityLinkColor = Preference<Color>(                    "Renderer/Colors/Occluded entity link",                         Color(0.1f,  0.3f,  0.6f,  0.5f ));
        const Preference<Color> SelectedEntityLinkColor = Preference<Color>(                    "Renderer/Colors/Selected entity link",                         Color(0.8f,  0.4f,  0.1f,  1.0f ));
        const Preference<Color> OccludedSelectedEntityLinkColor = Preference<Color>(            "Renderer/Colors/Occluded selected entity link",                Color(0.8f,  0.4f,  0.1f,  0.5f ));

        const Preference<Color> EntityKillLinkColor = Preference<Color>(                        "Renderer/Colors/Entity kill link",                             Color(0.1f,  0.6f,  0.3f,  1.0f ));
        const Preference<Color> OccludedEntityKillLinkColor = Preference<Color>(                "Renderer/Colors/Occluded entity kill link",                    Color(0.1f,  0.6f,  0.3f,  0.5f ));
        const Preference<Color> SelectedEntityKillLinkColor = Preference<Color>(                "Renderer/Colors/Selected entity kill link",                    Color(0.8f,  0.4f,  0.1f,  1.0f ));
        const Preference<Color> OccludedSelectedEntityKillLinkColor = Preference<Color>(        "Renderer/Colors/Occluded selected entity kill link",           Color(0.8f,  0.4f,  0.1f,  0.5f ));

        const Preference<Color> FaceColor = Preference<Color>(                                  "Renderer/Colors/Face",                                         Color(0.2f,  0.2f,  0.2f,  1.0f ));
        const Preference<Color> SelectedFaceColor = Preference<Color>(                          "Renderer/Colors/Selected face",                                Color(0.6f,  0.35f, 0.35f, 1.0f ));
        const Preference<Color> LockedFaceColor = Preference<Color>(                            "Renderer/Colors/Locked face",                                  Color(0.35f, 0.35f, 0.6f,  1.0f ));
        const Preference<Color> ClippedFaceColor = Preference<Color>(                           "Renderer/Colors/Clipped face",                                 Color(0.6f,  0.3f,  0.0f,  1.0f ));
        const Preference<float> TransparentFaceAlpha = Preference<float>(                       "Renderer/Colors/Transparent face alpha",                       0.65f);

        const Preference<Color> EdgeColor = Preference<Color>(                                  "Renderer/Colors/Edge",                                         Color(0.7f,  0.7f,  0.7f,  1.0f ));
        const Preference<Color> SelectedEdgeColor = Preference<Color>(                          "Renderer/Colors/Selected edge",                                Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEdgeColor = Preference<Color>(                  "Renderer/Colors/Occluded selected edge",                       Color(1.0f,  0.0f,  0.0f,  0.5f ));
        const Preference<Color> LockedEdgeColor = Preference<Color>(                            "Renderer/Colors/Locked edge",                                  Color(0.13f, 0.3f,  1.0f,  1.0f ));
        const Preference<Color> ClippedEdgeColor = Preference<Color>(                           "Renderer/Colors/Clipped edge",                                 Color(1.0f,  0.5f,  0.0f,  1.0f ));
        const Preference<Color> OccludedClippedEdgeColor = Preference<Color>(                   "Renderer/Colors/Occluded clipped edge",                        Color(1.0f,  0.5f,  0.0f,  0.5f ));

        const Preference<Color> SelectedEntityColor = Preference<Color>(                        "Renderer/Colors/Selected entity",                              Color(0.6f,  0.35f, 0.35f, 1.0f ));
        const Preference<Color> EntityBoundsColor = Preference<Color>(                          "Renderer/Colors/Entity bounds",                                Color(0.5f,  0.5f,  0.5f,  1.0f ));
        const Preference<Color> SelectedEntityBoundsColor = Preference<Color>(                  "Renderer/Colors/Selected entity bounds",                       Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEntityBoundsColor = Preference<Color>(          "Renderer/Colors/Occluded selected entity bounds",              Color(1.0f,  0.0f,  0.0f,  0.5f ));
        const Preference<Color> LockedEntityColor = Preference<Color>(                          "Renderer/Colors/Locked entity",                                Color(0.35f, 0.35f, 0.6f,  1.0f ));
        const Preference<Color> LockedEntityBoundsColor = Preference<Color>(                    "Renderer/Colors/Locked entity bounds",                         Color(0.13f, 0.3f,  1.0f,  1.0f ));
        const Preference<Color> EntityBoundsWireframeColor = Preference<Color>(                 "Renderer/Colors/Entity bounds (wireframe mode)",               Color(0.13f, 0.3f,  1.0f,  1.0f ));

        const Preference<Color> SelectionGuideColor = Preference<Color>(                        "Renderer/Colors/Selection guide",                              Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectionGuideColor = Preference<Color>(                "Renderer/Colors/Occluded selection guide",                     Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> InfoOverlayTextColor = Preference<Color>(                       "Renderer/Colors/Info overlay text",                            Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> InfoOverlayBackgroundColor = Preference<Color>(                 "Renderer/Colors/Info overlay background",                      Color(0.0f,  0.0f,  0.0f,  0.6f ));
        const Preference<Color> OccludedInfoOverlayTextColor = Preference<Color>(               "Renderer/Colors/Occluded info overlay text",                   Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> OccludedInfoOverlayBackgroundColor = Preference<Color>(         "Renderer/Colors/Occluded info overlay background",             Color(0.0f,  0.0f,  0.0f,  0.3f ));
        const Preference<Color> SelectedInfoOverlayTextColor = Preference<Color>(               "Renderer/Colors/Selected info overlay text",                   Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> SelectedInfoOverlayBackgroundColor = Preference<Color>(         "Renderer/Colors/Selected info overlay backtround",             Color(1.0f,  0.0f,  0.0f,  0.6f ));
        const Preference<Color> OccludedSelectedInfoOverlayTextColor = Preference<Color>(       "Renderer/Colors/Occluded selected info overlay text",          Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> OccludedSelectedInfoOverlayBackgroundColor = Preference<Color>( "Renderer/Colors/Occluded selected info overlay background",    Color(1.0f,  0.0f,  0.0f,  0.3f ));
        const Preference<Color> LockedInfoOverlayTextColor = Preference<Color>(                 "Renderer/Colors/Locked info overlay text",                     Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> LockedInfoOverlayBackgroundColor = Preference<Color>(           "Renderer/Colors/Locked info overlay background",               Color(0.13f, 0.3f,  1.0f,  0.6f ));

        const Preference<Color> HandleHighlightColor = Preference<Color>(                       "Renderer/Colors/Handle highlight",                             Color(1.0f,  1.0f,  1.0f,  1.0f));
        const Preference<Color> VertexHandleColor = Preference<Color>(                          "Renderer/Colors/Vertex handle",                                Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedVertexHandleColor = Preference<Color>(                  "Renderer/Colors/Occluded vertex handle",                       Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedVertexHandleColor = Preference<Color>(                  "Renderer/Colors/Selected vertex handle",                       Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedVertexHandleColor = Preference<Color>(          "Renderer/Colors/Occluded selected vertex handle",              Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> SplitHandleColor = Preference<Color>(                           "Renderer/Colors/Split handle",                                 Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedSplitHandleColor = Preference<Color>(                   "Renderer/Colors/Occluded split handle",                        Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedSplitHandleColor = Preference<Color>(                   "Renderer/Colors/Selected split handle",                        Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedSplitHandleColor = Preference<Color>(           "Renderer/Colors/Occluded selected split handle",               Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> EdgeHandleColor = Preference<Color>(                            "Renderer/Colors/Edge handle",                                  Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedEdgeHandleColor = Preference<Color>(                    "Renderer/Colors/Occluded edge handle",                         Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedEdgeHandleColor = Preference<Color>(                    "Renderer/Colors/Selected edge handle",                         Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEdgeHandleColor = Preference<Color>(            "Renderer/Colors/Occluded selected edge handle",                Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> FaceHandleColor = Preference<Color>(                            "Renderer/Colors/Face handle",                                  Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedFaceHandleColor = Preference<Color>(                    "Renderer/Colors/Occluded face handle",                         Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedFaceHandleColor = Preference<Color>(                    "Renderer/Colors/Selected face handle",                         Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedFaceHandleColor = Preference<Color>(            "Renderer/Colors/Occluded selected face handle",                Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> ClipHandleColor = Preference<Color>(                            "Renderer/Colors/Clip handle",                                  Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedClipHandleColor = Preference<Color>(                    "Renderer/Colors/Occluded clip handle",                         Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedClipHandleColor = Preference<Color>(                    "Renderer/Colors/Selected clip handle",                         Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> ClipPlaneColor = Preference<Color>(                             "Renderer/Colors/Clip plane",                                   Color(1.0f,  1.0f,  1.0f,  0.25f ));

        const Preference<Color> ResizeBrushFaceColor = Preference<Color>(                        "Renderer/Colors/Face color when resizing",                    Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedResizeBrushFaceColor = Preference<Color>(                "Renderer/Colors/Occluded face color when resizing",           Color(1.0f,  1.0f,  1.0f,  0.5f ));

        const Preference<Color> BrowserTextColor = Preference<Color>(                           "Texture browser/Texture color",                                Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> SelectedTextureColor = Preference<Color>(                       "Texture browser/Selected texture color",                       Color(0.8f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> UsedTextureColor = Preference<Color>(                           "Texture browser/Used texture color",                           Color(0.8f,  0.8f,  0.0f,  1.0f ));
        const Preference<Color> OverriddenTextureColor = Preference<Color>(                     "Texture browser/Overridden texture color",                     Color(0.5f,  0.5f,  0.5f,  1.0f ));
        const Preference<Color> BrowserGroupBackgroundColor = Preference<Color>(                "Texture browser/Group background color",                       Color(0.5f,  0.5f,  0.5f,  0.5f ));
        const Preference<int>   TextureBrowserFontSize = Preference<int>(                       "Texture browser/Font size",                                    12);
        const Preference<int>   EntityBrowserFontSize = Preference<int>(                        "Entity browser/Font size",                                     12);
        const Preference<float> TextureBrowserIconSize = Preference<float>(                     "Texture browser/Icon size",                                    1.0f);

#if defined _WIN32
        const Preference<float> CameraMoveSpeed = Preference<float>(                            "Controls/Camera/Move speed",                                   0.3f);
        const Preference<String> QuakePath = Preference<String>(                                "General/Quake path",                                           "C:\\Program Files\\Quake");
        const Preference<String> RendererFontName = Preference<String>(                         "Renderer/Font name",                                           "Arial");
#elif defined __APPLE__
        const Preference<float> CameraMoveSpeed = Preference<float>(                            "Controls/Camera/Move speed",                                   0.3f);
        const Preference<String> QuakePath = Preference<String>(                                "General/Quake path",                                           "/Applications/Quake");
        const Preference<String> RendererFontName = Preference<String>(                         "Renderer/Font name",                                           "LucidaGrande");
#elif defined __linux__
        const Preference<float> CameraMoveSpeed = Preference<float>(                            "Controls/Camera/Move speed",                                   0.5f);
        const Preference<String> QuakePath = Preference<String>(                                "General/Quake path",                                           "/Quake");
        const Preference<String> RendererFontName = Preference<String>(                         "Renderer/Font name",                                           "Arial");
#endif

        const Preference<int>   RendererInstancingMode = Preference<int>(                       "Renderer/Instancing mode",                                     0);
        const int               RendererInstancingModeAutodetect    = 0;
        const int               RendererInstancingModeForceOn       = 1;
        const int               RendererInstancingModeForceOff      = 2;

        const Preference<KeyboardShortcut>  CameraMoveForward = Preference<KeyboardShortcut>(   "Controls/Camera/Move Forward",     KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraForward, 'W', KeyboardShortcut::SCAny, "Move Camera Forward"));
        const Preference<KeyboardShortcut>  CameraMoveBackward = Preference<KeyboardShortcut>(  "Controls/Camera/Move Backward",    KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraForward, 'S', KeyboardShortcut::SCAny, "Move Camera Backward"));
        const Preference<KeyboardShortcut>  CameraMoveLeft = Preference<KeyboardShortcut>(      "Controls/Camera/Move Left",        KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraForward, 'A', KeyboardShortcut::SCAny, "Move Camera Left"));
        const Preference<KeyboardShortcut>  CameraMoveRight = Preference<KeyboardShortcut>(     "Controls/Camera/Move Right",       KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraForward, 'D', KeyboardShortcut::SCAny, "Move Camera Right"));

        const String FileMenu = "File";
        const String EditMenu = "Edit";
        const String ViewMenu = "View";

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
            return Utility::join(components, "/");
        }

        ShortcutMenuItem::ShortcutMenuItem(MenuItemType type, const KeyboardShortcut& shortcut, MenuItemParent* parent) :
        TextMenuItem(type, parent),
        m_shortcut(shortcut),
        m_preference(path(), m_shortcut) {
            assert(type == MITAction || type == MITCheck);
            PreferenceManager& prefs = PreferenceManager::preferences();
            const String p = path();
            Preference<KeyboardShortcut> preference(p, m_shortcut);
            m_shortcut = prefs.getKeyboardShortcut(preference);
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
            return Utility::join(components, " > ");
        }

        const KeyboardShortcut& ShortcutMenuItem::shortcut() const {
            return m_shortcut;
        }

        void ShortcutMenuItem::setShortcut(const KeyboardShortcut& shortcut) const {
            PreferenceManager& prefs = PreferenceManager::preferences();
            const String p = path();
            prefs.setKeyboardShortcut(m_preference, shortcut);
            m_shortcut = m_preference.value();
        }

        const KeyboardShortcut* ShortcutMenuItem::shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
            if (m_shortcut.matches(key, modifierKey1, modifierKey2, modifierKey3))
                return &m_shortcut;
            return NULL;
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

        Menu& MultiMenu::addMenu(const String& text, const int menuId) {
            Menu* menu = new Menu(text, this, menuId);
            addItem(MenuItem::Ptr(menu));
            return *menu;
        }

        PreferenceManager::PreferenceManager() {
#if defined __APPLE__
            m_saveInstantly = true;
#else
            m_saveInstantly = false;
#endif
        }

        void PreferenceManager::markAsUnsaved(const PreferenceBase* preference, ValueHolderBase* valueHolder) {
            UnsavedPreferences::iterator it = m_unsavedPreferences.find(preference);
            if (it == m_unsavedPreferences.end())
                m_unsavedPreferences[preference] = valueHolder;
            else
                delete valueHolder;
        }

        const Menu::MenuMap PreferenceManager::buildMenus() const {
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
#if defined __APPLE__
            editMenu->addActionItem(KeyboardShortcut(wxID_DELETE, WXK_BACK, KeyboardShortcut::SCAny, "Delete"));
#else
            editMenu->addActionItem(KeyboardShortcut(wxID_DELETE, WXK_DELETE, KeyboardShortcut::SCAny, "Delete"));
#endif
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
#ifdef __linux__ // unmodified cursor keys are not allowed as a menu accelerator on GTK
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesUp, WXK_SHIFT, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesDown, WXK_SHIFT, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesLeft, WXK_SHIFT, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesRight, WXK_SHIFT, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_SHIFT, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_SHIFT, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
#else
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesUp, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesDown, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesLeft, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesRight, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCW, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
#endif
            faceActionMenu.addSeparator();
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesUpFine, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCTextures, "Move Up by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesDownFine, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesLeftFine, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveTexturesRightFine, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCWFine, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 1"));
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditRotateTexturesCWFine, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 1"));
            faceActionMenu.addSeparator();
            faceActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPrintFilePositions, KeyboardShortcut::SCTextures, "Print Line Numbers"));

            Menu& objectActionMenu = actionMenu.addMenu("Objects", View::CommandIds::Menu::EditObjectActions);
#ifdef __linux__ // unmodified cursor keys are not allowed as a menu accelerator on GTK
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsForward, WXK_SHIFT, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsBackward, WXK_SHIFT, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsLeft, WXK_SHIFT, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsRight, WXK_SHIFT, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsUp, WXK_SHIFT, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsDown, WXK_SHIFT, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
#else
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsForward, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsBackward, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsLeft, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsRight, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsUp, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveObjectsDown, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
#endif
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsForward, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCObjects, "Duplicate & Move Forward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsBackward, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Backward"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCObjects, "Duplicate & Move Left"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCObjects, "Duplicate & Move Right"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsUp, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Duplicate & Move Up"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjectsDown, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Down"));
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditDuplicateObjects, WXK_CONTROL, 'D', KeyboardShortcut::SCObjects, "Duplicate"));
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
            MenuItem::Ptr snapVerticesItem = objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditSnapVertices, KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Snap Vertices"));
            objectActionMenu.addSeparator();
#ifdef __linux__ // tab is not allowed as a menu accelerator on GTK
            MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleAxisRestriction, 'X', KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Toggle Movement Axis"));
#else
            MenuItem::Ptr toggleAxisItem = objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleAxisRestriction, WXK_TAB, KeyboardShortcut::SCObjects | KeyboardShortcut::SCVertexTool, "Toggle Movement Axis"));
#endif
            objectActionMenu.addSeparator();
            objectActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPrintFilePositions, KeyboardShortcut::SCObjects | KeyboardShortcut::SCTextures, "Print Line Numbers"));

            Menu& vertexActionMenu = actionMenu.addMenu("Vertices", View::CommandIds::Menu::EditVertexActions);
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesForward, WXK_UP, KeyboardShortcut::SCVertexTool, "Move Forward"));
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesBackward, WXK_DOWN, KeyboardShortcut::SCVertexTool, "Move Backward"));
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesLeft, WXK_LEFT, KeyboardShortcut::SCVertexTool, "Move Left"));
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesRight, WXK_RIGHT, KeyboardShortcut::SCVertexTool, "Move Right"));
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesUp, WXK_PAGEUP, KeyboardShortcut::SCVertexTool, "Move Up"));
            vertexActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditMoveVerticesDown, WXK_PAGEDOWN, KeyboardShortcut::SCVertexTool, "Move Down"));
            vertexActionMenu.addSeparator();
            vertexActionMenu.addItem(snapVerticesItem);
            vertexActionMenu.addSeparator();
            vertexActionMenu.addItem(toggleAxisItem);

            Menu& clipActionMenu = actionMenu.addMenu("Clip Tool", View::CommandIds::Menu::EditClipActions);
            clipActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleClipSide, WXK_CONTROL, WXK_RETURN, KeyboardShortcut::SCClipTool, "Toggle Clip Side"));
            clipActionMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditPerformClip, WXK_RETURN, KeyboardShortcut::SCClipTool, "Perform Clip"));

            editMenu->addSeparator();
            editMenu->addCheckItem(KeyboardShortcut(View::CommandIds::Menu::EditToggleTextureLock, KeyboardShortcut::SCAny, "Texture Lock"));
#ifdef __linux__ // escape key is not allowed as a menu accelerator on GTK
            editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditNavigateUp, KeyboardShortcut::SCAny, "Navigate Up"));
#else
            editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditNavigateUp, WXK_ESCAPE, KeyboardShortcut::SCAny, "Navigate Up"));
#endif
            editMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::EditShowMapProperties, KeyboardShortcut::SCAny, "Map Properties..."));

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
            cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraToNextPoint, WXK_SHIFT, '+', KeyboardShortcut::SCAny, "Move to Next Point"));
            cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewMoveCameraToPreviousPoint, WXK_SHIFT, '-', KeyboardShortcut::SCAny, "Move to Previous Point"));
            cameraMenu.addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewCenterCameraOnSelection, WXK_CONTROL, WXK_SHIFT, 'C', KeyboardShortcut::SCAny, "Center on Selection"));

            viewMenu->addSeparator();
            viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToEntityTab, '1', KeyboardShortcut::SCAny, "Switch to Entity Inspector"));
            viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToFaceTab, '2', KeyboardShortcut::SCAny, "Switch to Face Inspector"));
            viewMenu->addActionItem(KeyboardShortcut(View::CommandIds::Menu::ViewSwitchToViewTab, '3', KeyboardShortcut::SCAny, "Switch to View Inspector"));
            return menus;
        }

        bool PreferenceManager::saveInstantly() const {
            return m_saveInstantly;
        }

        PreferenceBase::Set PreferenceManager::saveChanges() {
            PreferenceBase::Set changedPreferences;
            UnsavedPreferences::iterator it, end;
            for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                it->first->save(wxConfig::Get());
                changedPreferences.insert(it->first);
                delete it->second;
            }

            m_unsavedPreferences.clear();
            return changedPreferences;
        }

        PreferenceBase::Set PreferenceManager::discardChanges() {
            PreferenceBase::Set changedPreferences;
            UnsavedPreferences::iterator it, end;
            for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                it->first->setValue(it->second);
                changedPreferences.insert(it->first);
                delete it->second;
            }

            m_unsavedPreferences.clear();
            return changedPreferences;
        }

        bool PreferenceManager::getBool(const Preference<bool>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setBool(const Preference<bool>& preference, bool value) {
            bool previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<bool>(previousValue));
        }

        int PreferenceManager::getInt(const Preference<int>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setInt(const Preference<int>& preference, int value) {
            int previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<int>(previousValue));
        }

        float PreferenceManager::getFloat(const Preference<float>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setFloat(const Preference<float>& preference, float value) {
            float previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<float>(previousValue));
        }

        const String& PreferenceManager::getString(const Preference<String>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setString(const Preference<String>& preference, const String& value) {
            String previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<String>(previousValue));
        }

        const Color& PreferenceManager::getColor(const Preference<Color>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setColor(const Preference<Color>& preference, const Color& value) {
            Color previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<Color>(previousValue));
        }

        const KeyboardShortcut& PreferenceManager::getKeyboardShortcut(const Preference<KeyboardShortcut>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());

            return preference.value();
        }

        void PreferenceManager::setKeyboardShortcut(const Preference<KeyboardShortcut>& preference, const KeyboardShortcut& value) {
            KeyboardShortcut previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly)
                preference.save(wxConfig::Get());
            else
                markAsUnsaved(&preference, new ValueHolder<KeyboardShortcut>(previousValue));
        }
    }
}
