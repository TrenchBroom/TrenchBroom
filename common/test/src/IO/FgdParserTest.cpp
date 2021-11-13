/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "IO/FgdParser.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "Assets/PropertyDefinition.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"

#include <kdl/vector_utils.h>

#include <algorithm>
#include <string>

#include "Catch2.h"

namespace TrenchBroom {
namespace IO {
TEST_CASE("FgdParserTest.parseIncludedFgdFiles", "[FgdParserTest]") {
  const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
  const std::vector<Path> cfgFiles =
    Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("fgd"));

  for (const Path& path : cfgFiles) {
    auto file = Disk::openFile(path);
    auto reader = file->reader().buffer();

    const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
    FgdParser parser(reader.stringView(), defaultColor, path);

    TestParserStatus status;
    UNSCOPED_INFO("Parsing FGD file " << path.asString() << " failed");
    CHECK_NOTHROW(parser.parseDefinitions(status));

    /* Disabled because our files are full of previously undetected problems
    if (status.countStatus(LogLevel::Warn) > 0u) {
        UNSCOPED_INFO("Parsing FGD file " << path.asString() << " produced warnings");
        for (const auto& message : status.messages(LogLevel::Warn)) {
            UNSCOPED_INFO(message);
        }
        CHECK(status.countStatus(LogLevel::Warn) == 0u);
    }

    if (status.countStatus(LogLevel::Error) > 0u) {
        UNSCOPED_INFO("Parsing FGD file " << path.asString() << " produced errors");
        for (const auto& message : status.messages(LogLevel::Error)) {
            UNSCOPED_INFO(message);
        }
        CHECK(status.countStatus(LogLevel::Error) == 0u);
    }
    */
  }
}

TEST_CASE("FgdParserTest.parseEmptyFile", "[FgdParserTest]") {
  const std::string file = "";
  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseWhitespaceFile", "[FgdParserTest]") {
  const std::string file = "     \n  \t \n  ";
  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseCommentsFile", "[FgdParserTest]") {
  const std::string file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseEmptyFlagDescription", "[FgdParserTest]") {
  const std::string file = "@PointClass color(0 255 0) size(-2 -2 -12, 2 2 12) = light_mine1 : "
                           "\"Dusty fluorescent light fixture\"\n"
                           "[\n"
                           "    spawnflags(Flags) =\n"
                           "    [\n"
                           "        1 : \"\" : 0\n"
                           "    ]\n"
                           "]\n"
                           "// 0221 - changed inheritance from \"light\" to \"light_min1\"\n";
  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseSolidClass", "[FgdParserTest]") {
  const std::string file = "@SolidClass = worldspawn : \"World entity\"\n"
                           "[\n"
                           "   message(string) : \"Text on entering the world\"\n"
                           "   worldtype(choices) : \"Ambience\" : 0 =\n"
                           "   [\n"
                           "       0 : \"Medieval\"\n"
                           "       1 : \"Metal (runic)\"\n"
                           "       2 : \"Base\"\n"
                           "   ]\n"
                           "   sounds(integer) : \"CD track to play\" : 0\n"
                           "   light(integer) : \"Ambient light\"\n"
                           "   _sunlight(integer) : \"Sunlight\"\n"
                           "   _sun_mangle(string) : \"Sun mangle (Yaw pitch roll)\"\n"
                           "]";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::BrushEntity);
  CHECK(definition->name() == std::string("worldspawn"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("World entity"));

  const auto& propertyDefinitions = definition->propertyDefinitions();
  CHECK(propertyDefinitions.size() == 6u);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parsePointClass", "[FgdParserTest]") {
  const std::string file = "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
                           "[\n"
                           "	use(string) : \"self.use\"\n"
                           "	think(string) : \"self.think\"\n"
                           "	nextthink(integer) : \"nextthink\"\n"
                           "	noise(string) : \"noise\"\n"
                           "	touch(string) : \"self.touch\"\n"
                           "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  const auto& propertyDefinitions = definition->propertyDefinitions();
  CHECK(propertyDefinitions.size() == 5u);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseBaseProperty", "[FgdParserTest]") {
  const std::string file = "@baseclass = Appearflags [\n"
                           "	spawnflags(Flags) =\n"
                           "	[\n"
                           "		256 : \"Not on Easy\" : 0\n"
                           "		512 : \"Not on Normal\" : 0\n"
                           "		1024 : \"Not on Hard\" : 0\n"
                           "		2048 : \"Not in Deathmatch\" : 0\n"
                           "	]\n"
                           "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.empty());
  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parsePointClassWithBaseClasses", "[FgdParserTest]") {
  const std::string file = "@baseclass = Appearflags [\n"
                           "	spawnflags(Flags) =\n"
                           "	[\n"
                           "		256 : \"Not on Easy\" : 0\n"
                           "		512 : \"Not on Normal\" : 0\n"
                           "		1024 : \"Not on Hard\" : 0\n"
                           "		2048 : \"Not in Deathmatch\" : 0\n"
                           "	]\n"
                           "]\n"
                           "@baseclass = Targetname [ targetname(target_source) : \"Name\" ]\n"
                           "@baseclass = Target [ \n"
                           "	target(target_destination) : \"Target\" \n"
                           "	killtarget(target_destination) : \"Killtarget\"\n"
                           "]\n"
                           "@PointClass base(Appearflags, Target, Targetname) = info_notnull : "
                           "\"Wildcard entity\" // I love you\n"
                           "[\n"
                           "	use(string) : \"self.use\"\n"
                           "	think(string) : \"self.think\"\n"
                           "	nextthink(integer) : \"nextthink\"\n"
                           "	noise(string) : \"noise\"\n"
                           "	touch(string) : \"self.touch\"\n"
                           "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  const auto& propertyDefinitions = definition->propertyDefinitions();
  CHECK(propertyDefinitions.size() == 9u);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseType_TargetSourcePropertyDefinition", "[FgdParserTest]") {
  const std::string file =
    "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
    "[\n"
    "	targetname(target_source) : \"Source\" : : \"A long description\" \n"
    "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  const auto& propertyDefinitions = definition->propertyDefinitions();
  CHECK(propertyDefinitions.size() == 1u);

  auto propertyDefinition = propertyDefinitions[0];
  CHECK(propertyDefinition->type() == Assets::PropertyDefinitionType::TargetSourceProperty);
  CHECK(propertyDefinition->key() == std::string("targetname"));
  CHECK(propertyDefinition->shortDescription() == std::string("Source"));
  CHECK(propertyDefinition->longDescription() == std::string("A long description"));

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseType_TargetDestinationPropertyDefinition", "[FgdParserTest]") {
  const std::string file = "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
                           "[\n"
                           "	target(target_destination) : \"Target\" \n"
                           "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  const auto& propertyDefinitions = definition->propertyDefinitions();
  CHECK(propertyDefinitions.size() == 1u);

  auto propertyDefinition = propertyDefinitions[0];
  CHECK(propertyDefinition->type() == Assets::PropertyDefinitionType::TargetDestinationProperty);
  CHECK(propertyDefinition->key() == std::string("target"));
  CHECK(propertyDefinition->shortDescription() == std::string("Target"));
  CHECK(propertyDefinition->longDescription() == std::string(""));

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseStringPropertyDefinition", "[FgdParserTest]") {
  const std::string file =
    "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
    "[\n"
    "   message(string) : \"Text on entering the world\" : : \"Long description 1\"\n"
    "   message2(string) : \"With a default value\" : \"DefaultValue\" : \"Long description 2\"\n"
    "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 2u);

  const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("message");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == Assets::PropertyDefinitionType::StringProperty);

