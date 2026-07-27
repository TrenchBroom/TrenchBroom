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

#include "FileLocation.h"
#include "Logger.h"
#include "ParserStatus.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{
namespace
{

class TestStatus : public ParserStatus
{
public:
  std::vector<std::string> messages;

  TestStatus(Logger& logger, std::string prefix)
    : ParserStatus{logger, std::move(prefix)}
  {
  }

private:
  void doProgress(double) override {}

  void doLog(LogLevel, const std::string& str) override { messages.push_back(str); }
};

} // namespace

TEST_CASE("ParserStatus")
{
  auto logger = NullLogger{};

  SECTION("error")
  {
    // clang-format off
    const auto
    [prefix, location,           expectedMessage                                 ] = GENERATE(table<std::string, std::optional<FileLocation>, std::string>({
    {"",     FileLocation{1, 1}, "At line 1, column 1: Brush is incomplete"       },
    {"",     FileLocation{7},    "At line 7: Brush is incomplete"                 },
    {"",     std::nullopt,       "At unknown location: Brush is incomplete"       },
    {"Map",  FileLocation{1, 1}, "Map: At line 1, column 1: Brush is incomplete"  },
    {"Map",  std::nullopt,       "Map: At unknown location: Brush is incomplete"  },
    }));
    // clang-format on

    CAPTURE(prefix, location);

    auto status = TestStatus{logger, prefix};
    if (location)
    {
      status.error(*location, "Brush is incomplete");
    }
    else
    {
      status.error("Brush is incomplete");
    }

    CHECK(status.messages == std::vector<std::string>{expectedMessage});
  }
}

} // namespace tb
