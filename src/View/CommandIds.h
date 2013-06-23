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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
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
                const int EditToggleClipTool                 = Lowest +  12;
                const int EditToggleClipSide                 = Lowest +  13;
                const int EditPerformClip                    = Lowest +  14;
                const int EditToggleVertexTool               = Lowest +  15;
                const int EditMoveTexturesUp                 = Lowest +  18;
                const int EditMoveTexturesRight              = Lowest +  19;
                const int EditMoveTexturesDown               = Lowest +  20;
                const int EditMoveTexturesLeft               = Lowest +  21;
                const int EditRotateTexturesCW               = Lowest +  22;
                const int EditRotateTexturesCCW              = Lowest +  23;
                const int EditMoveObjectsForward             = Lowest +  24;
                const int EditMoveObjectsRight               = Lowest +  25;
                const int EditMoveObjectsBackward            = Lowest +  26;
                const int EditMoveObjectsLeft                = Lowest +  27;
                const int EditMoveObjectsUp                  = Lowest +  28;
                const int EditMoveObjectsDown                = Lowest +  29;
                const int EditRollObjectsCW                  = Lowest +  30;
                const int EditRollObjectsCCW                 = Lowest +  31;
                const int EditPitchObjectsCW                 = Lowest +  32;
                const int EditPitchObjectsCCW                = Lowest +  33;
                const int EditYawObjectsCW                   = Lowest +  34;
                const int EditYawObjectsCCW                  = Lowest +  35;
                const int EditFlipObjectsHorizontally        = Lowest +  36;
                const int EditFlipObjectsVertically          = Lowest +  37;
                const int EditDuplicateObjects               = Lowest +  38;
                const int EditActions                        = Lowest +  39;
                const int EditCreatePointEntity              = Lowest +  40;
                const int EditCreateBrushEntity              = Lowest +  41;
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
                const int EditMoveVerticesForward            = Lowest +  60;
                const int EditMoveVerticesBackward           = Lowest +  61;
                const int EditMoveVerticesLeft               = Lowest +  62;
                const int EditMoveVerticesRight              = Lowest +  63;
                const int EditMoveVerticesUp                 = Lowest +  64;
                const int EditMoveVerticesDown               = Lowest +  65;
                const int EditMoveTexturesUpFine             = Lowest +  66;
                const int EditMoveTexturesRightFine          = Lowest +  67;
                const int EditMoveTexturesDownFine           = Lowest +  68;
                const int EditMoveTexturesLeftFine           = Lowest +  69;
                const int EditRotateTexturesCWFine           = Lowest +  70;
                const int EditRotateTexturesCCWFine          = Lowest +  71;
                const int ViewCenterCameraOnSelection        = Lowest +  72;
                const int EditToggleRotateObjectsTool        = Lowest +  73;
                const int ViewIncGridSize                    = Lowest +  74;
                const int ViewDecGridSize                    = Lowest +  75;
                const int FileLoadPointFile                  = Lowest +  76;
                const int FileUnloadPointFile                = Lowest +  77;
                const int ViewMoveCameraToNextPoint          = Lowest +  78;
                const int ViewMoveCameraToPreviousPoint      = Lowest +  79;
                const int EditShowMapProperties              = Lowest +  80;
                const int EditSnapVertices                   = Lowest +  81;
                const int EditCorrectVertices                = Lowest +  82;
                const int HelpShowHelp                       = Lowest +  83;
                const int EditPasteAtOriginalPosition        = Lowest +  84;
                const int EditSelectByFilePosition           = Lowest +  85;
                const int ViewSwitchToEntityTab              = Lowest +  86;
                const int ViewSwitchToFaceTab                = Lowest +  87;
                const int ViewSwitchToViewTab                = Lowest +  88;
                const int EditDuplicateObjectsForward        = Lowest +  89;
                const int EditDuplicateObjectsRight          = Lowest +  90;
                const int EditDuplicateObjectsBackward       = Lowest +  91;
                const int EditDuplicateObjectsLeft           = Lowest +  92;
                const int EditDuplicateObjectsUp             = Lowest +  93;
                const int EditDuplicateObjectsDown           = Lowest +  94;
                const int EditNavigateUp                     = Lowest +  95;
                const int FileOpenRecent                     = Lowest +  96;
                const int EditVertexActions                  = Lowest +  97;
                const int EditClipActions                    = Lowest +  98;
                const int EditObjectActions                  = Lowest +  99;
                const int EditFaceActions                    = Lowest + 100;
                const int EditPrintFilePositions             = Lowest + 101;
                const int EditToggleAxisRestriction          = Lowest + 102;
                const int Highest                            = Lowest + 199;
            }
        }
    }
}

#endif
