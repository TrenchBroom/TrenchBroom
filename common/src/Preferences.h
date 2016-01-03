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

#ifndef TrenchBroom_Preferences_h
#define TrenchBroom_Preferences_h

#include "TrenchBroom.h"
#include "Color.h"
#include "Preference.h"
#include "PreferenceManager.h"
#include "View/KeyboardShortcut.h"
#include "View/MapViewLayout.h"
#include "View/ViewShortcut.h"

#include <vector>

namespace TrenchBroom {
    namespace Preferences {
        extern Preference<int> MapViewLayout;
        
        extern Preference<bool>  ShowAxes;
        extern Preference<Color> BackgroundColor;
        extern Preference<float> AxisLength;
        extern Preference<Color> XAxisColor;
        extern Preference<Color> YAxisColor;
        extern Preference<Color> ZAxisColor;
        extern Preference<Color> PointFileColor;
        
        Preference<Color>& axisColor(Math::Axis::Type axis);
        
        extern Preference<Color> CompassBackgroundColor;
        extern Preference<Color> CompassBackgroundOutlineColor;
        extern Preference<Color> CompassAxisOutlineColor;
        
        extern Preference<Color> CameraFrustumColor;
        
        extern Preference<Color> DefaultGroupColor;
        
        extern Preference<Color> FaceColor;
        extern Preference<Color> SelectedFaceColor;
        extern Preference<Color> LockedFaceColor;
        extern Preference<float> TransparentFaceAlpha;
        extern Preference<Color> EdgeColor;
        extern Preference<Color> SelectedEdgeColor;
        extern Preference<Color> OccludedSelectedEdgeColor;
        extern Preference<Color> LockedEdgeColor;
        extern Preference<Color> UndefinedEntityColor;
        
        extern Preference<Color> SelectionBoundsColor;
        
        extern Preference<Color> InfoOverlayTextColor;
        extern Preference<Color> InfoOverlayBackgroundColor;
        extern Preference<Color> WeakInfoOverlayBackgroundColor;
        extern Preference<Color> SelectedInfoOverlayTextColor;
        extern Preference<Color> SelectedInfoOverlayBackgroundColor;
        extern Preference<Color> LockedInfoOverlayTextColor;
        extern Preference<Color> LockedInfoOverlayBackgroundColor;
        
        extern Preference<float> HandleRadius;
        extern Preference<float> MaximumHandleDistance;
        extern Preference<Color> HandleColor;
        extern Preference<Color> OccludedHandleColor;
        extern Preference<Color> SelectedHandleColor;
        extern Preference<Color> OccludedSelectedHandleColor;

        extern Preference<Color> ClipHandleColor;
        extern Preference<Color> ClipFaceColor;
        
        extern Preference<Color> ResizeHandleColor;
        extern Preference<float> RotateHandleRadius;
        extern Preference<Color> RotateHandleColor;
        
        extern Preference<Color> MoveTraceColor;
        extern Preference<Color> OccludedMoveTraceColor;

        extern Preference<Color> MoveIndicatorOutlineColor;
        extern Preference<Color> MoveIndicatorFillColor;
        
        extern Preference<Color> AngleIndicatorColor;
        
        extern Preference<Color> TextureSeamColor;
        
        extern Preference<float> Brightness;
        extern Preference<float> GridAlpha;
        extern Preference<Color> GridColor2D;
        
        extern Preference<int> TextureMinFilter;
        extern Preference<int> TextureMagFilter;
        
        Preference<IO::Path>& RendererFontPath();
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
        extern Preference<bool> CameraMouseWheelInvert;
        extern Preference<bool> CameraEnableAltMove;
        extern Preference<bool> CameraAltMoveInvert;
        extern Preference<bool> CameraMoveInCursorDir;
        
        extern Preference<float> CameraFlySpeed;
        extern Preference<bool> CameraFlyInvertV;
        
        extern Preference<bool> Link2DCameras;
        
        extern Preference<View::KeyboardShortcut> CameraFlyForward;
        extern Preference<View::KeyboardShortcut> CameraFlyBackward;
        extern Preference<View::KeyboardShortcut> CameraFlyLeft;
        extern Preference<View::KeyboardShortcut> CameraFlyRight;
    }
}

#endif
