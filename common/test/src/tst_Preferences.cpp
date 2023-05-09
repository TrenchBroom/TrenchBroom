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

#include <QString>
#include <QTextStream>

#include "Assets/EntityDefinition.h"
#include "Color.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "QtPrettyPrinters.h"
#include "View/Actions.h"

#include <kdl/vector_utils.h>

#include <vecmath/approx.h>
#include <vecmath/bbox.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "Catch2.h"


inline std::ostream& operator<<(std::ostream& lhs, const QJsonValue& rhs)
{
  lhs << rhs.toString().toStdString();
  return lhs;
}

namespace TrenchBroom
{
namespace
{
QJsonValue getValue(
  const std::map<std::filesystem::path, QJsonValue>& map,
  const std::filesystem::path& key)
{
  auto it = map.find(key);
  return it != map.end() ? it->second : QJsonValue{QJsonValue::Undefined};
}
} // namespace

static void testPrefs(const std::map<std::filesystem::path, QJsonValue>& prefs)
{
  CHECK(getValue(prefs, "Controls/Camera/Field of vision") == QJsonValue{108});
  CHECK(getValue(prefs, "Controls/Camera/Move down") == QJsonValue{"R"});
  CHECK(getValue(prefs, "Controls/Camera/Move up") == QJsonValue{"W"});
  CHECK(getValue(prefs, "Controls/Camera/Move right") == QJsonValue{"F"});
  CHECK(getValue(prefs, "Controls/Camera/Move left") == QJsonValue{"S"});
  CHECK(getValue(prefs, "Controls/Camera/Move backward") == QJsonValue{"D"});
  CHECK(getValue(prefs, "Controls/Camera/Move forward") == QJsonValue{"E"});
  CHECK(
    static_cast<float>(getValue(prefs, "Controls/Camera/Fly move speed").toDouble())
    == vm::approx{0.425781f});
  CHECK(getValue(prefs, "Controls/Camera/Move camera in cursor dir") == QJsonValue{true});
  CHECK(getValue(prefs, "Controls/Camera/Use alt to move") == QJsonValue{true});
  CHECK(
    static_cast<float>(getValue(prefs, "Controls/Camera/Move speed").toDouble())
    == vm::approx{0.35f});
  CHECK(getValue(prefs, "Controls/Camera/Invert mouse wheel") == QJsonValue{true});
  CHECK(getValue(prefs, "Controls/Camera/Invert vertical pan") == QJsonValue{true});
  CHECK(getValue(prefs, "Controls/Camera/Invert horizontal pan") == QJsonValue{true});
  CHECK(
    static_cast<float>(getValue(prefs, "Controls/Camera/Pan speed").toDouble())
    == vm::approx{0.55f});
  CHECK(getValue(prefs, "Controls/Camera/Invert vertical look") == QJsonValue{true});
  CHECK(getValue(prefs, "Controls/Camera/Invert horizontal look") == QJsonValue{true});
  CHECK(
    static_cast<float>(getValue(prefs, "Controls/Camera/Look speed").toDouble())
    == vm::approx{0.44f});
  CHECK(
    static_cast<float>(getValue(prefs, "Texture Browser/Icon size").toDouble())
    == vm::approx{1.5f});
  CHECK(getValue(prefs, "Renderer/Font size") == QJsonValue{14});
  CHECK(getValue(prefs, "Renderer/Texture mode mag filter") == QJsonValue{9729});
  CHECK(getValue(prefs, "Renderer/Texture mode min filter") == QJsonValue{9987});
  CHECK(
    static_cast<float>(getValue(prefs, "Renderer/Brightness").toDouble())
    == vm::approx{0.925f});
  CHECK(getValue(prefs, "Renderer/Show axes") == QJsonValue{false});
  CHECK(
    static_cast<float>(getValue(prefs, "Renderer/Grid/Alpha").toDouble())
    == vm::approx{0.22f});
  CHECK(
    getValue(prefs, "Renderer/Colors/Edges")
    == QJsonValue{"0.921569 0.666667 0.45098 1"});
  CHECK(
    getValue(prefs, "Renderer/Colors/Background")
    == QJsonValue{"0.321569 0.0470588 0.141176 1"});
  CHECK(
    getValue(prefs, "Rendere/Grid/Color2D")
    == QJsonValue{"0.290196 0.643137 0.486275 1"});
  CHECK(getValue(prefs, "Views/Map view layout") == QJsonValue{2});
  CHECK(getValue(prefs, "Games/Quake/Path") == QJsonValue{"/home/ericwa/Quake Dev"});
  CHECK(getValue(prefs, "Games/Generic/Path") == QJsonValue{"/home/ericwa/foo=bar"});
  CHECK(
    getValue(prefs, "Games/Quake 3/Path") == QJsonValue{"/home/ericwa/Quake 3 Arena"});
  CHECK(getValue(prefs, "Menu/File/Export/Wavefront OBJ...") == QJsonValue{"Ctrl+Alt+W"});
  CHECK(
    getValue(prefs, "Menu/View/Grid/Set Grid Size 0.125") == QJsonValue{"Ctrl+Alt+2"});
  CHECK(getValue(prefs, "Filters/Tags/Detail/Toggle Visible") == QJsonValue{"Alt+D"});
  CHECK(getValue(prefs, "Tags/Detail/Enable") == QJsonValue{"D"});
  CHECK(getValue(prefs, "Tags/Detail/Disable") == QJsonValue{"Alt+Shift+D"});
  CHECK(getValue(prefs, "Entities/monster_hell_knight/Create") == QJsonValue{"H"});

  // We don't bother migrating these ones
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/x")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/y")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/w")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/h")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/Maximized")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/Iconized")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/decor_l")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/decor_r")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/decor_t")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/Window/MapFrame/decor_b")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(
      prefs, "Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(
      prefs,
      "Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(
      prefs, "Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(
    getValue(prefs, "Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio")
    == QJsonValue{QJsonValue::Undefined});
  CHECK(getValue(prefs, "RecentDocuments/0") == QJsonValue{QJsonValue::Undefined});
}

