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

#include "kdl/cmd_utils.h"

#include "catch2.h"

namespace kdl
{

TEST_CASE("cmd_parse_args")
{
  CHECK(cmd_parse_args(R"()") == std::vector<std::string>{});
  CHECK(cmd_parse_args(R"( )") == std::vector<std::string>{});
  CHECK(cmd_parse_args(R"(  )") == std::vector<std::string>{});
  CHECK(cmd_parse_args(R"(a)") == std::vector<std::string>{"a"});
  CHECK(cmd_parse_args(R"(a b)") == std::vector<std::string>{"a", "b"});
  CHECK(cmd_parse_args(R"( a   b )") == std::vector<std::string>{"a", "b"});
  CHECK(
    cmd_parse_args(R"( a  b " c d " )") == std::vector<std::string>{"a", "b", " c d "});
  CHECK(
    cmd_parse_args(R"( a  b \" c d \" )")
    == std::vector<std::string>{"a", "b", R"(\")", "c", "d", R"(\")"});
  CHECK(cmd_parse_args(R"( a  b " c d  )") == std::vector<std::string>{"a", "b"});

  // MSVC cannot parse this inline :-)
  const auto str = R"( a  b \" c d " )";
  CHECK(cmd_parse_args(str) == std::vector<std::string>{"a", "b", R"(\")", "c", "d"});
}

} // namespace kdl
