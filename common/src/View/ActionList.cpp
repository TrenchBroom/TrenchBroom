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

#include "ActionList.h"

#include "Preference.h"
#include "PreferenceManager.h"
#include "View/ActionContext.h"
#include "View/KeyboardShortcut.h"

namespace TrenchBroom {
    namespace View {
        QKeySequence ActionInfo::key() const {
            return pref(preference()).keySequence();
        }

        void ActionInfo::setKey(const QKeySequence& keySequence) {
            auto& prefs = PreferenceManager::instance();
            auto pref = preference();
            prefs.set(pref, KeyboardShortcut(keySequence));
        }

        Preference<KeyboardShortcut> ActionInfo::preference() const {
            return Preference<KeyboardShortcut>(preferencePath, View::KeyboardShortcut(defaultKey));
        }

        ActionInfo ActionList::addShortcut(const IO::Path& path, QKeySequence keySequence, int actionContext, bool modifiable) {
            ActionInfo result;
            result.preferencePath = path;
            result.defaultKey = std::move(keySequence);
            result.actionContext = actionContext;
            result.modifiable = modifiable;
            m_list.push_back(result);
            return result;
        }

        ActionInfo ActionList::addAction(const IO::Path& path, QKeySequence keySequence, bool modifiable) {
            ActionInfo result;
            result.preferencePath = path;
            result.defaultKey = std::move(keySequence);
            result.actionContext = 0;
            result.modifiable = modifiable;
            m_list.push_back(result);
            return result;
        }

