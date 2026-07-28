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

#include "base/FileLocation.h"

#include <optional>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("FileLocation")
{
  SECTION("prependLocation")
  {
    // clang-format off
    const auto
    [location,                     str,      expectedResult] = GENERATE(table<std::optional<FileLocation>, std::string_view, std::string>({
    {std::nullopt,                 "",       "At unknown location:"},
    {std::nullopt,                 "asdf",   "At unknown location: asdf"},
    {FileLocation{0},              "asdf",   "At line 0: asdf"},
    {FileLocation{1},              "",       "At line 1:"},
    {FileLocation{1},              "asdf",   "At line 1: asdf"},
    {FileLocation{1, 0},           "asdf",   "At line 1, column 0: asdf"},
    {FileLocation{1, 2},           "",       "At line 1, column 2:"},
    {FileLocation{1, 2},           "asdf",   "At line 1, column 2: asdf"},
    }));
    // clang-format on

    CAPTURE(location, str);

    CHECK(prependLocation(location, str) == expectedResult);
  }
}

} // namespace tb
