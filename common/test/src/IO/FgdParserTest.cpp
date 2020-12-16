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

#include "TestUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "Assets/PropertyDefinition.h"
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

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("FgdParserTest.parseIncludedFgdFiles", "[FgdParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const std::vector<Path> cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("fgd"));

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

            const auto& propertyDefinitions = definition->propertyDefinitions();
            ASSERT_EQ(6u, propertyDefinitions.size());

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

            const auto& propertyDefinitions = definition->propertyDefinitions();
            ASSERT_EQ(5u, propertyDefinitions.size());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseBaseProperty", "[FgdParserTest]") {
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

            const auto& propertyDefinitions = definition->propertyDefinitions();
            ASSERT_EQ(9u, propertyDefinitions.size());

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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            const auto& propertyDefinitions = definition->propertyDefinitions();
            ASSERT_EQ(1u, propertyDefinitions.size());

            auto propertyDefinition = propertyDefinitions[0];
            ASSERT_EQ(Assets::PropertyDefinitionType::TargetSourceProperty, propertyDefinition->type());
            ASSERT_EQ(std::string("targetname"), propertyDefinition->key());
            ASSERT_EQ(std::string("Source"), propertyDefinition->shortDescription());
            ASSERT_EQ(std::string("A long description"), propertyDefinition->longDescription());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseType_TargetDestinationPropertyDefinition", "[FgdParserTest]") {
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

            const auto& propertyDefinitions = definition->propertyDefinitions();
            ASSERT_EQ(1u, propertyDefinitions.size());

            auto propertyDefinition = propertyDefinitions[0];
            ASSERT_EQ(Assets::PropertyDefinitionType::TargetDestinationProperty, propertyDefinition->type());
            ASSERT_EQ(std::string("target"), propertyDefinition->key());
            ASSERT_EQ(std::string("Target"), propertyDefinition->shortDescription());
            ASSERT_EQ(std::string(""), propertyDefinition->longDescription());

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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("message");
            ASSERT_TRUE(propertyDefinition1 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, propertyDefinition1->type());

            const Assets::StringPropertyDefinition* stringPropertyDefinition1 = static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition1);
            ASSERT_EQ(std::string("message"), stringPropertyDefinition1->key());
            ASSERT_EQ(std::string("Text on entering the world"), stringPropertyDefinition1->shortDescription());
            ASSERT_EQ(std::string("Long description 1"), stringPropertyDefinition1->longDescription());
            ASSERT_FALSE(stringPropertyDefinition1->hasDefaultValue());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("message2");
            ASSERT_TRUE(propertyDefinition2 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, propertyDefinition2->type());

            const Assets::StringPropertyDefinition* stringPropertyDefinition2 = static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition2);
            ASSERT_EQ(std::string("message2"), stringPropertyDefinition2->key());
            ASSERT_EQ(std::string("With a default value"), stringPropertyDefinition2->shortDescription());
            ASSERT_EQ(std::string("Long description 2"), stringPropertyDefinition2->longDescription());
            ASSERT_TRUE(stringPropertyDefinition2->hasDefaultValue());
            ASSERT_EQ(std::string("DefaultValue"), stringPropertyDefinition2->defaultValue());

            kdl::vec_clear_and_delete(definitions);
        }

        /**
         * Support having an integer (or decimal) as a default for a string propertyDefinition. Technically
         * a type mismatch, but appears in the wild; see: https://github.com/TrenchBroom/TrenchBroom/issues/2833
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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("name");
            ASSERT_TRUE(propertyDefinition1 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, propertyDefinition1->type());

            const Assets::StringPropertyDefinition* stringPropertyDefinition1 = static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition1);
            ASSERT_EQ(std::string("name"), stringPropertyDefinition1->key());
            ASSERT_EQ(std::string("Description"), stringPropertyDefinition1->shortDescription());
            ASSERT_EQ(std::string(), stringPropertyDefinition1->longDescription());
            ASSERT_TRUE(stringPropertyDefinition1->hasDefaultValue());
            ASSERT_EQ(std::string("3"), stringPropertyDefinition1->defaultValue());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("other");
            ASSERT_TRUE(propertyDefinition2 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::StringProperty, propertyDefinition2->type());

            const Assets::StringPropertyDefinition* stringPropertyDefinition2 = static_cast<const Assets::StringPropertyDefinition*>(propertyDefinition2);
            ASSERT_EQ(std::string("other"), stringPropertyDefinition2->key());
            ASSERT_EQ(std::string(), stringPropertyDefinition2->shortDescription());
            ASSERT_EQ(std::string(), stringPropertyDefinition2->longDescription());
            ASSERT_TRUE(stringPropertyDefinition2->hasDefaultValue());
            ASSERT_EQ(std::string("1.5"), stringPropertyDefinition2->defaultValue());

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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(2u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("sounds");
            ASSERT_TRUE(propertyDefinition1 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::IntegerProperty, propertyDefinition1->type());

            const Assets::IntegerPropertyDefinition* intPropertyDefinition1 = static_cast<const Assets::IntegerPropertyDefinition*>(propertyDefinition1);
            ASSERT_EQ(std::string("sounds"), intPropertyDefinition1->key());
            ASSERT_EQ(std::string("CD track to play"), intPropertyDefinition1->shortDescription());
            ASSERT_EQ(std::string("Longer description"), intPropertyDefinition1->longDescription());
            ASSERT_FALSE(intPropertyDefinition1->hasDefaultValue());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("sounds2");
            ASSERT_TRUE(propertyDefinition2 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::IntegerProperty, propertyDefinition2->type());

            const Assets::IntegerPropertyDefinition* intPropertyDefinition2 = static_cast<const Assets::IntegerPropertyDefinition*>(propertyDefinition2);
            ASSERT_EQ(std::string("sounds2"), intPropertyDefinition2->key());
            ASSERT_EQ(std::string("CD track to play with default"), intPropertyDefinition2->shortDescription());
            ASSERT_EQ(std::string("Longer description"), intPropertyDefinition2->longDescription());
            ASSERT_TRUE(intPropertyDefinition2->hasDefaultValue());
            ASSERT_EQ(2, intPropertyDefinition2->defaultValue());

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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(2u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("sounds");
            ASSERT_TRUE(propertyDefinition1->readOnly());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("sounds2");
            ASSERT_FALSE(propertyDefinition2->readOnly());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseFloatPropertyDefinition", "[FgdParserTest]") {
            const std::string file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   test(float) : \"Some test propertyDefinition\" : : \"Longer description 1\"\n"
            "   test2(float) : \"Some test propertyDefinition with default\" : \"2.7\" : \"Longer description 2\"\n"
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

            ASSERT_EQ(2u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("test");
            ASSERT_TRUE(propertyDefinition1 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::FloatProperty, propertyDefinition1->type());

            const Assets::FloatPropertyDefinition* floatPropertyDefinition1 = static_cast<const Assets::FloatPropertyDefinition*>(propertyDefinition1);
            ASSERT_EQ(std::string("test"), floatPropertyDefinition1->key());
            ASSERT_EQ(std::string("Some test propertyDefinition"), floatPropertyDefinition1->shortDescription());
            ASSERT_EQ(std::string("Longer description 1"), floatPropertyDefinition1->longDescription());
            ASSERT_FALSE(floatPropertyDefinition1->hasDefaultValue());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("test2");
            ASSERT_TRUE(propertyDefinition2 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::FloatProperty, propertyDefinition2->type());

            const Assets::FloatPropertyDefinition* floatPropertyDefinition2 = static_cast<const Assets::FloatPropertyDefinition*>(propertyDefinition2);
            ASSERT_EQ(std::string("test2"), floatPropertyDefinition2->key());
            ASSERT_EQ(std::string("Some test propertyDefinition with default"), floatPropertyDefinition2->shortDescription());
            ASSERT_EQ(std::string("Longer description 2"), floatPropertyDefinition2->longDescription());
            ASSERT_TRUE(floatPropertyDefinition2->hasDefaultValue());
            ASSERT_FLOAT_EQ(2.7f, floatPropertyDefinition2->defaultValue());

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
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(std::string("Wildcard entity"), definition->description());

            ASSERT_EQ(5u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition1 = definition->propertyDefinition("worldtype");
            ASSERT_TRUE(propertyDefinition1 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::ChoiceProperty, propertyDefinition1->type());

            const Assets::ChoicePropertyDefinition* choicePropertyDefinition1 = static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition1);
            ASSERT_EQ(std::string("worldtype"), choicePropertyDefinition1->key());
            ASSERT_EQ(std::string("Ambience"), choicePropertyDefinition1->shortDescription());
            ASSERT_EQ(std::string("Long description 1"), choicePropertyDefinition1->longDescription());
            ASSERT_FALSE(choicePropertyDefinition1->hasDefaultValue());

            const Assets::ChoicePropertyOption::List& options1 = choicePropertyDefinition1->options();
            ASSERT_EQ(3u, options1.size());
            ASSERT_EQ(std::string("0"), options1[0].value());
            ASSERT_EQ(std::string("Medieval"), options1[0].description());
            ASSERT_EQ(std::string("1"), options1[1].value());
            ASSERT_EQ(std::string("Metal (runic)"), options1[1].description());
            ASSERT_EQ(std::string("2"), options1[2].value());
            ASSERT_EQ(std::string("Base"), options1[2].description());

            const Assets::PropertyDefinition* propertyDefinition2 = definition->propertyDefinition("worldtype2");
            ASSERT_TRUE(propertyDefinition2 != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::ChoiceProperty, propertyDefinition2->type());

            const Assets::ChoicePropertyDefinition* choicePropertyDefinition2 = static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition2);
            ASSERT_EQ(std::string("worldtype2"), choicePropertyDefinition2->key());
            ASSERT_EQ(std::string("Ambience with default"), choicePropertyDefinition2->shortDescription());
            ASSERT_EQ(std::string("Long description 2"), choicePropertyDefinition2->longDescription());
            ASSERT_TRUE(choicePropertyDefinition2->hasDefaultValue());
            ASSERT_EQ("1", choicePropertyDefinition2->defaultValue());

            const Assets::ChoicePropertyOption::List& options2 = choicePropertyDefinition2->options();
            ASSERT_EQ(2u, options2.size());
            ASSERT_EQ(std::string("0"), options2[0].value());
            ASSERT_EQ(std::string("Medieval"), options2[0].description());
            ASSERT_EQ(std::string("1"), options2[1].value());
            ASSERT_EQ(std::string("Metal (runic)"), options2[1].description());

            const Assets::PropertyDefinition* propertyDefinition3 = definition->propertyDefinition("puzzle_id");
            const Assets::ChoicePropertyDefinition* choicePropertyDefinition3 = static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition3);
            ASSERT_EQ(std::string("puzzle_id"), choicePropertyDefinition3->key());
            ASSERT_EQ(std::string("Puzzle id"), choicePropertyDefinition3->shortDescription());
            ASSERT_EQ(std::string(""), choicePropertyDefinition3->longDescription());
            ASSERT_TRUE(choicePropertyDefinition3->hasDefaultValue());
            ASSERT_EQ("cskey", choicePropertyDefinition3->defaultValue());

            const Assets::ChoicePropertyOption::List& options3 = choicePropertyDefinition3->options();
            ASSERT_EQ(3u, options3.size());
            ASSERT_EQ(std::string("keep3"), options3[0].value());
            ASSERT_EQ(std::string("Mill key"), options3[0].description());
            ASSERT_EQ(std::string("cskey"), options3[1].value());
            ASSERT_EQ(std::string("Castle key"), options3[1].description());
            ASSERT_EQ(std::string("scrol"), options3[2].value());
            ASSERT_EQ(std::string("Disrupt Magic Scroll"), options3[2].description());

            const Assets::PropertyDefinition* propertyDefinition4 = definition->propertyDefinition("floaty");
            const Assets::ChoicePropertyDefinition* choicePropertyDefinition4 = static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition4);
            ASSERT_EQ(std::string("floaty"), choicePropertyDefinition4->key());
            ASSERT_EQ(std::string("Floaty"), choicePropertyDefinition4->shortDescription());
            ASSERT_EQ(std::string(""), choicePropertyDefinition4->longDescription());
            ASSERT_TRUE(choicePropertyDefinition4->hasDefaultValue());
            ASSERT_EQ("2.3", choicePropertyDefinition4->defaultValue());

            const Assets::ChoicePropertyOption::List& options4 = choicePropertyDefinition4->options();
            ASSERT_EQ(3u, options4.size());
            ASSERT_EQ(std::string("1.0"), options4[0].value());
            ASSERT_EQ(std::string("Something"), options4[0].description());
            ASSERT_EQ(std::string("2.3"), options4[1].value());
            ASSERT_EQ(std::string("Something else"), options4[1].description());
            ASSERT_EQ(std::string("0.1"), options4[2].value());
            ASSERT_EQ(std::string("Yet more"), options4[2].description());

            const Assets::PropertyDefinition* propertyDefinition5 = definition->propertyDefinition("negative");
            const Assets::ChoicePropertyDefinition* choicePropertyDefinition5 = static_cast<const Assets::ChoicePropertyDefinition*>(propertyDefinition5);
            ASSERT_EQ(std::string("negative"), choicePropertyDefinition5->key());
            ASSERT_EQ(std::string("Negative values"), choicePropertyDefinition5->shortDescription());
            ASSERT_EQ(std::string(""), choicePropertyDefinition5->longDescription());
            ASSERT_TRUE(choicePropertyDefinition5->hasDefaultValue());
            ASSERT_EQ("-1", choicePropertyDefinition5->defaultValue());

            const Assets::ChoicePropertyOption::List& options5 = choicePropertyDefinition5->options();
            ASSERT_EQ(3u, options5.size());
            ASSERT_EQ(std::string("-2"), options5[0].value());
            ASSERT_EQ(std::string("Something"), options5[0].description());
            ASSERT_EQ(std::string("-1"), options5[1].value());
            ASSERT_EQ(std::string("Something else"), options5[1].description());
            ASSERT_EQ(std::string("1"), options5[2].value());
            ASSERT_EQ(std::string("Yet more"), options5[2].description());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("FgdParserTest.parseFlagsPropertyDefinition", "[FgdParserTest]") {
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

            ASSERT_EQ(1u, definition->propertyDefinitions().size());

            const Assets::PropertyDefinition* propertyDefinition = definition->propertyDefinition("spawnflags");
            ASSERT_TRUE(propertyDefinition != nullptr);
            ASSERT_EQ(Assets::PropertyDefinitionType::FlagsProperty, propertyDefinition->type());

            const Assets::FlagsPropertyDefinition* flagsPropertyDefinition = static_cast<const Assets::FlagsPropertyDefinition*>(propertyDefinition);
            ASSERT_EQ(std::string("spawnflags"), flagsPropertyDefinition->key());
            ASSERT_EQ(std::string(""), flagsPropertyDefinition->shortDescription());
            ASSERT_EQ(2560, flagsPropertyDefinition->defaultValue());

            const Assets::FlagsPropertyOption::List& options = flagsPropertyDefinition->options();
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
            FgdParser parser(reader.stringView(), defaultColor, file->path());

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
            FgdParser parser(reader.stringView(), defaultColor, file->path());

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
            FgdParser parser(reader.stringView(), defaultColor, file->path());

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
