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

#include <iostream>
#include <optional>
#include <string>

#include <QString>
#include <QTextStream>

#include "Catch2.h"

namespace TrenchBroom
{
static QJsonValue getValue(const std::map<IO::Path, QJsonValue>& map, const IO::Path& key)
{
  auto it = map.find(key);
  if (it == map.end())
  {
    return QJsonValue(QJsonValue::Undefined);
  }
  return it->second;
}

TEST_CASE("PreferencesTest.migrateLocalV1Settings")
{
  const std::map<IO::Path, QJsonValue> reg = readV1Settings();

  [[maybe_unused]] const auto migrated = migrateV1ToV2(reg);

  // Can't really test anything because we can't assume the test system
  // has any settings on it.
}

TEST_CASE("PreferencesTest.parseV1")
{
  const std::map<IO::Path, QJsonValue> parsed =
    getINISettingsV1("fixture/test/preferences-v1.ini");

  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Field of vision"))
    == QJsonValue("108.000000"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move down")) == QJsonValue("82:0:0:0"));
  CHECK(getValue(parsed, IO::Path("Controls/Camera/Move up")) == QJsonValue("87:0:0:0"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move right")) == QJsonValue("70:0:0:0"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move left")) == QJsonValue("83:0:0:0"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move backward"))
    == QJsonValue("68:0:0:0"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move forward")) == QJsonValue("69:0:0:0"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Fly move speed"))
    == QJsonValue("0.425781"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move camera in cursor dir"))
    == QJsonValue("1"));
  CHECK(getValue(parsed, IO::Path("Controls/Camera/Use alt to move")) == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Move speed")) == QJsonValue("0.350000"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Invert mouse wheel")) == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Invert vertical pan")) == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Invert horizontal pan"))
    == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Pan speed")) == QJsonValue("0.550000"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Invert vertical look"))
    == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Invert horizontal look"))
    == QJsonValue("1"));
  CHECK(
    getValue(parsed, IO::Path("Controls/Camera/Look speed")) == QJsonValue("0.440000"));
  CHECK(
    getValue(parsed, IO::Path("Texture Browser/Icon size")) == QJsonValue("1.500000"));
  CHECK(getValue(parsed, IO::Path("Renderer/Font size")) == QJsonValue("14"));
  CHECK(
    getValue(parsed, IO::Path("Renderer/Texture mode mag filter")) == QJsonValue("9729"));
  CHECK(
    getValue(parsed, IO::Path("Renderer/Texture mode min filter")) == QJsonValue("9987"));
  CHECK(getValue(parsed, IO::Path("Renderer/Brightness")) == QJsonValue("0.925000"));
  CHECK(getValue(parsed, IO::Path("Renderer/Show axes")) == QJsonValue("0"));
  CHECK(getValue(parsed, IO::Path("Renderer/Grid/Alpha")) == QJsonValue("0.220000"));
  CHECK(
    getValue(parsed, IO::Path("Renderer/Colors/Edges"))
    == QJsonValue("0.921569 0.666667 0.45098 1"));
  CHECK(
    getValue(parsed, IO::Path("Renderer/Colors/Background"))
    == QJsonValue("0.321569 0.0470588 0.141176 1"));
  CHECK(
    getValue(parsed, IO::Path("Rendere/Grid/Color2D"))
    == QJsonValue("0.290196 0.643137 0.486275 1"));
  CHECK(getValue(parsed, IO::Path("Views/Map view layout")) == QJsonValue("2"));
  CHECK(
    getValue(parsed, IO::Path("Games/Quake/Path"))
    == QJsonValue("/home/ericwa/Quake Dev"));
  CHECK(
    getValue(parsed, IO::Path("Games/Generic/Path"))
    == QJsonValue("/home/ericwa/foo=bar"));
  CHECK(
    getValue(parsed, IO::Path("Games/Quake 3/Path"))
    == QJsonValue("/home/ericwa/Quake 3 Arena"));
  CHECK(
    getValue(parsed, IO::Path("Menu/File/Export/Wavefront OBJ..."))
    == QJsonValue("87:308:307:0"));
  CHECK(
    getValue(parsed, IO::Path("Menu/View/Grid/Set Grid Size 0.125"))
    == QJsonValue("50:308:307:0"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/x"))
    == QJsonValue("859"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/y"))
    == QJsonValue("473"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/w"))
    == QJsonValue("1024"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/h"))
    == QJsonValue("768"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Maximized"))
    == QJsonValue("0"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/Iconized"))
    == QJsonValue("0"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_l"))
    == QJsonValue("0"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_r"))
    == QJsonValue("0"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_t"))
    == QJsonValue("37"));
  CHECK(
    getValue(parsed, IO::Path("Persistent_Options/Window/MapFrame/decor_b"))
    == QJsonValue("0"));
  CHECK(
    getValue(
      parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio"))
    == QJsonValue("6533"));
  CHECK(
    getValue(
      parsed, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio"))
    == QJsonValue("8306"));
  CHECK(
    getValue(
      parsed,
      IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio"))
    == QJsonValue("4857"));
  CHECK(
    getValue(
      parsed,
      IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio"))
    == QJsonValue("4850"));
  CHECK(
    getValue(
      parsed,
      IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio"))
    == QJsonValue("2742"));
  CHECK(
    getValue(
      parsed,
      IO::Path(
        "Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio"))
    == QJsonValue("3333"));
  CHECK(
    getValue(
      parsed,
      IO::Path(
        "Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio"))
    == QJsonValue("-10000"));
  CHECK(
    getValue(
      parsed,
      IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio"))
    == QJsonValue("3656"));
  CHECK(
    getValue(parsed, IO::Path("RecentDocuments/0"))
    == QJsonValue("/home/ericwa/unnamed.map"));
  CHECK(
    getValue(parsed, IO::Path("Filters/Tags/Detail/Toggle Visible"))
    == QJsonValue("68:307:0:0"));
  CHECK(getValue(parsed, IO::Path("Tags/Detail/Enable")) == QJsonValue("68:0:0:0"));
  CHECK(getValue(parsed, IO::Path("Tags/Detail/Disable")) == QJsonValue("68:307:306:0"));
  CHECK(
    getValue(parsed, IO::Path("Entities/monster_hell_knight/Create"))
    == QJsonValue("72:0:0:0"));
}

static void testV2Prefs(const std::map<IO::Path, QJsonValue>& v2)
{
  CHECK(getValue(v2, IO::Path("Controls/Camera/Field of vision")) == QJsonValue(108));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move down")) == QJsonValue("R"));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move up")) == QJsonValue("W"));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move right")) == QJsonValue("F"));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move left")) == QJsonValue("S"));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move backward")) == QJsonValue("D"));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Move forward")) == QJsonValue("E"));
  CHECK(
    static_cast<float>(
      getValue(v2, IO::Path("Controls/Camera/Fly move speed")).toDouble())
    == vm::approx(0.425781f));
  CHECK(
    getValue(v2, IO::Path("Controls/Camera/Move camera in cursor dir"))
    == QJsonValue(true));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Use alt to move")) == QJsonValue(true));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Move speed")).toDouble())
    == vm::approx(0.35f));
  CHECK(getValue(v2, IO::Path("Controls/Camera/Invert mouse wheel")) == QJsonValue(true));
  CHECK(
    getValue(v2, IO::Path("Controls/Camera/Invert vertical pan")) == QJsonValue(true));
  CHECK(
    getValue(v2, IO::Path("Controls/Camera/Invert horizontal pan")) == QJsonValue(true));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Pan speed")).toDouble())
    == vm::approx(0.55f));
  CHECK(
    getValue(v2, IO::Path("Controls/Camera/Invert vertical look")) == QJsonValue(true));
  CHECK(
    getValue(v2, IO::Path("Controls/Camera/Invert horizontal look")) == QJsonValue(true));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Controls/Camera/Look speed")).toDouble())
    == vm::approx(0.44f));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Texture Browser/Icon size")).toDouble())
    == vm::approx(1.5f));
  CHECK(getValue(v2, IO::Path("Renderer/Font size")) == QJsonValue(14));
  CHECK(getValue(v2, IO::Path("Renderer/Texture mode mag filter")) == QJsonValue(9729));
  CHECK(getValue(v2, IO::Path("Renderer/Texture mode min filter")) == QJsonValue(9987));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Renderer/Brightness")).toDouble())
    == vm::approx(0.925f));
  CHECK(getValue(v2, IO::Path("Renderer/Show axes")) == QJsonValue(false));
  CHECK(
    static_cast<float>(getValue(v2, IO::Path("Renderer/Grid/Alpha")).toDouble())
    == vm::approx(0.22f));
  CHECK(
    getValue(v2, IO::Path("Renderer/Colors/Edges"))
    == QJsonValue("0.921569 0.666667 0.45098 1"));
  CHECK(
    getValue(v2, IO::Path("Renderer/Colors/Background"))
    == QJsonValue("0.321569 0.0470588 0.141176 1"));
  CHECK(
    getValue(v2, IO::Path("Rendere/Grid/Color2D"))
    == QJsonValue("0.290196 0.643137 0.486275 1"));
  CHECK(getValue(v2, IO::Path("Views/Map view layout")) == QJsonValue(2));
  CHECK(
    getValue(v2, IO::Path("Games/Quake/Path")) == QJsonValue("/home/ericwa/Quake Dev"));
  CHECK(
    getValue(v2, IO::Path("Games/Generic/Path")) == QJsonValue("/home/ericwa/foo=bar"));
  CHECK(
    getValue(v2, IO::Path("Games/Quake 3/Path"))
    == QJsonValue("/home/ericwa/Quake 3 Arena"));
  CHECK(
    getValue(v2, IO::Path("Menu/File/Export/Wavefront OBJ..."))
    == QJsonValue("Ctrl+Alt+W"));
  CHECK(
    getValue(v2, IO::Path("Menu/View/Grid/Set Grid Size 0.125"))
    == QJsonValue("Ctrl+Alt+2"));
  CHECK(
    getValue(v2, IO::Path("Filters/Tags/Detail/Toggle Visible")) == QJsonValue("Alt+D"));
  CHECK(getValue(v2, IO::Path("Tags/Detail/Enable")) == QJsonValue("D"));
  CHECK(getValue(v2, IO::Path("Tags/Detail/Disable")) == QJsonValue("Alt+Shift+D"));
  CHECK(getValue(v2, IO::Path("Entities/monster_hell_knight/Create")) == QJsonValue("H"));

  // We don't bother migrating these ones
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/x"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/y"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/w"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/h"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Maximized"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/Iconized"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_l"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_r"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_t"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(v2, IO::Path("Persistent_Options/Window/MapFrame/decor_b"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameHSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2, IO::Path("Persistent_Options/SplitterWindow2/MapFrameVSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewHSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2, IO::Path("Persistent_Options/SplitterWindow2/3PaneMapViewVSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2,
      IO::Path("Persistent_Options/SplitterWindow2/EntityInspectorSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2,
      IO::Path(
        "Persistent_Options/SplitterWindow2/EntityAttributeEditorSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2,
      IO::Path(
        "Persistent_Options/SplitterWindow2/EntityDocumentationSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(
    getValue(
      v2, IO::Path("Persistent_Options/SplitterWindow2/FaceInspectorSplitter/SplitRatio"))
    == QJsonValue(QJsonValue::Undefined));
  CHECK(getValue(v2, IO::Path("RecentDocuments/0")) == QJsonValue(QJsonValue::Undefined));
}

TEST_CASE("PreferencesTest.migrateV1")
{
  const std::map<IO::Path, QJsonValue> v1 =
    getINISettingsV1("fixture/test/preferences-v1.ini");
  const std::map<IO::Path, QJsonValue> v2 = migrateV1ToV2(v1);

  testV2Prefs(v2);

  // CHECK(writeV2SettingsToPath("C:\\Users\\Eric\\Desktop\\Preferences.json", v2));
}

TEST_CASE("PreferencesTest.readV2")
{
  // Invalid JSON -> parse error -> parseV2SettingsFromJSON() is expected to return
  // nullopt
  CHECK(parseV2SettingsFromJSON(QByteArray())
          .is_error_type<PreferenceErrors::JsonParseError>());
  CHECK(parseV2SettingsFromJSON(QByteArray("abc"))
          .is_error_type<PreferenceErrors::JsonParseError>());
  CHECK(parseV2SettingsFromJSON(QByteArray(R"({"foo": "bar",})"))
          .is_error_type<PreferenceErrors::JsonParseError>());

  // Valid JSON
  CHECK(parseV2SettingsFromJSON(QByteArray(R"({"foo": "bar"})")).is_success());
  CHECK(parseV2SettingsFromJSON(QByteArray("{}")).is_success());

  readV2SettingsFromPath("fixture/test/preferences-v2.json")
    .transform([](const std::map<IO::Path, QJsonValue>& prefs) { testV2Prefs(prefs); })
    .or_else([](const auto&) { FAIL_CHECK(); });
}

TEST_CASE("PreferencesTest.testWriteReadV2")
{
  const std::map<IO::Path, QJsonValue> v1 =
    getINISettingsV1("fixture/test/preferences-v1.ini");
  const std::map<IO::Path, QJsonValue> v2 = migrateV1ToV2(v1);

  const QByteArray v2Serialized = writeV2SettingsToJSON(v2);
  parseV2SettingsFromJSON(v2Serialized)
    .transform([&](const std::map<IO::Path, QJsonValue>& prefs) { CHECK(v2 == prefs); })
    .or_else([](const auto&) { FAIL_CHECK(); });
}

/**
 * Helper template so we don't need to use out parameters in the tests
 */
template <class Serializer, class PrimitiveType>
static std::optional<PrimitiveType> maybeDeserialize(const QJsonValue& string)
{
  const Serializer s;
  PrimitiveType result;
  if (s.readFromJSON(string, result))
  {
    return {result};
  }
  return std::nullopt;
}

template <class Serializer, class PrimitiveType>
static QJsonValue serialize(const PrimitiveType& value)
{
  const Serializer s;
  return s.writeToJSON(value);
}

template <class Serializer, class PrimitiveType>
static void testSerialize(const QJsonValue& str, const PrimitiveType& value)
{
  const auto testDeserializeOption = maybeDeserialize<Serializer, PrimitiveType>(str);
  const QJsonValue testSerialize = serialize<Serializer, PrimitiveType>(value);

  REQUIRE(testDeserializeOption.has_value());

  CHECK(testDeserializeOption.value() == value);
  CHECK(testSerialize == str);
}

TEST_CASE("PreferencesTest.serializeV1Bool")
{
  CHECK_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("").has_value()));
  CHECK_FALSE((maybeDeserialize<PreferenceSerializerV1, bool>("-1").has_value()));

  testSerialize<PreferenceSerializerV1, bool>(QJsonValue("0"), false);
  testSerialize<PreferenceSerializerV1, bool>(QJsonValue("1"), true);
}