TEST_CASE("PreferencesTest.read")
{
  CHECK(parsePreferencesFromJson(QByteArray())
          .is_error_type<PreferenceErrors::JsonParseError>());
  CHECK(parsePreferencesFromJson(QByteArray("abc"))
          .is_error_type<PreferenceErrors::JsonParseError>());
  CHECK(parsePreferencesFromJson(QByteArray(R"({"foo": "bar",})"))
          .is_error_type<PreferenceErrors::JsonParseError>());

  // Valid JSON
  CHECK(parsePreferencesFromJson(QByteArray(R"({"foo": "bar"})")).is_success());
  CHECK(parsePreferencesFromJson(QByteArray("{}")).is_success());

  readPreferencesFromFile("fixture/test/preferences-v2.json")
    .transform(
      [](const std::map<std::filesystem::path, QJsonValue>& prefs) { testPrefs(prefs); })
    .transform_error([](const auto&) { FAIL_CHECK(); });
}

TEST_CASE("PreferencesTest.testWriteRead")
{
  const auto fromFile =
    readPreferencesFromFile("fixture/test/preferences-v2.json").value();

  const QByteArray serialized = writePreferencesToJson(fromFile);
  parsePreferencesFromJson(serialized)
    .transform([&](const std::map<std::filesystem::path, QJsonValue>& prefs) {
      CHECK(fromFile == prefs);
    })
    .transform_error([](const auto&) { FAIL_CHECK(); });
}

/**
 * Helper template so we don't need to use out parameters in the tests
 */
template <class Serializer, class PrimitiveType>
static std::optional<PrimitiveType> maybeDeserialize(const QJsonValue& string)
{
  const auto serializer = Serializer{};
  auto result = PrimitiveType{};
  return serializer.readFromJson(string, result) ? std::optional{result} : std::nullopt;
}

template <class Serializer, class PrimitiveType>
static QJsonValue serialize(const PrimitiveType& value)
{
  const auto serializer = Serializer{};
  return serializer.writeToJson(value);
}

