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

#include <QKeySequence>

#include "ui/MapViewLayout.h"

#include "vm/util.h"

namespace tb::Preferences
{

// Must be set to false for tests, see TestPreferenceManager::initialize
Preference<bool> AskForAutoUpdates("updater/Ask for auto updates", true);
Preference<bool> AutoCheckForUpdates("updater/Check for updates automatically", false);
Preference<bool> IncludePreReleaseUpdates("updater/Include pre-releases", false);
Preference<bool> IncludeDraftReleaseUpdates(
  "updater/Include draft releases", false, PreferencePersistencePolicy::Transient);

Preference<int> MapViewLayout(
  "Views/Map view layout", static_cast<int>(ui::MapViewLayout::OnePane));

QString systemTheme()
{
  return QStringLiteral("System");
}
QString darkTheme()
{
  return QStringLiteral("Dark");
}
Preference<QString> Theme("Theme", systemTheme());

Preference<bool> ShowAxes("render/Show axes", true);
Preference<Color> SoftMapBoundsColor(
  "render/Colors/Soft map bounds color", RgbB(241, 125, 37));
Preference<Color> BackgroundColor("render/Colors/Background", RgbB(38, 38, 38));
Preference<float> AxisLength("render/Axis length", 128.0f);
Preference<Color> XAxisColor(
  "render/Colors/X axis",
  RgbaF(1.0f, 0.24f, 0.0f, 0.7f),
  PreferencePersistencePolicy::ReadOnly);
Preference<Color> YAxisColor(
  "render/Colors/Y axis",
  RgbaF(0.29f, 0.58f, 0.0f, 0.7f),
  PreferencePersistencePolicy::ReadOnly);
Preference<Color> ZAxisColor(
  "render/Colors/Z axis",
  RgbaF(0.06f, 0.61f, 1.0f, 0.7f),
  PreferencePersistencePolicy::ReadOnly);
Preference<Color> PointFileColor("render/Colors/Point file", RgbF(0.0f, 1.0f, 0.0f));
Preference<Color> PortalFileBorderColor(
  "render/Colors/Portal file border", RgbaF(1.0f, 1.0f, 1.0f, 0.5f));
Preference<Color> PortalFileFillColor(
  "render/Colors/Portal file fill", RgbaF(1.0f, 0.4f, 0.4f, 0.2f));
Preference<bool> ShowFPS("render/Show FPS", false);

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
  "render/Colors/Compass background",
  RgbaF(0.5f, 0.5f, 0.5f, 0.5f),
  PreferencePersistencePolicy::ReadOnly);
Preference<Color> CompassBackgroundOutlineColor(
  "render/Colors/Compass background outline",
  RgbaF(1.0f, 1.0f, 1.0f, 0.5f),
  PreferencePersistencePolicy::ReadOnly);
Preference<Color> CompassAxisOutlineColor(
  "render/Colors/Compass axis outline",
  RgbF(1.0f, 1.0f, 1.0f),
  PreferencePersistencePolicy::ReadOnly);

Preference<Color> CameraFrustumColor(
  "render/Colors/Camera frustum", RgbF(0.0f, 1.0f, 1.0f));

Preference<Color> DefaultGroupColor("render/Colors/Groups", RgbF(0.7f, 0.4f, 1.0f));
Preference<Color> LinkedGroupColor(
  "render/Colors/Linked Groups", RgbF(1.0f, 0.35f, 0.87f));

Preference<Color> TutorialOverlayTextColor(
  "render/Colors/Tutorial overlay text", RgbF(1.0f, 1.0f, 1.0f));
Preference<Color> TutorialOverlayBackgroundColor(
  "render/Colors/Tutorial overlay background", RgbaF(1.0f, 0.5f, 0.0f, 0.6f));

Preference<Color> FaceColor("render/Colors/Faces", RgbF(0.2f, 0.2f, 0.2f));
Preference<Color> SelectedFaceColor(
  "render/Colors/Selected faces", RgbF(1.0f, 0.85f, 0.85f));
Preference<Color> LockedFaceColor("render/Colors/Locked faces", RgbF(0.85f, 0.85f, 1.0f));
Preference<float> TransparentFaceAlpha("render/Colors/Transparent faces", 0.4f);
Preference<Color> EdgeColor("render/Colors/Edges", RgbF(0.9f, 0.9f, 0.9f));
Preference<Color> SelectedEdgeColor(
  "render/Colors/Selected edges", RgbF(1.0f, 0.0f, 0.0f));
Preference<float> OccludedSelectedEdgeAlpha(
  "render/Colors/Occluded selected edge alpha", 0.4f);
Preference<Color> LockedEdgeColor("render/Colors/Locked edges", RgbF(0.13f, 0.3f, 1.0f));
Preference<Color> UndefinedEntityColor(
  "render/Colors/Undefined entity", RgbF(0.5f, 0.5f, 0.5f));

Preference<Color> SelectionBoundsColor(
  "render/Colors/Selection bounds", RgbaF(1.0f, 0.0f, 0.0f, 0.35f));

Preference<Color> InfoOverlayTextColor(
  "render/Colors/Info overlay text", RgbF(1.0f, 1.0f, 1.0f));
Preference<Color> GroupInfoOverlayTextColor(
  "render/Colors/Group info overlay text", RgbF(0.7f, 0.4f, 1.0f));
Preference<Color> InfoOverlayBackgroundColor(
  "render/Colors/Info overlay background", RgbaF(0.0f, 0.0f, 0.0f, 0.6f));
Preference<float> WeakInfoOverlayBackgroundAlpha(
  "render/Colors/Weak info overlay background alpha", 0.3f);
Preference<Color> SelectedInfoOverlayTextColor(
  "render/Colors/Selected info overlay text", RgbF(1.0f, 1.0f, 1.0f));
