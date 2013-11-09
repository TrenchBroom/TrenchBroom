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

#ifndef TrenchBroom_CommandIds_h
#define TrenchBroom_CommandIds_h

#include <wx/wx.h>

namespace TrenchBroom {
    namespace View {
        namespace CommandIds {
            namespace Menu {
                static const int Lowest                             = wxID_HIGHEST +  1;
                static const int EditSelectAll                      = Lowest +   2;
                static const int EditSelectSiblings                 = Lowest +   3;
                static const int EditSelectTouching                 = Lowest +   4;
                static const int EditSelectNone                     = Lowest +   5;
                static const int EditHideSelected                   = Lowest +   6;
                static const int EditHideUnselected                 = Lowest +   7;
                static const int EditUnhideAll                      = Lowest +   8;
                static const int EditLockSelected                   = Lowest +   9;
                static const int EditLockUnselected                 = Lowest +  10;
                static const int EditUnlockAll                      = Lowest +  11;
                static const int EditToggleClipTool                 = Lowest +  12;
                static const int EditToggleClipSide                 = Lowest +  13;
                static const int EditPerformClip                    = Lowest +  14;
                static const int EditToggleVertexTool               = Lowest +  15;
                static const int EditMoveTexturesUp                 = Lowest +  18;
                static const int EditMoveTexturesRight              = Lowest +  19;
                static const int EditMoveTexturesDown               = Lowest +  20;
                static const int EditMoveTexturesLeft               = Lowest +  21;
                static const int EditRotateTexturesCW               = Lowest +  22;
                static const int EditRotateTexturesCCW              = Lowest +  23;
                static const int EditMoveObjectsForward             = Lowest +  24;
                static const int EditMoveObjectsRight               = Lowest +  25;
                static const int EditMoveObjectsBackward            = Lowest +  26;
                static const int EditMoveObjectsLeft                = Lowest +  27;
                static const int EditMoveObjectsUp                  = Lowest +  28;
                static const int EditMoveObjectsDown                = Lowest +  29;
                static const int EditRollObjectsCW                  = Lowest +  30;
                static const int EditRollObjectsCCW                 = Lowest +  31;
                static const int EditPitchObjectsCW                 = Lowest +  32;
                static const int EditPitchObjectsCCW                = Lowest +  33;
                static const int EditYawObjectsCW                   = Lowest +  34;
                static const int EditYawObjectsCCW                  = Lowest +  35;
                static const int EditFlipObjectsHorizontally        = Lowest +  36;
                static const int EditFlipObjectsVertically          = Lowest +  37;
                static const int EditDuplicateObjects               = Lowest +  38;
                static const int EditActions                        = Lowest +  39;
                static const int EditCreatePointEntity              = Lowest +  40;
                static const int EditCreateBrushEntity              = Lowest +  41;
                static const int EditToggleTextureLock              = Lowest +  42;
                static const int ViewToggleShowGrid                 = Lowest +  43;
                static const int ViewToggleSnapToGrid               = Lowest +  44;
                static const int ViewSetGridSize1                   = Lowest +  45;
                static const int ViewSetGridSize2                   = Lowest +  46;
                static const int ViewSetGridSize4                   = Lowest +  47;
                static const int ViewSetGridSize8                   = Lowest +  48;
                static const int ViewSetGridSize16                  = Lowest +  49;
                static const int ViewSetGridSize32                  = Lowest +  50;
                static const int ViewSetGridSize64                  = Lowest +  51;
                static const int ViewSetGridSize128                 = Lowest +  52;
                static const int ViewSetGridSize256                 = Lowest +  53;
                static const int ViewMoveCameraForward              = Lowest +  54;
                static const int ViewMoveCameraBackward             = Lowest +  55;
                static const int ViewMoveCameraLeft                 = Lowest +  56;
                static const int ViewMoveCameraRight                = Lowest +  57;
                static const int ViewMoveCameraUp                   = Lowest +  58;
                static const int ViewMoveCameraDown                 = Lowest +  59;
                static const int EditMoveVerticesForward            = Lowest +  60;
                static const int EditMoveVerticesBackward           = Lowest +  61;
                static const int EditMoveVerticesLeft               = Lowest +  62;
                static const int EditMoveVerticesRight              = Lowest +  63;
                static const int EditMoveVerticesUp                 = Lowest +  64;
                static const int EditMoveVerticesDown               = Lowest +  65;
                static const int EditMoveTexturesUpFine             = Lowest +  66;
                static const int EditMoveTexturesRightFine          = Lowest +  67;
                static const int EditMoveTexturesDownFine           = Lowest +  68;
                static const int EditMoveTexturesLeftFine           = Lowest +  69;
                static const int EditRotateTexturesCWFine           = Lowest +  70;
                static const int EditRotateTexturesCCWFine          = Lowest +  71;
                static const int ViewCenterCameraOnSelection        = Lowest +  72;
                static const int EditToggleRotateObjectsTool        = Lowest +  73;
                static const int ViewIncGridSize                    = Lowest +  74;
                static const int ViewDecGridSize                    = Lowest +  75;
                static const int FileLoadPointFile                  = Lowest +  76;
                static const int FileUnloadPointFile                = Lowest +  77;
                static const int ViewMoveCameraToNextPoint          = Lowest +  78;
                static const int ViewMoveCameraToPreviousPoint      = Lowest +  79;
                static const int EditShowMapProperties              = Lowest +  80;
                static const int EditSnapVertices                   = Lowest +  81;
                static const int EditCorrectVertices                = Lowest +  82;
                static const int HelpShowHelp                       = Lowest +  83;
                static const int EditPasteAtOriginalPosition        = Lowest +  84;
                static const int EditSelectByFilePosition           = Lowest +  85;
                static const int ViewSwitchToEntityTab              = Lowest +  86;
                static const int ViewSwitchToFaceTab                = Lowest +  87;
                static const int ViewSwitchToViewTab                = Lowest +  88;
                static const int EditDuplicateObjectsForward        = Lowest +  89;
                static const int EditDuplicateObjectsRight          = Lowest +  90;
                static const int EditDuplicateObjectsBackward       = Lowest +  91;
                static const int EditDuplicateObjectsLeft           = Lowest +  92;
                static const int EditDuplicateObjectsUp             = Lowest +  93;
                static const int EditDuplicateObjectsDown           = Lowest +  94;
                static const int EditNavigateUp                     = Lowest +  95;
                static const int FileOpenRecent                     = Lowest +  96;
                static const int EditVertexActions                  = Lowest +  97;
                static const int EditClipActions                    = Lowest +  98;
                static const int EditObjectActions                  = Lowest +  99;
                static const int EditFaceActions                    = Lowest + 100;
                static const int EditPrintFilePositions             = Lowest + 101;
                static const int EditToggleAxisRestriction          = Lowest + 102;
                static const int Highest                            = Lowest + 199;
            }
            
