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

#include "mdl/EntityDefinitionFileSpec.h"

#include "kdl/path_utils.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("EntityDefinitionFileSpec")
{
  SECTION("parse")
  {
    using namespace std::string_literals;

    using T = std::tuple<std::string, std::optional<EntityDefinitionFileSpec>>;

    const auto [str, expectedSpec] = GENERATE(values<T>({
      {"", std::nullopt},
      {"builtin:entities.txt",
       EntityDefinitionFileSpec::makeBuiltin(kdl::parse_path("entities.txt"s))},
      {"builtin:C:\\some\\path\\entities.txt",
       EntityDefinitionFileSpec::makeBuiltin(
         kdl::parse_path("C:\\some\\path\\entities.txt"s))},
      {"external:/path/to/entities.txt",
       EntityDefinitionFileSpec::makeExternal(kdl::parse_path("/path/to/entities.txt"s))},
      {"external:entities.txt",
       EntityDefinitionFileSpec::makeExternal(kdl::parse_path("entities.txt"s))},
      {"external:", EntityDefinitionFileSpec::makeExternal("")},
      {"external", std::nullopt},
      {"foo:bar", std::nullopt},
    }));

    CHECK(EntityDefinitionFileSpec::parse(str) == expectedSpec);
  }

  SECTION("makeBuiltin")
  {
    CHECK(
      EntityDefinitionFileSpec::makeBuiltin("some/path/entities.txt")
      == EntityDefinitionFileSpec{
        EntityDefinitionFileSpec::Type::Builtin, "some/path/entities.txt"});
  }

  SECTION("makeExternal")
  {
    CHECK(
      EntityDefinitionFileSpec::makeExternal("some/path/entities.txt")
      == EntityDefinitionFileSpec{
        EntityDefinitionFileSpec::Type::External, "some/path/entities.txt"});
  }

  SECTION("asString")
  {
    using T = std::tuple<EntityDefinitionFileSpec, std::string>;
    const auto [spec, expectedStr] = GENERATE(values<T>({
      {EntityDefinitionFileSpec::makeBuiltin("some/path/entities.txt"),
       "builtin:some/path/entities.txt"},
      {EntityDefinitionFileSpec::makeBuiltin("C:\\some\\path\\entities.txt"),
       "builtin:C:/some/path/entities.txt"},
      {EntityDefinitionFileSpec::makeExternal("some/path/entities.txt"),
       "external:some/path/entities.txt"},
    }));

    CHECK(spec.asString() == expectedStr);
  }
}

} // namespace tb::mdl
