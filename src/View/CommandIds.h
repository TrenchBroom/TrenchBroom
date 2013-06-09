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
                static const int EditSelectAll                      = wxWindow::NewControlId();
                static const int EditSelectSiblings                 = wxWindow::NewControlId();
                static const int EditSelectTouching                 = wxWindow::NewControlId();
                static const int EditSelectNone                     = wxWindow::NewControlId();
                static const int EditHideSelected                   = wxWindow::NewControlId();
                static const int EditHideUnselected                 = wxWindow::NewControlId();
                static const int EditUnhideAll                      = wxWindow::NewControlId();
                static const int EditLockSelected                   = wxWindow::NewControlId();
                static const int EditLockUnselected                 = wxWindow::NewControlId();
                static const int EditUnlockAll                      = wxWindow::NewControlId();
                static const int EditToggleClipTool                 = wxWindow::NewControlId();
                static const int EditToggleClipSide                 = wxWindow::NewControlId();
                static const int EditPerformClip                    = wxWindow::NewControlId();
                static const int EditToggleVertexTool               = wxWindow::NewControlId();
                static const int EditMoveTexturesUp                 = wxWindow::NewControlId();
                static const int EditMoveTexturesRight              = wxWindow::NewControlId();
                static const int EditMoveTexturesDown               = wxWindow::NewControlId();
                static const int EditMoveTexturesLeft               = wxWindow::NewControlId();
                static const int EditRotateTexturesCW               = wxWindow::NewControlId();
                static const int EditRotateTexturesCCW              = wxWindow::NewControlId();
                static const int EditMoveObjectsForward             = wxWindow::NewControlId();
                static const int EditMoveObjectsRight               = wxWindow::NewControlId();
                static const int EditMoveObjectsBackward            = wxWindow::NewControlId();
                static const int EditMoveObjectsLeft                = wxWindow::NewControlId();
                static const int EditMoveObjectsUp                  = wxWindow::NewControlId();
                static const int EditMoveObjectsDown                = wxWindow::NewControlId();
                static const int EditRollObjectsCW                  = wxWindow::NewControlId();
                static const int EditRollObjectsCCW                 = wxWindow::NewControlId();
                static const int EditPitchObjectsCW                 = wxWindow::NewControlId();
                static const int EditPitchObjectsCCW                = wxWindow::NewControlId();
                static const int EditYawObjectsCW                   = wxWindow::NewControlId();
                static const int EditYawObjectsCCW                  = wxWindow::NewControlId();
                static const int EditFlipObjectsHorizontally        = wxWindow::NewControlId();
                static const int EditFlipObjectsVertically          = wxWindow::NewControlId();
                static const int EditDuplicateObjects               = wxWindow::NewControlId();
                static const int EditActions                        = wxWindow::NewControlId();
                static const int EditCreatePointEntity              = wxWindow::NewControlId();
                static const int EditCreateBrushEntity              = wxWindow::NewControlId();
                static const int EditToggleTextureLock              = wxWindow::NewControlId();
                static const int ViewToggleShowGrid                 = wxWindow::NewControlId();
                static const int ViewToggleSnapToGrid               = wxWindow::NewControlId();
                static const int ViewSetGridSize1                   = wxWindow::NewControlId();
                static const int ViewSetGridSize2                   = wxWindow::NewControlId();
                static const int ViewSetGridSize4                   = wxWindow::NewControlId();
                static const int ViewSetGridSize8                   = wxWindow::NewControlId();
                static const int ViewSetGridSize16                  = wxWindow::NewControlId();
                static const int ViewSetGridSize32                  = wxWindow::NewControlId();
                static const int ViewSetGridSize64                  = wxWindow::NewControlId();
                static const int ViewSetGridSize128                 = wxWindow::NewControlId();
                static const int ViewSetGridSize256                 = wxWindow::NewControlId();
                static const int ViewMoveCameraForward              = wxWindow::NewControlId();
                static const int ViewMoveCameraBackward             = wxWindow::NewControlId();
                static const int ViewMoveCameraLeft                 = wxWindow::NewControlId();
                static const int ViewMoveCameraRight                = wxWindow::NewControlId();
                static const int ViewMoveCameraUp                   = wxWindow::NewControlId();
                static const int ViewMoveCameraDown                 = wxWindow::NewControlId();
                static const int EditMoveVerticesForward            = wxWindow::NewControlId();
                static const int EditMoveVerticesBackward           = wxWindow::NewControlId();
                static const int EditMoveVerticesLeft               = wxWindow::NewControlId();
                static const int EditMoveVerticesRight              = wxWindow::NewControlId();
                static const int EditMoveVerticesUp                 = wxWindow::NewControlId();
                static const int EditMoveVerticesDown               = wxWindow::NewControlId();
                static const int EditMoveTexturesUpFine             = wxWindow::NewControlId();
                static const int EditMoveTexturesRightFine          = wxWindow::NewControlId();
                static const int EditMoveTexturesDownFine           = wxWindow::NewControlId();
                static const int EditMoveTexturesLeftFine           = wxWindow::NewControlId();
                static const int EditRotateTexturesCWFine           = wxWindow::NewControlId();
                static const int EditRotateTexturesCCWFine          = wxWindow::NewControlId();
                static const int ViewCenterCameraOnSelection        = wxWindow::NewControlId();
                static const int EditToggleRotateObjectsTool        = wxWindow::NewControlId();
                static const int ViewIncGridSize                    = wxWindow::NewControlId();
                static const int ViewDecGridSize                    = wxWindow::NewControlId();
                static const int FileLoadPointFile                  = wxWindow::NewControlId();
                static const int FileUnloadPointFile                = wxWindow::NewControlId();
                static const int ViewMoveCameraToNextPoint          = wxWindow::NewControlId();
                static const int ViewMoveCameraToPreviousPoint      = wxWindow::NewControlId();
                static const int EditShowMapProperties              = wxWindow::NewControlId();
                static const int EditSnapVertices                   = wxWindow::NewControlId();
                static const int EditCorrectVertices                = wxWindow::NewControlId();
                static const int HelpShowHelp                       = wxWindow::NewControlId();
                static const int EditPasteAtOriginalPosition        = wxWindow::NewControlId();
                static const int EditSelectByFilePosition           = wxWindow::NewControlId();
                static const int ViewSwitchToEntityTab              = wxWindow::NewControlId();
                static const int ViewSwitchToFaceTab                = wxWindow::NewControlId();
                static const int ViewSwitchToViewTab                = wxWindow::NewControlId();
                static const int EditDuplicateObjectsForward        = wxWindow::NewControlId();
                static const int EditDuplicateObjectsRight          = wxWindow::NewControlId();
                static const int EditDuplicateObjectsBackward       = wxWindow::NewControlId();
                static const int EditDuplicateObjectsLeft           = wxWindow::NewControlId();
                static const int EditDuplicateObjectsUp             = wxWindow::NewControlId();
                static const int EditDuplicateObjectsDown           = wxWindow::NewControlId();
                static const int EditNavigateUp                     = wxWindow::NewControlId();
                static const int FileOpenRecent                     = wxWindow::NewControlId();
                static const int EditVertexActions                  = wxWindow::NewControlId();
                static const int EditClipActions                    = wxWindow::NewControlId();
                static const int EditObjectActions                  = wxWindow::NewControlId();
                static const int EditFaceActions                    = wxWindow::NewControlId();
                static const int EditPrintFilePositions             = wxWindow::NewControlId();
                static const int EditToggleAxisRestriction          = wxWindow::NewControlId();
            }
        }
    }
}

#endif
