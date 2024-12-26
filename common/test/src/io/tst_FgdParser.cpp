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

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("FgdParserTest.parseIncludedFgdFiles")
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
    CHECK_NOTHROW(parser.parseDefinitions(status));

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

TEST_CASE("FgdParserTest.parseEmptyFile")
{
  const auto file = "";
  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
}

TEST_CASE("FgdParserTest.parseWhitespaceFile")
{
  const auto file = "     \n  \t \n  ";
  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
}

TEST_CASE("FgdParserTest.parseCommentsFile")
{
  const auto file = R"(// asdfasdfasdf
//kj3k4jkdjfkjdf
)";
  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
}

TEST_CASE("FgdParserTest.parseEmptyFlagDescription")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);
}

TEST_CASE("FgdParserTest.parseSolidClass")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::BrushEntity);
  CHECK(definition.name() == "worldspawn");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "World entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 6u);
}

TEST_CASE("FgdParserTest.parsePointClass")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 5u);
}

TEST_CASE("FgdParserTest.parseBaseProperty")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
}

TEST_CASE("FgdParserTest.parsePointClassWithBaseClasses")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 9u);
}

TEST_CASE("FgdParserTest.parsePointClassWithUnknownClassProperties")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 5u);
}

TEST_CASE("FgdParserTest.parseType_TargetSourcePropertyDefinition")
{
  const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	targetname(target_source) : "Source" : : "A long description" 
    ]
)";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 1u);

  auto propertyDefinition = propertyDefinitions[0];
  CHECK(propertyDefinition->type() == mdl::PropertyDefinitionType::TargetSourceProperty);
  CHECK(propertyDefinition->key() == "targetname");
  CHECK(propertyDefinition->shortDescription() == "Source");
  CHECK(propertyDefinition->longDescription() == "A long description");
}

TEST_CASE("FgdParserTest.parseType_TargetDestinationPropertyDefinition")
{
  const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
    	target(target_destination) : "Target" 
    ]
)";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  const auto& propertyDefinitions = definition.propertyDefinitions();
  CHECK(propertyDefinitions.size() == 1u);

  auto propertyDefinition = propertyDefinitions[0];
  CHECK(
    propertyDefinition->type() == mdl::PropertyDefinitionType::TargetDestinationProperty);
  CHECK(propertyDefinition->key() == "target");
  CHECK(propertyDefinition->shortDescription() == "Target");
  CHECK(propertyDefinition->longDescription().empty());
}

TEST_CASE("FgdParserTest.parseStringPropertyDefinition")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 2u);

  const auto* propertyDefinition1 = definition.propertyDefinition("message");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == mdl::PropertyDefinitionType::StringProperty);

  const auto* stringPropertyDefinition1 =
    static_cast<const mdl::StringPropertyDefinition*>(propertyDefinition1);
  CHECK(stringPropertyDefinition1->key() == "message");
  CHECK(stringPropertyDefinition1->shortDescription() == "Text on entering the world");
  CHECK(stringPropertyDefinition1->longDescription() == "Long description 1");
  CHECK_FALSE(stringPropertyDefinition1->hasDefaultValue());

  const auto* propertyDefinition2 = definition.propertyDefinition("message2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == mdl::PropertyDefinitionType::StringProperty);

  const auto* stringPropertyDefinition2 =
    static_cast<const mdl::StringPropertyDefinition*>(propertyDefinition2);
  CHECK(stringPropertyDefinition2->key() == "message2");
  CHECK(stringPropertyDefinition2->shortDescription() == "With a default value");
  CHECK(stringPropertyDefinition2->longDescription() == "Long description 2");
  CHECK(stringPropertyDefinition2->hasDefaultValue());
  CHECK(stringPropertyDefinition2->defaultValue() == "DefaultValue");
}

TEST_CASE("FgdParserTest.parsePropertyDefinitionWithNumericKey")
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
  auto definitions = parser.parseDefinitions(status);
  REQUIRE(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.propertyDefinition("123") != nullptr);
  CHECK(definition.propertyDefinition("456") != nullptr);
}

/**
 * Support having an integer (or decimal) as a default for a string propertyDefinition.
 * Technically a type mismatch, but appears in the wild; see:
 * https://github.com/TrenchBroom/TrenchBroom/issues/2833
 */
