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

#include "base/Logger.h"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb
{
namespace
{

using LoggedMessage = std::tuple<LogLevel, std::string>;

class TestLogger : public Logger
{
public:
  std::vector<LoggedMessage> messages;

private:
  void doLog(const LogLevel level, const std::string_view message) override
  {
    messages.emplace_back(level, std::string{message});
  }
};

} // namespace

TEST_CASE("Logger")
{
  auto logger = TestLogger{};

  SECTION("stream")
  {
    SECTION("logs nothing until it is destroyed")
    {
      {
        auto stream = logger.info();
        stream << "asdf";
        CHECK(logger.messages.empty());
      }

      CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}});
    }

    SECTION("concatenates everything that was streamed into it")
    {
      logger.info() << "asdf" << 1 << ' ' << 2.5;

      CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf1 2.5"}});
    }

    SECTION("logs nothing if nothing was streamed into it")
    {
      logger.info();

      CHECK(logger.messages == std::vector<LoggedMessage>{});
    }

    SECTION("logs nothing if only empty strings were logged")
    {
      logger.info() << "" << "";

      CHECK(logger.messages == std::vector<LoggedMessage>{});
    }

    SECTION("logs nothing if only blank strings were logged")
    {
      logger.info() << " " << "\t";

      CHECK(logger.messages == std::vector<LoggedMessage>{});
    }
  }

  SECTION("debug")
  {
    logger.debug() << "asdf";

#ifdef NDEBUG
    // debug messages are suppressed in release builds
    CHECK(logger.messages.empty());
#else
    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Debug, "asdf"}});
#endif
  }

  SECTION("info")
  {
    logger.info() << "asdf";

    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}});
  }

  SECTION("warn")
  {
    logger.warn() << "asdf";

    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Warn, "asdf"}});
  }

  SECTION("error")
  {
    logger.error() << "asdf";

    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Error, "asdf"}});
  }

  SECTION("log")
  {
    logger.log(LogLevel::Info, "asdf");
    logger.log(LogLevel::Warn, "fdsa");

    CHECK(
      logger.messages
      == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}, {LogLevel::Warn, "fdsa"}});
  }
}

TEST_CASE("NullLogger")
{
  auto logger = NullLogger{};

  SECTION("discards everything that is logged to it")
  {
    CHECK_NOTHROW(logger.log(LogLevel::Error, "asdf"));
    CHECK_NOTHROW(logger.error() << "asdf");
  }
}

} // namespace tb