  const Assets::StringPropertyDefinition* stringPropertyDefinition1 =
    static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition1);
  CHECK(stringPropertyDefinition1->key() == std::string("message"));
  CHECK(stringPropertyDefinition1->shortDescription() == std::string("Text on entering the world"));
  CHECK(stringPropertyDefinition1->longDescription() == std::string("Long description 1"));
  CHECK_FALSE(stringPropertyDefinition1->hasDefaultValue());

  const Assets::PropertyDefinition* propertyDefinition2 =
    definition->propertyDefinition("message2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == Assets::PropertyDefinitionType::StringProperty);

  const Assets::StringPropertyDefinition* stringPropertyDefinition2 =
    static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition2);
  CHECK(stringPropertyDefinition2->key() == std::string("message2"));
  CHECK(stringPropertyDefinition2->shortDescription() == std::string("With a default value"));
  CHECK(stringPropertyDefinition2->longDescription() == std::string("Long description 2"));
  CHECK(stringPropertyDefinition2->hasDefaultValue());
  CHECK(stringPropertyDefinition2->defaultValue() == std::string("DefaultValue"));

  kdl::vec_clear_and_delete(definitions);
}

/**
 * Support having an integer (or decimal) as a default for a string propertyDefinition. Technically
 * a type mismatch, but appears in the wild; see:
 * https://github.com/TrenchBroom/TrenchBroom/issues/2833
 */
