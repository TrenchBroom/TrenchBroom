/*
 Copyright (C) 2010 Kristian Duske

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

#include "vm/util.h"

namespace tb::Preferences
{

Preference<Color>& axisColor(const vm::axis::type axis)
{
  switch (axis)
  {
  case vm::axis::x:
    return Preferences::XAxisColor;
  case vm::axis::y:
    return Preferences::YAxisColor;
  case vm::axis::z:
  default:
    return Preferences::ZAxisColor;
  }
}

const std::vector<PreferenceBase*>& staticPreferences()
{
  static const std::vector<PreferenceBase*> list{
    &MapViewLayout,
    &Theme,
    &ShowAxes,
    &BackgroundColor,
    &AxisLength,
    &XAxisColor,
    &YAxisColor,
    &ZAxisColor,
    &PointFileColor,
    &PortalFileBorderColor,
    &PortalFileFillColor,
    &ShowFPS,
    &CompassBackgroundColor,
    &CompassBackgroundOutlineColor,
    &CompassAxisOutlineColor,
    &CameraFrustumColor,
    &DefaultGroupColor,
    &TutorialOverlayTextColor,
    &TutorialOverlayBackgroundColor,
    &FaceColor,
    &SelectedFaceColor,
    &LockedFaceColor,
    &TransparentFaceAlpha,
    &EdgeColor,
    &SelectedEdgeColor,
    &OccludedSelectedEdgeAlpha,
    &LockedEdgeColor,
    &UndefinedEntityColor,
    &SelectionBoundsColor,
    &InfoOverlayTextColor,
    &GroupInfoOverlayTextColor,
    &InfoOverlayBackgroundColor,
    &WeakInfoOverlayBackgroundAlpha,
    &SelectedInfoOverlayTextColor,
    &SelectedInfoOverlayBackgroundColor,
    &LockedInfoOverlayTextColor,
    &LockedInfoOverlayBackgroundColor,
    &HandleRadius,
    &MaximumHandleDistance,
    &HandleColor,
    &OccludedHandleColor,
    &SelectedHandleColor,
    &OccludedSelectedHandleColor,
    &ClipHandleColor,
    &ClipFaceColor,
    &ExtrudeHandleColor,
    &RotateHandleRadius,
    &RotateHandleColor,
    &ScaleHandleColor,
    &ScaleFillColor,
    &ScaleOutlineColor,
    &ScaleOutlineDimAlpha,
    &ShearFillColor,
    &ShearOutlineColor,
    &MoveTraceColor,
    &OccludedMoveTraceColor,
    &MoveIndicatorOutlineColor,
    &MoveIndicatorFillColor,
    &AngleIndicatorColor,
    &TextureSeamColor,
    &Brightness,
    &GridAlpha,
    &GridColor2D,
    &TextureMinFilter,
    &TextureMagFilter,
    &AlignmentLock,
    &UVLock,
    &RendererFontPath,
    &RendererFontSize,
    &BrowserFontSize,
    &BrowserTextColor,
    &BrowserSubTextColor,
    &BrowserBackgroundColor,
    &BrowserGroupBackgroundColor,
    &MaterialBrowserIconSize,
    &MaterialBrowserDefaultColor,
    &MaterialBrowserSelectedColor,
    &MaterialBrowserUsedColor,
    &CameraLookSpeed,
    &CameraLookInvertH,
    &CameraLookInvertV,
    &CameraPanSpeed,
    &CameraPanInvertH,
    &CameraPanInvertV,
    &CameraMouseWheelInvert,
    &CameraMoveSpeed,
    &CameraEnableAltMove,
    &CameraAltMoveInvert,
    &CameraMoveInCursorDir,
    &CameraFov,
    &CameraFlyMoveSpeed,
    &Link2DCameras,
    &CameraFlyForward,
    &CameraFlyBackward,
    &CameraFlyLeft,
    &CameraFlyRight,
    &CameraFlyUp,
    &CameraFlyDown,
    &ShowEntityClassnames,
    &ShowGroupBounds,
    &ShowBrushEntityBounds,
    &ShowPointEntityBounds,
    &ShowPointEntityModels,
    &FaceRenderMode,
    &ShadeFaces,
    &ShowFog,
    &ShowEdges,
    &ShowSoftMapBounds,
    &ShowPointEntities,
    &ShowBrushes,
    &EntityLinkMode};

  return list;
}

std::vector<Preference<QKeySequence>*> keyPreferences()
{
  std::vector<Preference<QKeySequence>*> result;

  for (PreferenceBase* pref : staticPreferences())
  {
    auto* keyPref = dynamic_cast<Preference<QKeySequence>*>(pref);
    if (keyPref != nullptr)
    {
      result.push_back(keyPref);
    }
  }

  return result;
}

} // namespace tb::Preferences
