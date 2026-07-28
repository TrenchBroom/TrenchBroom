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
#include "base/LoggingHub.h"

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

TEST_CASE("LoggingHub")
{
  auto hub = LoggingHub{};
  auto logger = TestLogger{};

  SECTION("caches messages until a target logger is set")
  {
    hub.log(LogLevel::Info, "asdf");
    hub.log(LogLevel::Error, "fdsa");

    REQUIRE(logger.messages.empty());

    hub.setTargetLogger(&logger);

    CHECK(
      logger.messages
      == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}, {LogLevel::Error, "fdsa"}});
  }

  SECTION("passes messages on directly once a target logger is set")
  {
    hub.setTargetLogger(&logger);
    REQUIRE(logger.messages.empty());

    hub.log(LogLevel::Info, "asdf");

    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}});
  }

  SECTION("caches messages again after the target logger is unset")
  {
    hub.setTargetLogger(&logger);
    hub.log(LogLevel::Info, "asdf");
    REQUIRE(logger.messages.size() == 1u);

    hub.setTargetLogger(nullptr);
    hub.log(LogLevel::Error, "fdsa");

    // the message was cached instead of being passed on
    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}});

    hub.setTargetLogger(&logger);

    CHECK(
      logger.messages
      == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}, {LogLevel::Error, "fdsa"}});
  }

  SECTION("setting a target logger twice does not repeat the cached messages")
  {
    hub.log(LogLevel::Info, "asdf");

    hub.setTargetLogger(&logger);
    REQUIRE(logger.messages.size() == 1u);

    hub.setTargetLogger(&logger);

    CHECK(logger.messages == std::vector<LoggedMessage>{{LogLevel::Info, "asdf"}});
  }
}

} // namespace tb
