/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "IO/DefParser.h"
#include "Model/ModelTypes.h"
#include "Model/EntityDefinition.h"
#include "Model/PropertyDefinition.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        TEST(DefParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseCommentsFile) {
            const String file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::BrushEntity, definition->type());
            ASSERT_EQ(String("worldspawn"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Only used for the world entity. "
                             "Set message to the level name. "
                             "Set sounds to the cd track to play. "
                             "\"worldtype\"	type of world"), definition->description());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(1u, properties.size());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(DefParserTest, parsePointClass) {
            const String file =
            "/*QUAKED monster_zombie (1.0 0.0 0.0) (-16 -16 -24) (16 16 32) Crucified ambush\n"
            "If crucified, stick the bounding box 12 pixels back into a wall to look right.\n"
            "*/\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("monster_zombie"), definition->name());
            ASSERT_VEC_EQ(Color(1.0f, 0.0f, 0.0f, 1.0f), definition->color());
            ASSERT_EQ(String("If crucified, stick the bounding box 12 pixels back into a wall to look right."), definition->description());
            
            Model::PointEntityDefinition* pointDefinition = static_cast<Model::PointEntityDefinition*>(definition);
            ASSERT_VEC_EQ(Vec3(-16.0, -16.0, -24.0), pointDefinition->bounds().min);
            ASSERT_VEC_EQ(Vec3(16.0, 16.0, 32.0), pointDefinition->bounds().max);
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(1u, properties.size()); // spawnflags
            
            const Model::PropertyDefinitionPtr property = properties[0];
            ASSERT_EQ(Model::PropertyDefinition::FlagsProperty, property->type());
            
            const Model::FlagsPropertyDefinition* spawnflags = definition->spawnflags();
            ASSERT_TRUE(spawnflags != NULL);
            ASSERT_EQ(0, spawnflags->defaultValue());
            
            const Model::FlagsPropertyOption::List& options = spawnflags->options();
            ASSERT_EQ(2u, options.size());
            ASSERT_EQ(1, options[0].value());
            ASSERT_EQ(String("Crucified"), options[0].description());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(2, options[1].value());
            ASSERT_EQ(String("ambush"), options[1].description());
            ASSERT_FALSE(options[1].isDefault());
            
            
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("light"), definition->name());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(2u, properties.size()); // spawn flags and style
            
            Model::PropertyDefinitionPtr spawnflags = properties[0];
            ASSERT_EQ(Model::PropertyKeys::Spawnflags, spawnflags->name());
            ASSERT_EQ(Model::PropertyDefinition::FlagsProperty, spawnflags->type());

            Model::PropertyDefinitionPtr style = properties[1];
            ASSERT_EQ(String("style"), style->name());
            ASSERT_EQ(Model::PropertyDefinition::ChoiceProperty, style->type());
            
            const Model::ChoicePropertyDefinition* choice = static_cast<const Model::ChoicePropertyDefinition*>(definition->propertyDefinition("style"));
            ASSERT_EQ(12u, choice->options().size());
            
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(DefParserTest, parseStaticModelProperties) {
            const String file =
            "/*QUAKED item_cells (0 .5 .8) (0 0 0) (32 32 32) BIG\n"
            "{\n"
            "model(\":maps/b_batt0.bsp\");\n"
            "model(\":maps/b_batt1.bsp\" 0 0 spawnflags = 1);\n"
            "}\n"
            "6 ammo points (cells) for the\n"
            "Thunderbolt (Lightning).\n"
            "\n"
            "Flags:\n"
            "\"big\"\n"
            " gives 12 instead of 6\n"
            "*/\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("item_cells"), definition->name());
            
            const Model::ModelDefinitionList& models = static_cast<Model::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(2u, models.size());
        }
        
        TEST(DefParserTest, parseDynamicModelProperty) {
            const String file =
            "/*QUAKED item_cells (0 .5 .8) (0 0 0) (32 32 32) BIG\n"
            "{\n"
            "model(pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\");\n"
            "}\n"
            "6 ammo points (cells) for the\n"
            "Thunderbolt (Lightning).\n"
            "\n"
            "Flags:\n"
            "\"big\"\n"
            " gives 12 instead of 6\n"
            "*/\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            DefParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("item_cells"), definition->name());
            
            const Model::ModelDefinitionList& models = static_cast<Model::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(1u, models.size());
        }
    }
}