TEST_CASE("FgdParserTest.parseStringPropertyDefinition_IntDefault")
{
  const auto file = R"(@PointClass = info_notnull : "Wildcard entity"
[
    name(string) : "Description" : 3
    other(string) : "" : 1.5
])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 2u);

  const auto* propertyDefinition1 = definition.propertyDefinition("name");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == mdl::PropertyDefinitionType::StringProperty);

  const auto* stringPropertyDefinition1 =
    static_cast<const mdl::StringPropertyDefinition*>(propertyDefinition1);
  CHECK(stringPropertyDefinition1->key() == "name");
  CHECK(stringPropertyDefinition1->shortDescription() == "Description");
  CHECK(stringPropertyDefinition1->longDescription().empty());
  CHECK(stringPropertyDefinition1->hasDefaultValue());
  CHECK(stringPropertyDefinition1->defaultValue() == "3");

  const auto* propertyDefinition2 = definition.propertyDefinition("other");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == mdl::PropertyDefinitionType::StringProperty);

  const auto* stringPropertyDefinition2 =
    static_cast<const mdl::StringPropertyDefinition*>(propertyDefinition2);
  CHECK(stringPropertyDefinition2->key() == "other");
  CHECK(stringPropertyDefinition2->shortDescription().empty());
  CHECK(stringPropertyDefinition2->longDescription().empty());
  CHECK(stringPropertyDefinition2->hasDefaultValue());
  CHECK(stringPropertyDefinition2->defaultValue() == "1.5");
}

TEST_CASE("FgdParserTest.parseIntegerPropertyDefinition")
{
  const auto file = R"(
    @PointClass = info_notnull : "Wildcard entity" // I love you
    [
       sounds(integer) : "CD track to play" : : "Longer description"
       sounds2(integer) : "CD track to play with default" : 2 : "Longer description"
    ])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 2u);

  const auto* propertyDefinition1 = definition.propertyDefinition("sounds");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == mdl::PropertyDefinitionType::IntegerProperty);

  const auto* intPropertyDefinition1 =
    static_cast<const mdl::IntegerPropertyDefinition*>(propertyDefinition1);
  CHECK(intPropertyDefinition1->key() == "sounds");
  CHECK(intPropertyDefinition1->shortDescription() == "CD track to play");
  CHECK(intPropertyDefinition1->longDescription() == "Longer description");
  CHECK_FALSE(intPropertyDefinition1->hasDefaultValue());

  const auto* propertyDefinition2 = definition.propertyDefinition("sounds2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == mdl::PropertyDefinitionType::IntegerProperty);

  const auto* intPropertyDefinition2 =
    static_cast<const mdl::IntegerPropertyDefinition*>(propertyDefinition2);
  CHECK(intPropertyDefinition2->key() == "sounds2");
  CHECK(intPropertyDefinition2->shortDescription() == "CD track to play with default");
  CHECK(intPropertyDefinition2->longDescription() == "Longer description");
  CHECK(intPropertyDefinition2->hasDefaultValue());
  CHECK(intPropertyDefinition2->defaultValue() == 2);
}

TEST_CASE("FgdParserTest.parseReadOnlyPropertyDefinition")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.propertyDefinitions().size() == 2u);

  const auto* propertyDefinition1 = definition.propertyDefinition("sounds");
  CHECK(propertyDefinition1->readOnly());

  const auto* propertyDefinition2 = definition.propertyDefinition("sounds2");
  CHECK_FALSE(propertyDefinition2->readOnly());
}

TEST_CASE("FgdParserTest.parseFloatPropertyDefinition")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  ;
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 2u);

  const auto* propertyDefinition1 = definition.propertyDefinition("test");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == mdl::PropertyDefinitionType::FloatProperty);

  const auto* floatPropertyDefinition1 =
    static_cast<const mdl::FloatPropertyDefinition*>(propertyDefinition1);
  CHECK(floatPropertyDefinition1->key() == "test");
  CHECK(floatPropertyDefinition1->shortDescription() == "Some test propertyDefinition");
  CHECK(floatPropertyDefinition1->longDescription() == "Longer description 1");
  CHECK_FALSE(floatPropertyDefinition1->hasDefaultValue());

  const auto* propertyDefinition2 = definition.propertyDefinition("test2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == mdl::PropertyDefinitionType::FloatProperty);

  const auto* floatPropertyDefinition2 =
    static_cast<const mdl::FloatPropertyDefinition*>(propertyDefinition2);
  CHECK(floatPropertyDefinition2->key() == "test2");
  CHECK(
    floatPropertyDefinition2->shortDescription()
    == "Some test propertyDefinition with default");
  CHECK(floatPropertyDefinition2->longDescription() == "Longer description 2");
  CHECK(floatPropertyDefinition2->hasDefaultValue());
  CHECK(floatPropertyDefinition2->defaultValue() == 2.7f);
}

