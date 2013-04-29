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

        const String FileMenu = "File";
        const String EditMenu = "Edit";
        const String ViewMenu = "View";

        String ShortcutMenuItem::path() const {
            StringList components;
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
        m_shortcut(shortcut) {
            assert(type == MITAction || type == MITCheck);
            PreferenceManager& prefs = PreferenceManager::preferences();
            const String p = path();
            Preference<KeyboardShortcut> preference(p, m_shortcut);
            prefs.getKeyboardShortcut(preference);
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
            Preference<KeyboardShortcut> preference(p, m_shortcut);
            prefs.setKeyboardShortcut(preference, shortcut);
            m_shortcut = preference.value();
        }

        const Menu* MultiMenu::menuById(const int menuId) const {
            List::const_iterator it, end;
            for (it = m_items.begin(), end = m_items.end(); it != end; ++it) {
                const Menu* menu = static_cast<Menu*>(it->get());
                if (menu->menuId() == menuId)
                    return menu;
            }
            return NULL;
        }

        Menu& MultiMenu::addMenu(const String& text, const int menuId) {
            Menu* menu = new Menu(text, this, menuId);
            m_items.push_back(MenuItem::Ptr(menu));
            return *menu;
        }
    }
}
