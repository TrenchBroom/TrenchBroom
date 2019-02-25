/*
 Copyright (C) 2010-2017 Kristian Duske

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
                const int Lowest                                     = wxID_HIGHEST +  1;
                const int FileExportObj                              = Lowest + 1;
                const int FileLoadPointFile                          = FileExportObj + 1;
                const int FileUnloadPointFile                        = FileLoadPointFile + 1;
                const int FileReloadPointFile                        = FileUnloadPointFile + 1;
                const int FileLoadPortalFile                         = FileReloadPointFile + 1;
                const int FileUnloadPortalFile                       = FileLoadPortalFile + 1;
                const int FileReloadPortalFile                       = FileUnloadPortalFile + 1;
                const int FileReloadTextureCollections               = FileReloadPortalFile + 1;
                const int FileReloadEntityDefinitions                = FileReloadTextureCollections + 1;

                const int FileOpenRecent                             = FileReloadEntityDefinitions + 1;
                const int FileRecentDocuments                        = FileOpenRecent + 1;
                const int FileRecentDocumentsLast                    = FileRecentDocuments + 100;

                const int EditSelectAll                              = FileRecentDocumentsLast + 1;
                const int EditSelectNone                             = EditSelectAll + 1;
                const int EditSelectSiblings                         = EditSelectNone + 1;
                const int EditSelectTouching                         = EditSelectSiblings + 1;
                const int EditSelectInside                           = EditSelectTouching + 1;
                const int EditSelectTall                             = EditSelectInside + 1;
                const int EditSelectByFilePosition                   = EditSelectTall + 1;
                const int EditPasteAtOriginalPosition                = EditSelectByFilePosition + 1;
                const int EditSnapVerticesToInteger                  = EditPasteAtOriginalPosition + 1;
                const int EditSnapVerticesToGrid                     = EditSnapVerticesToInteger + 1;
                const int EditToggleTextureLock                      = EditSnapVerticesToGrid + 1;
                const int EditToggleUVLock                           = EditToggleTextureLock + 1;
                const int EditPrintFilePositions                     = EditToggleUVLock + 1;
                const int EditRepeat                                 = EditPrintFilePositions + 1;
                const int EditClearRepeat                            = EditRepeat + 1;
                const int EditReplaceTexture                         = EditClearRepeat + 1;
                const int EditToggleCreateComplexBrushTool           = EditReplaceTexture + 1;
                const int EditToggleRotateObjectsTool                = EditToggleCreateComplexBrushTool + 1;
                const int EditToggleScaleObjectsTool                 = EditToggleRotateObjectsTool + 1;
                const int EditToggleShearObjectsTool                 = EditToggleScaleObjectsTool + 1;
                const int EditToggleClipTool                         = EditToggleShearObjectsTool + 1;
                const int EditDeactivateTool                         = EditToggleClipTool + 1;
                const int EditToggleVertexTool                       = EditDeactivateTool + 1;
                const int EditToggleEdgeTool                         = EditToggleVertexTool + 1;
                const int EditToggleFaceTool                         = EditToggleEdgeTool + 1;
                const int EditCsgConvexMerge                         = EditToggleFaceTool + 1;
                const int EditCsgSubtract                            = EditCsgConvexMerge + 1;
                const int EditCsgIntersect                           = EditCsgSubtract + 1;
                const int EditCsgHollow                              = EditCsgIntersect + 1;
                const int EditGroupSelection                         = EditCsgHollow + 1;
                const int EditUngroupSelection                       = EditGroupSelection + 1;

                const int ViewToggleShowGrid                         = EditUngroupSelection + 1;
                const int ViewToggleSnapToGrid                       = ViewToggleShowGrid + 1;
                const int ViewSetGridSize0Point125                   = ViewToggleSnapToGrid + 1;
                const int ViewSetGridSize0Point25                    = ViewSetGridSize0Point125 + 1;
                const int ViewSetGridSize0Point5                     = ViewSetGridSize0Point25 + 1;
                const int ViewSetGridSize1                           = ViewSetGridSize0Point5 + 1;
                const int ViewSetGridSize2                           = ViewSetGridSize1 + 1;
                const int ViewSetGridSize4                           = ViewSetGridSize2 + 1;
                const int ViewSetGridSize8                           = ViewSetGridSize4 + 1;
                const int ViewSetGridSize16                          = ViewSetGridSize8 + 1;
                const int ViewSetGridSize32                          = ViewSetGridSize16 + 1;
                const int ViewSetGridSize64                          = ViewSetGridSize32 + 1;
                const int ViewSetGridSize128                         = ViewSetGridSize64 + 1;
                const int ViewSetGridSize256                         = ViewSetGridSize128 + 1;
                const int ViewFocusCameraOnSelection                 = ViewSetGridSize256 + 1;
                const int ViewIncGridSize                            = ViewFocusCameraOnSelection + 1;
                const int ViewDecGridSize                            = ViewIncGridSize + 1;
                const int ViewMoveCameraToNextPoint                  = ViewDecGridSize + 1;
                const int ViewMoveCameraToPreviousPoint              = ViewMoveCameraToNextPoint + 1;
                const int ViewSwitchToMapInspector                   = ViewMoveCameraToPreviousPoint + 1;
                const int ViewSwitchToEntityInspector                = ViewSwitchToMapInspector + 1;
                const int ViewSwitchToFaceInspector                  = ViewSwitchToEntityInspector + 1;
                const int ViewToggleMaximizeCurrentView              = ViewSwitchToFaceInspector + 1;
                const int ViewToggleInfoPanel                        = ViewToggleMaximizeCurrentView + 1;
                const int ViewToggleInspector                        = ViewToggleInfoPanel + 1;
                const int ViewMoveCameraToPosition                   = ViewToggleInspector + 1;
                const int ViewHideSelection                          = ViewMoveCameraToPosition + 1;
                const int ViewIsolateSelection                       = ViewHideSelection + 1;
                const int ViewUnhideAll                              = ViewIsolateSelection + 1;

                const int RunCompile                                 = ViewUnhideAll + 1;
                const int RunLaunch                                  = RunCompile + 1;

                const int DebugPrintVertices                         = RunLaunch + 1;
                const int DebugCreateBrush                           = DebugPrintVertices + 1;
                const int DebugCopyJSShortcuts                       = DebugCreateBrush + 1;
                const int DebugCrash                                 = DebugCopyJSShortcuts + 1;
                const int DebugCreateCube                            = DebugCrash + 1;
                const int DebugClipWithFace                          = DebugCreateCube + 1;
                const int DebugCrashReportDialog                     = DebugClipWithFace + 1;
                const int DebugSetWindowSize                         = DebugCrashReportDialog + 1;
                const int DebugThrowExceptionDuringCommand           = DebugSetWindowSize + 1;

                const int Highest                                    = DebugThrowExceptionDuringCommand + 200;
            }

            namespace Actions {
                const int Nothing                                    = wxID_NONE;
                const int Lowest                                     = Menu::Highest + 1;
                const int PerformCreateBrush                         = Lowest + 1;
                const int ToggleClipTool                             = PerformCreateBrush + 1;
                const int ToggleClipSide                             = ToggleClipTool + 1;
                const int PerformClip                                = ToggleClipSide + 1;

                const int MoveVerticesForward                        = PerformClip + 1;
                const int MoveVerticesBackward                       = MoveVerticesForward + 1;
                const int MoveVerticesLeft                           = MoveVerticesBackward + 1;
                const int MoveVerticesRight                          = MoveVerticesLeft + 1;
                const int MoveVerticesUp                             = MoveVerticesRight + 1;
                const int MoveVerticesDown                           = MoveVerticesUp + 1;

                const int MoveObjectsForward                         = MoveVerticesDown + 1;
                const int MoveObjectsRight                           = MoveObjectsForward + 1;
                const int MoveObjectsBackward                        = MoveObjectsRight + 1;
                const int MoveObjectsLeft                            = MoveObjectsBackward + 1;
                const int MoveObjectsUp                              = MoveObjectsLeft + 1;
                const int MoveObjectsDown                            = MoveObjectsUp + 1;

                const int RollObjectsCW                              = MoveObjectsDown + 1;
                const int RollObjectsCCW                             = RollObjectsCW + 1;
                const int PitchObjectsCW                             = RollObjectsCCW + 1;
                const int PitchObjectsCCW                            = PitchObjectsCW + 1;
                const int YawObjectsCW                               = PitchObjectsCCW + 1;
                const int YawObjectsCCW                              = YawObjectsCW + 1;

                const int FlipObjectsHorizontally                    = YawObjectsCCW + 1;
                const int FlipObjectsVertically                      = FlipObjectsHorizontally + 1;

                const int DuplicateObjectsForward                    = FlipObjectsVertically + 1;
                const int DuplicateObjectsRight                      = DuplicateObjectsForward + 1;
                const int DuplicateObjectsBackward                   = DuplicateObjectsRight + 1;
                const int DuplicateObjectsLeft                       = DuplicateObjectsBackward + 1;
                const int DuplicateObjectsUp                         = DuplicateObjectsLeft + 1;
                const int DuplicateObjectsDown                       = DuplicateObjectsUp + 1;

                const int MoveTexturesUp                             = DuplicateObjectsDown + 1;
                const int MoveTexturesRight                          = MoveTexturesUp + 1;
                const int MoveTexturesDown                           = MoveTexturesRight + 1;
                const int MoveTexturesLeft                           = MoveTexturesDown + 1;
                const int RotateTexturesCW                           = MoveTexturesLeft + 1;
                const int RotateTexturesCCW                          = RotateTexturesCW + 1;

                const int Cancel                                     = RotateTexturesCCW + 1;

                const int MoveRotationCenterForward                  = Cancel + 1;
                const int MoveRotationCenterBackward                 = MoveRotationCenterForward + 1;
                const int MoveRotationCenterLeft                     = MoveRotationCenterBackward + 1;
                const int MoveRotationCenterRight                    = MoveRotationCenterLeft + 1;
                const int MoveRotationCenterUp                       = MoveRotationCenterRight + 1;
                const int MoveRotationCenterDown                     = MoveRotationCenterUp + 1;

                const int MakeStructural                             = MoveRotationCenterDown + 1;

                const int CycleMapViews                              = MakeStructural + 1;

                const int ResetZoom                                  = CycleMapViews + 1;

                const int DeactivateTool                             = ResetZoom + 1;

                const int ToggleShowEntityClassnames                 = DeactivateTool + 1;
                const int ToggleShowGroupBounds                      = ToggleShowEntityClassnames + 1;
                const int ToggleShowBrushEntityBounds                = ToggleShowGroupBounds + 1;
                const int ToggleShowPointEntityBounds                = ToggleShowBrushEntityBounds + 1;
                const int ToggleShowPointEntities                    = ToggleShowPointEntityBounds + 1;
                const int ToggleShowPointEntityModels                = ToggleShowPointEntities + 1;
                const int ToggleShowBrushes                          = ToggleShowPointEntityModels + 1;
                const int RenderModeShowTextures                     = ToggleShowBrushes + 1;
                const int RenderModeHideTextures                     = RenderModeShowTextures + 1;
                const int RenderModeHideFaces                        = RenderModeHideTextures + 1;
                const int RenderModeShadeFaces                       = RenderModeHideFaces + 1;
                const int RenderModeUseFog                           = RenderModeShadeFaces + 1;
                const int RenderModeShowEdges                        = RenderModeUseFog + 1;
                const int RenderModeShowAllEntityLinks               = RenderModeShowEdges + 1;
                const int RenderModeShowTransitiveEntityLinks        = RenderModeShowAllEntityLinks + 1;
                const int RenderModeShowDirectEntityLinks            = RenderModeShowTransitiveEntityLinks + 1;
                const int RenderModeHideEntityLinks                  = RenderModeShowDirectEntityLinks + 1;

                const int MaxTagCommandIds                           = 1000;

                const int LowestToggleTagCommandId                   = RenderModeHideEntityLinks + 1;
                const int HighestToggleTagCommandId                  = LowestToggleTagCommandId + MaxTagCommandIds;

                const int LowestEnableTagCommandId                   = HighestToggleTagCommandId + 1;
                const int HighestEnableTagCommandId                  = LowestEnableTagCommandId + MaxTagCommandIds;

                const int LowestDisableTagCommandId                  = HighestEnableTagCommandId + 1;
                const int HighestDisableTagCommandId                 = LowestDisableTagCommandId + MaxTagCommandIds;

                const int MaxEntityDefinitionCommandIds              = 5000;

                const int LowestToggleEntityDefinitionCommandId      = HighestDisableTagCommandId + 1;
                const int HighestToggleEntityDefinitionCommandId     = LowestToggleEntityDefinitionCommandId + MaxEntityDefinitionCommandIds;

                const int LowestCreateEntityCommandId                = HighestToggleEntityDefinitionCommandId + 1;
                const int HighestCreateEntityCommandId               = LowestCreateEntityCommandId + MaxEntityDefinitionCommandIds;

                const int Highest                                    = HighestCreateEntityCommandId;
            }

            namespace MapViewPopupMenu {
                static const int Lowest                              = Actions::Highest + 1;

                static const int LowestPointEntityItem               = Lowest;
                static const int HighestPointEntityItem              = LowestPointEntityItem + 1000;

                static const int LowestBrushEntityItem               = HighestPointEntityItem + 1;
                static const int HighestBrushEntityItem              = LowestBrushEntityItem + 1000;

                static const int AddObjectsToGroup                   = HighestBrushEntityItem + 1;
                static const int RemoveObjectsFromGroup              = AddObjectsToGroup + 1;
                static const int MoveBrushesToEntity                 = RemoveObjectsFromGroup + 1;
                static const int MoveBrushesToWorld                  = MoveBrushesToEntity + 1;
                static const int GroupObjects                        = MoveBrushesToWorld + 1;
                static const int UngroupObjects                      = GroupObjects + 1;
                static const int RenameGroups                        = UngroupObjects + 1;
                static const int MergeGroups                         = RenameGroups + 1;
                static const int ShowPopupMenu                       = MergeGroups + 1;

                static const int Highest                             = ShowPopupMenu;
            }

            namespace ToggleTagPopupMenu {
                static const int Lowest                              = MapViewPopupMenu::Highest + 1;
            }
        }
    }
}

#endif