TEST_CASE("FgdParserTest.parseStringPropertyDefinition_IntDefault", "[FgdParserTest]") {
  const std::string file = R"(@PointClass = info_notnull : "Wildcard entity"
[
    name(string) : "Description" : 3
    other(string) : "" : 1.5
])";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 2u);

  const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("name");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == Assets::PropertyDefinitionType::StringProperty);

  const Assets::StringPropertyDefinition* stringPropertyDefinition1 =
    static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition1);
  CHECK(stringPropertyDefinition1->key() == std::string("name"));
  CHECK(stringPropertyDefinition1->shortDescription() == std::string("Description"));
  CHECK(stringPropertyDefinition1->longDescription() == std::string());
  CHECK(stringPropertyDefinition1->hasDefaultValue());
  CHECK(stringPropertyDefinition1->defaultValue() == std::string("3"));

  const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("other");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == Assets::PropertyDefinitionType::StringProperty);

  const Assets::StringPropertyDefinition* stringPropertyDefinition2 =
    static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition2);
  CHECK(stringPropertyDefinition2->key() == std::string("other"));
  CHECK(stringPropertyDefinition2->shortDescription() == std::string());
  CHECK(stringPropertyDefinition2->longDescription() == std::string());
  CHECK(stringPropertyDefinition2->hasDefaultValue());
  CHECK(stringPropertyDefinition2->defaultValue() == std::string("1.5"));

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseIntegerPropertyDefinition", "[FgdParserTest]") {
  const std::string file =
    "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
    "[\n"
    "   sounds(integer) : \"CD track to play\" : : \"Longer description\"\n"
    "   sounds2(integer) : \"CD track to play with default\" : 2 : \"Longer description\"\n"
    "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 2u);

  const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("sounds");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == Assets::PropertyDefinitionType::IntegerProperty);

  const Assets::IntegerPropertyDefinition* intPropertyDefinition1 =
    static_cast<const Assets::IntegerPropertyDefinition*>(propertyDefinition1);
  CHECK(intPropertyDefinition1->key() == std::string("sounds"));
  CHECK(intPropertyDefinition1->shortDescription() == std::string("CD track to play"));
  CHECK(intPropertyDefinition1->longDescription() == std::string("Longer description"));
  CHECK_FALSE(intPropertyDefinition1->hasDefaultValue());

  const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("sounds2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == Assets::PropertyDefinitionType::IntegerProperty);

  const Assets::IntegerPropertyDefinition* intPropertyDefinition2 =
    static_cast<const Assets::IntegerPropertyDefinition*>(propertyDefinition2);
  CHECK(intPropertyDefinition2->key() == std::string("sounds2"));
  CHECK(intPropertyDefinition2->shortDescription() == std::string("CD track to play with default"));
  CHECK(intPropertyDefinition2->longDescription() == std::string("Longer description"));
  CHECK(intPropertyDefinition2->hasDefaultValue());
  CHECK(intPropertyDefinition2->defaultValue() == 2);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseReadOnlyPropertyDefinition", "[FgdParserTest]") {
  const std::string file =
    "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
    "[\n"
    "   sounds(integer) readonly : \"CD track to play\" : : \"Longer description\"\n"
    "   sounds2(integer) : \"CD track to play with default\" : 2 : \"Longer description\"\n"
    "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->propertyDefinitions().size() == 2u);

  const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("sounds");
  CHECK(propertyDefinition1->readOnly());

  const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("sounds2");
  CHECK_FALSE(propertyDefinition2->readOnly());

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseFloatPropertyDefinition", "[FgdParserTest]") {
  const std::string file =
    "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
    "[\n"
    "   test(float) : \"Some test propertyDefinition\" : : \"Longer description 1\"\n"
    "   test2(float) : \"Some test propertyDefinition with default\" : \"2.7\" : \"Longer "
    "description 2\"\n"
    "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  ;
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 2u);

  const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("test");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == Assets::PropertyDefinitionType::FloatProperty);

  const Assets::FloatPropertyDefinition* floatPropertyDefinition1 =
    static_cast<const Assets::FloatPropertyDefinition*>(propertyDefinition1);
  CHECK(floatPropertyDefinition1->key() == std::string("test"));
  CHECK(
    floatPropertyDefinition1->shortDescription() == std::string("Some test propertyDefinition"));
  CHECK(floatPropertyDefinition1->longDescription() == std::string("Longer description 1"));
  CHECK_FALSE(floatPropertyDefinition1->hasDefaultValue());

  const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("test2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == Assets::PropertyDefinitionType::FloatProperty);

  const Assets::FloatPropertyDefinition* floatPropertyDefinition2 =
    static_cast<const Assets::FloatPropertyDefinition*>(propertyDefinition2);
  CHECK(floatPropertyDefinition2->key() == std::string("test2"));
  CHECK(
    floatPropertyDefinition2->shortDescription() ==
    std::string("Some test propertyDefinition with default"));
  CHECK(floatPropertyDefinition2->longDescription() == std::string("Longer description 2"));
  CHECK(floatPropertyDefinition2->hasDefaultValue());
  CHECK(floatPropertyDefinition2->defaultValue() == 2.7f);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseChoicePropertyDefinition", "[FgdParserTest]") {
  const std::string file = R"%(
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
            )%";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  ;
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 5u);

  const Assets::PropertyDefinition* propertyDefinition1 =
    definition->propertyDefinition("worldtype");
  CHECK(propertyDefinition1 != nullptr);
  CHECK(propertyDefinition1->type() == Assets::PropertyDefinitionType::ChoiceProperty);

  const Assets::ChoicePropertyDefinition* choicePropertyDefinition1 =
    static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition1);
  CHECK(choicePropertyDefinition1->key() == std::string("worldtype"));
  CHECK(choicePropertyDefinition1->shortDescription() == std::string("Ambience"));
  CHECK(choicePropertyDefinition1->longDescription() == std::string("Long description 1"));
  CHECK_FALSE(choicePropertyDefinition1->hasDefaultValue());

  const Assets::ChoicePropertyOption::List& options1 = choicePropertyDefinition1->options();
  CHECK(options1.size() == 3u);
  CHECK(options1[0].value() == std::string("0"));
  CHECK(options1[0].description() == std::string("Medieval"));
  CHECK(options1[1].value() == std::string("1"));
  CHECK(options1[1].description() == std::string("Metal (runic)"));
  CHECK(options1[2].value() == std::string("2"));
  CHECK(options1[2].description() == std::string("Base"));

  const Assets::PropertyDefinition* propertyDefinition2 =
    definition->propertyDefinition("worldtype2");
  CHECK(propertyDefinition2 != nullptr);
  CHECK(propertyDefinition2->type() == Assets::PropertyDefinitionType::ChoiceProperty);

  const Assets::ChoicePropertyDefinition* choicePropertyDefinition2 =
    static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition2);
  CHECK(choicePropertyDefinition2->key() == std::string("worldtype2"));
  CHECK(choicePropertyDefinition2->shortDescription() == std::string("Ambience with default"));
  CHECK(choicePropertyDefinition2->longDescription() == std::string("Long description 2"));
  CHECK(choicePropertyDefinition2->hasDefaultValue());
  CHECK(choicePropertyDefinition2->defaultValue() == "1");

  const Assets::ChoicePropertyOption::List& options2 = choicePropertyDefinition2->options();
  CHECK(options2.size() == 2u);
  CHECK(options2[0].value() == std::string("0"));
  CHECK(options2[0].description() == std::string("Medieval"));
  CHECK(options2[1].value() == std::string("1"));
  CHECK(options2[1].description() == std::string("Metal (runic)"));

  const Assets::PropertyDefinition* propertyDefinition3 =
    definition->propertyDefinition("puzzle_id");
  const Assets::ChoicePropertyDefinition* choicePropertyDefinition3 =
    static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition3);
  CHECK(choicePropertyDefinition3->key() == std::string("puzzle_id"));
  CHECK(choicePropertyDefinition3->shortDescription() == std::string("Puzzle id"));
  CHECK(choicePropertyDefinition3->longDescription() == std::string(""));
  CHECK(choicePropertyDefinition3->hasDefaultValue());
  CHECK(choicePropertyDefinition3->defaultValue() == "cskey");

  const Assets::ChoicePropertyOption::List& options3 = choicePropertyDefinition3->options();
  CHECK(options3.size() == 3u);
  CHECK(options3[0].value() == std::string("keep3"));
  CHECK(options3[0].description() == std::string("Mill key"));
  CHECK(options3[1].value() == std::string("cskey"));
  CHECK(options3[1].description() == std::string("Castle key"));
  CHECK(options3[2].value() == std::string("scrol"));
  CHECK(options3[2].description() == std::string("Disrupt Magic Scroll"));

  const Assets::PropertyDefinition* propertyDefinition4 = definition->propertyDefinition("floaty");
  const Assets::ChoicePropertyDefinition* choicePropertyDefinition4 =
    static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition4);
  CHECK(choicePropertyDefinition4->key() == std::string("floaty"));
  CHECK(choicePropertyDefinition4->shortDescription() == std::string("Floaty"));
  CHECK(choicePropertyDefinition4->longDescription() == std::string(""));
  CHECK(choicePropertyDefinition4->hasDefaultValue());
  CHECK(choicePropertyDefinition4->defaultValue() == "2.3");

  const Assets::ChoicePropertyOption::List& options4 = choicePropertyDefinition4->options();
  CHECK(options4.size() == 3u);
  CHECK(options4[0].value() == std::string("1.0"));
  CHECK(options4[0].description() == std::string("Something"));
  CHECK(options4[1].value() == std::string("2.3"));
  CHECK(options4[1].description() == std::string("Something else"));
  CHECK(options4[2].value() == std::string("0.1"));
  CHECK(options4[2].description() == std::string("Yet more"));

  const Assets::PropertyDefinition* propertyDefinition5 =
    definition->propertyDefinition("negative");
  const Assets::ChoicePropertyDefinition* choicePropertyDefinition5 =
    static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition5);
  CHECK(choicePropertyDefinition5->key() == std::string("negative"));
  CHECK(choicePropertyDefinition5->shortDescription() == std::string("Negative values"));
  CHECK(choicePropertyDefinition5->longDescription() == std::string(""));
  CHECK(choicePropertyDefinition5->hasDefaultValue());
  CHECK(choicePropertyDefinition5->defaultValue() == "-1");

  const Assets::ChoicePropertyOption::List& options5 = choicePropertyDefinition5->options();
  CHECK(options5.size() == 3u);
  CHECK(options5[0].value() == std::string("-2"));
  CHECK(options5[0].description() == std::string("Something"));
  CHECK(options5[1].value() == std::string("-1"));
  CHECK(options5[1].description() == std::string("Something else"));
  CHECK(options5[2].value() == std::string("1"));
  CHECK(options5[2].description() == std::string("Yet more"));

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseFlagsPropertyDefinition", "[FgdParserTest]") {
  const std::string file = "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
                           "[\n"
                           "	spawnflags(Flags) =\n"
                           "	[\n"
                           "		256 : \"Not on Easy\" : 0\n"
                           "		512 : \"Not on Normal\" : 1\n"
                           "		1024 : \"Not on Hard\" : 0\n"
                           "		2048 : \"Not in Deathmatch\" : 1\n"
                           "	]\n"
                           "]\n";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  Assets::EntityDefinition* definition = definitions[0];
  CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
  CHECK(definition->name() == std::string("info_notnull"));
  CHECK(definition->color() == defaultColor);
  ;
  CHECK(definition->description() == std::string("Wildcard entity"));

  CHECK(definition->propertyDefinitions().size() == 1u);

  const Assets::PropertyDefinition* propertyDefinition =
    definition->propertyDefinition("spawnflags");
  CHECK(propertyDefinition != nullptr);
  CHECK(propertyDefinition->type() == Assets::PropertyDefinitionType::FlagsProperty);

  const Assets::FlagsPropertyDefinition* flagsPropertyDefinition =
    static_cast<const Assets::FlagsPropertyDefinition*>(propertyDefinition);
  CHECK(flagsPropertyDefinition->key() == std::string("spawnflags"));
  CHECK(flagsPropertyDefinition->shortDescription() == std::string(""));
  CHECK(flagsPropertyDefinition->defaultValue() == 2560);

  const Assets::FlagsPropertyOption::List& options = flagsPropertyDefinition->options();
  CHECK(options.size() == 4u);
  CHECK(options[0].value() == 256);
  CHECK(options[0].shortDescription() == std::string("Not on Easy"));
  CHECK_FALSE(options[0].isDefault());
  CHECK(options[1].value() == 512);
  CHECK(options[1].shortDescription() == std::string("Not on Normal"));
  CHECK(options[1].isDefault());
  CHECK(options[2].value() == 1024);
  CHECK(options[2].shortDescription() == std::string("Not on Hard"));
  CHECK_FALSE(options[2].isDefault());
  CHECK(options[3].value() == 2048);
  CHECK(options[3].shortDescription() == std::string("Not in Deathmatch"));
  CHECK(options[3].isDefault());

  kdl::vec_clear_and_delete(definitions);
}

