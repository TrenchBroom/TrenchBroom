/*
 Copyright (C) 2010 Kristian Duske

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

#include "el/ELTestUtils.h"
#include "io/DiskIO.h"
#include "io/FgdParser.h"
#include "io/Reader.h"
#include "io/TestParserStatus.h"
#include "io/TraversalMode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionTestUtils.h"
#include "mdl/PropertyDefinition.h"

#include <algorithm>
#include <filesystem>
#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{

TEST_CASE("FgdParser")
{
  SECTION("parseIncludedFgdFiles")
  {
    const auto basePath = std::filesystem::current_path() / "fixture/games/";
    const auto cfgFiles =
      Disk::find(basePath, TraversalMode::Recursive, makeExtensionPathMatcher({".fgd"}))
      | kdl::value();

    for (const auto& path : cfgFiles)
    {
      CAPTURE(path);

      auto file = Disk::openFile(path) | kdl::value();
      auto reader = file->reader().buffer();

      auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

      auto status = TestParserStatus{};
      CHECK(parser.parseDefinitions(status));

      /* Disabled because our files are full of previously undetected problems
      if (status.countStatus(LogLevel::Warn) > 0u) {
          UNSCOPED_INFO("Parsing FGD file " << path.string() << " produced warnings");
          for (const auto& message : status.messages(LogLevel::Warn)) {
              UNSCOPED_INFO(message);
          }
          CHECK(status.countStatus(LogLevel::Warn) == 0u);
      }

      if (status.countStatus(LogLevel::Error) > 0u) {
          UNSCOPED_INFO("Parsing FGD file " << path.string() << " produced errors");
          for (const auto& message : status.messages(LogLevel::Error)) {
              UNSCOPED_INFO(message);
          }
          CHECK(status.countStatus(LogLevel::Error) == 0u);
      }
      */
    }
  }

  SECTION("parseEmptyFile")
  {
    const auto file = "";
    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseWhitespaceFile")
  {
    const auto file = "     \n  \t \n  ";
    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseCommentsFile")
  {
    const auto file = R"(// asdfasdfasdf
//kj3k4jkdjfkjdf
)";
    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parseEmptyFlagDescription")
  {
    const auto file = R"(
    @PointClass color(0 255 0) size(-2 -2 -12, 2 2 12) = light_mine1 : 
    "Dusty fluorescent light fixture"
    [
        spawnflags(Flags) =
        [
            1 : "" : 0
        ]
    ]
    // 0221 - changed inheritance from "light" to "light_min1"
)";
    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "light_mine1",
          Color{0, 255, 0},
          "Dusty fluorescent light fixture",
          {
            {"spawnflags",
             mdl::PropertyValueTypes::Flags{
               {
                 {1, "", ""},
               },
             },
             "",
             ""},
          },
          mdl::PointEntityDefinition{{{-2, -2, -12}, {2, 2, 12}}, {}, {}},
        },
      });
  }

  SECTION("parseSolidClass")
  {
    const auto file = R"-(
    @SolidClass = worldspawn : "World entity"
    [
       message(string) : "Text on entering the world"
       worldtype(choices) : "Ambience" : 0 =
       [
           0 : "Medieval"
           1 : "Metal (runic)"
           2 : "Base"
       ]
       sounds(integer) : "CD track to play" : 0
       light(integer) : "Ambient light"
       _sunlight(integer) : "Sunlight"
       _sun_mangle(string) : "Sun mangle (Yaw pitch roll)"
    ])-";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "worldspawn",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "World entity",
          {
            {"message",
             mdl::PropertyValueTypes::String{},
             "Text on entering the world",
             ""},
            {"worldtype",
             mdl::PropertyValueTypes::Choice{
               {
                 {"0", "Medieval"},
                 {"1", "Metal (runic)"},
                 {"2", "Base"},
               },
               "0"},
             "Ambience",
             ""},
            {"sounds", mdl::PropertyValueTypes::Integer{0}, "CD track to play", ""},
            {"light", mdl::PropertyValueTypes::Integer{}, "Ambient light", ""},
            {"_sunlight", mdl::PropertyValueTypes::Integer{}, "Sunlight", ""},
            {"_sun_mangle",
             mdl::PropertyValueTypes::String{},
             "Sun mangle (Yaw pitch roll)",
             ""},
          },
        },
      });
  }

  SECTION("parsePointClass")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	use(string) : "self.use"
    	think(string) : "self.think"
    	nextthink(integer) : "nextthink"
    	noise(string) : "noise"
    	touch(string) : "self.touch"
    ])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {"info_notnull",
         Color{1.0f, 1.0f, 1.0f, 1.0f},
         "Wildcard entity",
         {
           {"use", mdl::PropertyValueTypes::String{}, "self.use", ""},
           {"think", mdl::PropertyValueTypes::String{}, "self.think", ""},
           {"nextthink", mdl::PropertyValueTypes::Integer{}, "nextthink", ""},
           {"noise", mdl::PropertyValueTypes::String{}, "noise", ""},
           {"touch", mdl::PropertyValueTypes::String{}, "self.touch", ""},
         },
         mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}}},
      });
  }

  SECTION("parseBaseProperty")
  {
    const auto file = R"(
    @baseclass = Appearflags [
    	spawnflags(Flags) =
    	[
    		256 : "Not on Easy" : 0
    		512 : "Not on Normal" : 0
    		1024 : "Not on Hard" : 0
    		2048 : "Not in Deathmatch" : 0
    	]
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status) == std::vector<mdl::EntityDefinition>{});
  }

  SECTION("parsePointClassWithBaseClasses")
  {
    const auto file = R"(
    @baseclass = Appearflags [
    	spawnflags(Flags) =
    	[
    		256 : "Not on Easy" : 0
    		512 : "Not on Normal" : 0
    		1024 : "Not on Hard" : 0
    		2048 : "Not in Deathmatch" : 0
    	]
    ]
    @baseclass = Targetname [ targetname(target_source) : "Name" ]
    @baseclass = Target [ 
    	target(target_destination) : "Target" 
    	killtarget(target_destination) : "Killtarget"
    ]
    @PointClass base(Appearflags, Target, Targetname) = info_notnull : "Wildcard entity" // I love you
    [
    	use(string) : "self.use"
    	think(string) : "self.think"
    	nextthink(integer) : "nextthink"
    	noise(string) : "noise"
    	touch(string) : "self.touch"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {"info_notnull",
         Color{1.0f, 1.0f, 1.0f, 1.0f},
         "Wildcard entity",
         {
           {"use", mdl::PropertyValueTypes::String{}, "self.use", ""},
           {"think", mdl::PropertyValueTypes::String{}, "self.think", ""},
           {"nextthink", mdl::PropertyValueTypes::Integer{}, "nextthink", ""},
           {"noise", mdl::PropertyValueTypes::String{}, "noise", ""},
           {"touch", mdl::PropertyValueTypes::String{}, "self.touch", ""},
           {"spawnflags",
            mdl::PropertyValueTypes::Flags{{
              {256, "Not on Easy", ""},
              {512, "Not on Normal", ""},
              {1024, "Not on Hard", ""},
              {2048, "Not in Deathmatch", ""},
            }},
            "",
            ""},
           {"target", mdl::PropertyValueTypes::LinkSource{}, "Target", ""},
           {"killtarget", mdl::PropertyValueTypes::LinkSource{}, "Killtarget", ""},
           {"targetname", mdl::PropertyValueTypes::LinkTarget{}, "Name", ""},
         },
         mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}}},
      });
  }

  SECTION("parsePointClassWithUnknownClassProperties")
  {
    const auto file = R"(
    @PointClass unknown1 unknown2(spaghetti) = info_notnull : "Wildcard entity" // I love you
    [
    	use(string) : "self.use"
    	think(string) : "self.think"
    	nextthink(integer) : "nextthink"
    	noise(string) : "noise"
    	touch(string) : "self.touch"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {"info_notnull",
         Color{1.0f, 1.0f, 1.0f, 1.0f},
         "Wildcard entity",
         {
           {"use", mdl::PropertyValueTypes::String{}, "self.use", ""},
           {"think", mdl::PropertyValueTypes::String{}, "self.think", ""},
           {"nextthink", mdl::PropertyValueTypes::Integer{}, "nextthink", ""},
           {"noise", mdl::PropertyValueTypes::String{}, "noise", ""},
           {"touch", mdl::PropertyValueTypes::String{}, "self.touch", ""},
         },
         mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}}},
      });
  }

  SECTION("parseType_TargetSourcePropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	targetname(target_source) : "Source" : : "A long description" 
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"targetname",
             mdl::PropertyValueTypes::LinkTarget{},
             "Source",
             "A long description"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseType_TargetDestinationPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	target(target_destination) : "Target" 
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};


    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"target", mdl::PropertyValueTypes::LinkSource{}, "Target", ""},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseStringPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       message(string) : "Text on entering the world" : : "Long description 1"
       message2(string) : "With a default value" : "DefaultValue" : "Long description 2"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"message",
             mdl::PropertyValueTypes::String{},
             "Text on entering the world",
             "Long description 1"},
            {"message2",
             mdl::PropertyValueTypes::String{"DefaultValue"},
             "With a default value",
             "Long description 2"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parsePropertyDefinitionWithNumericKey")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       123(string) : "Something" : : "Long description 1"
       456(string) : "Something" : : "Long description 1"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"123", mdl::PropertyValueTypes::String{}, "Something", "Long description 1"},
            {"456", mdl::PropertyValueTypes::String{}, "Something", "Long description 1"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  /**
   * Support having an integer (or decimal) as a default for a string propertyDefinition.
   * Technically a type mismatch, but appears in the wild; see:
   * https://github.com/TrenchBroom/TrenchBroom/issues/2833
   */
  SECTION("parseStringPropertyDefinition_IntDefault")
  {
    const auto file = R"(@PointClass = info_notnull : "Wildcard entity"
[
    name(string) : "Description" : 3
    other(string) : "" : 1.5
])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"name", mdl::PropertyValueTypes::String{"3"}, "Description", ""},
            {"other", mdl::PropertyValueTypes::String{"1.5"}, "", ""},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseIntegerPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       sounds(integer) : "CD track to play" : : "Longer description"
       sounds2(integer) : "CD track to play with default" : 2 : "Longer description"
    ])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"sounds",
             mdl::PropertyValueTypes::Integer{},
             "CD track to play",
             "Longer description"},
            {"sounds2",
             mdl::PropertyValueTypes::Integer{2},
             "CD track to play with default",
             "Longer description"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseReadOnlyPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       sounds(integer) readonly : "CD track to play" : : "Longer description"
       sounds2(integer) : "CD track to play with default" : 2 : "Longe
    description"
    ])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"sounds",
             mdl::PropertyValueTypes::Integer{},
             "CD track to play",
             "Longer description",
             true},
            {"sounds2",
             mdl::PropertyValueTypes::Integer{2},
             "CD track to play with default",
             R"(Longe
    description)"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseFloatPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       test(float) : "Some test propertyDefinition" : : "Longer description 1"
       test2(float) : "Some test propertyDefinition with default" : "2.7" : "Longer description 2"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"test",
             mdl::PropertyValueTypes::Float{},
             "Some test propertyDefinition",
             "Longer description 1"},
            {"test2",
             mdl::PropertyValueTypes::Float{2.7f},
             "Some test propertyDefinition with default",
             "Longer description 2"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseChoicePropertyDefinition")
  {
    const auto file = R"-(
            @PointClass = info_notnull : "Wildcard entity" // I love you\n
[
    worldtype(choices) : "Ambience" : : "Long description 1" =
    [
        0 : "Medieval"
        1 : "Metal (runic)"
        2 : "Base"
    ]
    worldtype2(choices) : "Ambience with default" : 1 : "Long description 2" =
    [
        0 : "Medieval"
        1 : "Metal (runic)"
    ]
    puzzle_id(choices) : "Puzzle id" : "cskey" =
    [
        "keep3" : "Mill key"
        "cskey" : "Castle key"
        "scrol" : "Disrupt Magic Scroll"
    ]
    floaty(choices) : "Floaty" : 2.3 =
    [
        1.0 : "Something"
        2.3 : "Something else"
        0.1 : "Yet more"
    ]
    negative(choices) : "Negative values" : -1 =
    [
        -2 : "Something"
        -1 : "Something else"
         1 : "Yet more"
    ]
]
            )-";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"worldtype",
             mdl::PropertyValueTypes::Choice{{
               {"0", "Medieval"},
               {"1", "Metal (runic)"},
               {"2", "Base"},
             }},
             "Ambience",
             "Long description 1"},
            {"worldtype2",
             mdl::PropertyValueTypes::Choice{
               {
                 {"0", "Medieval"},
                 {"1", "Metal (runic)"},
               },
               "1"},
             "Ambience with default",
             "Long description 2"},
            {"puzzle_id",
             mdl::PropertyValueTypes::Choice{
               {
                 {"keep3", "Mill key"},
                 {"cskey", "Castle key"},
                 {"scrol", "Disrupt Magic Scroll"},
               },
               "cskey"},
             "Puzzle id",
             ""},
            {"floaty",
             mdl::PropertyValueTypes::Choice{
               {
                 {"1.0", "Something"},
                 {"2.3", "Something else"},
                 {"0.1", "Yet more"},
               },
               "2.3"},
             "Floaty",
             ""},
            {"negative",
             mdl::PropertyValueTypes::Choice{
               {
                 {"-2", "Something"},
                 {"-1", "Something else"},
                 {"1", "Yet more"},
               },
               "-1"},
             "Negative values",
             ""},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseFlagsPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	spawnflags(Flags) =
    	[
    		256 : "Not on Easy" : 0
    		512 : "Not on Normal" : 1
    		1024 : "Not on Hard" : 0
    		2048 : "Not in Deathmatch" : 1
    	]
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"spawnflags",
             mdl::PropertyValueTypes::Flags{
               {
                 {256, "Not on Easy", ""},
                 {512, "Not on Normal", ""},
                 {1024, "Not on Hard", ""},
                 {2048, "Not in Deathmatch", ""},
               },
               512 | 2048},
             "",
             ""},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  SECTION("parseOriginPropertyDefinition")
  {
    const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       origin(origin) : "Entity origin" : "1 2 3" : "Long description 1"
    ]
)";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "info_notnull",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Wildcard entity",
          {
            {"origin",
             mdl::PropertyValueTypes::Origin{"1 2 3"},
             "Entity origin",
             "Long description 1"},
          },
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }

  static const auto FgdModelDefinitionTemplate =
    R"(@PointClass model(${MODEL}) = item_shells : "Shells" [])";

  using mdl::getModelSpecification;

  SECTION("parseLegacyStaticModelDefinition")
  {
    static const auto ModelDefinition =
      R"(":maps/b_shell0.bsp", ":maps/b_shell1.bsp" spawnflags = 1)";

    // CHECK(
    //   getModelSpecification<FgdParser>(ModelDefinition, FgdModelDefinitionTemplate)
    //   == mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0});
    CHECK(
      getModelSpecification<FgdParser>(
        ModelDefinition, FgdModelDefinitionTemplate, "{ 'spawnflags': 1 }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0});
  }

  SECTION("parseLegacyDynamicModelDefinition")
  {
    static const auto ModelDefinition =
      R"(pathKey = "model" skinKey = "skin" frameKey = "frame")";

    CHECK(
      getModelSpecification<FgdParser>(
        ModelDefinition, FgdModelDefinitionTemplate, "{ 'model': 'maps/b_shell1.bsp' }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0});
    CHECK(
      getModelSpecification<FgdParser>(
        ModelDefinition,
        FgdModelDefinitionTemplate,
        "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }")
      == mdl::ModelSpecification{"maps/b_shell1.bsp", 1, 2});
  }

  SECTION("parseELModelDefinition")
  {
    static const auto ModelDefinition =
      R"({{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }})";

    CHECK(
      getModelSpecification<FgdParser>(ModelDefinition, FgdModelDefinitionTemplate)
      == mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0});
  }

  SECTION("parseLegacyModelWithParseError")
  {
    const auto file = R"(
@PointClass base(Monster) size(-16 -16 -24, 16 16 40) model(":progs/polyp.mdl" 0 153, ":progs/polyp.mdl" startonground = "1") = monster_polyp: "Polyp"
[
  startonground(choices) : "Starting pose" : 0 =
  [
    0 : "Flying"
    1 : "On ground"
  ]
])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "monster_polyp",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Polyp",
          {
            {"startonground",
             mdl::PropertyValueTypes::Choice{
               {
                 {"0", "Flying"},
                 {"1", "On ground"},
               },
               "0"},
             "Starting pose",
             ""},
          },
          mdl::PointEntityDefinition{
            {{-16, -16, -24}, {16, 16, 40}},
            mdl::ModelDefinition{el::swt({
              el::cs(
                el::eq(el::var("startonground"), el::lit("1")),
                el::lit(el::MapType{{"path", el::Value{":progs/polyp.mdl"}}})),
              el::lit(el::MapType{
                {"path", el::Value{":progs/polyp.mdl"}},
                {"frame", el::Value{153}},
                {"skin", el::Value{0}},
              }),
            })},
            {}},
        },
      });
  }

  static const auto FgdDecalDefinitionTemplate =
    R"(@PointClass decal(${DECAL}) = infodecal : "Decal" [])";

  using mdl::assertDecalDefinition;

  SECTION("parseEmptyDecalDefinition")
  {
    static const auto DecalDefinition = "";

    assertDecalDefinition<FgdParser>(
      mdl::DecalSpecification{"decal1"},
      DecalDefinition,
      FgdDecalDefinitionTemplate,
      R"({ "texture": "decal1" })");
  }

  SECTION("parseELDecalDefinition")
  {
    static const auto DecalDefinition = R"({ texture: "decal1" })";

    assertDecalDefinition<FgdParser>(
      mdl::DecalSpecification{"decal1"}, DecalDefinition, FgdDecalDefinitionTemplate);
  }

  static const auto FgdSpriteDefinitionTemplate =
    R"(@PointClass sprite(${MODEL}) = env_sprite : "Sprite" [])";

  SECTION("parseEmptySpriteDefinition")
  {
    static const auto SpriteDefinition = "";

    CHECK(
      getModelSpecification<FgdParser>(
        SpriteDefinition, FgdSpriteDefinitionTemplate, R"({ "model": "spritex.spr" })")
      == mdl::ModelSpecification{"spritex.spr", 0, 0});
  }

  SECTION("parseELSpriteDefinition")
  {
    static const auto SpriteDefinition = R"({ path: "spritex.spr" })";

    CHECK(
      getModelSpecification<FgdParser>(SpriteDefinition, FgdSpriteDefinitionTemplate)
      == mdl::ModelSpecification{"spritex.spr", 0, 0});
  }

  SECTION("parseELSpriteDefinitionShorthand")
  {
    static const auto SpriteDefinition = R"("spritex.spr")";

    CHECK(
      getModelSpecification<FgdParser>(SpriteDefinition, FgdSpriteDefinitionTemplate)
      == mdl::ModelSpecification{"spritex.spr", 0, 0});
  }

  SECTION("parseMissingBounds")
  {
    const auto file = R"(
@PointClass model({"path" : ":progs/goddess-statue.mdl" }) =
decor_goddess_statue : "Goddess Statue" []
)";

    auto parser = FgdParser(file, Color{1.0f, 1.0f, 1.0f, 1.0f});
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "decor_goddess_statue",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Goddess Statue",
          {},
          mdl::PointEntityDefinition{
            {{-8, -8, -8}, {8, 8, 8}},
            mdl::ModelDefinition{
              el::lit(el::MapType{{"path", el::Value{":progs/goddess-statue.mdl"}}})},
            {}},
        },
      });
  }

  SECTION("parseInvalidBounds")
  {
    const auto file = R"(
@PointClass size(32 32 0, -32 -32 256) model({"path" : ":progs/goddess-statue.mdl" }) =
decor_goddess_statue : "Goddess Statue" [])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "decor_goddess_statue",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          "Goddess Statue",
          {},
          mdl::PointEntityDefinition{
            {{-32, -32, 0}, {32, 32, 256}},
            mdl::ModelDefinition{
              el::lit(el::MapType{{"path", el::Value{":progs/goddess-statue.mdl"}}})},
            {}},
        },
      });
  }

  SECTION("parseInvalidModel")
  {
    const auto file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({1}) =
decor_goddess_statue : "Goddess Statue" [])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status).is_error());
  }

  SECTION("parseErrorAfterModel")
  {
    const auto file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({"path"
       : ":progs/goddess-statue.mdl" }) = decor_goddess_statue ; "Goddess Statue" [])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};

    CHECK(parser.parseDefinitions(status).is_error());
  }

  SECTION("parseInclude")
  {
    const auto path =
      std::filesystem::current_path() / "fixture/test/io/Fgd/parseInclude/host.fgd";
    auto file = Disk::openFile(path) | kdl::value();
    auto reader = file->reader().buffer();

    auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

    auto status = TestParserStatus{};
    auto defs = parser.parseDefinitions(status);
    REQUIRE(defs);
    CHECK(defs.value().size() == 2u);
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "worldspawn"; }));
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "info_player_start"; }));
  }

  SECTION("parseNestedInclude")
  {
    const auto path =
      std::filesystem::current_path() / "fixture/test/io/Fgd/parseNestedInclude/host.fgd";
    auto file = Disk::openFile(path) | kdl::value();
    auto reader = file->reader().buffer();

    auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

    auto status = TestParserStatus{};
    auto defs = parser.parseDefinitions(status);
    REQUIRE(defs);
    CHECK(defs.value().size() == 3u);
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "worldspawn"; }));
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "info_player_start"; }));
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "info_player_coop"; }));
  }

  SECTION("parseRecursiveInclude")
  {
    const auto path = std::filesystem::current_path()
                      / "fixture/test/io/Fgd/parseRecursiveInclude/host.fgd";
    auto file = Disk::openFile(path) | kdl::value();
    auto reader = file->reader().buffer();

    auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

    auto status = TestParserStatus{};
    auto defs = parser.parseDefinitions(status);
    REQUIRE(defs);
    CHECK(defs.value().size() == 1u);
    CHECK(std::ranges::any_of(
      defs.value(), [](const auto& def) { return def.name == "worldspawn"; }));
  }

  SECTION("parseStringContinuations")
  {
    const auto file = R"(
@PointClass = cont_description :
  "This is an example description for"+
  " this example entity. It will appear"+
  " in the help dialog for this entity"
[])";

    auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};
    auto status = TestParserStatus{};


    CHECK(
      parser.parseDefinitions(status)
      == std::vector<mdl::EntityDefinition>{
        {
          "cont_description",
          Color{1.0f, 1.0f, 1.0f, 1.0f},
          R"(This is an example description for this example entity. It will appear in the help dialog for this entity)",
          {},
          mdl::PointEntityDefinition{{{-8, -8, -8}, {8, 8, 8}}, {}, {}},
        },
      });
  }
}

} // namespace tb::io