            namespace CreateEntityPopupMenu {
                static const int LowestPointEntityItem              = wxID_HIGHEST + 2000;
                static const int HighestPointEntityItem             = LowestPointEntityItem + 999;
                static const int LowestBrushEntityItem              = HighestPointEntityItem + 1;
                static const int HighestBrushEntityItem             = LowestBrushEntityItem + 999;
                static const int ReparentBrushes                    = HighestBrushEntityItem + 1;
                static const int MoveBrushesToWorld                 = HighestBrushEntityItem + 2;
            }

            namespace ViewInspector {
                static const int Lowest                             = Menu::Highest +  1;
                static const int ShowEntitiesCheckBoxId             = Lowest +   1;
                static const int ShowEntityModelsCheckBoxId         = Lowest +   2;
                static const int ShowEntityBoundsCheckBoxId         = Lowest +   3;
                static const int ShowEntityClassnamesCheckBoxId     = Lowest +   4;
                static const int ShowBrushesCheckBoxId              = Lowest +   5;
                static const int ShowClipBrushesCheckBoxId          = Lowest +   6;
                static const int ShowSkipBrushesCheckBoxId          = Lowest +   7;
                static const int FaceRenderModeChoiceId             = Lowest +   8;
                static const int RenderEdgesCheckBoxId              = Lowest +   9;
                static const int FaceShadingCheckBoxId              = Lowest +  10;
                static const int FogCheckBoxId                      = Lowest +  11;
                static const int LinkDisplayModeChoiceId            = Lowest +  12;
                static const int ShowHintBrushesCheckBoxId          = Lowest +  13;
                static const int ShowLiquidBrushesCheckBoxId        = Lowest +  14;
                static const int ShowTriggerBrushesCheckBoxId       = Lowest +  15;
                static const int Highest                            = Lowest +  99;
            }
            