TEST_CASE("PreferencesTest.serializeV1Color")
{
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV1, Color>(QJsonValue("0.921569 0.666667"))
       .has_value())); // must give 3 or 4 components

  testSerialize<PreferenceSerializerV1, Color>(
    QJsonValue("0.921569 0.666667 0.45098 0.5"),
    Color(0.921569f, 0.666667f, 0.45098f, 0.5f));
}

TEST_CASE("PreferencesTest.serializeV1float")
{
  testSerialize<PreferenceSerializerV1, float>(QJsonValue("0.921569"), 0.921569f);
}

TEST_CASE("PreferencesTest.serializeV1int")
{
  testSerialize<PreferenceSerializerV1, int>(QJsonValue("0"), 0);
  testSerialize<PreferenceSerializerV1, int>(QJsonValue("-1"), -1);
  testSerialize<PreferenceSerializerV1, int>(QJsonValue("1000"), 1000);
}

TEST_CASE("PreferencesTest.serializeV1Path")
{
#ifdef _WIN32
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("c:\\foo\\bar"), IO::Path("c:\\foo\\bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("c:\\foo\\bar"), IO::Path("c:/foo/bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("\\home\\foo\\bar"), IO::Path("/home/foo/bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("\\home\\foo\\bar"), IO::Path("\\home\\foo\\bar"));
#else
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("c:/foo/bar"), IO::Path("c:\\foo\\bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("c:/foo/bar"), IO::Path("c:/foo/bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("/home/foo/bar"), IO::Path("/home/foo/bar"));
  testSerialize<PreferenceSerializerV1, IO::Path>(
    QJsonValue("home/foo/bar"), IO::Path("\\home\\foo\\bar"));
#endif
  testSerialize<PreferenceSerializerV1, IO::Path>(QJsonValue(""), IO::Path());
}

TEST_CASE("PreferencesTest.serializeV1KeyboardShortcut")
{
  // These come from wxWidgets TrenchBroom 2019.6, on Windows
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("87:307:306:0"), QKeySequence::fromString("Alt+Shift+W"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("87:307:0:0"), QKeySequence::fromString("Alt+W"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("87:308:307:0"), QKeySequence::fromString("Ctrl+Alt+W"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("87:306:0:0"), QKeySequence::fromString("Shift+W"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("77:308:0:0"), QKeySequence::fromString("Ctrl+M"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("65:308:307:306"), QKeySequence::fromString("Ctrl+Alt+Shift+A"));
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("348:306:0:0"), QKeySequence::fromString("Shift+F9"));

  // From macOS
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("80:308:0:0"),
    QKeySequence::fromString("Ctrl+P")); // "Ctrl" in Qt = Command in macOS
  testSerialize<PreferenceSerializerV1, QKeySequence>(
    QJsonValue("80:307:0:0"),
    QKeySequence::fromString("Alt+P")); // "Alt" in Qt = Alt in macOS
}

TEST_CASE("PreferencesTest.serializeV2Bool")
{
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV2, bool>(QJsonValue("")).has_value()));
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV2, bool>(QJsonValue("0")).has_value()));

  testSerialize<PreferenceSerializerV2, bool>(QJsonValue(false), false);
  testSerialize<PreferenceSerializerV2, bool>(QJsonValue(true), true);
}