static const std::string FgdModelDefinitionTemplate =
  "@PointClass\n"
  "    model(${MODEL}) = item_shells : \"Shells\" []\n";

using Assets::assertModelDefinition;

TEST_CASE("FgdParserTest.parseLegacyStaticModelDefinition", "[FgdParserTest]") {
  static const std::string ModelDefinition =
    "\":maps/b_shell0.bsp\", \":maps/b_shell1.bsp\" spawnflags = 1";

  assertModelDefinition<FgdParser>(
    Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp"), 0, 0), ModelDefinition,
    FgdModelDefinitionTemplate);
  assertModelDefinition<FgdParser>(
    Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 0, 0), ModelDefinition,
    FgdModelDefinitionTemplate, "{ 'spawnflags': 1 }");
}

TEST_CASE("FgdParserTest.parseLegacyDynamicModelDefinition", "[FgdParserTest]") {
  static const std::string ModelDefinition =
    "pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\"";

  assertModelDefinition<FgdParser>(
    Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 0, 0), ModelDefinition,
    FgdModelDefinitionTemplate, "{ 'model': 'maps/b_shell1.bsp' }");
  assertModelDefinition<FgdParser>(
    Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2), ModelDefinition,
    FgdModelDefinitionTemplate, "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
}

TEST_CASE("FgdParserTest.parseELModelDefinition", "[FgdParserTest]") {
  static const std::string ModelDefinition =
    "{{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }}";

  assertModelDefinition<FgdParser>(
    Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp"), 0, 0), ModelDefinition,
    FgdModelDefinitionTemplate);
}

