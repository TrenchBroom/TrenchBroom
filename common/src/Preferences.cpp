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

#include "Preferences.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "View/ActionContext.h"
#include "View/CommandIds.h"

namespace TrenchBroom {
    namespace Preferences {
        Preference<int> MapViewLayout(IO::Path("Views/Map view layout"), View::MapViewLayout_1Pane);
        
        Preference<bool>  ShowAxes(IO::Path("Renderer/Show axes"), true);
        Preference<Color> BackgroundColor(IO::Path("Renderer/Colors/Background"), Color(38, 38, 38));
        Preference<float> AxisLength(IO::Path("Renderer/Axis length"), 128.0f);
        Preference<Color> XAxisColor(IO::Path("Renderer/Colors/X axis"), Color(0xFF, 0x3D, 0x00, 0.7f));
        Preference<Color> YAxisColor(IO::Path("Renderer/Colors/Y axis"), Color(0x4B, 0x95, 0x00, 0.7f));
        Preference<Color> ZAxisColor(IO::Path("Renderer/Colors/Z axis"), Color(0x10, 0x9C, 0xFF, 0.7f));
        Preference<Color> PointFileColor(IO::Path("Renderer/Colors/Point file"), Color(0.0f, 1.0f, 0.0f, 1.0f));
        
        Preference<Color>& axisColor(Math::Axis::Type axis) {
            switch (axis) {
                case Math::Axis::AX:
                    return Preferences::XAxisColor;
                case Math::Axis::AY:
                    return Preferences::YAxisColor;
                case Math::Axis::AZ:
                default:
                    return Preferences::ZAxisColor;
            }
        }

