/*
 Copyright (C) 2020-2020 Eric Wasylishen

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

#include "Exceptions.h"
#include "IO/GameEngineConfigParser.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        static GameEngineConfigParser makeParser(const std::string& config) {
            return GameEngineConfigParser(config.data(), config.data() + config.size(), Path());
        }

        TEST_CASE("GameEngineConfigParserTest.parseBlankConfig", "[GameEngineConfigParserTest]") {
            const std::string config(R"(   )");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseEmptyConfig", "[GameEngineConfigParserTest]") {
            const std::string config(R"( { } )");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseEmptyConfigWithTrailingGarbage", "[GameEngineConfigParserTest]") {
            const std::string config(R"(  {  } asdf)");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseMissingProfiles", "[GameEngineConfigParserTest]") {
            const std::string config(R"(  { 'version' : 1 } )");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseMissingVersion", "[GameEngineConfigParserTest]") {
            const std::string config(R"(  { 'profiles': {} } )");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseEmptyProfiles", "[GameEngineConfigParserTest]") {
            const std::string config(R"(  { 'version': 1, 'profiles': [] } )");
            auto parser = makeParser(config);
            auto result = parser.parse();
            REQUIRE(result.profileCount() == 0u);
        }

        TEST_CASE("GameEngineConfigParserTest.parseOneProfileWithMissingAttributes", "[GameEngineConfigParserTest]") {
            const std::string config(R"(
{
	"profiles": [
		{
		}
	],
	"version": 1
}
)");
            auto parser = makeParser(config);
            REQUIRE_THROWS_AS(parser.parse(), ParserException);
        }

        TEST_CASE("GameEngineConfigParserTest.parseTwoProfiles", "[GameEngineConfigParserTest]") {
            const std::string config(R"(
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
)");
            auto parser = makeParser(config);
            auto result = parser.parse();
            REQUIRE(result.profileCount() == 2u);

            CHECK(result.profile(0)->name() == "winquake");
            CHECK(result.profile(0)->parameterSpec() == "-flag1 -flag2");
            CHECK(result.profile(0)->path() == Path("C:\\Quake\\winquake.exe"));

            CHECK(result.profile(1)->name() == "glquake");
            CHECK(result.profile(1)->parameterSpec() == "-flag3 -flag4");
            CHECK(result.profile(1)->path() == Path("C:\\Quake\\glquake.exe"));
        }
    }
}