TEST_CASE("FgdParserTest.parseChoicePropertyDefinition")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  ;
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 5u);

  const auto* propertyDefinition1 = definition.propertyDefinition("worldtype");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == mdl::PropertyDefinitionType::ChoiceProperty);

  const auto* choicePropertyDefinition1 =
    static_cast<const mdl::ChoicePropertyDefinition*>(propertyDefinition1);
  CHECK(choicePropertyDefinition1->key() == "worldtype");
  CHECK(choicePropertyDefinition1->shortDescription() == "Ambience");
  CHECK(choicePropertyDefinition1->longDescription() == "Long description 1");
  CHECK_FALSE(choicePropertyDefinition1->hasDefaultValue());

  CHECK(
    choicePropertyDefinition1->options()
    == std::vector<mdl::ChoicePropertyOption>{
      {"0", "Medieval"},
      {"1", "Metal (runic)"},
      {"2", "Base"},
    });

  const auto* propertyDefinition2 = definition.propertyDefinition("worldtype2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == mdl::PropertyDefinitionType::ChoiceProperty);

  const auto* choicePropertyDefinition2 =
    static_cast<const mdl::ChoicePropertyDefinition*>(propertyDefinition2);
  CHECK(choicePropertyDefinition2->key() == "worldtype2");
  CHECK(choicePropertyDefinition2->shortDescription() == "Ambience with default");
  CHECK(choicePropertyDefinition2->longDescription() == "Long description 2");
  CHECK(choicePropertyDefinition2->hasDefaultValue());
  CHECK(choicePropertyDefinition2->defaultValue() == "1");

  CHECK(
    choicePropertyDefinition2->options()
    == std::vector<mdl::ChoicePropertyOption>{
      {"0", "Medieval"},
      {"1", "Metal (runic)"},
    });

  const auto* propertyDefinition3 = definition.propertyDefinition("puzzle_id");
  const auto* choicePropertyDefinition3 =
    static_cast<const mdl::ChoicePropertyDefinition*>(propertyDefinition3);
  CHECK(choicePropertyDefinition3->key() == "puzzle_id");
  CHECK(choicePropertyDefinition3->shortDescription() == "Puzzle id");
  CHECK(choicePropertyDefinition3->longDescription().empty());
  CHECK(choicePropertyDefinition3->hasDefaultValue());
  CHECK(choicePropertyDefinition3->defaultValue() == "cskey");

  CHECK(
    choicePropertyDefinition3->options()
    == std::vector<mdl::ChoicePropertyOption>{
      {"keep3", "Mill key"},
      {"cskey", "Castle key"},
      {"scrol", "Disrupt Magic Scroll"},
    });

  const auto* propertyDefinition4 = definition.propertyDefinition("floaty");
  const auto* choicePropertyDefinition4 =
    static_cast<const mdl::ChoicePropertyDefinition*>(propertyDefinition4);
  CHECK(choicePropertyDefinition4->key() == "floaty");
  CHECK(choicePropertyDefinition4->shortDescription() == "Floaty");
  CHECK(choicePropertyDefinition4->longDescription().empty());
  CHECK(choicePropertyDefinition4->hasDefaultValue());
  CHECK(choicePropertyDefinition4->defaultValue() == "2.3");

  CHECK(
    choicePropertyDefinition4->options()
    == std::vector<mdl::ChoicePropertyOption>{
      {"1.0", "Something"},
      {"2.3", "Something else"},
      {"0.1", "Yet more"},
    });

  const auto* propertyDefinition5 = definition.propertyDefinition("negative");
  const auto* choicePropertyDefinition5 =
    static_cast<const mdl::ChoicePropertyDefinition*>(propertyDefinition5);
  CHECK(choicePropertyDefinition5->key() == "negative");
  CHECK(choicePropertyDefinition5->shortDescription() == "Negative values");
  CHECK(choicePropertyDefinition5->longDescription().empty());
  CHECK(choicePropertyDefinition5->hasDefaultValue());
  CHECK(choicePropertyDefinition5->defaultValue() == "-1");

  CHECK(
    choicePropertyDefinition5->options()
    == std::vector<mdl::ChoicePropertyOption>{
      {"-2", "Something"},
      {"-1", "Something else"},
      {"1", "Yet more"},
    });
}