template <class Serializer, class PrimitiveType>
static void testSerialize(const QJsonValue& str, const PrimitiveType& value)
{
  const auto testDeserializeOption = maybeDeserialize<Serializer, PrimitiveType>(str);
  const auto testSerialize = serialize<Serializer, PrimitiveType>(value);

  REQUIRE(testDeserializeOption.has_value());

  CHECK(testDeserializeOption.value() == value);
  CHECK(testSerialize == str);
}

TEST_CASE("PreferencesTest.serializeBool")
{
  CHECK_FALSE((maybeDeserialize<PreferenceSerializer, bool>(QJsonValue{""}).has_value()));
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializer, bool>(QJsonValue{"0"}).has_value()));

  testSerialize<PreferenceSerializer, bool>(QJsonValue{false}, false);
  testSerialize<PreferenceSerializer, bool>(QJsonValue{true}, true);
}

TEST_CASE("PreferencesTest.serializefloat")
{
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializer, float>(QJsonValue{"1.25"}).has_value()));

  testSerialize<PreferenceSerializer, float>(QJsonValue{1.25}, 1.25f);
}

TEST_CASE("PreferencesTest.serializeint")
{
  CHECK_FALSE((maybeDeserialize<PreferenceSerializer, int>(QJsonValue{"0"}).has_value()));
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializer, int>(QJsonValue{"-1"}).has_value()));

  testSerialize<PreferenceSerializer, int>(QJsonValue{0}, 0);
  testSerialize<PreferenceSerializer, int>(QJsonValue{-1}, -1);
  testSerialize<PreferenceSerializer, int>(QJsonValue{1000}, 1000);
}

TEST_CASE("PreferencesTest.serializeKeyboardShortcut")
{
  testSerialize<PreferenceSerializer, QKeySequence>(
    QJsonValue{"Alt+Shift+W"}, QKeySequence::fromString("Alt+Shift+W"));
  testSerialize<PreferenceSerializer, QKeySequence>(
    QJsonValue{"Meta+W"},
    QKeySequence::fromString("Meta+W")); // "Meta" in Qt = Control in macOS
}

