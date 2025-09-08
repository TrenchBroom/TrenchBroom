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

#include <QKeySequence>
#include <QLockFile>
#include <QString>
#include <QTextStream>

#include "PreferenceManager.h"
#include "io/PathQt.h"

#include <kdl/path_utils.h>

#include "vm/approx.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>


inline std::ostream& operator<<(std::ostream& lhs, const QJsonValue& rhs)
{
  lhs << rhs.toString().toStdString();
  return lhs;
}

namespace tb
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

void testPrefs(const std::map<std::filesystem::path, QJsonValue>& prefs)
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
  CHECK(getValue(prefs, "render/Font size") == QJsonValue{14});
  CHECK(getValue(prefs, "render/Texture mode mag filter") == QJsonValue{9729});
  CHECK(getValue(prefs, "render/Texture mode min filter") == QJsonValue{9987});
  CHECK(
    static_cast<float>(getValue(prefs, "render/Brightness").toDouble())
    == vm::approx{0.925f});
  CHECK(getValue(prefs, "render/Show axes") == QJsonValue{false});
  CHECK(
    static_cast<float>(getValue(prefs, "render/Grid/Alpha").toDouble())
    == vm::approx{0.22f});
  CHECK(
    getValue(prefs, "render/Colors/Edges") == QJsonValue{"0.921569 0.666667 0.45098 1"});
  CHECK(
    getValue(prefs, "render/Colors/Background")
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

/**
 * Helper template so we don't need to use out parameters in the tests
 */
template <class Serializer, class PrimitiveType>
std::optional<PrimitiveType> maybeDeserialize(const QJsonValue& string)
{
  const auto serializer = Serializer{};
  auto result = PrimitiveType{};
  return serializer.readFromJson(string, result) ? std::optional{result} : std::nullopt;
}

template <class Serializer, class PrimitiveType>
QJsonValue serialize(const PrimitiveType& value)
{
  const auto serializer = Serializer{};
  return serializer.writeToJson(value);
}

template <class Serializer, class PrimitiveType>
void testSerialize(const QJsonValue& str, const PrimitiveType& value)
{
  const auto testDeserializeOption = maybeDeserialize<Serializer, PrimitiveType>(str);
  const auto testSerialize = serialize<Serializer, PrimitiveType>(value);

  REQUIRE(testDeserializeOption != std::nullopt);

  CHECK(*testDeserializeOption == value);
  CHECK(testSerialize == str);
}

} // namespace

TEST_CASE("Preferences")
{
  SECTION("readPreferencesFromFile")
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
      | kdl::transform([](const std::map<std::filesystem::path, QJsonValue>& prefs) {
          testPrefs(prefs);
        })
      | kdl::transform_error([](const auto&) { FAIL_CHECK(); });
  }

  SECTION("PreferencesTest.testWriteRead")
  {
    const auto fromFile =
      readPreferencesFromFile("fixture/test/preferences-v2.json") | kdl::value();

    const QByteArray serialized = writePreferencesToJson(fromFile);
    parsePreferencesFromJson(serialized)
      | kdl::transform([&](const std::map<std::filesystem::path, QJsonValue>& prefs) {
          CHECK(fromFile == prefs);
        })
      | kdl::transform_error([](const auto&) { FAIL_CHECK(); });
  }

  SECTION("serializeBool")
  {
    CHECK_FALSE(
      (maybeDeserialize<PreferenceSerializer, bool>(QJsonValue{""}).has_value()));
    CHECK_FALSE(
      (maybeDeserialize<PreferenceSerializer, bool>(QJsonValue{"0"}).has_value()));

    testSerialize<PreferenceSerializer, bool>(QJsonValue{false}, false);
    testSerialize<PreferenceSerializer, bool>(QJsonValue{true}, true);
  }

  SECTION("serializefloat")
  {
    CHECK_FALSE(
      (maybeDeserialize<PreferenceSerializer, float>(QJsonValue{"1.25"}).has_value()));

    testSerialize<PreferenceSerializer, float>(QJsonValue{1.25}, 1.25f);
  }

  SECTION("serializeint")
  {
    CHECK_FALSE(
      (maybeDeserialize<PreferenceSerializer, int>(QJsonValue{"0"}).has_value()));
    CHECK_FALSE(
      (maybeDeserialize<PreferenceSerializer, int>(QJsonValue{"-1"}).has_value()));

    testSerialize<PreferenceSerializer, int>(QJsonValue{0}, 0);
    testSerialize<PreferenceSerializer, int>(QJsonValue{-1}, -1);
    testSerialize<PreferenceSerializer, int>(QJsonValue{1000}, 1000);
  }

  SECTION("serializeKeyboardShortcut")
  {
    testSerialize<PreferenceSerializer, QKeySequence>(
      QJsonValue{"Alt+Shift+W"}, QKeySequence::fromString("Alt+Shift+W"));
    testSerialize<PreferenceSerializer, QKeySequence>(
      QJsonValue{"Meta+W"},
      QKeySequence::fromString("Meta+W")); // "Meta" in Qt = Control in macOS
  }

  SECTION("Preference lock file")
  {
// ensure that a lock file can be created in a directory with non-ASCII characters
#ifdef _WIN32
    const auto lockFilePath =
      std::filesystem::path{LR"(fixture\test\Кристиян\ぁ\preferences-v2.json.lck)"};
#else
    const auto lockFilePath =
      std::filesystem::path{R"(fixture/test/Кристиян/ぁ/preferences-v2.json.lck)"};
#endif
    std::filesystem::create_directories(lockFilePath.parent_path());

    auto lockFile = QLockFile{io::pathAsQPath(lockFilePath)};
    CHECK(lockFile.lock());
  }
}

} // namespace tb