TEST_CASE("FgdParserTest.parseFlagsPropertyDefinition")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions[0];
  CHECK(definition.type() == mdl::EntityDefinitionType::PointEntity);
  CHECK(definition.name() == "info_notnull");
  CHECK(definition.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  ;
  CHECK(definition.description() == "Wildcard entity");

  CHECK(definition.propertyDefinitions().size() == 1u);

  const auto* propertyDefinition = definition.propertyDefinition("spawnflags");
  CHECK(propertyDefinition != nullptr);
  CHECK(propertyDefinition->type() == mdl::PropertyDefinitionType::FlagsProperty);

  const auto* flagsPropertyDefinition =
    static_cast<const mdl::FlagsPropertyDefinition*>(propertyDefinition);
  CHECK(flagsPropertyDefinition->key() == "spawnflags");
  CHECK(flagsPropertyDefinition->shortDescription().empty());
  CHECK(flagsPropertyDefinition->defaultValue() == 2560);

  CHECK(
    flagsPropertyDefinition->options()
    == std::vector<mdl::FlagsPropertyOption>{
      {256, "Not on Easy", "", false},
      {512, "Not on Normal", "", true},
      {1024, "Not on Hard", "", false},
      {2048, "Not in Deathmatch", "", true},
    });
}

static const auto FgdModelDefinitionTemplate =
  R"(@PointClass model(${MODEL}) = item_shells : "Shells" [])";

using mdl::assertModelDefinition;

TEST_CASE("FgdParserTest.parseLegacyStaticModelDefinition")
{
  static const auto ModelDefinition =
    R"(":maps/b_shell0.bsp", ":maps/b_shell1.bsp" spawnflags = 1)";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0},
    ModelDefinition,
    FgdModelDefinitionTemplate);
  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0},
    ModelDefinition,
    FgdModelDefinitionTemplate,
    "{ 'spawnflags': 1 }");
}

TEST_CASE("FgdParserTest.parseLegacyDynamicModelDefinition")
{
  static const auto ModelDefinition =
    R"(pathKey = "model" skinKey = "skin" frameKey = "frame")";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"maps/b_shell1.bsp", 0, 0},
    ModelDefinition,
    FgdModelDefinitionTemplate,
    "{ 'model': 'maps/b_shell1.bsp' }");
  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"maps/b_shell1.bsp", 1, 2},
    ModelDefinition,
    FgdModelDefinitionTemplate,
    "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
}

TEST_CASE("FgdParserTest.parseELModelDefinition")
{
  static const auto ModelDefinition =
    R"({{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }})";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"maps/b_shell0.bsp", 0, 0},
    ModelDefinition,
    FgdModelDefinitionTemplate);
}

TEST_CASE("FgdParserTest.parseLegacyModelWithParseError")
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
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);
}

static const auto FgdDecalDefinitionTemplate =
  R"(@PointClass decal(${DECAL}) = infodecal : "Decal" [])";

using mdl::assertDecalDefinition;

TEST_CASE("FgdParserTest.parseEmptyDecalDefinition")
{
  static const auto DecalDefinition = "";

  assertDecalDefinition<FgdParser>(
    mdl::DecalSpecification{"decal1"},
    DecalDefinition,
    FgdDecalDefinitionTemplate,
    R"({ "texture": "decal1" })");
}

TEST_CASE("FgdParserTest.parseELDecalDefinition")
{
  static const auto DecalDefinition = R"({ texture: "decal1" })";

  assertDecalDefinition<FgdParser>(
    mdl::DecalSpecification{"decal1"}, DecalDefinition, FgdDecalDefinitionTemplate);
}

static const auto FgdSpriteDefinitionTemplate =
  R"(@PointClass sprite(${MODEL}) = env_sprite : "Sprite" [])";

TEST_CASE("FgdParserTest.parseEmptySpriteDefinition")
{
  static const auto SpriteDefinition = "";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"spritex.spr", 0, 0},
    SpriteDefinition,
    FgdSpriteDefinitionTemplate,
    R"({ "model": "spritex.spr" })");
}

