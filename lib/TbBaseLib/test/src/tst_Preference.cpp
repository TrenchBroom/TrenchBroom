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

#include "base/Preference.h"

#include <filesystem>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("Preference")
{
  SECTION("constructor")
  {
    SECTION("is persistent by default")
    {
      const auto pref = Preference<int>{"some/path", 7};

      CHECK(pref.path == std::filesystem::path{"some/path"});
      CHECK(pref.defaultValue == 7);
      CHECK(pref.persistencePolicy == PreferencePersistencePolicy::Persistent);
    }

    SECTION("stores the given persistence policy")
    {
      const auto pref =
        Preference<int>{"some/path", 7, PreferencePersistencePolicy::ReadOnly};

      CHECK(pref.persistencePolicy == PreferencePersistencePolicy::ReadOnly);
    }
  }
}

} // namespace tb