TEST_CASE("FgdParserTest.parseLegacyModelWithParseError", "[FgdParserTest]") {
  const std::string file =
    "@PointClass base(Monster) size(-16 -16 -24, 16 16 40) model(\":progs/polyp.mdl\" 0 153, "
    "\":progs/polyp.mdl\" startonground = \"1\") = monster_polyp: \"Polyp\""
    "["
    "startonground(choices) : \"Starting pose\" : 0 ="
    "["
    "0 : \"Flying\""
    "1 : \"On ground\""
    "]"
    "]";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseInvalidBounds", "[FgdParserTest]") {
  const std::string file = R"(
@PointClass size(32 32 0, -32 -32 256) model({"path" : ":progs/goddess-statue.mdl" }) =
decor_goddess_statue : "Goddess Statue" [])";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
  CHECK(
    definition->bounds() == vm::bbox3d(vm::vec3d(-32.0, -32.0, 0.0), vm::vec3d(32.0, 32.0, 256.0)));

  kdl::vec_clear_and_delete(definitions);
}

TEST_CASE("FgdParserTest.parseInvalidModel", "[FgdParserTest]") {
  const std::string file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({1}) =
decor_goddess_statue : "Goddess Statue" [])";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  CHECK_THROWS_WITH(
    [&]() {
      TestParserStatus status;
      auto definitions = parser.parseDefinitions(status);
      kdl::vec_clear_and_delete(definitions);
    }(),
    Catch::Matchers::StartsWith("At line 3, column 8:"));
}

