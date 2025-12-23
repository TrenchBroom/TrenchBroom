/*
 Copyright (C) 2020 Eric Wasylishen

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

#include "mdl/GameEngineConfig.h"
#include "mdl/GameEngineConfigParser.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("GameEngineConfigParserTest.parseBlankConfig")
{
  const auto config = R"(   )";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseEmptyConfig")
{
  const auto config = R"( { } )";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseEmptyConfigWithTrailingGarbage")
{
  const auto config = R"(  {  } asdf)";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseMissingProfiles")
{
  const auto config = R"(  { 'version' : 1 } )";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseMissingVersion")
{
  const auto config = R"(  { 'profiles': {} } )";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseEmptyProfiles")
{
  const auto config = R"(  { 'version': 1, 'profiles': [] } )";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse() == mdl::GameEngineConfig{});
}

TEST_CASE("GameEngineConfigParserTest.parseOneProfileWithMissingAttributes")
{
  const auto config = R"(
{
	"profiles": [
		{
		}
	],
	"version": 1
}
)";
  auto parser = GameEngineConfigParser{config};
  CHECK(parser.parse().is_error());
}

TEST_CASE("GameEngineConfigParserTest.parseTwoProfiles")
{
  const auto config = R"(
{
	"profiles": [
		{
			"name": "winquake",
			"parameters": "-flag1 -flag2",
			"path": "C:\\Quake\\winquake.exe"
		},
		{
			"name": "glquake",
			"parameters": "-flag3 -flag4",
			"path": "C:\\Quake\\glquake.exe",
            "extraKey": ""
		}
	],
	"version": 1,
    "extraKey": []
}
)";

  auto parser = GameEngineConfigParser{config};
  CHECK(
    parser.parse()
    == mdl::GameEngineConfig{
      {{"winquake", R"(C:\Quake\winquake.exe)", "-flag1 -flag2"},
       {"glquake", R"(C:\Quake\glquake.exe)", "-flag3 -flag4"}}});
}

} // namespace tb::mdl
