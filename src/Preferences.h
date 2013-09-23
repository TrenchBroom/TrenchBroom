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

#ifndef TrenchBroom_Preferences_h
#define TrenchBroom_Preferences_h

#include "Color.h"
#include "Preference.h"
#include "PreferenceManager.h"

namespace TrenchBroom {
    namespace Preferences {
        extern Preference<String> QuakePath;
        extern Preference<String> Quake2Path;
        extern Preference<String> Hexen2Path;
        
        extern Preference<Color> BackgroundColor;
        extern Preference<float> AxisLength;
        extern Preference<Color> XAxisColor;
        extern Preference<Color> YAxisColor;
        extern Preference<Color> ZAxisColor;
        extern Preference<Color> FaceColor;
        extern Preference<Color> SelectedFaceColor;
        extern Preference<Color> EdgeColor;
        extern Preference<Color> SelectedEdgeColor;
        extern Preference<Color> OccludedSelectedEdgeColor;
        extern Preference<Color> UndefinedEntityColor;
        
        extern Preference<Color> InfoOverlayTextColor;
        extern Preference<Color> InfoOverlayBackgroundColor;
        extern Preference<Color> SelectedInfoOverlayTextColor;
        extern Preference<Color> SelectedInfoOverlayBackgroundColor;
        
        extern Preference<double> HandleRadius;
        extern Preference<double> HandleScalingFactor;
        extern Preference<double> MaximumHandleDistance;
        extern Preference<Color> HandleColor;
        extern Preference<Color> OccludedHandleColor;
        extern Preference<Color> SelectedHandleColor;
        extern Preference<Color> OccludedSelectedHandleColor;
        extern Preference<Color> ClipPlaneColor;
        
        extern Preference<float> Brightness;
        extern Preference<float> GridAlpha;
        extern Preference<bool> GridCheckerboard;
        extern Preference<bool> ShadeFaces;
        extern Preference<bool> UseFog;
        
        extern Preference<String> RendererFontName;
        extern Preference<int> RendererFontSize;
        
        extern Preference<int> BrowserFontSize;
        extern Preference<Color> BrowserTextColor;
        extern Preference<Color> BrowserGroupBackgroundColor;
        extern Preference<float> TextureBrowserIconSize;
        extern Preference<Color> TextureBrowserDefaultColor;
        extern Preference<Color> TextureBrowserSelectedColor;
        extern Preference<Color> TextureBrowserUsedColor;
        
        extern Preference<float> CameraLookSpeed;
        extern Preference<bool> CameraLookInvertH;
        extern Preference<bool> CameraLookInvertV;
        extern Preference<float> CameraPanSpeed;
        extern Preference<bool> CameraPanInvertH;
        extern Preference<bool> CameraPanInvertV;
        extern Preference<float> CameraMoveSpeed;
        extern Preference<bool> CameraEnableAltMove;
        extern Preference<bool> CameraMoveInCursorDir;
    }
}

#endif