Preference<Color> SelectedInfoOverlayBackgroundColor(
  "render/Colors/Selected info overlay background", RgbaF(1.0f, 0.0f, 0.0f, 0.6f));
Preference<Color> LockedInfoOverlayTextColor(
  "render/Colors/Locked info overlay text", RgbF(0.35f, 0.35f, 0.6f));
Preference<Color> LockedInfoOverlayBackgroundColor(
  "render/Colors/Locked info overlay background", RgbaF(0.0f, 0.0f, 0.0f, 0.6f));

Preference<float> HandleRadius("Controls/Handle radius", 3.0f);
Preference<float> MaximumHandleDistance("Controls/Maximum handle distance", 1000.0f);
Preference<Color> HandleColor("render/Colors/Handle", RgbF(0.97f, 0.9f, 0.23f));
Preference<Color> OccludedHandleColor(
  "render/Colors/Occluded handle", RgbaF(0.87f, 0.9f, 0.23f, 0.4f));
Preference<Color> SelectedHandleColor(
  "render/Colors/Selected handle", RgbF(1.0f, 0.0f, 0.0f));
Preference<Color> OccludedSelectedHandleColor(
  "render/Colors/Occluded selected handle", RgbaF(1.0f, 0.0f, 0.0f, 0.4f));

Preference<Color> ClipHandleColor("render/Colors/Clip handle", RgbF(1.0f, 0.5f, 0.0f));
Preference<Color> ClipFaceColor(
  "render/Colors/Clip face", RgbaF(0.6f, 0.4f, 0.0f, 0.35f));

Preference<Color> ExtrudeHandleColor("render/Colors/Resize handle", RgbB(248, 230, 60));
Preference<float> RotateHandleRadius("Controls/Rotate handle radius", 64.0f);
Preference<Color> RotateHandleColor("render/Colors/Rotate handle", RgbB(248, 230, 60));

Preference<Color> ScaleHandleColor("render/Colors/Scale handle", RgbB(77, 255, 80));
Preference<Color> ScaleFillColor(
  "render/Colors/Scale fill", RgbaF(77 / 255.0f, 1.0f, 80 / 255.0f, 0.125f));
Preference<Color> ScaleOutlineColor("render/Colors/Scale outline", RgbB(77, 255, 80));
Preference<float> ScaleOutlineDimAlpha("render/Colors/Scale outline dim alpha", 0.3f);
Preference<Color> ShearFillColor(
  "render/Colors/Shear fill", RgbaF(45 / 255.0f, 133 / 255.0f, 1.0f, 0.125f));
Preference<Color> ShearOutlineColor("render/Colors/Shear outline", RgbB(45, 133, 255));

Preference<Color> MoveTraceColor("render/Colors/Move trace", RgbF(0.0f, 1.0f, 1.0f));
Preference<Color> OccludedMoveTraceColor(
  "render/Colors/Move trace", RgbaF(0.0f, 1.0f, 1.0f, 0.4f));

Preference<Color> MoveIndicatorOutlineColor(
  "render/Colors/Move indicator outline", RgbF(1.0f, 1.0f, 1.0f));
Preference<Color> MoveIndicatorFillColor(
  "render/Colors/Move indicator fill", RgbaF(0.0f, 0.0f, 0.0f, 0.5f));

Preference<Color> AngleIndicatorColor(
  "render/Colors/Angle indicator", RgbF(1.0f, 1.0f, 1.0f));

Preference<Color> TextureSeamColor("render/Colors/Texture seam", RgbF(1.0f, 1.0f, 0.0f));

Preference<float> Brightness("render/Brightness", 1.4f);
Preference<float> GridAlpha("render/Grid/Alpha", 0.5f);
Preference<Color> GridColor2D("render/Grid/Color2D", RgbaF(0.8f, 0.8f, 0.8f, 0.8f));

Preference<int> TextureMinFilter("render/Texture mode min filter", 0x2700);
Preference<int> TextureMagFilter("render/Texture mode mag filter", 0x2600);
Preference<bool> EnableMSAA("render/Enable multisampling", true);

Preference<bool> AlignmentLock("Editor/Texture lock", true);
Preference<bool> UVLock("Editor/UV lock", false);

Preference<std::filesystem::path>& RendererFontPath()
{
  static Preference<std::filesystem::path> fontPath(
    "render/Font name", "fonts/SourceSansPro-Regular.otf");
  return fontPath;
}

Preference<int> RendererFontSize("render/Font size", 13);

Preference<int> BrowserFontSize("Browser/Font size", 13);
Preference<Color> BrowserTextColor("Browser/Text color", RgbF(1.0f, 1.0f, 1.0f));
Preference<Color> BrowserSubTextColor(
  "Browser/Sub text color", RgbF(0.65f, 0.65f, 0.65f));
Preference<Color> BrowserGroupBackgroundColor(
  "Browser/Group background color", RgbaF(0.1f, 0.1f, 0.1f, 0.8f));
Preference<Color> BrowserBackgroundColor(
  "Browser/Background color", RgbF(0.14f, 0.14f, 0.14f));
Preference<float> MaterialBrowserIconSize("Texture Browser/Icon size", 1.0f);
Preference<Color> MaterialBrowserDefaultColor(
  "Texture Browser/Default color", RgbaF(0.0f, 0.0f, 0.0f, 0.0f));
Preference<Color> MaterialBrowserSelectedColor(
  "Texture Browser/Selected color", RgbF(1.0f, 0.0f, 0.0f));
Preference<Color> MaterialBrowserUsedColor(
  "Texture Browser/Used color", RgbF(1.0f, 0.7f, 0.0f));

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
    &AlignmentLock,
    &UVLock,
    &RendererFontPath(),
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

} // namespace tb::Preferences