            namespace GeneralPreferencePane {
                static const int Lowest                             = ViewInspector::Highest +  1;
                static const int ChooseQuakePathButtonId            = Lowest +   1;
                static const int BrightnessSliderId                 = Lowest +   2;
                static const int GridAlphaSliderId                  = Lowest +   3;
                static const int LookSpeedSliderId                  = Lowest +   4;
                static const int InvertLookXAxisCheckBoxId          = Lowest +   5;
                static const int InvertLookYAxisCheckBoxId          = Lowest +   6;
                static const int PanSpeedSliderId                   = Lowest +   7;
                static const int InvertPanXAxisCheckBoxId           = Lowest +   8;
                static const int InvertPanYAxisCheckBoxId           = Lowest +   9;
                static const int MoveSpeedSliderId                  = Lowest +  10;
                static const int GridModeChoiceId                   = Lowest +  11;
                static const int InstancingModeModeChoiceId         = Lowest +  12;
                static const int EnableAltMoveCheckBoxId            = Lowest +  13;
                static const int InvertAltMoveAxisCheckBoxId        = Lowest +  16;
                static const int MoveCameraInCursorDirCheckBoxId    = Lowest +  14;
                static const int TextureBrowserIconSideChoiceId     = Lowest +  15;
                static const int Highest                            = Lowest +  99;
            }

            namespace KeyboardPreferencePane {
                static const int Lowest                             = GeneralPreferencePane::Highest + 1;
                static const int ShortcutEditorId                   = Lowest +   1;
                static const int Highest                            = Lowest +  99;
            }
            
            namespace FaceInspector {
                static const int Lowest                             = KeyboardPreferencePane::Highest + 1;
                static const int TextureBrowserSortOrderChoiceId    = Lowest +   1;
                static const int TextureBrowserGroupButtonId        = Lowest +   2;
                static const int TextureBrowserUsedButtonId         = Lowest +   3;
                static const int TextureBrowserFilterBoxId          = Lowest +   4;
                static const int TextureBrowserId                   = Lowest +   5;
                static const int XOffsetEditorId                    = Lowest +   6;
                static const int YOffsetEditorId                    = Lowest +   7;
                static const int XScaleEditorId                     = Lowest +   8;
                static const int YScaleEditorId                     = Lowest +   9;
                static const int RotationEditorId                   = Lowest +  10;
                static const int TextureCollectionListId            = Lowest +  11;
                static const int AddTextureCollectionButtonId       = Lowest +  12;
                static const int RemoveTextureCollectionsButtonId   = Lowest +  13;
                static const int ResetFaceAttribsId                 = Lowest +  14;
                static const int FitTextureId                       = Lowest +  15;
                static const int AlignTextureId                     = Lowest +  16;
                static const int FlipTextureHorizontallyId          = Lowest +  17;
                static const int FlipTextureVerticallyId            = Lowest +  18;
                static const int Highest                            = Lowest +  99;
            }
            
            namespace EntityInspector {
                static const int Lowest                             = FaceInspector::Highest + 1;
                static const int EntityBrowserSortOrderChoiceId     = Lowest +   1;
                static const int EntityBrowserGroupButtonId         = Lowest +   2;
                static const int EntityBrowserUsedButtonId          = Lowest +   3;
                static const int EntityBrowserFilterBoxId           = Lowest +   4;
                static const int EntityBrowserId                    = Lowest +   5;
                static const int EntityPropertyViewId               = Lowest +   6;
                static const int AddEntityPropertyButtonId          = Lowest +   7;
                static const int RemoveEntityPropertiesButtonId     = Lowest +   8;
                static const int Highest                            = Lowest +  99;
            }
            
            namespace MapPropertiesDialog {
                static const int Lowest                             = EntityInspector::Highest + 1;
                static const int ModChoiceId                        = Lowest +   1;
                static const int DefChoiceId                        = Lowest +   2;
                static const int WadListId                          = Lowest +   3;
                static const int AddWadButtonId                     = Lowest +   4;
                static const int RemoveWadsButtonId                 = Lowest +   5;
                static const int MoveWadUpButtonId                  = Lowest +   6;
                static const int MoveWadDownButtonId                = Lowest +   7;
                static const int PathChoiceId                       = Lowest +   8;
                static const int ForceIntCoordsId                   = Lowest +   9;
                static const int Highest                            = Lowest +  99;
            }
        }
    }
}

#endif
