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

#ifndef TrenchBroom_ActionList
#define TrenchBroom_ActionList

#include "IO/Path.h"
#include "View/ActionContext.h"
#include "View/ViewShortcut.h"

#include <QMetaType>
#include <QKeySequence>
#include <QVariant>

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        struct ActionInfo {
            QKeySequence defaultKey;
            bool modifiable;
            IO::Path preferencePath;
            int actionContext;

            QVariant getQVariant() const;
        };
        const std::vector<ActionInfo>& getActionInfo();

        class ActionList {
        private:
            std::vector<ActionInfo> m_list;
            ActionList();

            ActionInfo addShortcut(const IO::Path& path, QKeySequence keySequence, int actionContext, bool modifiable);
            ActionInfo addAction(const IO::Path& path, QKeySequence keySequence, bool modifiable);
        public:
            static const ActionList& instance();

            ActionInfo controlsMapViewCreatebrushInfo;
            ActionInfo controlsMapViewToggleClipSideInfo;
            ActionInfo controlsMapViewPerformclipInfo;
            ActionInfo controlsMapViewMoveVerticesUpForwardInfo;
            ActionInfo controlsMapViewMoveVerticesDownBackwardInfo;
            ActionInfo controlsMapViewMoveVerticesLeftInfo;
            ActionInfo controlsMapViewMoveVerticesRightInfo;
            ActionInfo controlsMapViewMoveVerticesBackwardUpInfo;
            ActionInfo controlsMapViewMoveVerticesForwardDownInfo;
            ActionInfo controlsMapViewMoveRotationCenterUpForwardInfo;
            ActionInfo controlsMapViewMoveRotationCenterDownBackwardInfo;
            ActionInfo controlsMapViewMoveRotationCenterLeftInfo;
            ActionInfo controlsMapViewMoveRotationCenterRightInfo;
            ActionInfo controlsMapViewMoveRotationCenterBackwardUpInfo;
            ActionInfo controlsMapViewMoveRotationCenterForwardDownInfo;
            ActionInfo controlsMapViewMoveObjectsUpForwardInfo;
            ActionInfo controlsMapViewMoveObjectsDownBackwardInfo;
            ActionInfo controlsMapViewMoveObjectsLeftInfo;
            ActionInfo controlsMapViewMoveObjectsRightInfo;
            ActionInfo controlsMapViewMoveObjectsBackwardUupInfo;
            ActionInfo controlsMapViewMoveObjectsForwardDownInfo;
            ActionInfo controlsMapViewRollObjectsClockwiseInfo;
            ActionInfo controlsMapViewRollObjectsCounterClockwiseInfo;
            ActionInfo controlsMapViewYawObjectsClockwiseInfo;
            ActionInfo controlsMapViewYawObjectsCounterClockwiseInfo;
            ActionInfo controlsMapViewPitchobjectsClockwiseInfo;
            ActionInfo controlsMapViewPitchobjectsCounterClockwiseInfo;
            ActionInfo controlsMapViewFlipobjectsHorizontallyInfo;
            ActionInfo controlsMapViewFlipobjectsBerticallyInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsUpForwardInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsDownBackwardInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsLeftInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsRightInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsBackwardUpInfo;
            ActionInfo controlsMapViewDuplicateAndMoveObjectsForwardDownInfo;
            ActionInfo controlsMapViewMoveTexturesupInfo;
            ActionInfo controlsMapViewMoveTexturesupFineInfo;
            ActionInfo controlsMapViewMoveTexturesupCoarseInfo;
            ActionInfo controlsMapViewMoveTexturesdownInfo;
            ActionInfo controlsMapViewMoveTexturesdownFineInfo;
            ActionInfo controlsMapViewMoveTexturesdownCoarseInfo;
            ActionInfo controlsMapViewMoveTexturesleftInfo;
            ActionInfo controlsMapViewMoveTexturesleftFineInfo;
            ActionInfo controlsMapViewMoveTexturesleftCoarseInfo;
            ActionInfo controlsMapViewMoveTexturesrightInfo;
            ActionInfo controlsMapViewMoveTexturesrightFineInfo;
            ActionInfo controlsMapViewMoveTexturesrightCoarseInfo;
            ActionInfo controlsMapViewRotateTexturesclockwiseInfo;
            ActionInfo controlsMapViewRotateTexturesclockwiseFineInfo;
            ActionInfo controlsMapViewRotateTexturesclockwiseCoarseInfo;
            ActionInfo controlsMapViewRotateTexturescounterClockwiseInfo;
            ActionInfo controlsMapViewRotateTexturescounterClockwiseFineInfo;
            ActionInfo controlsMapViewRotateTexturescounterClockwiseCoarseInfo;
            ActionInfo controlsMapViewCycleMapViewInfo;
            ActionInfo controlsMapViewResetcamerazoomInfo;
            ActionInfo controlsMapViewCancelInfo;
            ActionInfo controlsMapViewDeactivatecurrenttoolInfo;

            ActionInfo menuFileNewInfo;
            ActionInfo menuFileOpenInfo;
            ActionInfo menuFileSaveInfo;
            ActionInfo menuFileSaveasInfo;
            ActionInfo menuFileExportWavefrontOBJInfo;
            ActionInfo menuFileLoadPointFileInfo;
            ActionInfo menuFileReloadPointFileInfo;
            ActionInfo menuFileUnloadPointFileInfo;
            ActionInfo menuFileLoadPortalFileInfo;
            ActionInfo menuFileReloadPortalFileInfo;
            ActionInfo menuFileUnloadPortalFileInfo;
            ActionInfo menuFileReloadTextureCollectionsInfo;
            ActionInfo menuFileReloadEntityDefinitionsInfo;
            ActionInfo menuFileCloseInfo;

            ActionInfo menuEditUndoInfo;
            ActionInfo menuEditRedoInfo;
            ActionInfo menuEditRepeatInfo;
            ActionInfo menuEditClearRepeatableCommandsInfo;
            ActionInfo menuEditCutInfo;
            ActionInfo menuEditCopyInfo;
            ActionInfo menuEditPasteInfo;
            ActionInfo menuEditPasteatOriginalPositionInfo;
            ActionInfo menuEditDuplicateInfo;
            ActionInfo menuEditDeleteInfo;
            ActionInfo menuEditSelectAllInfo;
            ActionInfo menuEditSelectSiblingsInfo;
            ActionInfo menuEditSelectTouchingInfo;
            ActionInfo menuEditSelectInsideInfo;
            ActionInfo menuEditSelectTallInfo;
            ActionInfo menuEditSelectbyLineNumberInfo;
            ActionInfo menuEditSelectNoneInfo;
            ActionInfo menuEditGroupInfo;
            ActionInfo menuEditUngroupInfo;
            ActionInfo menuEditToolsBrushToolInfo;
            ActionInfo menuEditToolsClipToolInfo;
            ActionInfo menuEditToolsRotateToolInfo;
            ActionInfo menuEditToolsScaleToolInfo;
            ActionInfo menuEditToolsShearToolInfo;
            ActionInfo menuEditToolsVertexToolInfo;
            ActionInfo menuEditToolsEdgeToolInfo;
            ActionInfo menuEditToolsFaceToolInfo;
            ActionInfo menuEditCSGConvexMergeInfo;
            ActionInfo menuEditCSGSubtractInfo;
            ActionInfo menuEditCSGHollowInfo;
            ActionInfo menuEditCSGIntersectInfo;
            ActionInfo menuEditSnapVerticestoIntegerInfo;
            ActionInfo menuEditSnapVerticestoGridInfo;
            ActionInfo menuEditTextureLockInfo;
            ActionInfo menuEditUVLockInfo;
            ActionInfo menuEditReplaceTextureInfo;
            ActionInfo menuViewGridShowGridInfo;
            ActionInfo menuViewGridSnaptoGridInfo;
            ActionInfo menuViewGridIncreaseGridSizeInfo;
            ActionInfo menuViewGridDecreaseGridSizeInfo;
            ActionInfo menuViewGridSetGridSize0125Info;
            ActionInfo menuViewGridSetGridSize025Info;
            ActionInfo menuViewGridSetGridSize05Info;
            ActionInfo menuViewGridSetGridSize1Info;
            ActionInfo menuViewGridSetGridSize2Info;
            ActionInfo menuViewGridSetGridSize4Info;
            ActionInfo menuViewGridSetGridSize8Info;
            ActionInfo menuViewGridSetGridSize16Info;
            ActionInfo menuViewGridSetGridSize32Info;
            ActionInfo menuViewGridSetGridSize64Info;
            ActionInfo menuViewGridSetGridSize128Info;
            ActionInfo menuViewGridSetGridSize256Info;
            ActionInfo menuViewCameraMovetoNextPointInfo;
            ActionInfo menuViewCameraMovetoPreviousPointInfo;
            ActionInfo menuViewCameraFocusonSelectionInfo;
            ActionInfo menuViewCameraMoveCameratoInfo;
            ActionInfo menuViewIsolateInfo;
            ActionInfo menuViewHideInfo;
            ActionInfo menuViewShowAllInfo;
            ActionInfo menuViewSwitchtoMapInspectorInfo;
            ActionInfo menuViewSwitchtoEntityInspectorInfo;
            ActionInfo menuViewSwitchtoFaceInspectorInfo;
            ActionInfo menuViewToggleInfoPanelInfo;
            ActionInfo menuViewToggleInspectorInfo;
            ActionInfo menuViewMaximizeCurrentViewInfo;
            ActionInfo menuViewPreferencesInfo;
            ActionInfo menuRunCompileInfo;
            ActionInfo menuRunLaunchInfo;
            ActionInfo menuDebugPrintVerticesInfo;
            ActionInfo menuDebugCreateBrushInfo;
            ActionInfo menuDebugCreateCubeInfo;
            ActionInfo menuDebugClipBrushInfo;
            ActionInfo menuDebugCopyJavascriptShortcutMapInfo;
            ActionInfo menuDebugCrashInfo;
            ActionInfo menuDebugThrowExceptionDuringCommandInfo;
            ActionInfo menuDebugShowCrashReportDialogInfo;
            ActionInfo menuDebugSetWindowSizeInfo;
            ActionInfo menuHelpTrenchBroomManualInfo;
            ActionInfo menuHelpAboutTrenchBroomInfo;
        };
    }
}

Q_DECLARE_METATYPE(const TrenchBroom::View::ActionInfo*)

#endif /* defined(TrenchBroom_ActionList) */
