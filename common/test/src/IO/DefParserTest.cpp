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
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "IO/DefParser.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "Model/EntityAttributes.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("DefParserTest.parseIncludedDefFiles", "[DefParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const std::vector<Path> cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("def"));

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                DefParser parser(std::begin(reader), std::end(reader), defaultColor);

                TestParserStatus status;
                UNSCOPED_INFO("Parsing DEF file " << path.asString() << " failed");
                CHECK_NOTHROW(parser.parseDefinitions(status));

                /* Disabled because our files are full of previously undetected problems
                if (status.countStatus(LogLevel::Warn) > 0u) {
                    UNSCOPED_INFO("Parsing DEF file " << path.asString() << " produced warnings");
                    for (const auto& message : status.messages(LogLevel::Warn)) {
                        UNSCOPED_INFO(message);
                    }
                    CHECK(status.countStatus(LogLevel::Warn) == 0u);
                }

                if (status.countStatus(LogLevel::Error) > 0u) {
                    UNSCOPED_INFO("Parsing DEF file " << path.asString() << " produced errors");
                    for (const auto& message : status.messages(LogLevel::Error)) {
                        UNSCOPED_INFO(message);
                    }
                    CHECK(status.countStatus(LogLevel::Error) == 0u);
                }
                */
            }
        }

        TEST_CASE("DefParserTest.parseExtraDefFiles", "[DefParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Def");
            const std::vector<Path> cfgFiles = Disk::findItems(basePath, [] (const Path& path, bool directory) {
                return !directory && kdl::ci::str_is_equal(path.extension(), "def");
            });

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                DefParser parser(std::begin(reader), std::end(reader), defaultColor);

                TestParserStatus status;
                ASSERT_NO_THROW(parser.parseDefinitions(status));
                ASSERT_EQ(0u, status.countStatus(LogLevel::Warn));
                ASSERT_EQ(0u, status.countStatus(LogLevel::Error));
            }
        }

        TEST_CASE("DefParserTest.parseEmptyFile", "[DefParserTest]") {
            const std::string file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseWhitespaceFile", "[DefParserTest]") {
            const std::string file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseCommentsFile", "[DefParserTest]") {
            const std::string file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseSolidClass", "[DefParserTest]") {
            const std::string file =
            "/*QUAKED worldspawn (0.0 0.0 0.0) ?\n"
            "{\n"
            "choice \"worldtype\"\n"
            " (\n"
            "  (0,\"medieval\")\n"
            "  (1,\"metal\")\n"
            "  (2,\"base\")\n"
            " );\n"
            "}\n"
            "Only used for the world entity. "
            "Set message to the level name. "
            "Set sounds to the cd track to play. "
            "\"worldtype\"	type of world\n"
            "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::BrushEntity, definition->type());
            ASSERT_EQ(std::string("worldspawn"), definition->name());
            ASSERT_VEC_EQ(Color(0.0f, 0.0f, 0.0f, 1.0f), definition->color());
            ASSERT_EQ(std::string("Only used for the world entity. "
                             "Set message to the level name. "
                             "Set sounds to the cd track to play. "
                             "\"worldtype\"	type of world"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());

            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parsePointClass", "[DefParserTest]") {
            const std::string file =
            "/*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush\n"
            "If crucified, stick the bounding box 12 pixels back into a wall to look right.\n"
            "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("monster_zombie"), definition->name());
            ASSERT_VEC_EQ(Color(1.0f, 0.0f, 0.0f, 1.0f), definition->color());
            ASSERT_EQ(std::string("If crucified, stick the bounding box 12 pixels back into a wall to look right."), definition->description());

            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            ASSERT_VEC_EQ(vm::vec3(-16.0, -16.0, -24.0), pointDefinition->bounds().min);
            ASSERT_VEC_EQ(vm::vec3(16.0, 16.0, 32.0), pointDefinition->bounds().max);

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags

            const auto attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinitionType::FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());

            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(2u, options.size());
            ASSERT_EQ(1, options[0].value());

            ASSERT_EQ(std::string("Crucified"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(std::string("ambush"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());


            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseSpawnflagWithSkip", "[DefParserTest]") {
            const std::string file =
                    "/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) - SUSPENDED SPIN - RESPAWN\n"
                    "some desc\n"
                    "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::PointEntity, definition->type());
            ASSERT_EQ(std::string("item_health"), definition->name());
            ASSERT_VEC_EQ(Color(0.3f, 0.3f, 1.0f, 1.0f), definition->color());
            ASSERT_EQ(std::string("some desc"), definition->description());

            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            ASSERT_VEC_EQ(vm::vec3(-16.0, -16.0, -16.0), pointDefinition->bounds().min);
            ASSERT_VEC_EQ(vm::vec3(16.0, 16.0, 16.0), pointDefinition->bounds().max);

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags

            const auto attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinitionType::FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());

            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(5u, options.size());

            ASSERT_EQ(std::string(""), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(std::string("SUSPENDED"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(std::string("SPIN"), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(4, options[2].value());
            ASSERT_EQ(std::string(""), options[3].shortDescription());
            ASSERT_FALSE(options[3].isDefault());
            ASSERT_EQ(8, options[3].value());
            ASSERT_EQ(std::string("RESPAWN"), options[4].shortDescription());
            ASSERT_FALSE(options[4].isDefault());
            ASSERT_EQ(16, options[4].value());


            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseBrushEntityWithMissingBBoxAndNoQuestionMark", "[DefParserTest]") {
            const std::string file =
                    "/*QUAKED item_health (.3 .3 1) SUSPENDED SPIN - RESPAWN\n"
                    "some desc\n"
                    "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinitionType::BrushEntity, definition->type());
            ASSERT_EQ(std::string("item_health"), definition->name());
            ASSERT_VEC_EQ(Color(0.3f, 0.3f, 1.0f, 1.0f), definition->color());
            ASSERT_EQ(std::string("some desc"), definition->description());

            const auto& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags

            const auto attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinitionType::FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());

            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(4u, options.size());

            ASSERT_EQ(std::string("SUSPENDED"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(std::string("SPIN"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(std::string(""), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(4, options[2].value());
            ASSERT_EQ(std::string("RESPAWN"), options[3].shortDescription());
            ASSERT_FALSE(options[3].isDefault());
            ASSERT_EQ(8, options[3].value());


            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parsePointClassWithBaseClasses", "[DefParserTest]") {
            const std::string file =
            "/*QUAKED _light_style\n"
            "{\n"
            "choice \"style\"\n"
            " (\n"
            "  (0,\"normal\")\n"
            "  (1,\"flicker (first variety)\")\n"
            "  (2,\"slow strong pulse\")\n"
            "  (3,\"candle (first variety)\")\n"
            "  (4,\"fast strobe\")\n"
            "  (5,\"gentle pulse 1\")\n"
            "  (6,\"flicker (second variety)\")\n"
            "  (7,\"candle (second variety)\")\n"
            "  (8,\"candle (third variety)\")\n"
            "  (9,\"slow strobe (fourth variety)\")\n"
            "  (10,\"fluorescent flicker\")\n"
            "  (11,\"slow pulse not fade to black\")\n"
            " );\n"
            "}\n"
            "*/\n"
            "\n"
            "/*QUAKED light (0.0 1.0 0.0) (-8 -8 -8) (8 8 8) START_OFF\n"
            "{\n"
            "base(\"_light_style\");\n"
            "}\n"
            "Non-displayed light.\n"
            "Default light value is 300\n"
            "If targeted, it will toggle between on or off.\n"
            "Default \"style\" is 0.\n"
            "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            CHECK(definitions.size() == 1u);

            Assets::EntityDefinition* definition = definitions[0];
            CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
            CHECK(definition->name() == "light");

            CHECK(definition->attributeDefinitions().size() == 2u);

            const auto* styleAttribute = definition->attributeDefinition("style");
            CHECK(styleAttribute != nullptr);
            CHECK(styleAttribute->name() == "style");
            CHECK(styleAttribute->type() == Assets::AttributeDefinitionType::ChoiceAttribute);

            const auto* spawnflagsAttribute = definition->attributeDefinition(Model::AttributeNames::Spawnflags);
            CHECK(spawnflagsAttribute != nullptr);
            CHECK(spawnflagsAttribute->name() == Model::AttributeNames::Spawnflags);
            CHECK(spawnflagsAttribute->type() == Assets::AttributeDefinitionType::FlagsAttribute);

            const Assets::ChoiceAttributeDefinition* choice = static_cast<const Assets::ChoiceAttributeDefinition*>(styleAttribute);
            CHECK(choice->options().size() == 12u);

            kdl::vec_clear_and_delete(definitions);
        }

        static const std::string DefModelDefinitionTemplate =
        "/*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush\n"
        "{\n"
        "model(${MODEL});\n"
        "}\n"
        "*/\n";

        using Assets::assertModelDefinition;

        TEST_CASE("DefParserTest.parseLegacyStaticModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "\":maps/b_shell0.bsp\", \":maps/b_shell1.bsp\" spawnflags = 1";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate);
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
        }

        TEST_CASE("DefParserTest.parseLegacyDynamicModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\"";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST_CASE("DefParserTest.parseELStaticModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "{{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }}";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate);
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'spawnflags': 2 }");
        }

        TEST_CASE("DefParserTest.parseELDynamicModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "{ 'path': model, 'skin': skin, 'frame': frame }";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST_CASE("DefParserTest.parseInvalidBounds", "[DefParserTest]") {
            const std::string file =
                "/*QUAKED light (0.0 1.0 0.0) (8 -8 -8) (-8 8 8) START_OFF\n"
                "{\n"
                "base(\"_light_style\");\n"
                "}\n"
                "Non-displayed light.\n"
                "Default light value is 300\n"
                "If targeted, it will toggle between on or off.\n"
                "Default \"style\" is 0.\n"
                "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
            ASSERT_EQ(vm::bbox3d(8.0), definition->bounds());

            kdl::vec_clear_and_delete(definitions);
        }
    }
}
