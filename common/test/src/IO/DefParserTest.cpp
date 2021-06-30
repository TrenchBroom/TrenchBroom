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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "Assets/PropertyDefinition.h"
#include "IO/DefParser.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "Model/EntityProperties.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("DefParserTest.parseIncludedDefFiles", "[DefParserTest]") {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const std::vector<Path> cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("def"));

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                DefParser parser(reader.stringView(), defaultColor);

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
                DefParser parser(reader.stringView(), defaultColor);

                TestParserStatus status;
                CHECK_NOTHROW(parser.parseDefinitions(status));
                CHECK(status.countStatus(LogLevel::Warn) == 0u);
                CHECK(status.countStatus(LogLevel::Error) == 0u);
            }
        }

        TEST_CASE("DefParserTest.parseEmptyFile", "[DefParserTest]") {
            const std::string file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            CHECK(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseWhitespaceFile", "[DefParserTest]") {
            const std::string file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            CHECK(definitions.empty());
            kdl::vec_clear_and_delete(definitions);
        }

        TEST_CASE("DefParserTest.parseCommentsFile", "[DefParserTest]") {
            const std::string file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            auto definitions = parser.parseDefinitions(status);
            CHECK(definitions.empty());
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
            CHECK(definitions.size() == 1u);

            Assets::EntityDefinition* definition = definitions[0];
            CHECK(definition->type() == Assets::EntityDefinitionType::BrushEntity);
            CHECK(definition->name() == std::string("worldspawn"));
            CHECK(definition->color() == Color(0.0f, 0.0f, 0.0f, 1.0f));
            CHECK(definition->description() == std::string("Only used for the world entity. "
                                                           "Set message to the level name. "
                                                           "Set sounds to the cd track to play. "
                                                           "\"worldtype\"	type of world"));

            const auto& properties = definition->propertyDefinitions();
            CHECK(properties.size() == 1u);

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
            CHECK(definitions.size() == 1u);

            Assets::EntityDefinition* definition = definitions[0];
            CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
            CHECK(definition->name() == std::string("monster_zombie"));
            CHECK(definition->color() == Color(1.0f, 0.0f, 0.0f, 1.0f));
            CHECK(definition->description() == std::string("If crucified, stick the bounding box 12 pixels back into a wall to look right."));

            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            CHECK(pointDefinition->bounds().min == vm::vec3(-16.0, -16.0, -24.0));
            CHECK(pointDefinition->bounds().max == vm::vec3(16.0, 16.0, 32.0));

            const auto& properties = definition->propertyDefinitions();
            CHECK(properties.size() == 1u); // spawnflags

            const auto property = properties[0];
            CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

            const Assets::FlagsPropertyDefinition* spawnflags = definition->spawnflags();
            CHECK(spawnflags != nullptr);
            CHECK(spawnflags->defaultValue() == 0);

            const Assets::FlagsPropertyOption::List& options = spawnflags->options();
            CHECK(options.size() == 2u);
            CHECK(options[0].value() == 1);

            CHECK(options[0].shortDescription() == std::string("Crucified"));
            CHECK_FALSE(options[0].isDefault());
            CHECK(options[1].value() == 2);
            CHECK(options[1].shortDescription() == std::string("ambush"));
            CHECK_FALSE(options[1].isDefault());


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
            CHECK(definitions.size() == 1u);

            Assets::EntityDefinition* definition = definitions[0];
            CHECK(definition->type() == Assets::EntityDefinitionType::PointEntity);
            CHECK(definition->name() == std::string("item_health"));
            CHECK(definition->color() == Color(0.3f, 0.3f, 1.0f, 1.0f));
            CHECK(definition->description() == std::string("some desc"));

            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            CHECK(pointDefinition->bounds().min == vm::vec3(-16.0, -16.0, -16.0));
            CHECK(pointDefinition->bounds().max == vm::vec3(16.0, 16.0, 16.0));

            const auto& properties = definition->propertyDefinitions();
            CHECK(properties.size() == 1u); // spawnflags

            const auto property = properties[0];
            CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

            const Assets::FlagsPropertyDefinition* spawnflags = definition->spawnflags();
            CHECK(spawnflags != nullptr);
            CHECK(spawnflags->defaultValue() == 0);

            const Assets::FlagsPropertyOption::List& options = spawnflags->options();
            CHECK(options.size() == 5u);

            CHECK(options[0].shortDescription() == std::string(""));
            CHECK_FALSE(options[0].isDefault());
            CHECK(options[0].value() == 1);
            CHECK(options[1].shortDescription() == std::string("SUSPENDED"));
            CHECK_FALSE(options[1].isDefault());
            CHECK(options[1].value() == 2);
            CHECK(options[2].shortDescription() == std::string("SPIN"));
            CHECK_FALSE(options[2].isDefault());
            CHECK(options[2].value() == 4);
            CHECK(options[3].shortDescription() == std::string(""));
            CHECK_FALSE(options[3].isDefault());
            CHECK(options[3].value() == 8);
            CHECK(options[4].shortDescription() == std::string("RESPAWN"));
            CHECK_FALSE(options[4].isDefault());
            CHECK(options[4].value() == 16);


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
            CHECK(definitions.size() == 1u);

            Assets::EntityDefinition* definition = definitions[0];
            CHECK(definition->type() == Assets::EntityDefinitionType::BrushEntity);
            CHECK(definition->name() == std::string("item_health"));
            CHECK(definition->color() == Color(0.3f, 0.3f, 1.0f, 1.0f));
            CHECK(definition->description() == std::string("some desc"));

            const auto& properties = definition->propertyDefinitions();
            CHECK(properties.size() == 1u); // spawnflags

            const auto property = properties[0];
            CHECK(property->type() == Assets::PropertyDefinitionType::FlagsProperty);

            const Assets::FlagsPropertyDefinition* spawnflags = definition->spawnflags();
            CHECK(spawnflags != nullptr);
            CHECK(spawnflags->defaultValue() == 0);

            const Assets::FlagsPropertyOption::List& options = spawnflags->options();
            CHECK(options.size() == 4u);

            CHECK(options[0].shortDescription() == std::string("SUSPENDED"));
            CHECK_FALSE(options[0].isDefault());
            CHECK(options[0].value() == 1);
            CHECK(options[1].shortDescription() == std::string("SPIN"));
            CHECK_FALSE(options[1].isDefault());
            CHECK(options[1].value() == 2);
            CHECK(options[2].shortDescription() == std::string(""));
            CHECK_FALSE(options[2].isDefault());
            CHECK(options[2].value() == 4);
            CHECK(options[3].shortDescription() == std::string("RESPAWN"));
            CHECK_FALSE(options[3].isDefault());
            CHECK(options[3].value() == 8);


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

            CHECK(definition->propertyDefinitions().size() == 2u);

            const auto* stylePropertyDefinition = definition->propertyDefinition("style");
            CHECK(stylePropertyDefinition != nullptr);
            CHECK(stylePropertyDefinition->key() == "style");
            CHECK(stylePropertyDefinition->type() == Assets::PropertyDefinitionType::ChoiceProperty);

            const auto* spawnflagsPropertyDefinition = definition->propertyDefinition(Model::PropertyKeys::Spawnflags);
            CHECK(spawnflagsPropertyDefinition != nullptr);
            CHECK(spawnflagsPropertyDefinition->key() == Model::PropertyKeys::Spawnflags);
            CHECK(spawnflagsPropertyDefinition->type() == Assets::PropertyDefinitionType::FlagsProperty);

            const Assets::ChoicePropertyDefinition* choice = static_cast<const Assets::ChoicePropertyDefinition*>(stylePropertyDefinition);
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

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp"), 0, 0),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate);
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 0, 0),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
        }

        TEST_CASE("DefParserTest.parseLegacyDynamicModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\"";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 0, 0),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST_CASE("DefParserTest.parseELModelDefinition", "[DefParserTest]") {
            static const std::string ModelDefinition = "{{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }}";

            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp"), 0, 0),
                                             ModelDefinition,
                                             DefModelDefinitionTemplate);
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
            CHECK(definitions.size() == 1u);

            const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
            CHECK(definition->bounds() == vm::bbox3d(8.0));

            kdl::vec_clear_and_delete(definitions);
        }
    }
}
