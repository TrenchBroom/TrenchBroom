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
        const Preference<bool> CameraLookInvertX = Preference<bool>(                            "Controls/Camera/Look X inverted",                              false);
        const Preference<bool> CameraLookInvertY = Preference<bool>(                            "Controls/Camera/Look Y inverted",                              false);
        const Preference<bool> CameraPanInvertX = Preference<bool>(                             "Controls/Camera/Pan X inverted",                               false);
        const Preference<bool> CameraPanInvertY = Preference<bool>(                             "Controls/Camera/Pan Y inverted",                               false);
        const Preference<float> VertexHandleSize = Preference<float>(                           "Controls/Vertex handle size",                                  1.5f);
        const Preference<float> CameraFieldOfVision = Preference<float>(                        "Renderer/Camera field of vision",                              90.0f);
        const Preference<float> CameraNearPlane = Preference<float>(                            "Renderer/Camera near plane",                                   1.0f);
        const Preference<float> CameraFarPlane = Preference<float>(                             "Renderer/Camera far plane",                                    5000.0f);

        const Preference<float> InfoOverlayFadeDistance = Preference<float>(                    "Renderer/Info overlay fade distance",                          400.0f);
        const Preference<float> SelectedInfoOverlayFadeDistance = Preference<float>(            "Renderer/Selected info overlay fade distance",                 400.0f);
        const Preference<int> RendererFontSize = Preference<int>(                               "Renderer/Font size",                                           16);
        const Preference<float> RendererBrightness = Preference<float>(                         "Renderer/Brightness",                                          1.0f);

        const Preference<Color> BackgroundColor = Preference<Color>(                            "Renderer/Colors/Background",                                   Color(0.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> GridColor = Preference<Color>(                                  "Renderer/Colors/Grid",                                         Color(1.0f,  1.0f,  1.0f,  0.2f ));

        const Preference<Color> FaceColor = Preference<Color>(                                  "Renderer/Colors/Face",                                         Color(0.2f,  0.2f,  0.2f,  1.0f ));
        const Preference<Color> SelectedFaceColor = Preference<Color>(                          "Renderer/Colors/Selected face",                                Color(0.6f,  0.35f, 0.35f, 1.0f ));
        const Preference<Color> LockedFaceColor = Preference<Color>(                            "Renderer/Colors/Locked face",                                  Color(0.5f,  0.5f,  0.6f,  1.0f ));

        const Preference<Color> EdgeColor = Preference<Color>(                                  "Renderer/Colors/Edge",                                         Color(0.6f,  0.6f,  0.6f,  1.0f ));
        const Preference<Color> SelectedEdgeColor = Preference<Color>(                          "Renderer/Colors/Selected edge",                                Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEdgeColor = Preference<Color>(                  "Renderer/Colors/Occluded selected edge",                       Color(1.0f,  0.0f,  0.0f,  0.5f ));
        const Preference<Color> LockedEdgeColor = Preference<Color>(                            "Renderer/Colors/Locked edge",                                  Color(0.13f, 0.3f,  1.0f,  1.0f ));

        const Preference<Color> EntityBoundsColor = Preference<Color>(                          "Renderer/Colors/Entity bounds",                                Color(0.5f,  0.5f,  0.5f,  1.0f ));
        const Preference<Color> SelectedEntityBoundsColor = Preference<Color>(                  "Renderer/Colors/Selected entity bounds",                       Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEntityBoundsColor = Preference<Color>(          "Renderer/Colors/Occluded selected entity bounds",              Color(1.0f,  0.0f,  0.0f,  0.5f ));
        const Preference<Color> LockedEntityBoundsColor = Preference<Color>(                    "Renderer/Colors/Locked entity bounds",                         Color(0.13f, 0.3f,  1.0f,  1.0f ));
        const Preference<Color> EntityBoundsWireframeColor = Preference<Color>(                 "Renderer/Colors/Entity bounds (wireframe mode)",               Color(0.13f, 0.3f,  1.0f,  1.0f ));

        const Preference<Color> SelectionGuideColor = Preference<Color>(                        "Renderer/Colors/Selection guide",                              Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectionGuideColor = Preference<Color>(                "Renderer/Colors/Occluded selection guide",                     Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> InfoOverlayTextColor = Preference<Color>(                       "Renderer/Colors/Info overlay text",                            Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> InfoOverlayBackgroundColor = Preference<Color>(                 "Renderer/Colors/Info overlay background",                      Color(0.0f,  0.0f,  0.0f,  0.6f ));
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

        const Preference<Color> EdgeHandleColor = Preference<Color>(                            "Renderer/Colors/edge handle",                                  Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedEdgeHandleColor = Preference<Color>(                    "Renderer/Colors/Occluded edge handle",                         Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedEdgeHandleColor = Preference<Color>(                    "Renderer/Colors/Selected edge handle",                         Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedEdgeHandleColor = Preference<Color>(            "Renderer/Colors/Occluded selected edge handle",                Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> FaceHandleColor = Preference<Color>(                            "Renderer/Colors/face handle",                                  Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> OccludedFaceHandleColor = Preference<Color>(                    "Renderer/Colors/Occluded face handle",                         Color(1.0f,  1.0f,  1.0f,  0.5f ));
        const Preference<Color> SelectedFaceHandleColor = Preference<Color>(                    "Renderer/Colors/Selected face handle",                         Color(1.0f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> OccludedSelectedFaceHandleColor = Preference<Color>(            "Renderer/Colors/Occluded selected face handle",                Color(1.0f,  0.0f,  0.0f,  0.5f ));

        const Preference<Color> BrowserTextureColor = Preference<Color>(                        "Texture browser/Texture color",                                Color(1.0f,  1.0f,  1.0f,  1.0f ));
        const Preference<Color> SelectedTextureColor = Preference<Color>(                       "Texture browser/Selected texture color",                       Color(0.8f,  0.0f,  0.0f,  1.0f ));
        const Preference<Color> UsedTextureColor = Preference<Color>(                           "Texture browser/Used texture color",                           Color(0.8f,  0.8f,  0.0f,  1.0f ));
        const Preference<Color> OverriddenTextureColor = Preference<Color>(                     "Texture browser/Overridden texture color",                     Color(0.5f,  0.5f,  0.5f,  1.0f ));

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
    }
}
