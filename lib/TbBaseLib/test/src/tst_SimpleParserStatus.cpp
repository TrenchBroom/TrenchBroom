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
#include "base/Logger.h"
#include "base/SimpleParserStatus.h"

#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb
{
namespace
{

class TestLogger : public Logger
{
public:
  std::vector<std::string> messages;

private:
  void doLog(LogLevel, const std::string_view message) override
  {
    messages.emplace_back(message);
  }
};

} // namespace

TEST_CASE("SimpleParserStatus")
{
  auto logger = TestLogger{};

  SECTION("passes messages on to the logger")
  {
    auto status = SimpleParserStatus{logger, "Map"};

    status.warn(FileLocation{1, 2}, "with location");
    status.error("without location");

    CHECK(
      logger.messages
      == std::vector<std::string>{
        "Map: At line 1, column 2: with location",
        "Map: At unknown location: without location"});
  }

  SECTION("omits the prefix if none was given")
  {
    auto status = SimpleParserStatus{logger};

    status.warn(FileLocation{1, 2}, "asdf");

    CHECK(logger.messages == std::vector<std::string>{"At line 1, column 2: asdf"});
  }

  SECTION("discards progress")
  {
    auto status = SimpleParserStatus{logger, "Map"};

    CHECK_NOTHROW(status.progress(0.0));
    CHECK_NOTHROW(status.progress(0.5));
    CHECK_NOTHROW(status.progress(1.0));

    CHECK(logger.messages.empty());
  }
}

} // namespace tb
