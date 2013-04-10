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

#include "View/CommandIds.h"

using namespace TrenchBroom::View::CommandIds;

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
        const Preference<int>   RendererFontSize = Preference<int>(                             "Renderer/Font size",                                           16);
        const Preference<float> RendererBrightness = Preference<float>(                         "Renderer/Brightness",                                          1.0f);
        const Preference<float> GridAlpha = Preference<float>(                                  "Renderer/Grid Alpha",                                          0.25f);
        const Preference<bool>  GridCheckerboard = Preference<bool>(                            "Renderer/Grid Checkerboard",                                   false);

        const Preference<Color> EntityRotationDecoratorColor = Preference<Color>(               "Renderer/Colors/Decorators/Entity rotation",                   Color(1.0f,  1.0f,  1.0f,  1.0f ));

        const Preference<Color> XColor = Preference<Color>(                                     "Renderer/Colors/X",                                            Color(0xFF, 0x3D, 0x00));
        const Preference<Color> YColor = Preference<Color>(                                     "Renderer/Colors/Y",                                            Color(0x89, 0xFF, 0x00));
        const Preference<Color> ZColor = Preference<Color>(                                     "Renderer/Colors/Z",                                            Color(0x10, 0x9C, 0xFF));
        const Preference<Color> BackgroundColor = Preference<Color>(                            "Renderer/Colors/Background",                                   Color(0.0f,  0.0f,  0.0f,  1.0f ));

        const Preference<Color> GuideColor = Preference<Color>(                                 "Renderer/Colors/Guide",                                        Color(1.0f,  0.0f,  0.0f,  0.3f ));
        const Preference<Color> HoveredGuideColor = Preference<Color>(                          "Renderer/Colors/Hovered guide",                                Color(1.0f,  0.0f,  0.0f,  0.7f ));

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

        const Preference<KeyboardShortcut>  FileNew = Preference<KeyboardShortcut>(                             "Menu/File/New",                                        KeyboardShortcut(wxID_NEW, WXK_CONTROL, 'N', KeyboardShortcut::SCAny, "New"));
        const Preference<KeyboardShortcut>  FileOpen = Preference<KeyboardShortcut>(                            "Menu/File/Open",                                       KeyboardShortcut(wxID_OPEN, WXK_CONTROL, 'O', KeyboardShortcut::SCAny, "Open..."));
        const Preference<KeyboardShortcut>  FileSave = Preference<KeyboardShortcut>(                            "Menu/File/Save",                                       KeyboardShortcut(wxID_SAVE, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save"));
        const Preference<KeyboardShortcut>  FileSaveAs = Preference<KeyboardShortcut>(                          "Menu/File/Save as",                                    KeyboardShortcut(wxID_SAVEAS, WXK_SHIFT, WXK_CONTROL, 'S', KeyboardShortcut::SCAny, "Save as..."));
        const Preference<KeyboardShortcut>  FileLoadPointFile = Preference<KeyboardShortcut>(                   "Menu/File/Load Point File",                            KeyboardShortcut(Menu::FileLoadPointFile, KeyboardShortcut::SCAny, "Load Point File..."));
        const Preference<KeyboardShortcut>  FileUnloadPointFile = Preference<KeyboardShortcut>(                 "Menu/File/Unload Point File",                          KeyboardShortcut(Menu::FileUnloadPointFile, KeyboardShortcut::SCAny, "Unload Point File"));
        const Preference<KeyboardShortcut>  FileClose = Preference<KeyboardShortcut>(                           "Menu/File/Close",                                      KeyboardShortcut(wxID_CLOSE, WXK_CONTROL, 'W', KeyboardShortcut::SCAny, "Close"));

        const Preference<KeyboardShortcut>  EditUndo = Preference<KeyboardShortcut>(                            "Menu/File/Undo",                                       KeyboardShortcut(wxID_UNDO, WXK_CONTROL, 'Z', KeyboardShortcut::SCAny, "Undo"));
        const Preference<KeyboardShortcut>  EditRedo = Preference<KeyboardShortcut>(                            "Menu/File/Redo",                                       KeyboardShortcut(wxID_REDO, WXK_CONTROL, WXK_SHIFT, 'Z', KeyboardShortcut::SCAny, "Redo"));
        const Preference<KeyboardShortcut>  EditCut = Preference<KeyboardShortcut>(                             "Menu/File/Cut",                                        KeyboardShortcut(wxID_CUT, WXK_CONTROL, 'X', KeyboardShortcut::SCAny, "Cut"));
        const Preference<KeyboardShortcut>  EditCopy = Preference<KeyboardShortcut>(                            "Menu/File/Copy",                                       KeyboardShortcut(wxID_COPY, WXK_CONTROL, 'C', KeyboardShortcut::SCAny, "Copy"));
        const Preference<KeyboardShortcut>  EditPaste = Preference<KeyboardShortcut>(                           "Menu/File/Paste",                                      KeyboardShortcut(wxID_PASTE, WXK_CONTROL, 'V', KeyboardShortcut::SCAny, "Paste"));
        const Preference<KeyboardShortcut>  EditPasteAtOriginalPosition = Preference<KeyboardShortcut>(         "Menu/File/Paste at Original Position",                 KeyboardShortcut(Menu::EditPasteAtOriginalPosition, WXK_CONTROL, WXK_SHIFT, 'V', KeyboardShortcut::SCAny, "Paste at Original Position"));
        const Preference<KeyboardShortcut>  EditDelete = Preference<KeyboardShortcut>(                          "Menu/File/Delete",                                     KeyboardShortcut(wxID_DELETE, WXK_BACK, KeyboardShortcut::SCAny, "Delete"));

        const Preference<KeyboardShortcut>  EditSelectAll = Preference<KeyboardShortcut>(                       "Menu/Edit/Select All",                                 KeyboardShortcut(Menu::EditSelectAll, WXK_CONTROL, 'A', KeyboardShortcut::SCAny, "Select All"));
        const Preference<KeyboardShortcut>  EditSelectSiblings = Preference<KeyboardShortcut>(                  "Menu/Edit/Select Siblings",                            KeyboardShortcut(Menu::EditSelectSiblings, WXK_CONTROL, WXK_ALT, 'A', KeyboardShortcut::SCAny, "Select Siblings"));
        const Preference<KeyboardShortcut>  EditSelectTouching = Preference<KeyboardShortcut>(                  "Menu/Edit/Select Touching",                            KeyboardShortcut(Menu::EditSelectTouching, WXK_CONTROL, 'T', KeyboardShortcut::SCAny, "Select Touching"));
        const Preference<KeyboardShortcut>  EditSelectByFilePosition = Preference<KeyboardShortcut>(            "Menu/Edit/Select by File Position",                    KeyboardShortcut(Menu::EditSelectByFilePosition, KeyboardShortcut::SCAny, "Select by Line Number"));
        const Preference<KeyboardShortcut>  EditSelectNone = Preference<KeyboardShortcut>(                      "Menu/Edit/Select None",                                KeyboardShortcut(Menu::EditSelectNone, WXK_CONTROL, WXK_SHIFT, 'A', KeyboardShortcut::SCAny, "Select None"));
        const Preference<KeyboardShortcut>  EditHideSelected = Preference<KeyboardShortcut>(                    "Menu/Edit/Hide Selected",                              KeyboardShortcut(Menu::EditHideSelected, WXK_CONTROL, 'H', KeyboardShortcut::SCAny, "Hide Selected"));
        const Preference<KeyboardShortcut>  EditHideUnselected = Preference<KeyboardShortcut>(                  "Menu/Edit/Hide Unselected",                            KeyboardShortcut(Menu::EditHideUnselected, WXK_CONTROL, WXK_ALT, 'H', KeyboardShortcut::SCAny, "Hide Unselected"));
        const Preference<KeyboardShortcut>  EditUnhideAll = Preference<KeyboardShortcut>(                       "Menu/Edit/Unhide All",                                 KeyboardShortcut(Menu::EditUnhideAll, WXK_CONTROL, WXK_SHIFT, 'H', KeyboardShortcut::SCAny, "Unhide All"));
        const Preference<KeyboardShortcut>  EditLockSelected = Preference<KeyboardShortcut>(                    "Menu/Edit/Lock Selected",                              KeyboardShortcut(Menu::EditLockSelected, WXK_CONTROL, 'L', KeyboardShortcut::SCAny, "Lock Selected"));
        const Preference<KeyboardShortcut>  EditLockUnselected = Preference<KeyboardShortcut>(                  "Menu/Edit/Lock Unselected",                            KeyboardShortcut(Menu::EditLockUnselected, WXK_CONTROL, WXK_ALT, 'L', KeyboardShortcut::SCAny, "Lock Unselected"));
        const Preference<KeyboardShortcut>  EditUnlockAll = Preference<KeyboardShortcut>(                       "Menu/Edit/Unlock All",                                 KeyboardShortcut(Menu::EditUnlockAll, WXK_CONTROL, WXK_SHIFT, 'L', KeyboardShortcut::SCAny, "Unlock All"));
        const Preference<KeyboardShortcut>  EditToggleTextureLock = Preference<KeyboardShortcut>(               "Menu/Edit/Toggle Texture Lock",                        KeyboardShortcut(Menu::EditToggleTextureLock, KeyboardShortcut::SCAny, "Texture Lock"));
        const Preference<KeyboardShortcut>  EditNavigateUp = Preference<KeyboardShortcut>(                      "Menu/Edit/Navigate Up",                                KeyboardShortcut(Menu::EditNavigateUp, WXK_ESCAPE, KeyboardShortcut::SCAny, "Navigate Up"));
        const Preference<KeyboardShortcut>  EditShowMapProperties = Preference<KeyboardShortcut>(               "Menu/Edit/Show Map Properties Dialog",                 KeyboardShortcut(Menu::EditShowMapProperties, KeyboardShortcut::SCAny, "Map Properties"));

        const Preference<KeyboardShortcut>  EditToolsToggleClipTool = Preference<KeyboardShortcut>(             "Menu/Edit/Tools/Toggle Clip Tool",                     KeyboardShortcut(Menu::EditToggleClipTool, 'C', KeyboardShortcut::SCAny, "Clip Tool"));
        const Preference<KeyboardShortcut>  EditToolsToggleClipSide = Preference<KeyboardShortcut>(             "Menu/Edit/Tools/Toggle Clip Side",                     KeyboardShortcut(Menu::EditToggleClipSide, WXK_TAB, KeyboardShortcut::SCAny, "Toggle Clip Side"));
        const Preference<KeyboardShortcut>  EditToolsPerformClip = Preference<KeyboardShortcut>(                "Menu/Edit/Tools/Perform Clip",                         KeyboardShortcut(Menu::EditPerformClip, WXK_RETURN, KeyboardShortcut::SCAny, "Perform Clip"));
        const Preference<KeyboardShortcut>  EditToolsToggleVertexTool = Preference<KeyboardShortcut>(           "Menu/Edit/Tools/Toggle Vertex Tool",                   KeyboardShortcut(Menu::EditToggleVertexTool, 'V', KeyboardShortcut::SCAny, "Vertex Tool"));
        const Preference<KeyboardShortcut>  EditToolsToggleRotateTool = Preference<KeyboardShortcut>(           "Menu/Edit/Tools/Toggle Rotate Tool",                   KeyboardShortcut(Menu::EditToggleRotateObjectsTool, 'R', KeyboardShortcut::SCAny, "Rotate Tool"));

        const Preference<KeyboardShortcut>  EditActionsMoveTexturesUp = Preference<KeyboardShortcut>(           "Menu/Edit/Actions/Move Textures Up (Coarse)",          KeyboardShortcut(Menu::EditMoveTexturesUp, WXK_UP, KeyboardShortcut::SCTextures, "Move Up"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesDown = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Move Textures Down (Coarse)",        KeyboardShortcut(Menu::EditMoveTexturesDown, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesLeft = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Move Textures Left (Coarse)",        KeyboardShortcut(Menu::EditMoveTexturesLeft, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesRight = Preference<KeyboardShortcut>(        "Menu/Edit/Actions/Move Textures Right (Coarse)",       KeyboardShortcut(Menu::EditMoveTexturesRight, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right"));
        const Preference<KeyboardShortcut>  EditActionsRotateTexturesCW = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Rotate Textures CW (Coarse)",        KeyboardShortcut(Menu::EditRotateTexturesCW, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 15"));
        const Preference<KeyboardShortcut>  EditActionsRotateTexturesCCW = Preference<KeyboardShortcut>(        "Menu/Edit/Actions/Rotate Textures CCW (Coarse)",       KeyboardShortcut(Menu::EditRotateTexturesCW, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 15"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesUpFine = Preference<KeyboardShortcut>(       "Menu/Edit/Actions/Move Textures Up (Fine)",            KeyboardShortcut(Menu::EditMoveTexturesUp, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCTextures, "Move Up by 1"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesDownFine = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Move Textures Down (Fine)",          KeyboardShortcut(Menu::EditMoveTexturesDown, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCTextures, "Move Down by 1"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesLeftFine = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Move Textures Left (Fine)",          KeyboardShortcut(Menu::EditMoveTexturesLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCTextures, "Move Left by 1"));
        const Preference<KeyboardShortcut>  EditActionsMoveTexturesRightFine = Preference<KeyboardShortcut>(    "Menu/Edit/Actions/Move Textures Right (Fine)",         KeyboardShortcut(Menu::EditMoveTexturesRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCTextures, "Move Right by 1"));
        const Preference<KeyboardShortcut>  EditActionsRotateTexturesCWFine = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Rotate Textures CW (Fine)",          KeyboardShortcut(Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCTextures, "Rotate Clockwise by 1"));
        const Preference<KeyboardShortcut>  EditActionsRotateTexturesCCWFine = Preference<KeyboardShortcut>(    "Menu/Edit/Actions/Rotate Textures CCW (Fine)",         KeyboardShortcut(Menu::EditRotateTexturesCW, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCTextures, "Rotate Counter-clockwise by 1"));

        const Preference<KeyboardShortcut>  EditActionsMoveObjectsForward = Preference<KeyboardShortcut>(       "Menu/Edit/Actions/Move Objects Forward",               KeyboardShortcut(Menu::EditMoveObjectsForward, WXK_UP, KeyboardShortcut::SCObjects, "Move Forward"));
        const Preference<KeyboardShortcut>  EditActionsMoveObjectsBackward = Preference<KeyboardShortcut>(      "Menu/Edit/Actions/Move Objects Backward",              KeyboardShortcut(Menu::EditMoveObjectsBackward, WXK_DOWN, KeyboardShortcut::SCObjects, "Move Backward"));
        const Preference<KeyboardShortcut>  EditActionsMoveObjectsLeft = Preference<KeyboardShortcut>(          "Menu/Edit/Actions/Move Objects Left",                  KeyboardShortcut(Menu::EditMoveObjectsLeft, WXK_LEFT, KeyboardShortcut::SCObjects, "Move Left"));
        const Preference<KeyboardShortcut>  EditActionsMoveObjectsRight = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Move Objects Right",                 KeyboardShortcut(Menu::EditMoveObjectsRight, WXK_RIGHT, KeyboardShortcut::SCObjects, "Move Right"));
        const Preference<KeyboardShortcut>  EditActionsMoveObjectsUp = Preference<KeyboardShortcut>(            "Menu/Edit/Actions/Move Objects Up",                    KeyboardShortcut(Menu::EditMoveObjectsUp, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Move Up"));
        const Preference<KeyboardShortcut>  EditActionsMoveObjectsDown = Preference<KeyboardShortcut>(          "Menu/Edit/Actions/Move Objects Down",                  KeyboardShortcut(Menu::EditMoveObjectsDown, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Move Down"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsForward = Preference<KeyboardShortcut>(  "Menu/Edit/Actions/Duplicate Objects Forward",          KeyboardShortcut(Menu::EditDuplicateObjectsForward, WXK_CONTROL, WXK_UP, KeyboardShortcut::SCObjects, "Duplicate & Move Forward"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsBackward = Preference<KeyboardShortcut>( "Menu/Edit/Actions/Duplicate Objects Backward",         KeyboardShortcut(Menu::EditDuplicateObjectsBackward, WXK_CONTROL, WXK_DOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Backward"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsLeft = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Duplicate Objects Left",             KeyboardShortcut(Menu::EditDuplicateObjectsLeft, WXK_CONTROL, WXK_LEFT, KeyboardShortcut::SCObjects, "Duplicate & Move Left"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsRight = Preference<KeyboardShortcut>(    "Menu/Edit/Actions/Duplicate Objects Right",            KeyboardShortcut(Menu::EditDuplicateObjectsRight, WXK_CONTROL, WXK_RIGHT, KeyboardShortcut::SCObjects, "Duplicate & Move Right"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsUp = Preference<KeyboardShortcut>(       "Menu/Edit/Actions/Duplicate Objects Up",               KeyboardShortcut(Menu::EditDuplicateObjectsUp, WXK_CONTROL, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Duplicate & Move Up"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjectsDown = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Duplicate Objects Down",             KeyboardShortcut(Menu::EditDuplicateObjectsDown, WXK_CONTROL, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Duplicate & Move Down"));
        const Preference<KeyboardShortcut>  EditActionsRollObjectsCW = Preference<KeyboardShortcut>(            "Menu/Edit/Actions/Roll Objects CW",                    KeyboardShortcut(Menu::EditRollObjectsCW, WXK_ALT, WXK_UP, KeyboardShortcut::SCObjects, "Rotate Clockwise by 90"));
        const Preference<KeyboardShortcut>  EditActionsRollObjectsCCW = Preference<KeyboardShortcut>(           "Menu/Edit/Actions/Roll Objects CCW",                   KeyboardShortcut(Menu::EditRollObjectsCCW, WXK_ALT, WXK_DOWN, KeyboardShortcut::SCObjects, "Rotate Counter-clockwise by 90"));
        const Preference<KeyboardShortcut>  EditActionsYawObjectsCW = Preference<KeyboardShortcut>(             "Menu/Edit/Actions/Yaw Objects CW",                     KeyboardShortcut(Menu::EditYawObjectsCW, WXK_ALT, WXK_LEFT, KeyboardShortcut::SCObjects, "Rotate Left by 90"));
        const Preference<KeyboardShortcut>  EditActionsYawObjectsCCW = Preference<KeyboardShortcut>(            "Menu/Edit/Actions/Yaw Objects CCW",                    KeyboardShortcut(Menu::EditYawObjectsCCW, WXK_ALT, WXK_RIGHT, KeyboardShortcut::SCObjects, "Rotate Right by 90"));
        const Preference<KeyboardShortcut>  EditActionsPitchObjectsCW = Preference<KeyboardShortcut>(           "Menu/Edit/Actions/Pitch Objects CW",                   KeyboardShortcut(Menu::EditPitchObjectsCW, WXK_ALT, WXK_PAGEUP, KeyboardShortcut::SCObjects, "Rotate Up by 90"));
        const Preference<KeyboardShortcut>  EditActionsPitchObjectsCCW = Preference<KeyboardShortcut>(          "Menu/Edit/Actions/Pitch Objects CCW",                  KeyboardShortcut(Menu::EditPitchObjectsCCW, WXK_ALT, WXK_PAGEDOWN, KeyboardShortcut::SCObjects, "Rotate Down by 90"));
        const Preference<KeyboardShortcut>  EditActionsFlipObjectsHorizontally = Preference<KeyboardShortcut>(  "Menu/Edit/Actions/Flip Objects Horizontally",          KeyboardShortcut(Menu::EditFlipObjectsHorizontally, WXK_CONTROL, 'F', KeyboardShortcut::SCObjects, "Flip Horizontally"));
        const Preference<KeyboardShortcut>  EditActionsFlipObjectsVertically = Preference<KeyboardShortcut>(    "Menu/Edit/Actions/Flip Objects Vertically",            KeyboardShortcut(Menu::EditFlipObjectsVertically, WXK_CONTROL, WXK_ALT, 'F', KeyboardShortcut::SCObjects, "Flip Vertically"));
        const Preference<KeyboardShortcut>  EditActionsDuplicateObjects = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Duplicate Objects",                  KeyboardShortcut(Menu::EditDuplicateObjects, WXK_CONTROL, 'D', KeyboardShortcut::SCObjects, "Duplicate"));

        const Preference<KeyboardShortcut>  EditActionsMoveVerticesForward = Preference<KeyboardShortcut>(      "Menu/Edit/Actions/Move Vertices Forward",              KeyboardShortcut(Menu::EditMoveVerticesForward, WXK_UP, KeyboardShortcut::SCVertexTool, "Move Forward"));
        const Preference<KeyboardShortcut>  EditActionsMoveVerticesBackward = Preference<KeyboardShortcut>(     "Menu/Edit/Actions/Move Vertices Backward",             KeyboardShortcut(Menu::EditMoveVerticesBackward, WXK_DOWN, KeyboardShortcut::SCVertexTool, "Move Backward"));
        const Preference<KeyboardShortcut>  EditActionsMoveVerticesLeft = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Move Vertices Left",                 KeyboardShortcut(Menu::EditMoveVerticesLeft, WXK_LEFT, KeyboardShortcut::SCVertexTool, "Move Left"));
        const Preference<KeyboardShortcut>  EditActionsMoveVerticesRight = Preference<KeyboardShortcut>(        "Menu/Edit/Actions/Move Vertices Right",                KeyboardShortcut(Menu::EditMoveVerticesRight, WXK_RIGHT, KeyboardShortcut::SCVertexTool, "Move Right"));
        const Preference<KeyboardShortcut>  EditActionsMoveVerticesUp = Preference<KeyboardShortcut>(           "Menu/Edit/Actions/Move Vertices Up",                   KeyboardShortcut(Menu::EditMoveVerticesUp, WXK_PAGEUP, KeyboardShortcut::SCVertexTool, "Move Up"));
        const Preference<KeyboardShortcut>  EditActionsMoveVerticesDown = Preference<KeyboardShortcut>(         "Menu/Edit/Actions/Move Vertices Down",                 KeyboardShortcut(Menu::EditMoveVerticesDown, WXK_PAGEDOWN, KeyboardShortcut::SCVertexTool, "Move Down"));

        const Preference<KeyboardShortcut>  EditActionsCorrectVertices = Preference<KeyboardShortcut>(          "Menu/Edit/Actions/Correct Vertices",                   KeyboardShortcut(Menu::EditCorrectVertices, KeyboardShortcut::SCVertexTool | KeyboardShortcut::SCObjects, "Correct Vertices"));
        const Preference<KeyboardShortcut>  EditActionsSnapVertices = Preference<KeyboardShortcut>(             "Menu/Edit/Actions/Snap Vertices",                      KeyboardShortcut(Menu::EditSnapVertices, KeyboardShortcut::SCVertexTool | KeyboardShortcut::SCObjects, "Snap Vertices"));

        const Preference<KeyboardShortcut>  ViewGridToggleShowGrid = Preference<KeyboardShortcut>(              "Menu/View/Grid/Toggle Show Grid",                      KeyboardShortcut(Menu::ViewToggleShowGrid, WXK_CONTROL, 'G', KeyboardShortcut::SCAny, "Show Grid"));
        const Preference<KeyboardShortcut>  ViewGridToggleSnapToGrid = Preference<KeyboardShortcut>(            "Menu/View/Grid/Toggle Snap to Grid",                   KeyboardShortcut(Menu::ViewToggleSnapToGrid, WXK_CONTROL, WXK_SHIFT, 'G', KeyboardShortcut::SCAny, "Snap to Grid"));
        const Preference<KeyboardShortcut>  ViewGridIncGridSize = Preference<KeyboardShortcut>(                 "Menu/View/Grid/Increase Grid Size",                    KeyboardShortcut(Menu::ViewIncGridSize, WXK_CONTROL, '+', KeyboardShortcut::SCAny, "Increase Grid Size"));
        const Preference<KeyboardShortcut>  ViewGridDecGridSize = Preference<KeyboardShortcut>(                 "Menu/View/Grid/Decrease Grid Size",                    KeyboardShortcut(Menu::ViewDecGridSize, WXK_CONTROL, '-', KeyboardShortcut::SCAny, "Decrease Grid Size"));
        const Preference<KeyboardShortcut>  ViewGridSetSize1 = Preference<KeyboardShortcut>(                    "Menu/View/Grid/Set Grid Size 1",                       KeyboardShortcut(Menu::ViewSetGridSize1, WXK_CONTROL, '1', KeyboardShortcut::SCAny, "Set Grid Size 1"));
        const Preference<KeyboardShortcut>  ViewGridSetSize2 = Preference<KeyboardShortcut>(                    "Menu/View/Grid/Set Grid Size 2",                       KeyboardShortcut(Menu::ViewSetGridSize2, WXK_CONTROL, '2', KeyboardShortcut::SCAny, "Set Grid Size 2"));
        const Preference<KeyboardShortcut>  ViewGridSetSize4 = Preference<KeyboardShortcut>(                    "Menu/View/Grid/Set Grid Size 4",                       KeyboardShortcut(Menu::ViewSetGridSize4, WXK_CONTROL, '3', KeyboardShortcut::SCAny, "Set Grid Size 4"));
        const Preference<KeyboardShortcut>  ViewGridSetSize8 = Preference<KeyboardShortcut>(                    "Menu/View/Grid/Set Grid Size 8",                       KeyboardShortcut(Menu::ViewSetGridSize8, WXK_CONTROL, '4', KeyboardShortcut::SCAny, "Set Grid Size 8"));
        const Preference<KeyboardShortcut>  ViewGridSetSize16 = Preference<KeyboardShortcut>(                   "Menu/View/Grid/Set Grid Size 16",                      KeyboardShortcut(Menu::ViewSetGridSize16, WXK_CONTROL, '5', KeyboardShortcut::SCAny, "Set Grid Size 16"));
        const Preference<KeyboardShortcut>  ViewGridSetSize32 = Preference<KeyboardShortcut>(                   "Menu/View/Grid/Set Grid Size 32",                      KeyboardShortcut(Menu::ViewSetGridSize32, WXK_CONTROL, '6', KeyboardShortcut::SCAny, "Set Grid Size 32"));
        const Preference<KeyboardShortcut>  ViewGridSetSize64 = Preference<KeyboardShortcut>(                   "Menu/View/Grid/Set Grid Size 64",                      KeyboardShortcut(Menu::ViewSetGridSize64, WXK_CONTROL, '7', KeyboardShortcut::SCAny, "Set Grid Size 64"));
        const Preference<KeyboardShortcut>  ViewGridSetSize128 = Preference<KeyboardShortcut>(                  "Menu/View/Grid/Set Grid Size 128",                     KeyboardShortcut(Menu::ViewSetGridSize128, WXK_CONTROL, '8', KeyboardShortcut::SCAny, "Set Grid Size 128"));
        const Preference<KeyboardShortcut>  ViewGridSetSize256 = Preference<KeyboardShortcut>(                  "Menu/View/Grid/Set Grid Size 256",                     KeyboardShortcut(Menu::ViewSetGridSize256, WXK_CONTROL, '9', KeyboardShortcut::SCAny, "Set Grid Size 256"));

        const Preference<KeyboardShortcut>  ViewCameraMoveForward = Preference<KeyboardShortcut>(               "Menu/View/Camera/Move Forward",                        KeyboardShortcut(Menu::ViewMoveCameraForward, 'W', KeyboardShortcut::SCAny, "Move Forward"));
        const Preference<KeyboardShortcut>  ViewCameraMoveBackward = Preference<KeyboardShortcut>(              "Menu/View/Camera/Move Backward",                       KeyboardShortcut(Menu::ViewMoveCameraBackward, 'S', KeyboardShortcut::SCAny, "Move Backward"));
        const Preference<KeyboardShortcut>  ViewCameraMoveLeft = Preference<KeyboardShortcut>(                  "Menu/View/Camera/Move Left",                           KeyboardShortcut(Menu::ViewMoveCameraLeft, 'A', KeyboardShortcut::SCAny, "Move Left"));
        const Preference<KeyboardShortcut>  ViewCameraMoveRight = Preference<KeyboardShortcut>(                 "Menu/View/Camera/Move Right",                          KeyboardShortcut(Menu::ViewMoveCameraRight, 'D', KeyboardShortcut::SCAny, "Move Right"));
        const Preference<KeyboardShortcut>  ViewCameraMoveUp = Preference<KeyboardShortcut>(                    "Menu/View/Camera/Move Up",                             KeyboardShortcut(Menu::ViewMoveCameraUp, WXK_SHIFT, 'W', KeyboardShortcut::SCAny, "Move Up"));
        const Preference<KeyboardShortcut>  ViewCameraMoveDown = Preference<KeyboardShortcut>(                  "Menu/View/Camera/Move Down",                           KeyboardShortcut(Menu::ViewMoveCameraDown, WXK_SHIFT, 'S', KeyboardShortcut::SCAny, "Move Down"));
        const Preference<KeyboardShortcut>  ViewCameraMoveToNextPoint = Preference<KeyboardShortcut>(           "Menu/View/Camera/Move to Next Point",                  KeyboardShortcut(Menu::ViewMoveCameraToNextPoint, WXK_SHIFT, '+', KeyboardShortcut::SCAny, "Move to Next Point"));
        const Preference<KeyboardShortcut>  ViewCameraMoveToPreviousPoint = Preference<KeyboardShortcut>(       "Menu/View/Camera/Move to Previous Point",              KeyboardShortcut(Menu::ViewMoveCameraToPreviousPoint, WXK_SHIFT, '-', KeyboardShortcut::SCAny, "Move to Previous Point"));
        const Preference<KeyboardShortcut>  ViewCameraCenterCameraOnSelection = Preference<KeyboardShortcut>(   "Menu/View/Camera/Center on Selection",                 KeyboardShortcut(Menu::ViewCenterCameraOnSelection, WXK_CONTROL, WXK_SHIFT, 'C', KeyboardShortcut::SCAny, "Center on Selection"));

        const Preference<KeyboardShortcut>  ViewSwitchToEntityTab = Preference<KeyboardShortcut>(               "Menu/View/Switch to Entity tab",                       KeyboardShortcut(Menu::ViewSwitchToEntityTab, '1', KeyboardShortcut::SCAny, "Switch to Entity Inspector"));
        const Preference<KeyboardShortcut>  ViewSwitchToFaceTab = Preference<KeyboardShortcut>(                 "Menu/View/Switch to Face tab",                         KeyboardShortcut(Menu::ViewSwitchToFaceTab, '2', KeyboardShortcut::SCAny, "Switch to Face Inspector"));
        const Preference<KeyboardShortcut>  ViewSwitchToViewTab = Preference<KeyboardShortcut>(                 "Menu/View/Switch to View tab",                         KeyboardShortcut(Menu::ViewSwitchToViewTab, '3', KeyboardShortcut::SCAny, "Switch to View Inspector"));
    }
}