TEST_CASE("PreferencesTest.testWxViewShortcutsAndMenuShortcutsRecognized")
{
  // All map view shortcuts, and all binadable menu items before the Qt port
  const auto preferenceKeys = std::vector<std::string>{
    "Controls/Map view/Create brush",
    "Controls/Map view/Toggle clip side",
    "Controls/Map view/Perform clip",
    // The Qt port dropped these - now they're merged with the "Move objects" actions
    //            "Controls/Map view/Move vertices up; Move vertices forward",
    //            "Controls/Map view/Move vertices down; Move vertices backward",
    //            "Controls/Map view/Move vertices left",
    //            "Controls/Map view/Move vertices right",
    //            "Controls/Map view/Move vertices backward; Move vertices up",
    //            "Controls/Map view/Move vertices forward; Move vertices down",
    //            "Controls/Map view/Move rotation center up; Move rotation center
    //            forward", "Controls/Map view/Move rotation center down; Move rotation
    //            center backward", "Controls/Map view/Move rotation center left",
    //            "Controls/Map view/Move rotation center right",
    //            "Controls/Map view/Move rotation center backward; Move rotation center
    //            up", "Controls/Map view/Move rotation center forward; Move rotation
    //            center down",
    "Controls/Map view/Move objects up; Move objects forward",
    "Controls/Map view/Move objects down; Move objects backward",
    "Controls/Map view/Move objects left",
    "Controls/Map view/Move objects right",
    "Controls/Map view/Move objects backward; Move objects up",
    "Controls/Map view/Move objects forward; Move objects down",
    "Controls/Map view/Roll objects clockwise",
    "Controls/Map view/Roll objects counter-clockwise",
    "Controls/Map view/Yaw objects clockwise",
    "Controls/Map view/Yaw objects counter-clockwise",
    "Controls/Map view/Pitch objects clockwise",
    "Controls/Map view/Pitch objects counter-clockwise",
    "Controls/Map view/Flip objects horizontally",
    "Controls/Map view/Flip objects vertically",
    "Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward",
    "Controls/Map view/Duplicate and move objects down; Duplicate and move objects "
    "backward",
    "Controls/Map view/Duplicate and move objects left",
    "Controls/Map view/Duplicate and move objects right",
    "Controls/Map view/Duplicate and move objects backward; Duplicate and move objects "
    "up",
    "Controls/Map view/Duplicate and move objects forward; Duplicate and move objects "
    "down",
    "Controls/Map view/Move textures up",
    "Controls/Map view/Move textures up (fine)",
    "Controls/Map view/Move textures up (coarse)",
    "Controls/Map view/Move textures down",
    "Controls/Map view/Move textures down (fine)",
    "Controls/Map view/Move textures down (coarse)",
    "Controls/Map view/Move textures left",
    "Controls/Map view/Move textures left (fine)",
    "Controls/Map view/Move textures left (coarse)",
    "Controls/Map view/Move textures right",
    "Controls/Map view/Move textures right (fine)",
    "Controls/Map view/Move textures right (coarse)",
    "Controls/Map view/Rotate textures clockwise",
    "Controls/Map view/Rotate textures clockwise (fine)",
    "Controls/Map view/Rotate textures clockwise (coarse)",
    "Controls/Map view/Rotate textures counter-clockwise",
    "Controls/Map view/Rotate textures counter-clockwise (fine)",
    "Controls/Map view/Rotate textures counter-clockwise (coarse)",
    "Controls/Map view/Cycle map view",
    "Controls/Map view/Reset camera zoom",
    "Controls/Map view/Cancel",
    "Controls/Map view/Deactivate current tool",
    "Controls/Map view/Make structural",
    "Controls/Map view/View Filter > Toggle show entity classnames",
    "Controls/Map view/View Filter > Toggle show group bounds",
    "Controls/Map view/View Filter > Toggle show brush entity bounds",
    "Controls/Map view/View Filter > Toggle show point entity bounds",
    "Controls/Map view/View Filter > Toggle show point entities",
    "Controls/Map view/View Filter > Toggle show point entity models",
    "Controls/Map view/View Filter > Toggle show brushes",
    "Controls/Map view/View Filter > Show textures",
    "Controls/Map view/View Filter > Hide textures",
    "Controls/Map view/View Filter > Hide faces",
    "Controls/Map view/View Filter > Shade faces",
    "Controls/Map view/View Filter > Use fog",
    "Controls/Map view/View Filter > Show edges",
    "Controls/Map view/View Filter > Show all entity links",
    "Controls/Map view/View Filter > Show transitively selected entity links",
    "Controls/Map view/View Filter > Show directly selected entity links",
    "Controls/Map view/View Filter > Hide entity links",
    "Menu/File/Export/Wavefront OBJ...",
    "Menu/File/Load Point File...",
    "Menu/File/Reload Point File",
    "Menu/File/Unload Point File",
    "Menu/File/Load Portal File...",
    "Menu/File/Reload Portal File",
    "Menu/File/Unload Portal File",
    "Menu/File/Reload Texture Collections",
    "Menu/File/Reload Entity Definitions",
    "Menu/Edit/Repeat",
    "Menu/Edit/Paste at Original Position",
    "Menu/Edit/Clear Repeatable Commands",
    "Menu/Edit/Duplicate",
    "Menu/Edit/Delete",
    "Menu/Edit/Select All",
    "Menu/Edit/Select Siblings",
    "Menu/Edit/Select Touching",
    "Menu/Edit/Select Inside",
    "Menu/Edit/Select Tall",
    "Menu/Edit/Select by Line Number",
    "Menu/Edit/Select None",
    "Menu/Edit/Group",
    "Menu/Edit/Ungroup",
    "Menu/Edit/Tools/Brush Tool",
    "Menu/Edit/Tools/Clip Tool",
    "Menu/Edit/Tools/Rotate Tool",
    "Menu/Edit/Tools/Scale Tool",
    "Menu/Edit/Tools/Shear Tool",
    "Menu/Edit/Tools/Vertex Tool",
    "Menu/Edit/Tools/Edge Tool",
    "Menu/Edit/Tools/Face Tool",
    "Menu/Edit/CSG/Convex Merge",
    "Menu/Edit/CSG/Subtract",
    "Menu/Edit/CSG/Hollow",
    "Menu/Edit/CSG/Intersect",
    "Menu/Edit/Snap Vertices to Integer",
    "Menu/Edit/Snap Vertices to Grid",
    "Menu/Edit/Texture Lock",
    "Menu/Edit/UV Lock",
    "Menu/Edit/Replace Texture...",
    "Menu/View/Grid/Show Grid",
    "Menu/View/Grid/Snap to Grid",
    "Menu/View/Grid/Increase Grid Size",
    "Menu/View/Grid/Decrease Grid Size",
    "Menu/View/Grid/Set Grid Size 0.125",
    "Menu/View/Grid/Set Grid Size 0.25",
    "Menu/View/Grid/Set Grid Size 0.5",
    "Menu/View/Grid/Set Grid Size 1",
    "Menu/View/Grid/Set Grid Size 2",
    "Menu/View/Grid/Set Grid Size 4",
    "Menu/View/Grid/Set Grid Size 8",
    "Menu/View/Grid/Set Grid Size 16",
    "Menu/View/Grid/Set Grid Size 32",
    "Menu/View/Grid/Set Grid Size 64",
    "Menu/View/Grid/Set Grid Size 128",
    "Menu/View/Grid/Set Grid Size 256",
    "Menu/View/Camera/Move to Next Point",
    "Menu/View/Camera/Move to Previous Point",
    "Menu/View/Camera/Focus on Selection",
    "Menu/View/Camera/Move Camera to...",
    "Menu/View/Isolate",
    "Menu/View/Hide",
    "Menu/View/Show All",
    "Menu/View/Switch to Map Inspector",
    "Menu/View/Switch to Entity Inspector",
    "Menu/View/Switch to Face Inspector",
    "Menu/View/Toggle Info Panel",
    "Menu/View/Toggle Inspector",
    "Menu/View/Maximize Current View",
    "Menu/Run/Compile...",
    "Menu/Run/Launch...",
  };

  auto& actionsMap = View::ActionManager::instance().actionsMap();
  for (const auto& preferenceKey : preferenceKeys)
  {
    CAPTURE(preferenceKey);

    const auto preferencePath = preferenceKey;
    CHECK(actionsMap.find(preferencePath) != actionsMap.end());
  }
}