TEST_CASE("PreferencesTest.serializeV2float")
{
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV2, float>(QJsonValue("1.25")).has_value()));

  testSerialize<PreferenceSerializerV2, float>(QJsonValue(1.25), 1.25f);
}

TEST_CASE("PreferencesTest.serializeV2int")
{
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV2, int>(QJsonValue("0")).has_value()));
  CHECK_FALSE(
    (maybeDeserialize<PreferenceSerializerV2, int>(QJsonValue("-1")).has_value()));

  testSerialize<PreferenceSerializerV2, int>(QJsonValue(0), 0);
  testSerialize<PreferenceSerializerV2, int>(QJsonValue(-1), -1);
  testSerialize<PreferenceSerializerV2, int>(QJsonValue(1000), 1000);
}

TEST_CASE("PreferencesTest.serializeV2KeyboardShortcut")
{
  testSerialize<PreferenceSerializerV2, QKeySequence>(
    QJsonValue("Alt+Shift+W"), QKeySequence::fromString("Alt+Shift+W"));
  testSerialize<PreferenceSerializerV2, QKeySequence>(
    QJsonValue("Meta+W"),
    QKeySequence::fromString("Meta+W")); // "Meta" in Qt = Control in macOS
}

TEST_CASE("PreferencesTest.testWxViewShortcutsAndMenuShortcutsRecognized")
{
  // All map view shortcuts, and all binadable menu items before the Qt port
  const std::vector<std::string> preferenceKeys{
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
    "Menu/Edit/Tools/Primitive Brush Tool",
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
  for (const std::string& preferenceKey : preferenceKeys)
  {
    const auto preferencePath = IO::Path(preferenceKey);
    const bool found = (actionsMap.find(preferencePath) != actionsMap.end());
    CHECK(found);

    if (!found)
    {
      std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
    }
  }
}

TEST_CASE("PreferencesTest.testWxEntityShortcuts")
{
  auto hellKnight = Assets::PointEntityDefinition(
    "monster_hell_knight",
    Color(0, 0, 0),
    vm::bbox3(),
    "",
    {},
    Assets::ModelDefinition());
  const auto defs = std::vector<Assets::EntityDefinition*>{&hellKnight};

  const std::vector<std::unique_ptr<View::Action>> actions =
    View::ActionManager::instance().createEntityDefinitionActions(defs);
  const std::vector<IO::Path> actualPrefPaths = kdl::vec_transform(
    actions, [](const auto& action) { return IO::Path(action->preferencePath()); });

  // example keys from 2019.6 for "monster_hell_knight" entity
  const std::vector<std::string> preferenceKeys{
    "Entities/monster_hell_knight/Create",
    "Entities/monster_hell_knight/Toggle" // new in 2020.1
  };

  for (const std::string& preferenceKey : preferenceKeys)
  {
    const bool found = kdl::vec_contains(actualPrefPaths, IO::Path(preferenceKey));
    CHECK(found);

    if (!found)
    {
      std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
    }
  }
}

TEST_CASE("PreferencesTest.testWxTagShortcuts")
{
  const auto tags = std::vector<Model::SmartTag>{Model::SmartTag(
    "Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27))};
  const std::vector<std::unique_ptr<View::Action>> actions =
    View::ActionManager::instance().createTagActions(tags);
  const std::vector<IO::Path> actualPrefPaths = kdl::vec_transform(
    actions, [](const auto& action) { return IO::Path(action->preferencePath()); });

  // example keys from 2019.6 for "Detail" tag
  const std::vector<std::string> preferenceKeys{
    "Filters/Tags/Detail/Toggle Visible",
    "Tags/Detail/Disable",
    "Tags/Detail/Enable",
  };

  for (const std::string& preferenceKey : preferenceKeys)
  {
    const bool found = kdl::vec_contains(actualPrefPaths, IO::Path(preferenceKey));
    CHECK(found);

    if (!found)
    {
      std::cerr << "Couldn't find key: '" << preferenceKey << "'\n";
    }
  }
}
} // namespace TrenchBroom