        Preference<Color> CompassBackgroundColor(IO::Path("Renderer/Colors/Compass background"), Color(0.5f, 0.5f, 0.5f, 0.5f));
        Preference<Color> CompassBackgroundOutlineColor(IO::Path("Renderer/Colors/Compass background outline"), Color(1.0f, 1.0f, 1.0f, 0.5f));
        Preference<Color> CompassAxisOutlineColor(IO::Path("Renderer/Colors/Compass axis outline"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        
        Preference<Color> CameraFrustumColor(IO::Path("Renderer/Colors/Camera frustum"), Color(0.0f, 1.0f, 1.0f, 1.0f));
        
        Preference<Color> DefaultGroupColor(IO::Path("Renderer/Colors/Groups"), Color(0.7f,  0.4f,  1.0f,  1.0f));

        Preference<Color> FaceColor(IO::Path("Renderer/Colors/Faces"), Color(0.2f,  0.2f,  0.2f,  1.0f));
        Preference<Color> SelectedFaceColor(IO::Path("Renderer/Colors/Selected faces"), Color(0.7f,  0.45f, 0.45f, 1.0f));
        Preference<Color> LockedFaceColor(IO::Path("Renderer/Colors/Locked faces"), Color(0.35f, 0.35f, 0.6f,  1.0f));
        Preference<float> TransparentFaceAlpha(IO::Path("Renderer/Colors/Transparent faces"), 0.4f);
        Preference<Color> EdgeColor(IO::Path("Renderer/Colors/Edges"), Color(0.9f,  0.9f,  0.9f,  1.0f));
        Preference<Color> SelectedEdgeColor(IO::Path("Renderer/Colors/Selected edges"), Color(1.0f,  0.0f,  0.0f,  1.0f));
        Preference<Color> OccludedSelectedEdgeColor(IO::Path("Renderer/Colors/Occluded selected edges"), Color(1.0f,  0.0f,  0.0f,  0.5f));
        Preference<Color> LockedEdgeColor(IO::Path("Renderer/Colors/Locked edges"), Color(0.13f, 0.3f,  1.0f,  1.0f));
        Preference<Color> UndefinedEntityColor(IO::Path("Renderer/Colors/Undefined entity"), Color(0.5f,  0.5f,  0.5f,  1.0f));
        
        Preference<Color> SelectionBoundsColor(IO::Path("Renderer/Colors/Selection bounds"), Color(1.0f, 0.0f, 0.0f, 0.5f));
        
        Preference<Color> InfoOverlayTextColor(IO::Path("Renderer/Colors/Info overlay text"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> InfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Info overlay background"), Color(0.0f, 0.0f, 0.0f, 0.6f));
        Preference<Color> WeakInfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Weak info overlay background"), Color(0.0f, 0.0f, 0.0f, 0.3f));
        Preference<Color> SelectedInfoOverlayTextColor(IO::Path("Renderer/Colors/Selected info overlay text"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> SelectedInfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Selected info overlay background"), Color(1.0f, 0.0f, 0.0f, 0.6f));
        Preference<Color> LockedInfoOverlayTextColor(IO::Path("Renderer/Colors/Locked info overlay text"), Color(0.35f, 0.35f, 0.6f,  1.0f));
        Preference<Color> LockedInfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Locked info overlay background"), Color(0.0f, 0.0f, 0.0f, 0.6f));
        
        Preference<float> HandleRadius(IO::Path("Controls/Handle radius"), 3.0f);
        Preference<float> MaximumHandleDistance(IO::Path("Controls/Maximum handle distance"), 1000.0f);
        Preference<Color> HandleColor(IO::Path("Renderer/Colors/Handle"), Color(248, 230, 60, 1.0f));
        Preference<Color> OccludedHandleColor(IO::Path("Renderer/Colors/Occluded handle"), Color(248, 230, 60, 0.5f));
        Preference<Color> SelectedHandleColor(IO::Path("Renderer/Colors/Selected handle"), Color(1.0f, 0.0f, 0.0f, 1.0f));
        Preference<Color> OccludedSelectedHandleColor(IO::Path("Renderer/Colors/Occluded selected handle"), Color(1.0f, 0.0f, 0.0f, 0.5f));
        
        Preference<Color> ClipHandleColor(IO::Path("Renderer/Colors/Clip handle"), Color(1.0f, 0.5f, 0.0f, 1.0f));
        Preference<Color> ClipFaceColor(IO::Path("Renderer/Colors/Clip face"), Color(0.6f,  0.4f, 0.0f, 0.35f));
        
        Preference<Color> ResizeHandleColor(IO::Path("Renderer/Colors/Resize handle"), Color(248, 230, 60, 1.0f));
        Preference<float> RotateHandleRadius(IO::Path("Controls/Rotate handle radius"), 64.0f);
        Preference<Color> RotateHandleColor(IO::Path("Renderer/Colors/Rotate handle"), Color(248, 230, 60, 1.0f));
        
        Preference<Color> MoveTraceColor(IO::Path("Renderer/Colors/Move trace"), Color(0.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> OccludedMoveTraceColor(IO::Path("Renderer/Colors/Move trace"), Color(0.0f, 1.0f, 1.0f, 0.6f));
        
        Preference<Color> MoveIndicatorOutlineColor(IO::Path("Renderer/Colors/Move indicator outline"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> MoveIndicatorFillColor(IO::Path("Renderer/Colors/Move indicator fill"), Color(0.0f, 0.0f, 0.0f, 0.5f));
        
        Preference<Color> AngleIndicatorColor(IO::Path("Renderer/Colors/Angle indicator"), Color(1.0f, 1.0f, 1.0f, 1.0f));

        Preference<Color> TextureSeamColor(IO::Path("Renderer/Colors/Texture seam"), Color(1.0f, 1.0f, 0.0f, 1.0f));

        Preference<float> Brightness(IO::Path("Renderer/Brightness"), 1.4f);
        Preference<float> GridAlpha(IO::Path("Renderer/Grid/Alpha"), 0.5f);
        Preference<Color> GridColor2D(IO::Path("Rendere/Grid/Color2D"), Color(0.8f, 0.8f, 0.8f, 0.8f));

        Preference<int> TextureMinFilter(IO::Path("Renderer/Texture mode min filter"), 0x2700);
        Preference<int> TextureMagFilter(IO::Path("Renderer/Texture mode mag filter"), 0x2600);

        Preference<IO::Path>& RendererFontPath() {
#if defined __APPLE__
            static Preference<IO::Path> fontPath(IO::Path("Renderer/Font name"), IO::SystemPaths::findFontFile("LucidaGrande"));
#elif defined _WIN32
            static Preference<IO::Path> fontPath(IO::Path("Renderer/Font name"), IO::SystemPaths::findFontFile("Tahoma"));
#else
            static Preference<IO::Path> fontPath(IO::Path("Renderer/Font name"), IO::Path("fonts/SourceSansPro-Regular.otf"));
#endif
            return fontPath;
        }
    
    Preference<int> RendererFontSize(IO::Path("Renderer/Font size"), 13);
        
        Preference<int> BrowserFontSize(IO::Path("Browser/Font size"), 13);
        Preference<Color> BrowserTextColor(IO::Path("Browser/Text color"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> BrowserGroupBackgroundColor(IO::Path("Browser/Group background color"), Color(0.8f, 0.8f, 0.8f, 0.8f));
        Preference<float> TextureBrowserIconSize(IO::Path("Texture Browser/Icon size"), 1.0f);
        Preference<Color> TextureBrowserDefaultColor(IO::Path("Texture Browser/Default color"), Color(0.0f, 0.0f, 0.0f, 0.0f));
        Preference<Color> TextureBrowserSelectedColor(IO::Path("Texture Browser/Selected color"), Color(1.0f, 0.0f, 0.0f, 1.0f));
        Preference<Color> TextureBrowserUsedColor(IO::Path("Texture Browser/Used color"), Color(1.0f, 0.7f, 0.0f, 1.0f));
        
        Preference<float> CameraLookSpeed(IO::Path("Controls/Camera/Look speed"), 0.5f);
        Preference<bool>  CameraLookInvertH(IO::Path("Controls/Camera/Invert horizontal look"), false);
        Preference<bool>  CameraLookInvertV(IO::Path("Controls/Camera/Invert vertical look"), false);
        Preference<float> CameraPanSpeed(IO::Path("Controls/Camera/Pan speed"), 0.5f);
        Preference<bool>  CameraPanInvertH(IO::Path("Controls/Camera/Invert horizontal pan"), false);
        Preference<bool>  CameraPanInvertV(IO::Path("Controls/Camera/Invert vertical pan"), false);
        Preference<bool> CameraMouseWheelInvert(IO::Path("Controls/Camera/Invert mouse wheel"), false);
        Preference<float> CameraMoveSpeed(IO::Path("Controls/Camera/Move speed"), 0.3f);
        Preference<bool> CameraEnableAltMove(IO::Path("Controls/Camera/Use alt to move"), false);
        Preference<bool> CameraAltMoveInvert(IO::Path("Controls/Camera/Invert zoom direction when using alt to move"), false);
        Preference<bool> CameraMoveInCursorDir(IO::Path("Controls/Camera/Move camera in cursor dir"), false);

        Preference<float> CameraFlySpeed(IO::Path("Controls/Camera/Fly speed"), 0.5f);
        Preference<bool> CameraFlyInvertV(IO::Path("Controls/Camera/Invert vertical fly"), false);

        Preference<bool> Link2DCameras(IO::Path("Controls/Camera/Link 2D cameras"), true);

        Preference<View::KeyboardShortcut> CameraFlyForward(IO::Path("Controls/Camera/Move forward"), 'W');
        Preference<View::KeyboardShortcut> CameraFlyBackward(IO::Path("Controls/Camera/Move backward"), 'S');
        Preference<View::KeyboardShortcut> CameraFlyLeft(IO::Path("Controls/Camera/Move left"), 'A');
        Preference<View::KeyboardShortcut> CameraFlyRight(IO::Path("Controls/Camera/Move right"), 'D');
    }
}
