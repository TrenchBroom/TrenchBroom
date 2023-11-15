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

#include "Preferences.h"

#include <QKeySequence>

#include "View/MapViewLayout.h"

#include <vecmath/util.h>

namespace TrenchBroom
{
namespace Preferences
{
Preference<int> MapViewLayout(
  "Views/Map view layout", static_cast<int>(View::MapViewLayout::OnePane));

QString systemTheme()
{
  return QStringLiteral("System");
}
QString darkTheme()
{
  return QStringLiteral("Dark");
}
Preference<QString> Theme("Theme", systemTheme());

Preference<bool> ShowAxes("Renderer/Show axes", true);
Preference<Color> SoftMapBoundsColor(
  "Renderer/Colors/Soft map bounds color", Color(241, 125, 37));
Preference<Color> BackgroundColor("Renderer/Colors/Background", Color(38, 38, 38));
Preference<float> AxisLength("Renderer/Axis length", 128.0f);
Preference<Color> XAxisColor(
  "Renderer/Colors/X axis", Color(0xFF, 0x3D, 0x00, 0.7f), true);
Preference<Color> YAxisColor(
  "Renderer/Colors/Y axis", Color(0x4B, 0x95, 0x00, 0.7f), true);
Preference<Color> ZAxisColor(
  "Renderer/Colors/Z axis", Color(0x10, 0x9C, 0xFF, 0.7f), true);
Preference<Color> PointFileColor(
  "Renderer/Colors/Point file", Color(0.0f, 1.0f, 0.0f, 1.0f));
Preference<Color> PortalFileBorderColor(
  "Renderer/Colors/Portal file border", Color(1.0f, 1.0f, 1.0f, 0.5f));
Preference<Color> PortalFileFillColor(
  "Renderer/Colors/Portal file fill", Color(1.0f, 0.4f, 0.4f, 0.2f));
Preference<bool> ShowFPS("Renderer/Show FPS", false);

Preference<Color>& axisColor(vm::axis::type axis)
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

Preference<Color> CompassBackgroundColor(
  "Renderer/Colors/Compass background", Color(0.5f, 0.5f, 0.5f, 0.5f), true);
Preference<Color> CompassBackgroundOutlineColor(
  "Renderer/Colors/Compass background outline", Color(1.0f, 1.0f, 1.0f, 0.5f), true);
Preference<Color> CompassAxisOutlineColor(
  "Renderer/Colors/Compass axis outline", Color(1.0f, 1.0f, 1.0f, 1.0f), true);

Preference<Color> CameraFrustumColor(
  "Renderer/Colors/Camera frustum", Color(0.0f, 1.0f, 1.0f, 1.0f));

Preference<Color> DefaultGroupColor(
  "Renderer/Colors/Groups", Color(0.7f, 0.4f, 1.0f, 1.0f));
Preference<Color> LinkedGroupColor(
  "Renderer/Colors/Linked Groups", Color(1.0f, 0.35f, 0.87f, 1.0f));

Preference<Color> TutorialOverlayTextColor(
  "Renderer/Colors/Tutorial overlay text", Color(1.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> TutorialOverlayBackgroundColor(
  "Renderer/Colors/Tutorial overlay background", Color(1.0f, 0.5f, 0.0f, 0.6f));

Preference<Color> FaceColor("Renderer/Colors/Faces", Color(0.2f, 0.2f, 0.2f, 1.0f));
Preference<Color> SelectedFaceColor(
  "Renderer/Colors/Selected faces", Color(1.0f, 0.85f, 0.85f, 1.0f));
Preference<Color> LockedFaceColor(
  "Renderer/Colors/Locked faces", Color(0.85f, 0.85f, 1.0f, 1.0f));
Preference<float> TransparentFaceAlpha("Renderer/Colors/Transparent faces", 0.4f);
Preference<Color> EdgeColor("Renderer/Colors/Edges", Color(0.9f, 0.9f, 0.9f, 1.0f));
Preference<Color> SelectedEdgeColor(
  "Renderer/Colors/Selected edges", Color(1.0f, 0.0f, 0.0f, 1.0f));
Preference<float> OccludedSelectedEdgeAlpha(
  "Renderer/Colors/Occluded selected edge alpha", 0.4f);
Preference<Color> LockedEdgeColor(
  "Renderer/Colors/Locked edges", Color(0.13f, 0.3f, 1.0f, 1.0f));
Preference<Color> UndefinedEntityColor(
  "Renderer/Colors/Undefined entity", Color(0.5f, 0.5f, 0.5f, 1.0f));

Preference<Color> SelectionBoundsColor(
  "Renderer/Colors/Selection bounds", Color(1.0f, 0.0f, 0.0f, 0.5f));

Preference<Color> InfoOverlayTextColor(
  "Renderer/Colors/Info overlay text", Color(1.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> GroupInfoOverlayTextColor(
  "Renderer/Colors/Group info overlay text", Color(0.7f, 0.4f, 1.0f, 1.0f));
Preference<Color> InfoOverlayBackgroundColor(
  "Renderer/Colors/Info overlay background", Color(0.0f, 0.0f, 0.0f, 0.6f));
Preference<float> WeakInfoOverlayBackgroundAlpha(
  "Renderer/Colors/Weak info overlay background alpha", 0.3f);
Preference<Color> SelectedInfoOverlayTextColor(
  "Renderer/Colors/Selected info overlay text", Color(1.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> SelectedInfoOverlayBackgroundColor(
  "Renderer/Colors/Selected info overlay background", Color(1.0f, 0.0f, 0.0f, 0.6f));
Preference<Color> LockedInfoOverlayTextColor(
  "Renderer/Colors/Locked info overlay text", Color(0.35f, 0.35f, 0.6f, 1.0f));
Preference<Color> LockedInfoOverlayBackgroundColor(
  "Renderer/Colors/Locked info overlay background", Color(0.0f, 0.0f, 0.0f, 0.6f));

Preference<float> HandleRadius("Controls/Handle radius", 3.0f);
Preference<float> MaximumHandleDistance("Controls/Maximum handle distance", 1000.0f);
Preference<Color> HandleColor("Renderer/Colors/Handle", Color(248, 230, 60, 1.0f));
Preference<Color> OccludedHandleColor(
  "Renderer/Colors/Occluded handle", Color(248, 230, 60, 0.4f));
Preference<Color> SelectedHandleColor(
  "Renderer/Colors/Selected handle", Color(1.0f, 0.0f, 0.0f, 1.0f));
Preference<Color> OccludedSelectedHandleColor(
  "Renderer/Colors/Occluded selected handle", Color(1.0f, 0.0f, 0.0f, 0.4f));

Preference<Color> ClipHandleColor(
  "Renderer/Colors/Clip handle", Color(1.0f, 0.5f, 0.0f, 1.0f));
Preference<Color> ClipFaceColor(
  "Renderer/Colors/Clip face", Color(0.6f, 0.4f, 0.0f, 0.35f));

Preference<Color> ExtrudeHandleColor(
  "Renderer/Colors/Resize handle", Color(248, 230, 60, 1.0f));
Preference<float> RotateHandleRadius("Controls/Rotate handle radius", 64.0f);
Preference<Color> RotateHandleColor(
  "Renderer/Colors/Rotate handle", Color(248, 230, 60, 1.0f));

Preference<Color> ScaleHandleColor(
  "Renderer/Colors/Scale handle", Color(77, 255, 80, 1.0f));
Preference<Color> ScaleFillColor(
  "Renderer/Colors/Scale fill", Color(77, 255, 80, 0.125f));
Preference<Color> ScaleOutlineColor(
  "Renderer/Colors/Scale outline", Color(77, 255, 80, 1.0f));
Preference<float> ScaleOutlineDimAlpha("Renderer/Colors/Scale outline dim alpha", 0.3f);
Preference<Color> ShearFillColor(
  "Renderer/Colors/Shear fill", Color(45, 133, 255, 0.125f));
Preference<Color> ShearOutlineColor(
  "Renderer/Colors/Shear outline", Color(45, 133, 255, 1.0f));

Preference<Color> MoveTraceColor(
  "Renderer/Colors/Move trace", Color(0.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> OccludedMoveTraceColor(
  "Renderer/Colors/Move trace", Color(0.0f, 1.0f, 1.0f, 0.4f));

Preference<Color> MoveIndicatorOutlineColor(
  "Renderer/Colors/Move indicator outline", Color(1.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> MoveIndicatorFillColor(
  "Renderer/Colors/Move indicator fill", Color(0.0f, 0.0f, 0.0f, 0.5f));

Preference<Color> AngleIndicatorColor(
  "Renderer/Colors/Angle indicator", Color(1.0f, 1.0f, 1.0f, 1.0f));

Preference<Color> TextureSeamColor(
  "Renderer/Colors/Texture seam", Color(1.0f, 1.0f, 0.0f, 1.0f));

Preference<float> Brightness("Renderer/Brightness", 1.4f);
Preference<float> GridAlpha("Renderer/Grid/Alpha", 0.5f);
Preference<Color> GridColor2D("Rendere/Grid/Color2D", Color(0.8f, 0.8f, 0.8f, 0.8f));

Preference<int> TextureMinFilter("Renderer/Texture mode min filter", 0x2700);
Preference<int> TextureMagFilter("Renderer/Texture mode mag filter", 0x2600);
Preference<bool> EnableMSAA("Renderer/Enable multisampling", true);

Preference<bool> TextureLock("Editor/Texture lock", true);
Preference<bool> UVLock("Editor/UV lock", false);

Preference<std::filesystem::path>& RendererFontPath()
{
  static Preference<std::filesystem::path> fontPath(
    "Renderer/Font name", "fonts/SourceSansPro-Regular.otf");
  return fontPath;
}

Preference<int> RendererFontSize("Renderer/Font size", 13);

Preference<int> BrowserFontSize("Browser/Font size", 13);
Preference<Color> BrowserTextColor("Browser/Text color", Color(1.0f, 1.0f, 1.0f, 1.0f));
Preference<Color> BrowserSubTextColor(
  "Browser/Sub text color", Color(0.65f, 0.65f, 0.65f, 1.0f));
Preference<Color> BrowserGroupBackgroundColor(
  "Browser/Group background color", Color(0.1f, 0.1f, 0.1f, 0.8f));
Preference<Color> BrowserBackgroundColor(
  "Browser/Background color", Color(0.14f, 0.14f, 0.14f, 1.0f));
Preference<float> TextureBrowserIconSize("Texture Browser/Icon size", 1.0f);
Preference<Color> TextureBrowserDefaultColor(
  "Texture Browser/Default color", Color(0.0f, 0.0f, 0.0f, 0.0f));
Preference<Color> TextureBrowserSelectedColor(
  "Texture Browser/Selected color", Color(1.0f, 0.0f, 0.0f, 1.0f));
Preference<Color> TextureBrowserUsedColor(
  "Texture Browser/Used color", Color(1.0f, 0.7f, 0.0f, 1.0f));

Preference<float> CameraLookSpeed("Controls/Camera/Look speed", 0.5f);
Preference<bool> CameraLookInvertH("Controls/Camera/Invert horizontal look", false);
Preference<bool> CameraLookInvertV("Controls/Camera/Invert vertical look", false);
Preference<float> CameraPanSpeed("Controls/Camera/Pan speed", 0.5f);
Preference<bool> CameraPanInvertH("Controls/Camera/Invert horizontal pan", false);
Preference<bool> CameraPanInvertV("Controls/Camera/Invert vertical pan", false);
Preference<bool> CameraMouseWheelInvert("Controls/Camera/Invert mouse wheel", false);
Preference<float> CameraMoveSpeed("Controls/Camera/Move speed", 0.3f);
Preference<bool> CameraEnableAltMove("Controls/Camera/Use alt to move", false);
Preference<bool> CameraAltMoveInvert(
  "Controls/Camera/Invert zoom direction when using alt to move", false);
Preference<bool> CameraMoveInCursorDir(
  "Controls/Camera/Move camera in cursor dir", false);
Preference<float> CameraFov("Controls/Camera/Field of vision", 90.0f);

Preference<float> CameraFlyMoveSpeed("Controls/Camera/Fly move speed", 0.5f);

Preference<bool> Link2DCameras("Controls/Camera/Link 2D cameras", true);

Preference<QKeySequence>& CameraFlyForward()
{
  static Preference<QKeySequence> pref("Controls/Camera/Move forward", QKeySequence('W'));
  return pref;
}
Preference<QKeySequence>& CameraFlyBackward()
{
  static Preference<QKeySequence> pref(
    "Controls/Camera/Move backward", QKeySequence('S'));
  return pref;
}
Preference<QKeySequence>& CameraFlyLeft()
{
  static Preference<QKeySequence> pref("Controls/Camera/Move left", QKeySequence('A'));
  return pref;
}
Preference<QKeySequence>& CameraFlyRight()
{
  static Preference<QKeySequence> pref("Controls/Camera/Move right", QKeySequence('D'));
  return pref;
}
Preference<QKeySequence>& CameraFlyUp()
{
  static Preference<QKeySequence> pref("Controls/Camera/Move up", QKeySequence('Q'));
  return pref;
}
Preference<QKeySequence>& CameraFlyDown()
{
  static Preference<QKeySequence> pref("Controls/Camera/Move down", QKeySequence('X'));
  return pref;
}

Preference<bool> ShowEntityClassnames("Map view/Show entity classnames", true);
Preference<bool> ShowGroupBounds("Map view/Show group bounds", true);
Preference<bool> ShowBrushEntityBounds("Map view/Show brush entity bounds", true);
Preference<bool> ShowPointEntityBounds("Map view/Show point entity bounds", true);
Preference<bool> ShowPointEntityModels("Map view/Show point entity models", true);

QString faceRenderModeTextured()
{
  return "textured";
}
QString faceRenderModeFlat()
{
  return "flat";
}
QString faceRenderModeSkip()
{
  return "skip";
}
Preference<QString> FaceRenderMode("Map view/Face render mode", "textured");

Preference<bool> ShadeFaces("Map view/Shade faces", true);
Preference<bool> ShowFog("Map view/Show fog", false);
Preference<bool> ShowEdges("Map view/Show edges", true);

Preference<bool> ShowSoftMapBounds("Map view/Show soft map bounds", true);

Preference<bool> ShowPointEntities("Map view/Show point entities", true);
Preference<bool> ShowBrushes("Map view/Show brushes", true);

QString entityLinkModeAll()
{
  return "all";
}
QString entityLinkModeTransitive()
{
  return "transitive";
}
QString entityLinkModeDirect()
{
  return "direct";
}
QString entityLinkModeNone()
{
  return "none";
}
Preference<QString> EntityLinkMode("Map view/Entity link mode", "direct");

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
    &TextureLock,
    &UVLock,
    &RendererFontPath(),
    &RendererFontSize,
    &BrowserFontSize,
    &BrowserTextColor,
    &BrowserSubTextColor,
    &BrowserBackgroundColor,
    &BrowserGroupBackgroundColor,
    &TextureBrowserIconSize,
    &TextureBrowserDefaultColor,
    &TextureBrowserSelectedColor,
    &TextureBrowserUsedColor,
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
    &CameraFlyForward(),
    &CameraFlyBackward(),
    &CameraFlyLeft(),
    &CameraFlyRight(),
    &CameraFlyUp(),
    &CameraFlyDown(),
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

const std::map<std::filesystem::path, PreferenceBase*>& staticPreferencesMap()
{
  static std::map<std::filesystem::path, PreferenceBase*> map;

  if (map.empty())
  {
    for (PreferenceBase* pref : staticPreferences())
    {
      map[pref->path()] = pref;
    }
  }

  return map;
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

DynamicPreferencePattern<QString> GamesPath("Games/*/Path");
DynamicPreferencePattern<QString> GamesToolPath("Games/*/Tool Path/*");
DynamicPreferencePattern<QString> GamesDefaultEngine("Games/*/Default Engine");
DynamicPreferencePattern<QKeySequence> FiltersTagsToggle("Filters/Tags/*/Toggle Visible");
DynamicPreferencePattern<QKeySequence> TagsEnable("Tags/*/Enable");
DynamicPreferencePattern<QKeySequence> TagsDisable("Tags/*/Disable");
DynamicPreferencePattern<QKeySequence> FiltersEntitiesToggleVisible(
  "Filters/Entities/*/Toggle Visible");
DynamicPreferencePattern<QKeySequence> EntitiesCreate("Entities/*/Create");

const std::vector<DynamicPreferencePatternBase*>& dynaimcPreferencePatterns()
{
  static const std::vector<DynamicPreferencePatternBase*> list{
    &GamesPath,
    &GamesToolPath,
    &GamesDefaultEngine,
    &FiltersTagsToggle,
    &TagsEnable,
    &TagsDisable,
    &FiltersEntitiesToggleVisible,
    &EntitiesCreate};
  return list;
}
} // namespace Preferences
} // namespace TrenchBroom
