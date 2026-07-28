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
#include "base/LoggerCache.h"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb
{
namespace
{

using CachedMessage = std::tuple<LogLevel, std::string>;

auto collectMessages(LoggerCache& cache)
{
  auto result = std::vector<CachedMessage>{};
  cache.getCachedMessages(
    [&](const auto level, const auto& message) { result.emplace_back(level, message); });
  return result;
}

} // namespace

TEST_CASE("LoggerCache")
{
  auto cache = LoggerCache{};

  SECTION("getCachedMessages")
  {
    SECTION("returns nothing if no message was cached")
    {
      CHECK(collectMessages(cache) == std::vector<CachedMessage>{});
    }

    SECTION("returns the cached messages in the order in which they were cached")
    {
      cache.cacheMessage(LogLevel::Info, "asdf");
      cache.cacheMessage(LogLevel::Error, "fdsa");
      cache.cacheMessage(LogLevel::Debug, "qwer");

      CHECK(
        collectMessages(cache)
        == std::vector<CachedMessage>{
          {LogLevel::Info, "asdf"},
          {LogLevel::Error, "fdsa"},
          {LogLevel::Debug, "qwer"}});
    }

    SECTION("clears the cache")
    {
      cache.cacheMessage(LogLevel::Info, "asdf");
      REQUIRE(collectMessages(cache).size() == 1u);

      CHECK(collectMessages(cache) == std::vector<CachedMessage>{});
    }
  }
}

} // namespace tb
