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

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "Model/ModelTypes.h"
#include "IO/DefParser.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        TEST(DefParserTest, parseIncludedDefFiles) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("data/games");
            const Path::List cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("def"));

            for (const Path& path : cfgFiles) {
                MappedFile::Ptr file = Disk::openFile(path);
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                DefParser parser(file->begin(), file->end(), defaultColor);

                TestParserStatus status;
                ASSERT_NO_THROW(parser.parseDefinitions(status)) << "Parsing DEF file " << path.asString() << " failed";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Warn)) << "Parsing DEF file " << path.asString() << " produced warnings";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Error)) << "Parsing DEF file " << path.asString() << " produced errors";
            }
        }

        TEST(DefParserTest, parseExtraDefFiles) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("data/IO/Def");
            const Path::List cfgFiles = Disk::findItems(basePath, [] (const Path& path, bool directory) {
                return !directory && StringUtils::caseInsensitiveEqual(path.extension(), "def");
            });

            for (const Path& path : cfgFiles) {
                MappedFile::Ptr file = Disk::openFile(path);
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                DefParser parser(file->begin(), file->end(), defaultColor);

                TestParserStatus status;
                ASSERT_NO_THROW(parser.parseDefinitions(status));
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Warn));
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Error));
            }
        }

        TEST(DefParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseCommentsFile) {
            const String file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseSolidClass) {
            const String file =
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
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_BrushEntity, definition->type());
            ASSERT_EQ(String("worldspawn"), definition->name());
            ASSERT_VEC_EQ(Color(0.0f, 0.0f, 0.0f, 1.0f), definition->color());
            ASSERT_EQ(String("Only used for the world entity. "
                             "Set message to the level name. "
                             "Set sounds to the cd track to play. "
                             "\"worldtype\"	type of world"), definition->description());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(DefParserTest, parsePointClass) {
            const String file =
            "/*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush\n"
            "If crucified, stick the bounding box 12 pixels back into a wall to look right.\n"
            "*/\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("monster_zombie"), definition->name());
            ASSERT_VEC_EQ(Color(1.0f, 0.0f, 0.0f, 1.0f), definition->color());
            ASSERT_EQ(String("If crucified, stick the bounding box 12 pixels back into a wall to look right."), definition->description());
            
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            ASSERT_VEC_EQ(vm::vec3(-16.0, -16.0, -24.0), pointDefinition->bounds().min);
            ASSERT_VEC_EQ(vm::vec3(16.0, 16.0, 32.0), pointDefinition->bounds().max);
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags
            
            const Assets::AttributeDefinitionPtr attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinition::Type_FlagsAttribute, attribute->type());
            
            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());
            
            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(2u, options.size());
            ASSERT_EQ(1, options[0].value());

            ASSERT_EQ(String("Crucified"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(String("ambush"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());
            
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(DefParserTest, parseSpawnflagWithSkip) {
            const String file =
                    "/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) - SUSPENDED SPIN - RESPAWN\n"
                    "some desc\n"
                    "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("item_health"), definition->name());
            ASSERT_VEC_EQ(Color(0.3f, 0.3f, 1.0f, 1.0f), definition->color());
            ASSERT_EQ(String("some desc"), definition->description());

            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(definition);
            ASSERT_VEC_EQ(vm::vec3(-16.0, -16.0, -16.0), pointDefinition->bounds().min);
            ASSERT_VEC_EQ(vm::vec3(16.0, 16.0, 16.0), pointDefinition->bounds().max);

            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags

            const Assets::AttributeDefinitionPtr attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinition::Type_FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());

            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(5u, options.size());

            ASSERT_EQ(String(""), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(String("SUSPENDED"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(String("SPIN"), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(4, options[2].value());
            ASSERT_EQ(String(""), options[3].shortDescription());
            ASSERT_FALSE(options[3].isDefault());
            ASSERT_EQ(8, options[3].value());
            ASSERT_EQ(String("RESPAWN"), options[4].shortDescription());
            ASSERT_FALSE(options[4].isDefault());
            ASSERT_EQ(16, options[4].value());


            VectorUtils::clearAndDelete(definitions);
        }

        TEST(DefParserTest, parseBrushEntityWithMissingBBoxAndNoQuestionMark) {
            const String file =
                    "/*QUAKED item_health (.3 .3 1) SUSPENDED SPIN - RESPAWN\n"
                    "some desc\n"
                    "*/\n";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_BrushEntity, definition->type());
            ASSERT_EQ(String("item_health"), definition->name());
            ASSERT_VEC_EQ(Color(0.3f, 0.3f, 1.0f, 1.0f), definition->color());
            ASSERT_EQ(String("some desc"), definition->description());

            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size()); // spawnflags

            const Assets::AttributeDefinitionPtr attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinition::Type_FlagsAttribute, attribute->type());

            const Assets::FlagsAttributeDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != nullptr);
            ASSERT_EQ(0, spawnflags->defaultValue());

            const Assets::FlagsAttributeOption::List& options = spawnflags->options();
            ASSERT_EQ(4u, options.size());

            ASSERT_EQ(String("SUSPENDED"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(String("SPIN"), options[1].shortDescription());
            ASSERT_FALSE(options[1].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(String(""), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(4, options[2].value());
            ASSERT_EQ(String("RESPAWN"), options[3].shortDescription());
            ASSERT_FALSE(options[3].isDefault());
            ASSERT_EQ(8, options[3].value());


            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parsePointClassWithBaseClasses) {
            const String file =
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
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("light"), definition->name());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(2u, attributes.size()); // spawn flags and style
            
            Assets::AttributeDefinitionPtr spawnflags = attributes[0];
            ASSERT_EQ(Model::AttributeNames::Spawnflags, spawnflags->name());
            ASSERT_EQ(Assets::AttributeDefinition::Type_FlagsAttribute, spawnflags->type());

            Assets::AttributeDefinitionPtr style = attributes[1];
            ASSERT_EQ(String("style"), style->name());
            ASSERT_EQ(Assets::AttributeDefinition::Type_ChoiceAttribute, style->type());
            
            const Assets::ChoiceAttributeDefinition* choice = static_cast<const Assets::ChoiceAttributeDefinition*>(definition->attributeDefinition("style"));
            ASSERT_EQ(12u, choice->options().size());
            
            VectorUtils::clearAndDelete(definitions);
        }
        
        static const String ModelDefinitionTemplate =
        "/*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush\n"
        "{\n"
        "model(${MODEL});\n"
        "}\n"
        "*/\n";
        
        using Assets::assertModelDefinition;
        
        TEST(DefParserTest, parseLegacyStaticModelDefinition) {
            static const String ModelDefinition = "\":maps/b_shell0.bsp\", \":maps/b_shell1.bsp\" spawnflags = 1";
            
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate);
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
        }
        
        TEST(DefParserTest, parseLegacyDynamicModelDefinition) {
            static const String ModelDefinition = "pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\"";
            
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }
        
        TEST(DefParserTest, parseELStaticModelDefinition) {
            static const String ModelDefinition = "{{ spawnflags == 1 -> 'maps/b_shell1.bsp', 'maps/b_shell0.bsp' }}";
            
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate);
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'spawnflags': 1 }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell0.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'spawnflags': 2 }");
        }
        
        TEST(DefParserTest, parseELDynamicModelDefinition) {
            static const String ModelDefinition = "{ 'path': model, 'skin': skin, 'frame': frame }";
            
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp")),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp' }");
            assertModelDefinition<DefParser>(Assets::ModelSpecification(IO::Path("maps/b_shell1.bsp"), 1, 2),
                                             ModelDefinition,
                                             ModelDefinitionTemplate,
                                             "{ 'model': 'maps/b_shell1.bsp', 'skin': 1, 'frame': 2 }");
        }

        TEST(DefParserTest, parseInvalidBounds) {
            const String file =
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
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            const auto definition = static_cast<Assets::PointEntityDefinition*>(definitions[0]);
            ASSERT_EQ(vm::bbox3d(8.0), definition->bounds());

            VectorUtils::clearAndDelete(definitions);
        }
    }
}
