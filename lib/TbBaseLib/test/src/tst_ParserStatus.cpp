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
#include "base/ParserException.h"
#include "base/ParserStatus.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

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

/**
 * A status that does not override doLog, so that messages are passed on to the logger.
 */
class LoggingStatus : public ParserStatus
{
public:
  LoggingStatus(Logger& logger, std::string prefix)
    : ParserStatus{logger, std::move(prefix)}
  {
  }

private:
  void doProgress(double) override {}
};

class TestStatus : public ParserStatus
{
public:
  std::vector<std::string> messages;
  std::vector<LogLevel> levels;
  std::vector<double> progressValues;

  TestStatus(Logger& logger, std::string prefix)
    : ParserStatus{logger, std::move(prefix)}
  {
  }

private:
  void doProgress(double progress) override { progressValues.push_back(progress); }

  void doLog(LogLevel level, const std::string& str) override
  {
    levels.push_back(level);
    messages.push_back(str);
  }
};

} // namespace

TEST_CASE("ParserStatus")
{
  auto logger = NullLogger{};

  SECTION("progress")
  {
    auto status = TestStatus{logger, "Map"};

    status.progress(0.0);
    status.progress(0.5);
    status.progress(1.0);

    CHECK(status.progressValues == std::vector{0.0, 0.5, 1.0});
  }

  SECTION("debug")
  {
    auto status = TestStatus{logger, "Map"};

    status.debug(FileLocation{1, 2}, "with location");
    status.debug("without location");

    CHECK(status.levels == std::vector{LogLevel::Debug, LogLevel::Debug});
    CHECK(
      status.messages
      == std::vector<std::string>{
        "Map: At line 1, column 2: with location",
        "Map: At unknown location: without location"});
  }

  SECTION("info")
  {
    auto status = TestStatus{logger, "Map"};

    status.info(FileLocation{1, 2}, "with location");
    status.info("without location");

    CHECK(status.levels == std::vector{LogLevel::Info, LogLevel::Info});
    CHECK(
      status.messages
      == std::vector<std::string>{
        "Map: At line 1, column 2: with location",
        "Map: At unknown location: without location"});
  }

  SECTION("warn")
  {
    auto status = TestStatus{logger, "Map"};

    status.warn(FileLocation{1, 2}, "with location");
    status.warn("without location");

    CHECK(status.levels == std::vector{LogLevel::Warn, LogLevel::Warn});
    CHECK(
      status.messages
      == std::vector<std::string>{
        "Map: At line 1, column 2: with location",
        "Map: At unknown location: without location"});
  }

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

    CHECK(status.levels == std::vector{LogLevel::Error});
    CHECK(status.messages == std::vector<std::string>{expectedMessage});
  }

  SECTION("errorAndThrow")
  {
    auto status = TestStatus{logger, "Map"};

    CHECK_THROWS_MATCHES(
      status.errorAndThrow(FileLocation{1, 2}, "with location"),
      ParserException,
      Catch::Matchers::Message("Map: At line 1, column 2: with location"));

    CHECK_THROWS_MATCHES(
      status.errorAndThrow("without location"),
      ParserException,
      Catch::Matchers::Message("Map: At unknown location: without location"));

    // the message is logged as an error before it is thrown
    CHECK(status.levels == std::vector{LogLevel::Error, LogLevel::Error});
    CHECK(
      status.messages
      == std::vector<std::string>{
        "Map: At line 1, column 2: with location",
        "Map: At unknown location: without location"});
  }

  SECTION("doLog")
  {
    // the default implementation passes the message on to the logger
    auto testLogger = TestLogger{};
    auto status = LoggingStatus{testLogger, "Map"};

    status.warn(FileLocation{1, 2}, "with location");

    CHECK(
      testLogger.messages
      == std::vector<std::string>{"Map: At line 1, column 2: with location"});
  }
}

} // namespace tb