        ActionList::ActionList() {
            controlsMapViewCreatebrushInfo = addShortcut(IO::Path("Controls/Map view/Create brush"), QKeySequence(Qt::Key_Return), ActionContext_CreateComplexBrushTool, true);
            controlsMapViewToggleClipSideInfo = addShortcut(IO::Path("Controls/Map view/Toggle clip side"), QKeySequence(Qt::Key_Return | Qt::CTRL), ActionContext_ClipTool, true);
            controlsMapViewPerformclipInfo = addShortcut(IO::Path("Controls/Map view/Perform clip"), QKeySequence(Qt::Key_Return), ActionContext_ClipTool, true);
            controlsMapViewMoveVerticesUpForwardInfo = addShortcut(IO::Path("Controls/Map view/Move vertices up; Move vertices forward"), QKeySequence(Qt::Key_Up), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveVerticesDownBackwardInfo = addShortcut(IO::Path("Controls/Map view/Move vertices down; Move vertices backward"), QKeySequence(Qt::Key_Down), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveVerticesLeftInfo = addShortcut(IO::Path("Controls/Map view/Move vertices left"), QKeySequence(Qt::Key_Left), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveVerticesRightInfo = addShortcut(IO::Path("Controls/Map view/Move vertices right"), QKeySequence(Qt::Key_Right), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveVerticesBackwardUpInfo = addShortcut(IO::Path("Controls/Map view/Move vertices backward; Move vertices up"), QKeySequence(Qt::Key_PageUp), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveVerticesForwardDownInfo = addShortcut(IO::Path("Controls/Map view/Move vertices forward; Move vertices down"), QKeySequence(Qt::Key_PageDown), ActionContext_AnyVertexTool, true);
            controlsMapViewMoveRotationCenterUpForwardInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center up; Move rotation center forward"), QKeySequence(Qt::Key_Up), ActionContext_RotateTool, true);
            controlsMapViewMoveRotationCenterDownBackwardInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center down; Move rotation center backward"), QKeySequence(Qt::Key_Down), ActionContext_RotateTool, true);
            controlsMapViewMoveRotationCenterLeftInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center left"), QKeySequence(Qt::Key_Left), ActionContext_RotateTool, true);
            controlsMapViewMoveRotationCenterRightInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center right"), QKeySequence(Qt::Key_Right), ActionContext_RotateTool, true);
            controlsMapViewMoveRotationCenterBackwardUpInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center backward; Move rotation center up"), QKeySequence(Qt::Key_PageUp), ActionContext_RotateTool, true);
            controlsMapViewMoveRotationCenterForwardDownInfo = addShortcut(IO::Path("Controls/Map view/Move rotation center forward; Move rotation center down"), QKeySequence(Qt::Key_PageDown), ActionContext_RotateTool, true);
            controlsMapViewMoveObjectsUpForwardInfo = addShortcut(IO::Path("Controls/Map view/Move objects up; Move objects forward"), QKeySequence(Qt::Key_Up), ActionContext_NodeSelection, true);
            controlsMapViewMoveObjectsDownBackwardInfo = addShortcut(IO::Path("Controls/Map view/Move objects down; Move objects backward"), QKeySequence(Qt::Key_Down), ActionContext_NodeSelection, true);
            controlsMapViewMoveObjectsLeftInfo = addShortcut(IO::Path("Controls/Map view/Move objects left"), QKeySequence(Qt::Key_Left), ActionContext_NodeSelection, true);
            controlsMapViewMoveObjectsRightInfo = addShortcut(IO::Path("Controls/Map view/Move objects right"), QKeySequence(Qt::Key_Right), ActionContext_NodeSelection, true);
            controlsMapViewMoveObjectsBackwardUupInfo = addShortcut(IO::Path("Controls/Map view/Move objects backward; Move objects up"), QKeySequence(Qt::Key_PageUp), ActionContext_NodeSelection, true);
            controlsMapViewMoveObjectsForwardDownInfo = addShortcut(IO::Path("Controls/Map view/Move objects forward; Move objects down"), QKeySequence(Qt::Key_PageDown), ActionContext_NodeSelection, true);
            controlsMapViewRollObjectsClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Roll objects clockwise"), QKeySequence(Qt::Key_Up | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewRollObjectsCounterClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Roll objects counter-clockwise"), QKeySequence(Qt::Key_Down | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewYawObjectsClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Yaw objects clockwise"), QKeySequence(Qt::Key_Left | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewYawObjectsCounterClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Yaw objects counter-clockwise"), QKeySequence(Qt::Key_Right | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewPitchobjectsClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Pitch objects clockwise"), QKeySequence(Qt::Key_PageUp | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewPitchobjectsCounterClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Pitch objects counter-clockwise"), QKeySequence(Qt::Key_PageDown | Qt::ALT), ActionContext_NodeSelection | ActionContext_RotateTool, true);
            controlsMapViewFlipobjectsHorizontallyInfo = addShortcut(IO::Path("Controls/Map view/Flip objects horizontally"), QKeySequence('F' | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewFlipobjectsBerticallyInfo = addShortcut(IO::Path("Controls/Map view/Flip objects vertically"), QKeySequence('F' | Qt::CTRL | Qt::ALT), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsUpForwardInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward"), QKeySequence(Qt::Key_Up | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsDownBackwardInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects down; Duplicate and move objects backward"), QKeySequence(Qt::Key_Down | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsLeftInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects left"), QKeySequence(Qt::Key_Left | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsRightInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects right"), QKeySequence(Qt::Key_Right | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsBackwardUpInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects backward; Duplicate and move objects up"), QKeySequence(Qt::Key_PageUp | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewDuplicateAndMoveObjectsForwardDownInfo = addShortcut(IO::Path("Controls/Map view/Duplicate and move objects forward; Duplicate and move objects down"), QKeySequence(Qt::Key_PageDown | Qt::CTRL), ActionContext_NodeSelection, true);
            controlsMapViewMoveTexturesupInfo = addShortcut(IO::Path("Controls/Map view/Move textures up"), QKeySequence(Qt::Key_Up), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesupFineInfo = addShortcut(IO::Path("Controls/Map view/Move textures up (fine)"), QKeySequence(Qt::Key_Up | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesupCoarseInfo = addShortcut(IO::Path("Controls/Map view/Move textures up (coarse)"), QKeySequence(Qt::Key_Up | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesdownInfo = addShortcut(IO::Path("Controls/Map view/Move textures down"), QKeySequence(Qt::Key_Down), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesdownFineInfo = addShortcut(IO::Path("Controls/Map view/Move textures down (fine)"), QKeySequence(Qt::Key_Down | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesdownCoarseInfo = addShortcut(IO::Path("Controls/Map view/Move textures down (coarse)"), QKeySequence(Qt::Key_Down | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesleftInfo = addShortcut(IO::Path("Controls/Map view/Move textures left"), QKeySequence(Qt::Key_Left), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesleftFineInfo = addShortcut(IO::Path("Controls/Map view/Move textures left (fine)"), QKeySequence(Qt::Key_Left | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesleftCoarseInfo = addShortcut(IO::Path("Controls/Map view/Move textures left (coarse)"), QKeySequence(Qt::Key_Left | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesrightInfo = addShortcut(IO::Path("Controls/Map view/Move textures right"), QKeySequence(Qt::Key_Right), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesrightFineInfo = addShortcut(IO::Path("Controls/Map view/Move textures right (fine)"), QKeySequence(Qt::Key_Right | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewMoveTexturesrightCoarseInfo = addShortcut(IO::Path("Controls/Map view/Move textures right (coarse)"), QKeySequence(Qt::Key_Right | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturesclockwiseInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures clockwise"), QKeySequence(Qt::Key_PageUp), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturesclockwiseFineInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures clockwise (fine)"), QKeySequence(Qt::Key_PageUp | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturesclockwiseCoarseInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures clockwise (coarse)"), QKeySequence(Qt::Key_PageUp | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturescounterClockwiseInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures counter-clockwise"), QKeySequence(Qt::Key_PageDown), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturescounterClockwiseFineInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures counter-clockwise (fine)"), QKeySequence(Qt::Key_PageDown | Qt::CTRL), ActionContext_FaceSelection, true);
            controlsMapViewRotateTexturescounterClockwiseCoarseInfo = addShortcut(IO::Path("Controls/Map view/Rotate textures counter-clockwise (coarse)"), QKeySequence(Qt::Key_PageDown | Qt::SHIFT), ActionContext_FaceSelection, true);
            controlsMapViewCycleMapViewInfo = addShortcut(IO::Path("Controls/Map view/Cycle map view"), QKeySequence(Qt::Key_Space), ActionContext_Any, true);
            controlsMapViewResetcamerazoomInfo = addShortcut(IO::Path("Controls/Map view/Reset camera zoom"), QKeySequence(Qt::Key_Escape | Qt::SHIFT), ActionContext_Any, true);
            controlsMapViewCancelInfo = addShortcut(IO::Path("Controls/Map view/Cancel"), QKeySequence(Qt::Key_Escape), ActionContext_Any, true);
            controlsMapViewDeactivatecurrenttoolInfo = addShortcut(IO::Path("Controls/Map view/Deactivate current tool"), QKeySequence(Qt::Key_Escape | Qt::CTRL), ActionContext_Any, true);

            menuFileNewInfo = addAction(IO::Path("Menu/File/New"), QKeySequence('N' | Qt::CTRL), false);
            menuFileOpenInfo = addAction(IO::Path("Menu/File/Open..."), QKeySequence('O' | Qt::CTRL), false);
            menuFileSaveInfo = addAction(IO::Path("Menu/File/Save"), QKeySequence('S' | Qt::CTRL), false);
            menuFileSaveasInfo = addAction(IO::Path("Menu/File/Save as..."), QKeySequence('S' | Qt::SHIFT | Qt::CTRL), false);
            menuFileExportWavefrontOBJInfo = addAction(IO::Path("Menu/File/Export/Wavefront OBJ..."), QKeySequence(), true);
            menuFileLoadPointFileInfo = addAction(IO::Path("Menu/File/Load Point File..."), QKeySequence(), true);
            menuFileReloadPointFileInfo = addAction(IO::Path("Menu/File/Reload Point File"), QKeySequence(), true);
            menuFileUnloadPointFileInfo = addAction(IO::Path("Menu/File/Unload Point File"), QKeySequence(), true);
            menuFileLoadPortalFileInfo = addAction(IO::Path("Menu/File/Load Portal File..."), QKeySequence(), true);
            menuFileReloadPortalFileInfo = addAction(IO::Path("Menu/File/Reload Portal File"), QKeySequence(), true);
            menuFileUnloadPortalFileInfo = addAction(IO::Path("Menu/File/Unload Portal File"), QKeySequence(), true);
            menuFileReloadTextureCollectionsInfo = addAction(IO::Path("Menu/File/Reload Texture Collections"), QKeySequence(Qt::Key_F5), true);
            menuFileReloadEntityDefinitionsInfo = addAction(IO::Path("Menu/File/Reload Entity Definitions"), QKeySequence(Qt::Key_F6), true);
            menuFileCloseInfo = addAction(IO::Path("Menu/File/Close"), QKeySequence('W' | Qt::CTRL), false);
            menuEditUndoInfo = addAction(IO::Path("Menu/Edit/Undo"), QKeySequence('Z' | Qt::CTRL), false);
            menuEditRedoInfo = addAction(IO::Path("Menu/Edit/Redo"), QKeySequence('Z' | Qt::CTRL | Qt::SHIFT), false);
            menuEditRepeatInfo = addAction(IO::Path("Menu/Edit/Repeat"), QKeySequence('R' | Qt::CTRL), true);
            menuEditClearRepeatableCommandsInfo = addAction(IO::Path("Menu/Edit/Clear Repeatable Commands"), QKeySequence('R' | Qt::CTRL | Qt::SHIFT), true);
            menuEditCutInfo = addAction(IO::Path("Menu/Edit/Cut"), QKeySequence('X' | Qt::CTRL), false);
            menuEditCopyInfo = addAction(IO::Path("Menu/Edit/Copy"), QKeySequence('C' | Qt::CTRL), false);
            menuEditPasteInfo = addAction(IO::Path("Menu/Edit/Paste"), QKeySequence('V' | Qt::CTRL), false);
            menuEditPasteatOriginalPositionInfo = addAction(IO::Path("Menu/Edit/Paste at Original Position"), QKeySequence('V' | Qt::CTRL | Qt::ALT), true);
            menuEditDuplicateInfo = addAction(IO::Path("Menu/Edit/Duplicate"), QKeySequence('D' | Qt::CTRL), true);
            menuEditDeleteInfo = addAction(IO::Path("Menu/Edit/Delete"), QKeySequence(Qt::Key_Delete), true);
            menuEditSelectAllInfo = addAction(IO::Path("Menu/Edit/Select All"), QKeySequence('A' | Qt::CTRL), true);
            menuEditSelectSiblingsInfo = addAction(IO::Path("Menu/Edit/Select Siblings"), QKeySequence('B' | Qt::CTRL), true);
            menuEditSelectTouchingInfo = addAction(IO::Path("Menu/Edit/Select Touching"), QKeySequence('T' | Qt::CTRL), true);
            menuEditSelectInsideInfo = addAction(IO::Path("Menu/Edit/Select Inside"), QKeySequence('E' | Qt::CTRL), true);
            menuEditSelectTallInfo = addAction(IO::Path("Menu/Edit/Select Tall"), QKeySequence('E' | Qt::CTRL | Qt::SHIFT), true);
            menuEditSelectbyLineNumberInfo = addAction(IO::Path("Menu/Edit/Select by Line Number"), QKeySequence(), true);
            menuEditSelectNoneInfo = addAction(IO::Path("Menu/Edit/Select None"), QKeySequence('A' | Qt::CTRL | Qt::SHIFT), true);
            menuEditGroupInfo = addAction(IO::Path("Menu/Edit/Group"), QKeySequence('G' | Qt::CTRL), true);
            menuEditUngroupInfo = addAction(IO::Path("Menu/Edit/Ungroup"), QKeySequence('G' | Qt::CTRL | Qt::SHIFT), true);
            menuEditToolsBrushToolInfo = addAction(IO::Path("Menu/Edit/Tools/Brush Tool"), QKeySequence('B'), true);
            menuEditToolsClipToolInfo = addAction(IO::Path("Menu/Edit/Tools/Clip Tool"), QKeySequence('C'), true);
            menuEditToolsRotateToolInfo = addAction(IO::Path("Menu/Edit/Tools/Rotate Tool"), QKeySequence('R'), true);
            menuEditToolsScaleToolInfo = addAction(IO::Path("Menu/Edit/Tools/Scale Tool"), QKeySequence('T'), true);
            menuEditToolsShearToolInfo = addAction(IO::Path("Menu/Edit/Tools/Shear Tool"), QKeySequence('G'), true);
            menuEditToolsVertexToolInfo = addAction(IO::Path("Menu/Edit/Tools/Vertex Tool"), QKeySequence('V'), true);
            menuEditToolsEdgeToolInfo = addAction(IO::Path("Menu/Edit/Tools/Edge Tool"), QKeySequence('E'), true);
            menuEditToolsFaceToolInfo = addAction(IO::Path("Menu/Edit/Tools/Face Tool"), QKeySequence('F'), true);
            menuEditCSGConvexMergeInfo = addAction(IO::Path("Menu/Edit/CSG/Convex Merge"), QKeySequence('J' | Qt::CTRL), true);
            menuEditCSGSubtractInfo = addAction(IO::Path("Menu/Edit/CSG/Subtract"), QKeySequence('K' | Qt::CTRL), true);
            menuEditCSGHollowInfo = addAction(IO::Path("Menu/Edit/CSG/Hollow"), QKeySequence('K' | Qt::CTRL | Qt::ALT), true);
            menuEditCSGIntersectInfo = addAction(IO::Path("Menu/Edit/CSG/Intersect"), QKeySequence('L' | Qt::CTRL), true);
            menuEditSnapVerticestoIntegerInfo = addAction(IO::Path("Menu/Edit/Snap Vertices to Integer"), QKeySequence('V' | Qt::SHIFT | Qt::CTRL), true);
            menuEditSnapVerticestoGridInfo = addAction(IO::Path("Menu/Edit/Snap Vertices to Grid"), QKeySequence('V' | Qt::SHIFT | Qt::CTRL | Qt::ALT), true);
            menuEditTextureLockInfo = addAction(IO::Path("Menu/Edit/Texture Lock"), QKeySequence(), true);
            menuEditUVLockInfo = addAction(IO::Path("Menu/Edit/UV Lock"), QKeySequence('U'), true);
            menuEditReplaceTextureInfo = addAction(IO::Path("Menu/Edit/Replace Texture..."), QKeySequence(), true);
            menuViewGridShowGridInfo = addAction(IO::Path("Menu/View/Grid/Show Grid"), QKeySequence('0'), true);
            menuViewGridSnaptoGridInfo = addAction(IO::Path("Menu/View/Grid/Snap to Grid"), QKeySequence('0' | Qt::ALT), true);
            menuViewGridIncreaseGridSizeInfo = addAction(IO::Path("Menu/View/Grid/Increase Grid Size"), QKeySequence('+'), true);
            menuViewGridDecreaseGridSizeInfo = addAction(IO::Path("Menu/View/Grid/Decrease Grid Size"), QKeySequence('-'), true);
            menuViewGridSetGridSize0125Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 0.125"), QKeySequence(), true);
            menuViewGridSetGridSize025Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 0.25"), QKeySequence(), true);
            menuViewGridSetGridSize05Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 0.5"), QKeySequence(), true);
            menuViewGridSetGridSize1Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 1"), QKeySequence('1'), true);
            menuViewGridSetGridSize2Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 2"), QKeySequence('2'), true);
            menuViewGridSetGridSize4Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 4"), QKeySequence('3'), true);
            menuViewGridSetGridSize8Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 8"), QKeySequence('4'), true);
            menuViewGridSetGridSize16Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 16"), QKeySequence('5'), true);
            menuViewGridSetGridSize32Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 32"), QKeySequence('6'), true);
            menuViewGridSetGridSize64Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 64"), QKeySequence('7'), true);
            menuViewGridSetGridSize128Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 128"), QKeySequence('8'), true);
            menuViewGridSetGridSize256Info = addAction(IO::Path("Menu/View/Grid/Set Grid Size 256"), QKeySequence('9'), true);
            menuViewCameraMovetoNextPointInfo = addAction(IO::Path("Menu/View/Camera/Move to Next Point"), QKeySequence('.'), true);
            menuViewCameraMovetoPreviousPointInfo = addAction(IO::Path("Menu/View/Camera/Move to Previous Point"), QKeySequence(','), true);
            menuViewCameraFocusonSelectionInfo = addAction(IO::Path("Menu/View/Camera/Focus on Selection"), QKeySequence('U' | Qt::CTRL), true);
            menuViewCameraMoveCameratoInfo = addAction(IO::Path("Menu/View/Camera/Move Camera to..."), QKeySequence(), true);
            menuViewIsolateInfo = addAction(IO::Path("Menu/View/Isolate"), QKeySequence('I' | Qt::CTRL), true);
            menuViewHideInfo = addAction(IO::Path("Menu/View/Hide"), QKeySequence('I' | Qt::CTRL | Qt::ALT), true);
            menuViewShowAllInfo = addAction(IO::Path("Menu/View/Show All"), QKeySequence('I' | Qt::CTRL | Qt::SHIFT), true);
            menuViewSwitchtoMapInspectorInfo = addAction(IO::Path("Menu/View/Switch to Map Inspector"), QKeySequence('1' | Qt::CTRL), true);
            menuViewSwitchtoEntityInspectorInfo = addAction(IO::Path("Menu/View/Switch to Entity Inspector"), QKeySequence('2' | Qt::CTRL), true);
            menuViewSwitchtoFaceInspectorInfo = addAction(IO::Path("Menu/View/Switch to Face Inspector"), QKeySequence('3' | Qt::CTRL), true);
            menuViewToggleInfoPanelInfo = addAction(IO::Path("Menu/View/Toggle Info Panel"), QKeySequence('4' | Qt::CTRL), true);
            menuViewToggleInspectorInfo = addAction(IO::Path("Menu/View/Toggle Inspector"), QKeySequence('5' | Qt::CTRL), true);
            menuViewMaximizeCurrentViewInfo = addAction(IO::Path("Menu/View/Maximize Current View"), QKeySequence(Qt::Key_Space | Qt::CTRL), true);
            menuViewPreferencesInfo = addAction(IO::Path("Menu/View/Preferences..."), QKeySequence(), false);
            menuRunCompileInfo = addAction(IO::Path("Menu/Run/Compile..."), QKeySequence(), true);
            menuRunLaunchInfo = addAction(IO::Path("Menu/Run/Launch..."), QKeySequence(), true);
            menuDebugPrintVerticesInfo = addAction(IO::Path("Menu/Debug/Print Vertices"), QKeySequence(), false);
            menuDebugCreateBrushInfo = addAction(IO::Path("Menu/Debug/Create Brush..."), QKeySequence(), false);
            menuDebugCreateCubeInfo = addAction(IO::Path("Menu/Debug/Create Cube..."), QKeySequence(), false);
            menuDebugClipBrushInfo = addAction(IO::Path("Menu/Debug/Clip Brush..."), QKeySequence(), false);
            menuDebugCopyJavascriptShortcutMapInfo = addAction(IO::Path("Menu/Debug/Copy Javascript Shortcut Map"), QKeySequence(), false);
            menuDebugCrashInfo = addAction(IO::Path("Menu/Debug/Crash..."), QKeySequence(), false);
            menuDebugThrowExceptionDuringCommandInfo = addAction(IO::Path("Menu/Debug/Throw Exception During Command"), QKeySequence(), false);
            menuDebugShowCrashReportDialogInfo = addAction(IO::Path("Menu/Debug/Show Crash Report Dialog"), QKeySequence(), false);
            menuDebugSetWindowSizeInfo = addAction(IO::Path("Menu/Debug/Set Window Size..."), QKeySequence(), false);
            menuHelpTrenchBroomManualInfo = addAction(IO::Path("Menu/Help/TrenchBroom Manual"), QKeySequence(), false);
            menuHelpAboutTrenchBroomInfo = addAction(IO::Path("Menu/Help/About TrenchBroom"), QKeySequence(), false);
        }

        const ActionList& ActionList::instance() {
            static ActionList singleton;
            return singleton;
        }

        const std::vector<ActionInfo>& ActionList::actions() const {
            return m_list;
        }
    }
}

