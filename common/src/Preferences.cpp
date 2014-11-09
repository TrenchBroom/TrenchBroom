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
        Preference<Color> BackgroundColor(IO::Path("Renderer/Colors/Background"), Color(255, 255, 255));
        Preference<float> AxisLength(IO::Path("Renderer/Axis length"), 128.0f);
        Preference<Color> XAxisColor(IO::Path("Renderer/Colors/X axis"), Color(0xFF, 0x3D, 0x00, 0.7f));
        Preference<Color> YAxisColor(IO::Path("Renderer/Colors/Y axis"), Color(0x4B, 0x95, 0x00, 0.7f));
        Preference<Color> ZAxisColor(IO::Path("Renderer/Colors/Z axis"), Color(0x10, 0x9C, 0xFF, 0.7f));
        Preference<Color> PointFileColor(IO::Path("Renderer/Colors/Point file"), Color(0.0f, 1.0f, 0.0f, 1.0f));
        
        Preference<Color> CompassBackgroundColor(IO::Path("Renderer/Colors/Compass background"), Color(0.5f, 0.5f, 0.5f, 0.5f));
        Preference<Color> CompassBackgroundOutlineColor(IO::Path("Renderer/Colors/Compass background outline"), Color(1.0f, 1.0f, 1.0f, 0.5f));
        Preference<Color> CompassAxisOutlineColor(IO::Path("Renderer/Colors/Compass axis outline"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        
        Preference<Color> CameraFrustumColor(IO::Path("Renderer/Colors/Camera frustum"), Color(0.0f, 1.0f, 1.0f, 1.0f));
        
        Preference<Color> FaceColor(IO::Path("Renderer/Colors/Faces"), Color(0.2f,  0.2f,  0.2f,  1.0f));
        Preference<Color> SelectedFaceColor(IO::Path("Renderer/Colors/Selected faces"), Color(0.7f,  0.45f, 0.45f, 1.0f));
        Preference<Color> LockedFaceColor(IO::Path("Renderer/Colors/Locked faces"), Color(0.35f, 0.35f, 0.6f,  1.0f));
        Preference<float> TransparentFaceAlpha(IO::Path("Renderer/Colors/Transparent faces"), 0.4f);
        Preference<Color> EdgeColor(IO::Path("Renderer/Colors/Edges"), Color(0.7f,  0.7f,  0.7f,  1.0f));
        Preference<Color> SelectedEdgeColor(IO::Path("Renderer/Colors/Selected edges"), Color(1.0f,  0.0f,  0.0f,  1.0f));
        Preference<Color> OccludedSelectedEdgeColor(IO::Path("Renderer/Colors/Occluded selected edges"), Color(1.0f,  0.0f,  0.0f,  0.5f));
        Preference<Color> LockedEdgeColor(IO::Path("Renderer/Colors/Locked edges"), Color(0.13f, 0.3f,  1.0f,  1.0f));
        Preference<Color> UndefinedEntityColor(IO::Path("Renderer/Colors/Undefined entity"), Color(0.5f,  0.5f,  0.5f,  1.0f));
        
        Preference<Color> InfoOverlayTextColor(IO::Path("Renderer/Colors/Info overlay text"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> InfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Info overlay background"), Color(0.0f, 0.0f, 0.0f, 0.6f));
        Preference<Color> SelectedInfoOverlayTextColor(IO::Path("Renderer/Colors/Selected info overlay text"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> SelectedInfoOverlayBackgroundColor(IO::Path("Renderer/Colors/Selected info overlay background"), Color(1.0f, 0.0f, 0.0f, 0.6f));
        
        Preference<float> HandleRadius(IO::Path("Controls/Handle radius"), 3.0f);
        Preference<float> HandleScalingFactor(IO::Path("Controls/Handle scaling factor"), 1.0f / 300.0f);
        Preference<float> MaximumHandleDistance(IO::Path("Controls/Maximum handle distance"), 1000.0f);
        Preference<Color> HandleColor(IO::Path("Renderer/Colors/Handle"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> OccludedHandleColor(IO::Path("Renderer/Colors/Occluded handle"), Color(1.0f, 1.0f, 1.0f, 0.5f));
        Preference<Color> SelectedHandleColor(IO::Path("Renderer/Colors/Selected handle"), Color(1.0f, 0.0f, 0.0f, 1.0f));
        Preference<Color> OccludedSelectedHandleColor(IO::Path("Renderer/Colors/Occluded selected handle"), Color(1.0f, 0.0f, 0.0f, 0.5f));
        
        Preference<Color> ClipPlaneColor(IO::Path("Renderer/Colors/Clip plane"), Color(1.0f, 1.0f, 1.0f, 0.25f));
        Preference<Color> ClipFaceColor(IO::Path("Renderer/Colors/Clip face"), Color(0.6f,  0.35f, 0.35f, 1.0f));
        Preference<Color> ClipEdgeColor(IO::Path("Renderer/Colors/Clip edge"), Color(1.0f,  0.0f,  0.0f,  1.0f));
        Preference<Color> ClipOccludedEdgeColor(IO::Path("Renderer/Colors/Clip edge"), Color(1.0f,  0.0f,  0.0f,  0.5f));
        
        Preference<Color> ResizeHandleColor(IO::Path("Renderer/Colors/Resize handle"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<float> RotateHandleRadius(IO::Path("Controls/Rotate handle radius"), 64.0f);
        Preference<Color> RotateHandleColor(IO::Path("Renderer/Colors/Rotate handle"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        
        Preference<Color> MoveTraceColor(IO::Path("Renderer/Colors/Move trace"), Color(0.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> OccludedMoveTraceColor(IO::Path("Renderer/Colors/Move trace"), Color(0.0f, 1.0f, 1.0f, 0.6f));
        
        Preference<Color> MoveIndicatorOutlineColor(IO::Path("Renderer/Colors/Move indicator outline"), Color(1.0f, 1.0f, 1.0f, 1.0f));
        Preference<Color> MoveIndicatorFillColor(IO::Path("Renderer/Colors/Move indicator fill"), Color(0.0f, 0.0f, 0.0f, 0.5f));
        
        Preference<Color> AngleIndicatorColor(IO::Path("Renderer/Colors/Angle indicator"), Color(1.0f, 1.0f, 1.0f, 1.0f));

        Preference<Color> TextureSeamColor(IO::Path("Renderer/Colors/Texture seam"), Color(1.0f, 1.0f, 0.0f, 1.0f));

        Preference<float> Brightness(IO::Path("Renderer/Brightness"), 1.4f);
        Preference<float> GridAlpha(IO::Path("Renderer/Grid/Alpha"), 0.5f);

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
        Preference<Color> BrowserTextColor(IO::Path("Browser/Text color"), Color(0.0f, 0.0f, 0.0f, 1.0f));
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
        Preference<float> CameraMoveSpeed(IO::Path("Controls/Camera/Move speed"), 0.3f);
        Preference<bool> CameraEnableAltMove(IO::Path("Controls/Camera/Use alt to move"), false);
        Preference<bool> CameraAltMoveInvert(IO::Path("Controls/Camera/Invert zoom direction when using alt to move"), false);
        Preference<bool> CameraMoveInCursorDir(IO::Path("Controls/Camera/Move camera in cursor dir"), false);

        Preference<float> CameraFlySpeed(IO::Path("Controls/Camera/Fly speed"), 0.5f);
        Preference<bool> CameraFlyInvertV(IO::Path("Controls/Camera/Invert vertical fly"), false);

        Preference<View::KeyboardShortcut> CameraFlyForward(IO::Path("Controls/Camera/Move forward"), 'W');
        Preference<View::KeyboardShortcut> CameraFlyBackward(IO::Path("Controls/Camera/Move backward"), 'S');
        Preference<View::KeyboardShortcut> CameraFlyLeft(IO::Path("Controls/Camera/Move left"), 'A');
        Preference<View::KeyboardShortcut> CameraFlyRight(IO::Path("Controls/Camera/Move right"), 'D');

        /*
        static const View::ViewShortcut ViewActions[] = {
            View::ViewShortcut(View::KeyboardShortcut('C'), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::ToggleClipTool, View::CommandIds::Actions::ToggleClipTool),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RETURN, WXK_CONTROL), View::ActionContext_ClipTool, View::CommandIds::Actions::ToggleClipSide, View::CommandIds::Actions::ToggleClipSide),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RETURN), View::ActionContext_ClipTool, View::CommandIds::Actions::PerformClip, View::CommandIds::Actions::PerformClip),
            View::ViewShortcut(View::KeyboardShortcut(WXK_BACK), View::ActionContext_ClipTool, View::CommandIds::Actions::DeleteLastClipPoint, View::CommandIds::Actions::DeleteLastClipPoint),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DELETE), View::ActionContext_ClipTool, View::CommandIds::Actions::DeleteLastClipPoint, View::CommandIds::Actions::DeleteLastClipPoint),

            View::ViewShortcut(View::KeyboardShortcut('V'), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::ToggleVertexTool, View::CommandIds::Actions::ToggleVertexTool),
            View::ViewShortcut(View::KeyboardShortcut(WXK_UP), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesUp, View::CommandIds::Actions::MoveVerticesForward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesDown, View::CommandIds::Actions::MoveVerticesBackward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesLeft, View::CommandIds::Actions::MoveVerticesLeft),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesRight, View::CommandIds::Actions::MoveVerticesRight),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesForward, View::CommandIds::Actions::MoveVerticesUp),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN), View::ActionContext_VertexTool, View::CommandIds::Actions::MoveVerticesBackward, View::CommandIds::Actions::MoveVerticesDown),

            View::ViewShortcut(View::KeyboardShortcut('R'), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::ToggleRotateObjectsTool, View::CommandIds::Actions::ToggleRotateObjectsTool),
            View::ViewShortcut(View::KeyboardShortcut(WXK_UP, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterUp, View::CommandIds::Actions::MoveRotationCenterForward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterDown, View::CommandIds::Actions::MoveRotationCenterBackward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterLeft, View::CommandIds::Actions::MoveRotationCenterLeft),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterRight, View::CommandIds::Actions::MoveRotationCenterRight),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterForward, View::CommandIds::Actions::MoveRotationCenterUp),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN, WXK_SHIFT), View::ActionContext_RotateTool, View::CommandIds::Actions::MoveRotationCenterBackward, View::CommandIds::Actions::MoveRotationCenterDown),
            
            View::ViewShortcut(View::KeyboardShortcut('F'), View::ActionContext_Any, View::CommandIds::Actions::None, View::CommandIds::Actions::ToggleFlyMode),

            View::ViewShortcut(View::KeyboardShortcut('X'), View::ActionContext_Any, View::CommandIds::Actions::None, View::CommandIds::Actions::ToggleMovementRestriction),

            View::ViewShortcut(View::KeyboardShortcut(WXK_BACK), View::ActionContext_NodeSelection, View::CommandIds::Actions::DeleteObjects, View::CommandIds::Actions::DeleteObjects),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DELETE), View::ActionContext_NodeSelection, View::CommandIds::Actions::DeleteObjects, View::CommandIds::Actions::DeleteObjects),

            View::ViewShortcut(View::KeyboardShortcut(WXK_UP), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsUp, View::CommandIds::Actions::MoveObjectsForward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsDown, View::CommandIds::Actions::MoveObjectsBackward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsLeft, View::CommandIds::Actions::MoveObjectsLeft),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsRight, View::CommandIds::Actions::MoveObjectsRight),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsForward, View::CommandIds::Actions::MoveObjectsUp),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::MoveObjectsBackward, View::CommandIds::Actions::MoveObjectsDown),

            View::ViewShortcut(View::KeyboardShortcut(WXK_UP, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::RollObjectsCW, View::CommandIds::Actions::RollObjectsCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::RollObjectsCCW, View::CommandIds::Actions::RollObjectsCCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::YawObjectsCW, View::CommandIds::Actions::YawObjectsCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::YawObjectsCCW, View::CommandIds::Actions::YawObjectsCCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::PitchObjectsCW, View::CommandIds::Actions::PitchObjectsCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::PitchObjectsCCW, View::CommandIds::Actions::PitchObjectsCCW),

            View::ViewShortcut(View::KeyboardShortcut('F', WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::FlipObjectsHorizontally, View::CommandIds::Actions::FlipObjectsHorizontally),
            View::ViewShortcut(View::KeyboardShortcut('F', WXK_CONTROL, WXK_ALT), View::ActionContext_NodeSelection | View::ActionContext_RotateTool, View::CommandIds::Actions::FlipObjectsVertically, View::CommandIds::Actions::FlipObjectsVertically),

            View::ViewShortcut(View::KeyboardShortcut('D', WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjects, View::CommandIds::Actions::DuplicateObjects),
            View::ViewShortcut(View::KeyboardShortcut(WXK_UP, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsUp, View::CommandIds::Actions::DuplicateObjectsForward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsDown, View::CommandIds::Actions::DuplicateObjectsBackward),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsLeft, View::CommandIds::Actions::DuplicateObjectsLeft),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsRight, View::CommandIds::Actions::DuplicateObjectsRight),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsForward, View::CommandIds::Actions::DuplicateObjectsUp),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), View::ActionContext_NodeSelection | View::ActionContext_AnyTool, View::CommandIds::Actions::DuplicateObjectsBackward, View::CommandIds::Actions::DuplicateObjectsDown),

            View::ViewShortcut(View::KeyboardShortcut(WXK_UP), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesUp),
            View::ViewShortcut(View::KeyboardShortcut(WXK_UP, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesUpCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_UP, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesUpFine),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesDown),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesDownCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_DOWN, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesDownFine),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesLeft),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesLeftCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_LEFT, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesLeftFine),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesRight),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesRightCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::MoveTexturesRightFine),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCWCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCWFine),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCCW),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN, WXK_SHIFT), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCCWCoarse),
            View::ViewShortcut(View::KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), View::ActionContext_FaceSelection, View::CommandIds::Actions::None, View::CommandIds::Actions::RotateTexturesCCWFine),

            View::ViewShortcut(View::KeyboardShortcut(WXK_ESCAPE), View::ActionContext_Any, View::CommandIds::Actions::Cancel, View::CommandIds::Actions::Cancel)
        };
        
        Preference<View::ViewShortcut::List> ViewShortcuts(IO::Path("Controls/Map view"), View::ViewShortcut::List(ViewActions, ViewActions + sizeof(ViewActions) / sizeof(ViewActions[0])));
         */
    }
}