TEST_CASE("FgdParserTest.parseErrorAfterModel", "[FgdParserTest]") {
  const std::string file = R"(@PointClass
size(-16 -16 -24, 16 16 40)
model({"path"
       : ":progs/goddess-statue.mdl" }) = decor_goddess_statue ; "Goddess Statue" [])";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  CHECK_THROWS_WITH(
    [&]() {
      TestParserStatus status;
      auto definitions = parser.parseDefinitions(status);
      kdl::vec_clear_and_delete(definitions);
    }(),
    Catch::Matchers::StartsWith("At line 4, column 64:"));
}

TEST_CASE("FgdParserTest.parseInclude", "[FgdParserTest]") {
  const Path path =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseInclude/host.fgd");
  auto file = Disk::openFile(path);
  auto reader = file->reader().buffer();

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(reader.stringView(), defaultColor, file->path());

  TestParserStatus status;
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 2u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "worldspawn";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "info_player_start";
  }));

  kdl::vec_clear_and_delete(defs);
}

TEST_CASE("FgdParserTest.parseNestedInclude", "[FgdParserTest]") {
  const Path path =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseNestedInclude/host.fgd");
  auto file = Disk::openFile(path);
  auto reader = file->reader().buffer();

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(reader.stringView(), defaultColor, file->path());

  TestParserStatus status;
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 3u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "worldspawn";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "info_player_start";
  }));
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "info_player_coop";
  }));

  kdl::vec_clear_and_delete(defs);
}

