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

#pragma once

#include "Color.h"
#include "IO/Path.h"
#include "Preference.h"

#include <vector>

#include <vecmath/util.h>

namespace TrenchBroom
{
namespace Preferences
{
// NOTE: any QKeySequence preferences must be functions like CameraFly*
// because QKeySequence docs specify that you can't create an instance before QApplication

// NOTE: When adding a new preference here, always update the staticPreferences()
// implementation

extern Preference<int> MapViewLayout;

QString systemTheme();
QString darkTheme();
extern Preference<QString> Theme;

extern Preference<bool> ShowAxes;
extern Preference<Color> SoftMapBoundsColor;
extern Preference<Color> BackgroundColor;
extern Preference<float> AxisLength;
extern Preference<Color> XAxisColor;
extern Preference<Color> YAxisColor;
extern Preference<Color> ZAxisColor;
extern Preference<Color> PointFileColor;
extern Preference<Color> PortalFileBorderColor;
extern Preference<Color> PortalFileFillColor;
extern Preference<bool> ShowFPS;

Preference<Color>& axisColor(vm::axis::type axis);

extern Preference<Color> CompassBackgroundColor;
extern Preference<Color> CompassBackgroundOutlineColor;
extern Preference<Color> CompassAxisOutlineColor;

extern Preference<Color> CameraFrustumColor;

extern Preference<Color> DefaultGroupColor;
extern Preference<Color> LinkedGroupColor;

extern Preference<Color> TutorialOverlayTextColor;
extern Preference<Color> TutorialOverlayBackgroundColor;

extern Preference<Color> FaceColor;
extern Preference<Color> SelectedFaceColor;
extern Preference<Color> LockedFaceColor;
extern Preference<float> TransparentFaceAlpha;
extern Preference<Color> EdgeColor;
extern Preference<Color> SelectedEdgeColor;
extern Preference<float> OccludedSelectedEdgeAlpha;
extern Preference<Color> LockedEdgeColor;
extern Preference<Color> UndefinedEntityColor;

extern Preference<Color> SelectionBoundsColor;

extern Preference<Color> InfoOverlayTextColor;
extern Preference<Color> GroupInfoOverlayTextColor;
extern Preference<Color> InfoOverlayBackgroundColor;
extern Preference<float> WeakInfoOverlayBackgroundAlpha;
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

extern Preference<Color> ExtrudeHandleColor;
extern Preference<float> RotateHandleRadius;
extern Preference<Color> RotateHandleColor;

extern Preference<Color> ScaleHandleColor;
extern Preference<Color> ScaleFillColor;
extern Preference<Color> ScaleOutlineColor;
extern Preference<float> ScaleOutlineDimAlpha;
extern Preference<Color> ShearFillColor;
extern Preference<Color> ShearOutlineColor;

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
extern Preference<bool> EnableMSAA;

extern Preference<bool> TextureLock;
extern Preference<bool> UVLock;

Preference<IO::Path>& RendererFontPath();
extern Preference<int> RendererFontSize;

extern Preference<int> BrowserFontSize;
extern Preference<Color> BrowserTextColor;
extern Preference<Color> BrowserSubTextColor;
extern Preference<Color> BrowserBackgroundColor;
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

extern Preference<float> CameraFov;

static constexpr auto MinCameraFlyMoveSpeed = 0.1f;
static constexpr auto MaxCameraFlyMoveSpeed = 10.0f;
extern Preference<float> CameraFlyMoveSpeed;

extern Preference<bool> Link2DCameras;

extern Preference<QKeySequence>& CameraFlyForward();
extern Preference<QKeySequence>& CameraFlyBackward();
extern Preference<QKeySequence>& CameraFlyLeft();
extern Preference<QKeySequence>& CameraFlyRight();
extern Preference<QKeySequence>& CameraFlyUp();
extern Preference<QKeySequence>& CameraFlyDown();

// Map view config
extern Preference<bool> ShowEntityClassnames;
extern Preference<bool> ShowGroupBounds;
extern Preference<bool> ShowBrushEntityBounds;
extern Preference<bool> ShowPointEntityBounds;
extern Preference<bool> ShowPointEntityModels;

QString faceRenderModeTextured();
QString faceRenderModeFlat();
QString faceRenderModeSkip();
extern Preference<QString> FaceRenderMode;

extern Preference<bool> ShadeFaces;
extern Preference<bool> ShowFog;
extern Preference<bool> ShowEdges;

extern Preference<bool> ShowSoftMapBounds;

// Editor context
extern Preference<bool> ShowPointEntities;
extern Preference<bool> ShowBrushes;

QString entityLinkModeAll();
QString entityLinkModeTransitive();
QString entityLinkModeDirect();
QString entityLinkModeNone();
extern Preference<QString> EntityLinkMode;

/**
 * Returns all Preferences declared in this file. Needed for migrating preference formats
 * or if we wanted to do a Path to Preference lookup.
 */
const std::vector<PreferenceBase*>& staticPreferences();
const std::map<IO::Path, PreferenceBase*>& staticPreferencesMap();
/**
 * Returns the subset of staticPreferences() that are key sequences, used by
 * dump-shortcuts.
 */
std::vector<Preference<QKeySequence>*> keyPreferences();

extern DynamicPreferencePattern<QString> GamesPath;
extern DynamicPreferencePattern<QString> GamesToolPath;
extern DynamicPreferencePattern<QString> GamesDefaultEngine;
extern DynamicPreferencePattern<QKeySequence> FiltersTagsToggle;
extern DynamicPreferencePattern<QKeySequence> TagsEnable;
extern DynamicPreferencePattern<QKeySequence> TagsDisable;
extern DynamicPreferencePattern<QKeySequence> FiltersEntitiesToggleVisible;
extern DynamicPreferencePattern<QKeySequence> EntitiesCreate;

const std::vector<DynamicPreferencePatternBase*>& dynaimcPreferencePatterns();
} // namespace Preferences
} // namespace TrenchBroom
