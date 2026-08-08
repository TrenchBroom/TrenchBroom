/*
 Copyright (C) 2025 Kristian Duske

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

#include "TestEnvironment.h"
#include "fs/TestEnvironment.h"
#include "ui/CatchConfig.h"
#include "ui/QPathUtils.h"
#include "ui/QPreferenceStoreUtils.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{
namespace
{

const auto preferenceRoot = getFixtureRoot() / "test" / "ui" / "QPreferenceStoreUtils";
const auto preferenceFile = pathAsQString(preferenceRoot / "preferences.json");

} // namespace

TEST_CASE("QPreferenceStoreUtils")
{
  SECTION("PreferenceErrors::JsonParseError equality")
  {
    const auto lhs =
      PreferenceErrors::JsonParseError{QJsonParseError{5, QJsonParseError::GarbageAtEnd}};

    CHECK(
      lhs
      == PreferenceErrors::JsonParseError{
        QJsonParseError{5, QJsonParseError::GarbageAtEnd}});
    CHECK(
      !(lhs
        == PreferenceErrors::JsonParseError{
          QJsonParseError{7, QJsonParseError::GarbageAtEnd}}));
    CHECK(
      !(lhs
        == PreferenceErrors::JsonParseError{
          QJsonParseError{5, QJsonParseError::UnterminatedObject}}));
  }

  SECTION("parsePreferencesFromJson")
  {
    CHECK(parsePreferencesFromJson(QByteArray{})
            .is_error_type<PreferenceErrors::JsonParseError>());
    CHECK(parsePreferencesFromJson(QByteArray{"abc"})
            .is_error_type<PreferenceErrors::JsonParseError>());
    CHECK(parsePreferencesFromJson(QByteArray{R"({"foo": "bar",})"})
            .is_error_type<PreferenceErrors::JsonParseError>());

    // Valid JSON
    CHECK(parsePreferencesFromJson(QByteArray(R"({"foo": "bar"})")).is_success());
    CHECK(parsePreferencesFromJson(QByteArray("{}")).is_success());
  }

  SECTION("readPreferencesFromFile")
  {
    SECTION("returns the parsed values, or NoFilePresent, depending on the file's state")
    {
      const auto [path, expectedResult] = GENERATE(table<QString, ReadPreferencesResult>({
        {preferenceFile,
         ReadPreferencesResult{PreferenceValues{
           {"Prefs/Values/Integer value", QJsonValue{108}},
           {"Prefs/Values/Float value", QJsonValue{0.425781}},
           {"Prefs/Values/Bool value", QJsonValue{true}},
           {"Prefs/Values/String value", QJsonValue{"this and that"}},
           {"Prefs/Values/Color value", QJsonValue{"0.290196 0.643137 0.486275 1"}},
           {"Prefs/Paths/Equal sign", QJsonValue{"/home/ericwa/foo=bar"}},
           {"Prefs/Paths/With spaces", QJsonValue{"/home/ericwa/Quake 3 Arena"}},
           {"Prefs/Key sequences/Single key", QJsonValue{"W"}},
           {"Prefs/Key sequences/Multiple keys", QJsonValue{"Ctrl+Alt+W"}},
         }}},
        {pathAsQString(preferenceRoot / "does-not-exist.json"),
         PreferenceErrors::NoFilePresent{}},
        {pathAsQString(preferenceRoot / "does-not-exist" / "some-file.json"),
         PreferenceErrors::NoFilePresent{}},
      }));

      CAPTURE(path);

      CHECK(readPreferencesFromFile(path) == expectedResult);
    }

    SECTION("returns FileAccessError when the containing directory cannot be created")
    {
      // A regular file in the way of a directory component makes mkpath() fail.
      auto env = fs::TestEnvironment{};
      env.createFile("blocker", "");
      const auto path =
        pathAsQString(env.dir() / "blocker" / "subdir" / "preferences.json");

      CHECK(
        readPreferencesFromFile(path).is_error_type<PreferenceErrors::FileAccessError>());
    }

    SECTION("returns FileAccessError when the file cannot be opened for reading")
    {
      // A directory where a regular file is expected exists, but cannot be opened as one.
      auto env = fs::TestEnvironment{};
      env.createDirectory("preferences.json");
      const auto path = pathAsQString(env.dir() / "preferences.json");

      CHECK(
        readPreferencesFromFile(path).is_error_type<PreferenceErrors::FileAccessError>());
    }
  }

  SECTION("writePreferencesToJson")
  {
    const auto preferenceValues = PreferenceValues{
      {"Prefs/Values/Integer value", QJsonValue{108}},
      {"Prefs/Values/Float value", QJsonValue{0.425781}},
      {"Prefs/Values/Bool value", QJsonValue{true}},
      {"Prefs/Values/String value", QJsonValue{"this and that"}},
      {"Prefs/Values/Color value", QJsonValue{"0.290196 0.643137 0.486275 1"}},
      {"Prefs/Paths/Equal sign", QJsonValue{"/home/ericwa/foo=bar"}},
      {"Prefs/Paths/With spaces", QJsonValue{"/home/ericwa/Quake 3 Arena"}},
      {"Prefs/Key sequences/Single key", QJsonValue{"W"}},
      {"Prefs/Key sequences/Multiple keys", QJsonValue{"Ctrl+Alt+W"}},
    };

    const auto serialized = writePreferencesToJson(preferenceValues);

    CHECK(parsePreferencesFromJson(serialized) == preferenceValues);
  }

  SECTION("writePreferencesToFile")
  {
    const auto preferenceValues = PreferenceValues{
      {"some/path", QJsonValue{"some value"}},
    };

    SECTION("writes the preference values to the file, creating its containing directory")
    {
      auto env = fs::TestEnvironment{};
      const auto path = pathAsQString(env.dir() / "does-not-exist" / "preferences.json");

      CHECK(writePreferencesToFile(path, preferenceValues).is_success());
      CHECK(readPreferencesFromFile(path) == ReadPreferencesResult{preferenceValues});
    }

    SECTION("returns FileAccessError when the containing directory cannot be created")
    {
      // A regular file in the way of a directory component makes mkpath() fail.
      auto env = fs::TestEnvironment{};
      env.createFile("blocker", "");
      const auto path =
        pathAsQString(env.dir() / "blocker" / "subdir" / "preferences.json");

      CHECK(writePreferencesToFile(path, preferenceValues)
              .is_error_type<PreferenceErrors::FileAccessError>());
    }

    SECTION("returns FileAccessError when the file cannot be opened for writing")
    {
      // A directory where a regular file is expected exists, but cannot be opened as one.
      auto env = fs::TestEnvironment{};
      env.createDirectory("preferences.json");
      const auto path = pathAsQString(env.dir() / "preferences.json");

      CHECK(writePreferencesToFile(path, preferenceValues)
              .is_error_type<PreferenceErrors::FileAccessError>());
    }
  }
}

} // namespace tb::ui
