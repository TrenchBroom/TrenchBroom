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

#include "io/PathQt.h"
#include "ui/QPreferenceStoreUtils.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

const auto preferenceFile = io::pathAsQString(
  std::filesystem::current_path() / "fixture" / "test" / "ui" / "QPreferenceStoreUtils"
  / "preferences.json");

}

TEST_CASE("parsePreferencesFromJson")
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

TEST_CASE("readPreferencesFromFile")
{
  CHECK(
    readPreferencesFromFile(preferenceFile)
    == PreferenceValues{
      {"Prefs/Values/Integer value", QJsonValue{108}},
      {"Prefs/Values/Float value", QJsonValue{0.425781}},
      {"Prefs/Values/Bool value", QJsonValue{true}},
      {"Prefs/Values/String value", QJsonValue{"this and that"}},
      {"Prefs/Values/Color value", QJsonValue{"0.290196 0.643137 0.486275 1"}},
      {"Prefs/Paths/Equal sign", QJsonValue{"/home/ericwa/foo=bar"}},
      {"Prefs/Paths/With spaces", QJsonValue{"/home/ericwa/Quake 3 Arena"}},
      {"Prefs/Key sequences/Single key", QJsonValue{"W"}},
      {"Prefs/Key sequences/Multiple keys", QJsonValue{"Ctrl+Alt+W"}},
    });
}

TEST_CASE("writePreferencesToJson")
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

} // namespace tb::ui
