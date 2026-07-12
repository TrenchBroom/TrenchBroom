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

#include "Uuid.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb
{

namespace
{

bool hasValidUuidFormat(const std::string& uuid)
{
  // Canonical form: 8-4-4-4-12 lower-case hex digits separated by hyphens.
  if (uuid.size() != 36u)
  {
    return false;
  }

  for (size_t i = 0u; i < uuid.size(); ++i)
  {
    if (i == 8u || i == 13u || i == 18u || i == 23u)
    {
      if (uuid[i] != '-')
      {
        return false;
      }
    }
    else if (std::isxdigit(static_cast<unsigned char>(uuid[i])) == 0)
    {
      return false;
    }
  }

  return true;
}

} // namespace

TEST_CASE("generateUuid")
{
  SECTION("produces canonically formatted UUIDs")
  {
    for (auto i = 0; i < 1000; ++i)
    {
      const auto uuid = generateUuid();
      CAPTURE(uuid);
      CHECK(hasValidUuidFormat(uuid));
    }
  }

  SECTION("produces unique UUIDs on a single thread")
  {
    constexpr auto count = 100'000u;

    auto seen = std::unordered_set<std::string>{};
    seen.reserve(count);
    for (auto i = 0u; i < count; ++i)
    {
      CHECK(seen.insert(generateUuid()).second);
    }
    CHECK(seen.size() == count);
  }

  SECTION("produces unique UUIDs across concurrent threads")
  {
    // The generator is seeded once per thread (thread_local). This is the case that would
    // break if two threads happened to seed identical engines, so exercise it directly.
    const auto threadCount = std::max(4u, std::thread::hardware_concurrency());
    constexpr auto perThread = 50'000u;

    auto perThreadResults = std::vector<std::vector<std::string>>(threadCount);
    auto threads = std::vector<std::thread>{};
    for (auto t = 0u; t < threadCount; ++t)
    {
      threads.emplace_back([&, t]() {
        auto& out = perThreadResults[t];
        out.reserve(perThread);
        for (auto i = 0u; i < perThread; ++i)
        {
          out.push_back(generateUuid());
        }
      });
    }
    for (auto& thread : threads)
    {
      thread.join();
    }

    auto seen = std::unordered_set<std::string>{};
    seen.reserve(threadCount * perThread);
    for (const auto& out : perThreadResults)
    {
      for (const auto& uuid : out)
      {
        CHECK(seen.insert(uuid).second);
      }
    }
    CHECK(seen.size() == threadCount * perThread);
  }
}

} // namespace tb