TEST_CASE("FgdParserTest.parseRecursiveInclude", "[FgdParserTest]") {
  const Path path =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseRecursiveInclude/host.fgd");
  auto file = Disk::openFile(path);
  auto reader = file->reader().buffer();

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(reader.stringView(), defaultColor, file->path());

  TestParserStatus status;
  auto defs = parser.parseDefinitions(status);
  CHECK(defs.size() == 1u);
  CHECK(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) {
    return def->name() == "worldspawn";
  }));

  kdl::vec_clear_and_delete(defs);
}

TEST_CASE("FgdParserTest.parseStringContinuations", "[FgdParserTest]") {
  const std::string file = "@PointClass = cont_description :\n"
                           "\n"
                           "        \"This is an example description for\"+\n"
                           "        \" this example entity. It will appear\"+\n"
                           "        \" in the help dialog for this entity\"\n"
                           "\n"
                           "[]";

  const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
  FgdParser parser(file, defaultColor);

  TestParserStatus status;
  auto definitions = parser.parseDefinitions(status);
  CHECK(definitions.size() == 1u);

  const auto* definition = definitions.front();
  CHECK(
    definition->description() == "This is an example description for this example entity. It will "
                                 "appear in the help dialog for this entity");

  kdl::vec_clear_and_delete(definitions);
}
} // namespace IO
} // namespace TrenchBroom