TEST_CASE("FgdParserTest.parseELSpriteDefinition")
{
  static const auto SpriteDefinition = R"({ path: "spritex.spr" })";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"spritex.spr", 0, 0},
    SpriteDefinition,
    FgdSpriteDefinitionTemplate);
}

TEST_CASE("FgdParserTest.parseELSpriteDefinitionShorthand")
{
  static const auto SpriteDefinition = R"("spritex.spr")";

  assertModelDefinition<FgdParser>(
    mdl::ModelSpecification{"spritex.spr", 0, 0},
    SpriteDefinition,
    FgdSpriteDefinitionTemplate);
}

TEST_CASE("FgdParserTest.parseMissingBounds")
{
  const auto file = R"(
@PointClass model({"path" : ":progs/goddess-statue.mdl" }) =
decor_goddess_statue : "Goddess Statue" []
)";

  auto parser = FgdParser(file, Color{1.0f, 1.0f, 1.0f, 1.0f});

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition =
    static_cast<const mdl::PointEntityDefinition&>(*definitions[0]);
  CHECK(definition.bounds() == vm::bbox3d{{-8.0, -8.0, -8.0}, {8.0, 8.0, 8.0}});
}

TEST_CASE("FgdParserTest.parseInvalidBounds")
{
  const auto file = R"(
@PointClass size(32 32 0, -32 -32 256) model({"path" : ":progs/goddess-statue.mdl" }) =
decor_goddess_statue : "Goddess Statue" [])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition =
    static_cast<const mdl::PointEntityDefinition&>(*definitions[0]);
  CHECK(definition.bounds() == vm::bbox3d{{-32.0, -32.0, 0.0}, {32.0, 32.0, 256.0}});
}

TEST_CASE("FgdParserTest.parseInvalidModel")
{
  const auto file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({1}) =
decor_goddess_statue : "Goddess Statue" [])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  CHECK_THROWS_WITH(
    [&]() {
      auto status = TestParserStatus{};
      parser.parseDefinitions(status);
    }(),
    Catch::Matchers::StartsWith("At line 3, column 8:"));
}

TEST_CASE("FgdParserTest.parseErrorAfterModel")
{
  const auto file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({"path"
       : ":progs/goddess-statue.mdl" }) = decor_goddess_statue ; "Goddess Statue" [])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  CHECK_THROWS_WITH(
    [&]() {
      auto status = TestParserStatus{};
      parser.parseDefinitions(status);
    }(),
    Catch::Matchers::StartsWith("At line 4, column 64:"));
}

TEST_CASE("FgdParserTest.parseInclude")
{
  const auto path =
    std::filesystem::current_path() / "fixture/test/io/Fgd/parseInclude/host.fgd";
  auto file = Disk::openFile(path) | kdl::value();
  auto reader = file->reader().buffer();

  auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

  auto status = TestParserStatus{};
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 2u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "worldspawn";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "info_player_start";
  }));
}

TEST_CASE("FgdParserTest.parseNestedInclude")
{
  const auto path =
    std::filesystem::current_path() / "fixture/test/io/Fgd/parseNestedInclude/host.fgd";
  auto file = Disk::openFile(path) | kdl::value();
  auto reader = file->reader().buffer();

  auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

  auto status = TestParserStatus{};
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 3u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "worldspawn";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "info_player_start";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "info_player_coop";
  }));
}

TEST_CASE("FgdParserTest.parseRecursiveInclude")
{
  const auto path = std::filesystem::current_path()
                    / "fixture/test/io/Fgd/parseRecursiveInclude/host.fgd";
  auto file = Disk::openFile(path) | kdl::value();
  auto reader = file->reader().buffer();

  auto parser = FgdParser{reader.stringView(), Color{1.0f, 1.0f, 1.0f, 1.0f}, path};

  auto status = TestParserStatus{};
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 1u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto& def) {
    return def->name() == "worldspawn";
  }));
}

TEST_CASE("FgdParserTest.parseStringContinuations")
{
  const auto file = R"(
@PointClass = cont_description :
  "This is an example description for"+
  " this example entity. It will appear"+
  " in the help dialog for this entity"
[])";

  auto parser = FgdParser{file, Color{1.0f, 1.0f, 1.0f, 1.0f}};

  auto status = TestParserStatus{};
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto& definition = *definitions.front();
  CHECK(
    definition.description()
    == R"(This is an example description for this example entity. It will appear in the help dialog for this entity)");
}

} // namespace tb::io
