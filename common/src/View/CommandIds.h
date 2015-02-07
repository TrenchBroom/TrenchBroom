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

#ifndef TrenchBroom_CommandIds_h
#define TrenchBroom_CommandIds_h

#include <wx/window.h>

namespace TrenchBroom {
    namespace View {
        namespace CommandIds {
            namespace Menu {
                const int Lowest                             = wxID_HIGHEST +  1;
                const int EditSelectAll                      = Lowest +   2;
                const int EditSelectSiblings                 = Lowest +   3;
                const int EditSelectTouching                 = Lowest +   4;
                const int EditSelectNone                     = Lowest +   5;
                const int EditHideSelected                   = Lowest +   6;
                const int EditHideUnselected                 = Lowest +   7;
                const int EditUnhideAll                      = Lowest +   8;
                const int EditLockSelected                   = Lowest +   9;
                const int EditLockUnselected                 = Lowest +  10;
                const int EditUnlockAll                      = Lowest +  11;
                const int EditSnapVertices                   = Lowest +  12;
                const int EditToggleTextureLock              = Lowest +  42;
                const int ViewToggleShowGrid                 = Lowest +  43;
                const int ViewToggleSnapToGrid               = Lowest +  44;
                const int ViewSetGridSize1                   = Lowest +  45;
                const int ViewSetGridSize2                   = Lowest +  46;
                const int ViewSetGridSize4                   = Lowest +  47;
                const int ViewSetGridSize8                   = Lowest +  48;
                const int ViewSetGridSize16                  = Lowest +  49;
                const int ViewSetGridSize32                  = Lowest +  50;
                const int ViewSetGridSize64                  = Lowest +  51;
                const int ViewSetGridSize128                 = Lowest +  52;
                const int ViewSetGridSize256                 = Lowest +  53;
                const int ViewMoveCameraForward              = Lowest +  54;
                const int ViewMoveCameraBackward             = Lowest +  55;
                const int ViewMoveCameraLeft                 = Lowest +  56;
                const int ViewMoveCameraRight                = Lowest +  57;
                const int ViewMoveCameraUp                   = Lowest +  58;
                const int ViewMoveCameraDown                 = Lowest +  59;
                const int ViewCenterCameraOnSelection        = Lowest +  72;
                const int ViewIncGridSize                    = Lowest +  74;
                const int ViewDecGridSize                    = Lowest +  75;
                const int FileLoadPointFile                  = Lowest +  76;
                const int FileUnloadPointFile                = Lowest +  77;
                const int ViewMoveCameraToNextPoint          = Lowest +  78;
                const int ViewMoveCameraToPreviousPoint      = Lowest +  79;

                const int HelpShowHelp                       = Lowest +  83;
                const int EditPasteAtOriginalPosition        = Lowest +  84;
                const int EditSelectByFilePosition           = Lowest +  85;
                const int ViewSwitchToMapInspector           = Lowest +  86;
                const int ViewSwitchToEntityInspector        = Lowest +  87;
                const int ViewSwitchToFaceInspector          = Lowest +  88;
                const int EditNavigateUp                     = Lowest +  95;
                const int FileOpenRecent                     = Lowest +  96;
                const int EditPrintFilePositions             = Lowest + 101;
                const int EditSelectInside                   = Lowest + 103;
                const int ViewToggleCameraFlyMode            = Lowest + 106;
                const int EditRepeat                         = Lowest + 107;
                const int EditClearRepeat                    = Lowest + 108;
                const int ViewMoveCameraToPosition           = Lowest + 109;
                const int EditReplaceTexture                 = Lowest + 110;

                const int FileRecentDocuments                = Lowest + 190;

                const int Highest                            = Lowest + 200;
            }
            
