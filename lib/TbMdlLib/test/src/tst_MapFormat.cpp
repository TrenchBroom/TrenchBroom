/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/MapFormat.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
namespace
{

std::string toString(const MapFormat format)
{
  auto str = std::ostringstream{};
  str << format;
  return str.str();
}

} // namespace

TEST_CASE("MapFormat")
{
  SECTION("formatFromName")
  {
    using T = std::tuple<std::string, MapFormat>;

    // clang-format off
    const auto [name, expectedFormat] = GENERATE(values<T>({
    {"Standard",        MapFormat::Standard},
    {"Quake2",          MapFormat::Quake2},
    {"Quake2 (Valve)",  MapFormat::Quake2_Valve},
    {"Valve",           MapFormat::Valve},
    {"Hexen2",          MapFormat::Hexen2},
    {"Daikatana",       MapFormat::Daikatana},
    {"Quake3 (legacy)", MapFormat::Quake3_Legacy},
    {"Quake3 (Valve)",  MapFormat::Quake3_Valve},
    {"Quake3",          MapFormat::Quake3},
    {"Unknown",         MapFormat::Unknown},
    {"",                MapFormat::Unknown},
    {"quake2",          MapFormat::Unknown},
    {"Quake 2",         MapFormat::Unknown},
    {"Quake4",          MapFormat::Unknown},
    }));
    // clang-format on

    CAPTURE(name);

    CHECK(formatFromName(name) == expectedFormat);
  }

  SECTION("formatName")
  {
    using T = std::tuple<MapFormat, std::string>;

    // clang-format off
    const auto [format, expectedName] = GENERATE(values<T>({
    {MapFormat::Unknown,       "Unknown"},
    {MapFormat::Standard,      "Standard"},
    {MapFormat::Quake2,        "Quake2"},
    {MapFormat::Quake2_Valve,  "Quake2 (Valve)"},
    {MapFormat::Valve,         "Valve"},
    {MapFormat::Hexen2,        "Hexen2"},
    {MapFormat::Daikatana,     "Daikatana"},
    {MapFormat::Quake3_Legacy, "Quake3 (legacy)"},
    {MapFormat::Quake3_Valve,  "Quake3 (Valve)"},
    {MapFormat::Quake3,        "Quake3"},
    }));
    // clang-format on

    CAPTURE(format);

    CHECK(formatName(format) == expectedName);

    SECTION("round trips through formatFromName")
    {
      CHECK(formatFromName(formatName(format)) == format);
    }
  }

  SECTION("operator<<")
  {
    using T = std::tuple<MapFormat, std::string>;

    // note that the stream operator spells the compound formats differently than
    // formatName does, so its output does not round trip through formatFromName
    // clang-format off
    const auto [format, expectedString] = GENERATE(values<T>({
    {MapFormat::Unknown,       "Unknown"},
    {MapFormat::Standard,      "Standard"},
    {MapFormat::Quake2,        "Quake2"},
    {MapFormat::Quake2_Valve,  "Quake2_Valve"},
    {MapFormat::Valve,         "Valve"},
    {MapFormat::Hexen2,        "Hexen2"},
    {MapFormat::Daikatana,     "Daikatana"},
    {MapFormat::Quake3_Legacy, "Quake3_Legacy"},
    {MapFormat::Quake3_Valve,  "Quake3_Valve"},
    {MapFormat::Quake3,        "Quake3"},
    }));
    // clang-format on

    CHECK(toString(format) == expectedString);
  }

  SECTION("compatibleFormats")
  {
    using T = std::tuple<MapFormat, std::vector<MapFormat>>;

    // clang-format off
    const auto [format, expectedFormats] = GENERATE(values<T>({
    {MapFormat::Unknown,       {MapFormat::Unknown}},
    {MapFormat::Standard,      {MapFormat::Standard, MapFormat::Valve}},
    {MapFormat::Quake2,        {MapFormat::Quake2, MapFormat::Quake2_Valve}},
    {MapFormat::Quake2_Valve,  {MapFormat::Quake2_Valve, MapFormat::Quake2}},
    {MapFormat::Valve,         {MapFormat::Valve, MapFormat::Standard}},
    {MapFormat::Hexen2,        {MapFormat::Hexen2}},
    {MapFormat::Daikatana,     {MapFormat::Daikatana}},
    {MapFormat::Quake3_Legacy, {MapFormat::Quake3_Legacy, MapFormat::Quake3_Valve, MapFormat::Quake3}},
    {MapFormat::Quake3_Valve,  {MapFormat::Quake3_Valve, MapFormat::Quake3, MapFormat::Quake3_Legacy}},
    {MapFormat::Quake3,        {MapFormat::Quake3, MapFormat::Quake3_Valve, MapFormat::Quake3_Legacy}},
    }));
    // clang-format on

    CAPTURE(format);

    CHECK(compatibleFormats(format) == expectedFormats);

    SECTION("starts with the given format")
    {
      CHECK(compatibleFormats(format).front() == format);
    }

    SECTION("is symmetric")
    {
      for (const auto compatibleFormat : compatibleFormats(format))
      {
        CAPTURE(compatibleFormat);

        const auto formats = compatibleFormats(compatibleFormat);
        CHECK(std::ranges::find(formats, format) != formats.end());
      }
    }
  }

  SECTION("isParallelUvCoordSystem")
  {
    using T = std::tuple<MapFormat, bool>;

    // clang-format off
    const auto [format, expectedResult] = GENERATE(values<T>({
    {MapFormat::Unknown,       false},
    {MapFormat::Standard,      false},
    {MapFormat::Quake2,        false},
    {MapFormat::Quake2_Valve,  true},
    {MapFormat::Valve,         true},
    {MapFormat::Hexen2,        false},
    {MapFormat::Daikatana,     false},
    {MapFormat::Quake3_Legacy, false},
    {MapFormat::Quake3_Valve,  true},
    {MapFormat::Quake3,        false},
    }));
    // clang-format on

    CAPTURE(format);

    CHECK(isParallelUvCoordSystem(format) == expectedResult);
  }

  SECTION("hasPatchSupport")
  {
    using T = std::tuple<MapFormat, bool>;

    // clang-format off
    const auto [format, expectedResult] = GENERATE(values<T>({
    {MapFormat::Unknown,       false},
    {MapFormat::Standard,      false},
    {MapFormat::Quake2,        false},
    {MapFormat::Quake2_Valve,  false},
    {MapFormat::Valve,         false},
    {MapFormat::Hexen2,        false},
    {MapFormat::Daikatana,     false},
    {MapFormat::Quake3_Legacy, true},
    {MapFormat::Quake3_Valve,  true},
    {MapFormat::Quake3,        true},
    }));
    // clang-format on

    CAPTURE(format);

    CHECK(hasPatchSupport(format) == expectedResult);
  }
}

} // namespace tb::mdl
