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

std::vector<Preference<Color>*> colorPreferences()
{
  return {
    &SoftMapBoundsColor,
    &BackgroundColor,
    &XAxisColor,
    &YAxisColor,
    &ZAxisColor,
    &PointFileColor,
    &PortalFileBorderColor,
    &PortalFileFillColor,
    &CompassBackgroundColor,
    &CompassBackgroundOutlineColor,
    &CompassAxisOutlineColor,
    &CameraFrustumColor,
    &DefaultGroupColor,
    &LinkedGroupColor,
    &TutorialOverlayTextColor,
    &TutorialOverlayBackgroundColor,
    &FaceColor,
    &SelectedFaceColor,
    &LockedFaceColor,
    &EdgeColor,
    &SelectedEdgeColor,
    &LockedEdgeColor,
    &UndefinedEntityColor,
    &SelectionBoundsColor,
    &InfoOverlayTextColor,
    &GroupInfoOverlayTextColor,
    &InfoOverlayBackgroundColor,
    &SelectedInfoOverlayTextColor,
    &SelectedInfoOverlayBackgroundColor,
    &LockedInfoOverlayTextColor,
    &LockedInfoOverlayBackgroundColor,
    &HandleColor,
    &OccludedHandleColor,
    &SelectedHandleColor,
    &OccludedSelectedHandleColor,
    &ClipHandleColor,
    &ClipFaceColor,
    &ExtrudeHandleColor,
    &RotateHandleColor,
    &ScaleHandleColor,
    &ScaleFillColor,
    &ScaleOutlineColor,
    &ShearFillColor,
    &ShearOutlineColor,
    &MoveTraceColor,
    &OccludedMoveTraceColor,
    &MoveIndicatorOutlineColor,
    &MoveIndicatorFillColor,
    &AngleIndicatorColor,
    &TextureSeamColor,
    &GridColor2D,
    &BrowserTextColor,
    &BrowserSubTextColor,
    &BrowserGroupBackgroundColor,
    &BrowserBackgroundColor,
    &MaterialBrowserDefaultColor,
    &MaterialBrowserSelectedColor,
    &MaterialBrowserUsedColor,
  };
}

std::vector<Preference<QKeySequence>*> keyPreferences()
{
  return {
    &CameraFlyForward,
    &CameraFlyBackward,
    &CameraFlyLeft,
    &CameraFlyRight,
    &CameraFlyUp,
    &CameraFlyDown,
  };
}

} // namespace tb::Preferences