            namespace Actions {
                const int Lowest                             = Menu::Highest + 1;
                const int Nothing                            = wxID_NONE;
                const int ToggleCreateBrushTool              = Lowest +   1;
                const int PerformCreateBrush                 = Lowest +   2;
                const int ToggleClipTool                     = Lowest +   3;
                const int ToggleClipSide                     = Lowest +   4;
                const int PerformClip                        = Lowest +   5;
                const int DeleteLastClipPoint                = Lowest +   6;

                const int ToggleVertexTool                   = Lowest +   7;
                const int MoveVerticesForward                = Lowest +   8;
                const int MoveVerticesBackward               = Lowest +   9;
                const int MoveVerticesLeft                   = Lowest +  10;
                const int MoveVerticesRight                  = Lowest +  11;
                const int MoveVerticesUp                     = Lowest +  12;
                const int MoveVerticesDown                   = Lowest +  13;

                const int ToggleRotateObjectsTool            = Lowest +  14;
                const int ToggleFlyMode                      = Lowest +  15;
                
                const int ToggleMovementRestriction          = Lowest +  16;

                const int DeleteObjects                      = Lowest +  17;
                
                const int MoveObjectsForward                 = Lowest +  18;
                const int MoveObjectsRight                   = Lowest +  19;
                const int MoveObjectsBackward                = Lowest +  20;
                const int MoveObjectsLeft                    = Lowest +  21;
                const int MoveObjectsUp                      = Lowest +  22;
                const int MoveObjectsDown                    = Lowest +  23;
                
                const int RollObjectsCW                      = Lowest +  24;
                const int RollObjectsCCW                     = Lowest +  25;
                const int PitchObjectsCW                     = Lowest +  26;
                const int PitchObjectsCCW                    = Lowest +  27;
                const int YawObjectsCW                       = Lowest +  28;
                const int YawObjectsCCW                      = Lowest +  29;
                
                const int FlipObjectsHorizontally            = Lowest +  30;
                const int FlipObjectsVertically              = Lowest +  31;
                
                const int DuplicateObjectsForward            = Lowest +  32;
                const int DuplicateObjectsRight              = Lowest +  33;
                const int DuplicateObjectsBackward           = Lowest +  34;
                const int DuplicateObjectsLeft               = Lowest +  35;
                const int DuplicateObjectsUp                 = Lowest +  36;
                const int DuplicateObjectsDown               = Lowest +  37;
            
                const int DuplicateObjects                   = Lowest +  38;

                const int MoveTexturesUp                     = Lowest +  39;
                const int MoveTexturesRight                  = Lowest +  40;
                const int MoveTexturesDown                   = Lowest +  41;
                const int MoveTexturesLeft                   = Lowest +  42;
                const int RotateTexturesCW                   = Lowest +  43;
                const int RotateTexturesCCW                  = Lowest +  44;

                const int Cancel                             = Lowest +  45;
                
                const int MoveRotationCenterForward          = Lowest +  46;
                const int MoveRotationCenterBackward         = Lowest +  47;
                const int MoveRotationCenterLeft             = Lowest +  48;
                const int MoveRotationCenterRight            = Lowest +  49;
                const int MoveRotationCenterUp               = Lowest +  50;
                const int MoveRotationCenterDown             = Lowest +  51;

                const int CycleMapViews                      = Lowest +  52;

                /*
                const int CorrectVertices                    = Lowest +  82;
                 */

                const int Highest                            = Lowest + 200;
            }

            namespace CreateEntityPopupMenu {
                static const int LowestPointEntityItem              = wxID_HIGHEST + 2000;
                static const int HighestPointEntityItem             = LowestPointEntityItem + 999;
                static const int LowestBrushEntityItem              = HighestPointEntityItem + 1;
                static const int HighestBrushEntityItem             = LowestBrushEntityItem + 999;
                static const int ReparentBrushes                    = HighestBrushEntityItem + 1;
                static const int MoveBrushesToWorld                 = HighestBrushEntityItem + 2;
            }
        }
    }
}

#endif
