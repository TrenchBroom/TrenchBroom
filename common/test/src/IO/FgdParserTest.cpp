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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TestUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "IO/DiskIO.h"
#include "IO/FgdParser.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"

#include <kdl/vector_utils.h>

#include <algorithm>
#include <string>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("FgdParserTest.parseIncludedFgdFiles", "[FgdParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const std::vector<Path> cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("fgd"));

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();

                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                FgdParser parser(std::begin(reader), std::end(reader), defaultColor, path);

                TestParserStatus status;
                UNSCOPED_INFO("Parsing FGD file " << path.asString() << " failed");
                ASSERT_NO_THROW(parser.parseDefinitions(status));

                UNSCOPED_INFO("Parsing FGD file " << path.asString() << " produced warnings");
                ASSERT_EQ(0u, status.countStatus(LogLevel::Warn));

                UNSCOPED_INFO("Parsing FGD file " << path.asString() << " produced errors");
                ASSERT_EQ(0u, status.countStatus(LogLevel::Error));
            }
        }

        TEST_CASE("FgdParserTest.parseEmptyFile", "[FgdParserTest]") {
            const std::string file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseWhitespaceFile", "[FgdParserTest]") {
            const std::string file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseCommentsFile", "[FgdParserTest]") {
            const std::string file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseEmptyFlagDescription", "[FgdParserTest]") {
            const std::string file =
            "@PointClass color(0 255 0) size(-2 -2 -12, 2 2 12) = light_mine1 : \"Dusty fluorescent light fixture\"\n"
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
            ASSERT_EQ(1u, definitions.size());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseSolidClass", "[FgdParserTest]") {
            const std::string file =
            "@SolidClass = worldspawn : \"World entity\"\n"
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::BrushEntity, definition->type());
            ASSERT_EQ(std::string("worldspawn"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("World entity"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(6u, attributes.size());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parsePointClass", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(5u, attributes.size());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseBaseClass", "[FgdParserTest]") {
            const std::string file =
            "@baseclass = Appearflags [\n"
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
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parsePointClassWithBaseClasses", "[FgdParserTest]") {
            const std::string file =
            "@baseclass = Appearflags [\n"
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
            "@PointClass base(Appearflags, Target, Targetname) = info_notnull : \"Wildcard entity\" // I love you\n"
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(9u, attributes.size());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseType_TargetSourceAttribute", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	targetname(target_source) : \"Source\" : : \"A long description\" \n"
            "]\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());

            auto attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinitionType::TargetSourceAttribute, attribute->type());
            ASSERT_EQ(std::string("targetname"), attribute->name());
            ASSERT_EQ(std::string("Source"), attribute->shortDescription());
            ASSERT_EQ(std::string("A long description"), attribute->longDescription());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseType_TargetDestinationAttribute", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	target(target_destination) : \"Target\" \n"
            "]\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());

            auto attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinitionType::TargetDestinationAttribute, attribute->type());
            ASSERT_EQ(std::string("target"), attribute->name());
            ASSERT_EQ(std::string("Target"), attribute->shortDescription());
            ASSERT_EQ(std::string(""), attribute->longDescription());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseStringAttribute", "[FgdParserTest]") {
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("message");
            ASSERT_TRUE(attribute1 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::StringAttribute, attribute1->type());

            const Assets::StringAttributeDefinition* stringAttribute1 = static_cast<const Assets::StringAttributeDefinition*>(attribute1);
            ASSERT_EQ(std::string("message"), stringAttribute1->name());
            ASSERT_EQ(std::string("Text on entering the world"), stringAttribute1->shortDescription());
            ASSERT_EQ(std::string("Long description 1"), stringAttribute1->longDescription());
            ASSERT_FALSE(stringAttribute1->hasDefaultValue());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("message2");
            ASSERT_TRUE(attribute2 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::StringAttribute, attribute2->type());

            const Assets::StringAttributeDefinition* stringAttribute2 = static_cast<const Assets::StringAttributeDefinition*>(attribute2);
            ASSERT_EQ(std::string("message2"), stringAttribute2->name());
            ASSERT_EQ(std::string("With a default value"), stringAttribute2->shortDescription());
            ASSERT_EQ(std::string("Long description 2"), stringAttribute2->longDescription());
            ASSERT_TRUE(stringAttribute2->hasDefaultValue());
            ASSERT_EQ(std::string("DefaultValue"), stringAttribute2->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        /**
         * Support having an integer (or decimal) as a default for a string attribute. Technically
         * a type mismatch, but appears in the wild; see: https://github.com/kduske/TrenchBroom/issues/2833
         */
        TEST_CASE("FgdParserTest.parseStringAttribute_IntDefault", "[FgdParserTest]") {
            const std::string file = R"(@PointClass = info_notnull : "Wildcard entity"
[
    name(string) : "Description" : 3
    other(string) : "" : 1.5
])";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("name");
            ASSERT_TRUE(attribute1 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::StringAttribute, attribute1->type());

            const Assets::StringAttributeDefinition* stringAttribute1 = static_cast<const Assets::StringAttributeDefinition*>(attribute1);
            ASSERT_EQ(std::string("name"), stringAttribute1->name());
            ASSERT_EQ(std::string("Description"), stringAttribute1->shortDescription());
            ASSERT_EQ(std::string(), stringAttribute1->longDescription());
            ASSERT_TRUE(stringAttribute1->hasDefaultValue());
            ASSERT_EQ(std::string("3"), stringAttribute1->defaultValue());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("other");
            ASSERT_TRUE(attribute2 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::StringAttribute, attribute2->type());

            const Assets::StringAttributeDefinition* stringAttribute2 = static_cast<const Assets::StringAttributeDefinition*>(attribute2);
            ASSERT_EQ(std::string("other"), stringAttribute2->name());
            ASSERT_EQ(std::string(), stringAttribute2->shortDescription());
            ASSERT_EQ(std::string(), stringAttribute2->longDescription());
            ASSERT_TRUE(stringAttribute2->hasDefaultValue());
            ASSERT_EQ(std::string("1.5"), stringAttribute2->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseIntegerAttribute", "[FgdParserTest]") {
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("sounds");
            ASSERT_TRUE(attribute1 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::IntegerAttribute, attribute1->type());

            const Assets::IntegerAttributeDefinition* intAttribute1 = static_cast<const Assets::IntegerAttributeDefinition*>(attribute1);
            ASSERT_EQ(std::string("sounds"), intAttribute1->name());
            ASSERT_EQ(std::string("CD track to play"), intAttribute1->shortDescription());
            ASSERT_EQ(std::string("Longer description"), intAttribute1->longDescription());
            ASSERT_FALSE(intAttribute1->hasDefaultValue());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("sounds2");
            ASSERT_TRUE(attribute2 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::IntegerAttribute, attribute2->type());

            const Assets::IntegerAttributeDefinition* intAttribute2 = static_cast<const Assets::IntegerAttributeDefinition*>(attribute2);
            ASSERT_EQ(std::string("sounds2"), intAttribute2->name());
            ASSERT_EQ(std::string("CD track to play with default"), intAttribute2->shortDescription());
            ASSERT_EQ(std::string("Longer description"), intAttribute2->longDescription());
            ASSERT_TRUE(intAttribute2->hasDefaultValue());
            ASSERT_EQ(2, intAttribute2->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseReadOnlyAttribute", "[FgdParserTest]") {
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(2u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("sounds");
            ASSERT_TRUE(attribute1->readOnly());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("sounds2");
            ASSERT_FALSE(attribute2->readOnly());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseFloatAttribute", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   test(float) : \"Some test attribute\" : : \"Longer description 1\"\n"
            "   test2(float) : \"Some test attribute with default\" : \"2.7\" : \"Longer description 2\"\n"
            "]\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("test");
            ASSERT_TRUE(attribute1 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::FloatAttribute, attribute1->type());

            const Assets::FloatAttributeDefinition* floatAttribute1 = static_cast<const Assets::FloatAttributeDefinition*>(attribute1);
            ASSERT_EQ(std::string("test"), floatAttribute1->name());
            ASSERT_EQ(std::string("Some test attribute"), floatAttribute1->shortDescription());
            ASSERT_EQ(std::string("Longer description 1"), floatAttribute1->longDescription());
            ASSERT_FALSE(floatAttribute1->hasDefaultValue());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("test2");
            ASSERT_TRUE(attribute2 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::FloatAttribute, attribute2->type());

            const Assets::FloatAttributeDefinition* floatAttribute2 = static_cast<const Assets::FloatAttributeDefinition*>(attribute2);
            ASSERT_EQ(std::string("test2"), floatAttribute2->name());
            ASSERT_EQ(std::string("Some test attribute with default"), floatAttribute2->shortDescription());
            ASSERT_EQ(std::string("Longer description 2"), floatAttribute2->longDescription());
            ASSERT_TRUE(floatAttribute2->hasDefaultValue());
            ASSERT_FLOAT_EQ(2.7f, floatAttribute2->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseChoiceAttribute", "[FgdParserTest]") {
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(5u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("worldtype");
            ASSERT_TRUE(attribute1 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::ChoiceAttribute, attribute1->type());

            const Assets::ChoiceAttributeDefinition* choiceAttribute1 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute1);
            ASSERT_EQ(std::string("worldtype"), choiceAttribute1->name());
            ASSERT_EQ(std::string("Ambience"), choiceAttribute1->shortDescription());
            ASSERT_EQ(std::string("Long description 1"), choiceAttribute1->longDescription());
            ASSERT_FALSE(choiceAttribute1->hasDefaultValue());

            const Assets::ChoiceAttributeOption::List& options1 = choiceAttribute1->options();
            ASSERT_EQ(3u, options1.size());
            ASSERT_EQ(std::string("0"), options1[0].value());
            ASSERT_EQ(std::string("Medieval"), options1[0].description());
            ASSERT_EQ(std::string("1"), options1[1].value());
            ASSERT_EQ(std::string("Metal (runic)"), options1[1].description());
            ASSERT_EQ(std::string("2"), options1[2].value());
            ASSERT_EQ(std::string("Base"), options1[2].description());

            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("worldtype2");
            ASSERT_TRUE(attribute2 != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::ChoiceAttribute, attribute2->type());

            const Assets::ChoiceAttributeDefinition* choiceAttribute2 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute2);
            ASSERT_EQ(std::string("worldtype2"), choiceAttribute2->name());
            ASSERT_EQ(std::string("Ambience with default"), choiceAttribute2->shortDescription());
            ASSERT_EQ(std::string("Long description 2"), choiceAttribute2->longDescription());
            ASSERT_TRUE(choiceAttribute2->hasDefaultValue());
            ASSERT_EQ("1", choiceAttribute2->defaultValue());

            const Assets::ChoiceAttributeOption::List& options2 = choiceAttribute2->options();
            ASSERT_EQ(2u, options2.size());
            ASSERT_EQ(std::string("0"), options2[0].value());
            ASSERT_EQ(std::string("Medieval"), options2[0].description());
            ASSERT_EQ(std::string("1"), options2[1].value());
            ASSERT_EQ(std::string("Metal (runic)"), options2[1].description());

            const Assets::AttributeDefinition* attribute3 = definition->attributeDefinition("puzzle_id");
            const Assets::ChoiceAttributeDefinition* choiceAttribute3 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute3);
            ASSERT_EQ(std::string("puzzle_id"), choiceAttribute3->name());
            ASSERT_EQ(std::string("Puzzle id"), choiceAttribute3->shortDescription());
            ASSERT_EQ(std::string(""), choiceAttribute3->longDescription());
            ASSERT_TRUE(choiceAttribute3->hasDefaultValue());
            ASSERT_EQ("cskey", choiceAttribute3->defaultValue());

            const Assets::ChoiceAttributeOption::List& options3 = choiceAttribute3->options();
            ASSERT_EQ(3u, options3.size());
            ASSERT_EQ(std::string("keep3"), options3[0].value());
            ASSERT_EQ(std::string("Mill key"), options3[0].description());
            ASSERT_EQ(std::string("cskey"), options3[1].value());
            ASSERT_EQ(std::string("Castle key"), options3[1].description());
            ASSERT_EQ(std::string("scrol"), options3[2].value());
            ASSERT_EQ(std::string("Disrupt Magic Scroll"), options3[2].description());

            const Assets::AttributeDefinition* attribute4 = definition->attributeDefinition("floaty");
            const Assets::ChoiceAttributeDefinition* choiceAttribute4 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute4);
            ASSERT_EQ(std::string("floaty"), choiceAttribute4->name());
            ASSERT_EQ(std::string("Floaty"), choiceAttribute4->shortDescription());
            ASSERT_EQ(std::string(""), choiceAttribute4->longDescription());
            ASSERT_TRUE(choiceAttribute4->hasDefaultValue());
            ASSERT_EQ("2.3", choiceAttribute4->defaultValue());

            const Assets::ChoiceAttributeOption::List& options4 = choiceAttribute4->options();
            ASSERT_EQ(3u, options4.size());
            ASSERT_EQ(std::string("1.0"), options4[0].value());
            ASSERT_EQ(std::string("Something"), options4[0].description());
            ASSERT_EQ(std::string("2.3"), options4[1].value());
            ASSERT_EQ(std::string("Something else"), options4[1].description());
            ASSERT_EQ(std::string("0.1"), options4[2].value());
            ASSERT_EQ(std::string("Yet more"), options4[2].description());

            const Assets::AttributeDefinition* attribute5 = definition->attributeDefinition("negative");
            const Assets::ChoiceAttributeDefinition* choiceAttribute5 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute5);
            ASSERT_EQ(std::string("negative"), choiceAttribute5->name());
            ASSERT_EQ(std::string("Negative values"), choiceAttribute5->shortDescription());
            ASSERT_EQ(std::string(""), choiceAttribute5->longDescription());
            ASSERT_TRUE(choiceAttribute5->hasDefaultValue());
            ASSERT_EQ("-1", choiceAttribute5->defaultValue());

            const Assets::ChoiceAttributeOption::List& options5 = choiceAttribute5->options();
            ASSERT_EQ(3u, options5.size());
            ASSERT_EQ(std::string("-2"), options5[0].value());
            ASSERT_EQ(std::string("Something"), options5[0].description());
            ASSERT_EQ(std::string("-1"), options5[1].value());
            ASSERT_EQ(std::string("Something else"), options5[1].description());
            ASSERT_EQ(std::string("1"), options5[2].value());
            ASSERT_EQ(std::string("Yet more"), options5[2].description());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseFlagsAttribute", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(1u, definition->attributeDefinitions().size());

            const Assets::AttributeDefinition* attribute = definition->attributeDefinition("spawnflags");
            ASSERT_TRUE(attribute != nullptr);
            ASSERT_EQ(Assets::AttributeDefinitionType::FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* flagsAttribute = static_cast<const Assets::FlagsAttributeDefinition*>(attribute);
            ASSERT_EQ(std::string("spawnflags"), flagsAttribute->name());
            ASSERT_EQ(std::string(""), flagsAttribute->shortDescription());
            ASSERT_EQ(2560, flagsAttribute->defaultValue());

            const Assets::FlagsAttributeOption::List& options = flagsAttribute->options();
            ASSERT_EQ(4u, options.size());
            ASSERT_EQ(256, options[0].value());
            ASSERT_EQ(std::string("Not on Easy"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(512, options[1].value());
            ASSERT_EQ(std::string("Not on Normal"), options[1].shortDescription());
            ASSERT_TRUE(options[1].isDefault());
            ASSERT_EQ(1024, options[2].value());
            ASSERT_EQ(std::string("Not on Hard"), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(2048, options[3].value());
            ASSERT_EQ(std::string("Not in Deathmatch"), options[3].shortDescription());
            ASSERT_TRUE(options[3].isDefault());

            kdl::vec_clear_and_delete(definitions);
        }

        static const std::string FgdModelDefinitionTemplate =
        "@PointClass\n"
        "    model(${MODEL}) = item_shells : \"Shells\" []\n";

        using Assets::assertModelDefinition;

        TEST_CASE("FgdParserTest.parseLegacyStaticModelDefinition", "[FgdParserTest]") {
            static const std::string ModelDefinition = "\":maps/b_shell0.bsp\", \":maps/b_shell1.bsp\" spawnflags = 1";

            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate);
            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
        }

        TEST_CASE("FgdParserTest.parseLegacyDynamicModelDefinition", "[FgdParserTest]") {
            static const std::string ModelDefinition = "pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\"";

            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST_CASE("FgdParserTest.parseELStaticModelDefinition", "[FgdParserTest]") {
            static const std::string ModelDefinition = "{{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }}";

            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate);
            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'spawnflags': 2 }");
        }

        TEST_CASE("FgdParserTest.parseELDynamicModelDefinition", "[FgdParserTest]") {
            static const std::string ModelDefinition = "{ 'path': model, 'skin': skin, 'frame': frame }";

            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<FgdParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             FgdModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST_CASE("FgdParserTest.parseLegacyModelWithParseError", "[FgdParserTest]") {
            const std::string file =
            "@PointClass base(Monster) size(-16 -16 -24, 16 16 40) model(\":progs/polyp.mdl\" 0 153, \":progs/polyp.mdl\" startonground = \"1\") = monster_polyp: \"Polyp\""
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
            ASSERT_EQ(1u, definitions.size());

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
            ASSERT_EQ(1u, definitions.size());

            const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
            ASSERT_EQ(vm::bbox3d(vm::vec3d(-32.0, -32.0, 0.0), vm::vec3d(32.0, 32.0, 256.0)), definition->bounds());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseInclude", "[FgdParserTest]") {
            const Path path = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseInclude/host.fgd");
            auto file = Disk::openFile(path);
            auto reader = file->reader().buffer();

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(std::begin(reader), std::end(reader), defaultColor, file->path());

            TestParserStatus status;
            auto defs = parser.parseDefinitions(status);
            ASSERT_EQ(2u, defs.size());
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "worldspawn"; }));
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "info_player_start"; }));

            kdl::vec_clear_and_delete(defs);
        }

        TEST_CASE("FgdParserTest.parseNestedInclude", "[FgdParserTest]") {
            const Path path = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseNestedInclude/host.fgd");
            auto file = Disk::openFile(path);
            auto reader = file->reader().buffer();

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(std::begin(reader), std::end(reader), defaultColor, file->path());

            TestParserStatus status;
            auto defs = parser.parseDefinitions(status);
            ASSERT_EQ(3u, defs.size());
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "worldspawn"; }));
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "info_player_start"; }));
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "info_player_coop"; }));

            kdl::vec_clear_and_delete(defs);
        }

        TEST_CASE("FgdParserTest.parseRecursiveInclude", "[FgdParserTest]") {
            const Path path = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Fgd/parseRecursiveInclude/host.fgd");
            auto file = Disk::openFile(path);
            auto reader = file->reader().buffer();

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(std::begin(reader), std::end(reader), defaultColor, file->path());

            TestParserStatus status;
            auto defs = parser.parseDefinitions(status);
            ASSERT_EQ(1u, defs.size());
            ASSERT_TRUE(std::any_of(std::begin(defs), std::end(defs), [](const auto* def) { return def->name() == "worldspawn"; }));

            kdl::vec_clear_and_delete(defs);
        }

        TEST_CASE("FgdParserTest.parseStringContinuations", "[FgdParserTest]") {
            const std::string file =
                "@PointClass = cont_description :\n"
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
            ASSERT_EQ(1u, definitions.size());

            const auto* definition = definitions.front();
            ASSERT_EQ("This is an example description for this example entity. It will appear in the help dialog for this entity", definition->description());

            kdl::vec_clear_and_delete(definitions);
        }
    }
}