TEST_CASE("PreferencesTest.testWxEntityShortcuts")
{
  auto hellKnight = Assets::PointEntityDefinition{
    "monster_hell_knight", {0, 0, 0}, {}, "", {}, Assets::ModelDefinition{}};
  const auto defs = std::vector<Assets::EntityDefinition*>{&hellKnight};

  const auto actions =
    View::ActionManager::instance().createEntityDefinitionActions(defs);
  const auto actualPrefPaths = kdl::vec_transform(
    actions, [](const auto& action) { return action->preferencePath(); });

  // example keys from 2019.6 for "monster_hell_knight" entity
  const auto preferenceKeys = std::vector<std::string>{
    "Entities/monster_hell_knight/Create",
    "Entities/monster_hell_knight/Toggle" // new in 2020.1
  };

  for (const auto& preferenceKey : preferenceKeys)
  {
    CAPTURE(preferenceKey);
    CHECK(kdl::vec_contains(actualPrefPaths, preferenceKey));
  }
}

TEST_CASE("PreferencesTest.testWxTagShortcuts")
{
  const auto tags = std::vector<Model::SmartTag>{Model::SmartTag{
    "Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27)}};
  const auto actions = View::ActionManager::instance().createTagActions(tags);
  const auto actualPrefPaths = kdl::vec_transform(
    actions, [](const auto& action) { return action->preferencePath(); });

  // example keys from 2019.6 for "Detail" tag
  const auto preferenceKeys = std::vector<std::string>{
    "Filters/Tags/Detail/Toggle Visible",
    "Tags/Detail/Disable",
    "Tags/Detail/Enable",
  };

  for (const auto& preferenceKey : preferenceKeys)
  {
    CAPTURE(preferenceKey);
    CHECK(kdl::vec_contains(actualPrefPaths, preferenceKey));
  }
}
} // namespace TrenchBroom
