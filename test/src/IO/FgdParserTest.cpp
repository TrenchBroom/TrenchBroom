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
#include "IO/FgdParser.h"
#include "Model/ModelTypes.h"
#include "Model/EntityDefinition.h"
#include "Model/PropertyDefinition.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        TEST(FgdParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseCommentsFile) {
            const String file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseSolidClass) {
            const String file =
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::BrushEntity, definition->type());
            ASSERT_EQ(String("worldspawn"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("World entity"), definition->description());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(6u, properties.size());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parsePointClass) {
            const String file =
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());

            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());

            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(5u, properties.size());
            
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseBaseClass) {
            const String file =
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parsePointClassWithBaseClasses) {
            const String file =
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(9u, properties.size());

            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseTargetSourceProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	targetname(target_source) : \"Source\" \n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(1u, properties.size());
            
            Model::PropertyDefinitionPtr property = properties[0];
            ASSERT_EQ(Model::PropertyDefinition::TargetSourceProperty, property->type());
            ASSERT_EQ(String("targetname"), property->name());
            ASSERT_EQ(String("Source"), property->description());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseTargetDestinationProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	target(target_destination) : \"Target\" \n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Model::PropertyDefinitionList& properties = definition->propertyDefinitions();
            ASSERT_EQ(1u, properties.size());
            
            Model::PropertyDefinitionPtr property = properties[0];
            ASSERT_EQ(Model::PropertyDefinition::TargetDestinationProperty, property->type());
            ASSERT_EQ(String("target"), property->name());
            ASSERT_EQ(String("Target"), property->description());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseStringProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   message(string) : \"Text on entering the world\"\n"
            "   message2(string) : \"With a default value\" : \"DefaultValue\"\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->propertyDefinitions().size());
            
            const Model::PropertyDefinition* property1 = definition->propertyDefinition("message");
            ASSERT_TRUE(property1 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::StringProperty, property1->type());

            const Model::StringPropertyDefinition* stringProperty1 = static_cast<const Model::StringPropertyDefinition*>(property1);
            ASSERT_EQ(String("message"), stringProperty1->name());
            ASSERT_EQ(String("Text on entering the world"), stringProperty1->description());
            ASSERT_FALSE(stringProperty1->hasDefaultValue());
            
            const Model::PropertyDefinition* property2 = definition->propertyDefinition("message2");
            ASSERT_TRUE(property2 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::StringProperty, property2->type());
            
            const Model::StringPropertyDefinition* stringProperty2 = static_cast<const Model::StringPropertyDefinition*>(property2);
            ASSERT_EQ(String("message2"), stringProperty2->name());
            ASSERT_EQ(String("With a default value"), stringProperty2->description());
            ASSERT_TRUE(stringProperty2->hasDefaultValue());
            ASSERT_EQ(String("DefaultValue"), stringProperty2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseIntegerProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   sounds(integer) : \"CD track to play\"\n"
            "   sounds2(integer) : \"CD track to play with default\" : 2\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->propertyDefinitions().size());
            
            const Model::PropertyDefinition* property1 = definition->propertyDefinition("sounds");
            ASSERT_TRUE(property1 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::IntegerProperty, property1->type());
            
            const Model::IntegerPropertyDefinition* intProperty1 = static_cast<const Model::IntegerPropertyDefinition*>(property1);
            ASSERT_EQ(String("sounds"), intProperty1->name());
            ASSERT_EQ(String("CD track to play"), intProperty1->description());
            ASSERT_FALSE(intProperty1->hasDefaultValue());
            
            const Model::PropertyDefinition* property2 = definition->propertyDefinition("sounds2");
            ASSERT_TRUE(property2 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::IntegerProperty, property2->type());
            
            const Model::IntegerPropertyDefinition* intProperty2 = static_cast<const Model::IntegerPropertyDefinition*>(property2);
            ASSERT_EQ(String("sounds2"), intProperty2->name());
            ASSERT_EQ(String("CD track to play with default"), intProperty2->description());
            ASSERT_TRUE(intProperty2->hasDefaultValue());
            ASSERT_EQ(2, intProperty2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseFloatProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   test(float) : \"Some test property\"\n"
            "   test2(float) : \"Some test property with default\" : \"2.7\"\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->propertyDefinitions().size());
            
            const Model::PropertyDefinition* property1 = definition->propertyDefinition("test");
            ASSERT_TRUE(property1 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::FloatProperty, property1->type());
            
            const Model::FloatPropertyDefinition* floatProperty1 = static_cast<const Model::FloatPropertyDefinition*>(property1);
            ASSERT_EQ(String("test"), floatProperty1->name());
            ASSERT_EQ(String("Some test property"), floatProperty1->description());
            ASSERT_FALSE(floatProperty1->hasDefaultValue());
            
            const Model::PropertyDefinition* property2 = definition->propertyDefinition("test2");
            ASSERT_TRUE(property2 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::FloatProperty, property2->type());
            
            const Model::FloatPropertyDefinition* floatProperty2 = static_cast<const Model::FloatPropertyDefinition*>(property2);
            ASSERT_EQ(String("test2"), floatProperty2->name());
            ASSERT_EQ(String("Some test property with default"), floatProperty2->description());
            ASSERT_TRUE(floatProperty2->hasDefaultValue());
            ASSERT_FLOAT_EQ(2.7, floatProperty2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseChoiceProperty) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   worldtype(choices) : \"Ambience\" =\n"
            "   [\n"
            "       0 : \"Medieval\"\n"
            "       1 : \"Metal (runic)\"\n"
            "       2 : \"Base\"\n"
            "   ]\n"
            "   worldtype2(choices) : \"Ambience with default\" : 1 =\n"
            "   [\n"
            "       0 : \"Medieval\"\n"
            "       1 : \"Metal (runic)\"\n"
            "   ]\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->propertyDefinitions().size());
            
            const Model::PropertyDefinition* property1 = definition->propertyDefinition("worldtype");
            ASSERT_TRUE(property1 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::ChoiceProperty, property1->type());
            
            const Model::ChoicePropertyDefinition* choiceProperty1 = static_cast<const Model::ChoicePropertyDefinition*>(property1);
            ASSERT_EQ(String("worldtype"), choiceProperty1->name());
            ASSERT_EQ(String("Ambience"), choiceProperty1->description());
            ASSERT_FALSE(choiceProperty1->hasDefaultValue());
            
            const Model::ChoicePropertyOption::List& options1 = choiceProperty1->options();
            ASSERT_EQ(3u, options1.size());
            ASSERT_EQ(String("0"), options1[0].value());
            ASSERT_EQ(String("Medieval"), options1[0].description());
            ASSERT_EQ(String("1"), options1[1].value());
            ASSERT_EQ(String("Metal (runic)"), options1[1].description());
            ASSERT_EQ(String("2"), options1[2].value());
            ASSERT_EQ(String("Base"), options1[2].description());
            
            const Model::PropertyDefinition* property2 = definition->propertyDefinition("worldtype2");
            ASSERT_TRUE(property2 != NULL);
            ASSERT_EQ(Model::PropertyDefinition::ChoiceProperty, property2->type());
            
            const Model::ChoicePropertyDefinition* choiceProperty2 = static_cast<const Model::ChoicePropertyDefinition*>(property2);
            ASSERT_EQ(String("worldtype2"), choiceProperty2->name());
            ASSERT_EQ(String("Ambience with default"), choiceProperty2->description());
            ASSERT_TRUE(choiceProperty2->hasDefaultValue());
            ASSERT_FLOAT_EQ(1, choiceProperty2->defaultValue());
            
            const Model::ChoicePropertyOption::List& options2 = choiceProperty1->options();
            ASSERT_EQ(3u, options2.size());
            ASSERT_EQ(String("0"), options2[0].value());
            ASSERT_EQ(String("Medieval"), options2[0].description());
            ASSERT_EQ(String("1"), options2[1].value());
            ASSERT_EQ(String("Metal (runic)"), options2[1].description());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseFlagsProperty) {
            const String file =
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
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(1u, definition->propertyDefinitions().size());
            
            const Model::PropertyDefinition* property = definition->propertyDefinition("spawnflags");
            ASSERT_TRUE(property != NULL);
            ASSERT_EQ(Model::PropertyDefinition::FlagsProperty, property->type());
            
            const Model::FlagsPropertyDefinition* flagsProperty = static_cast<const Model::FlagsPropertyDefinition*>(property);
            ASSERT_EQ(String("spawnflags"), flagsProperty->name());
            ASSERT_EQ(String(""), flagsProperty->description());
            ASSERT_EQ(2560, flagsProperty->defaultValue());
            
            const Model::FlagsPropertyOption::List& options = flagsProperty->options();
            ASSERT_EQ(4u, options.size());
            ASSERT_EQ(256, options[0].value());
            ASSERT_EQ(String("Not on Easy"), options[0].description());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(512, options[1].value());
            ASSERT_EQ(String("Not on Normal"), options[1].description());
            ASSERT_TRUE(options[1].isDefault());
            ASSERT_EQ(1024, options[2].value());
            ASSERT_EQ(String("Not on Hard"), options[2].description());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(2048, options[3].value());
            ASSERT_EQ(String("Not in Deathmatch"), options[3].description());
            ASSERT_TRUE(options[3].isDefault());
            
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseStaticModelProperties) {
            const String file =
            "@PointClass\n"
            "    model(\n"
            "        \":maps/b_shell0.bsp\",\n"
            "        \":maps/b_shell1.bsp\" spawnflags = 1\n"
            "    ) = item_shells : \"Shells\" []\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("item_shells"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Shells"), definition->description());
            
            ASSERT_TRUE(definition->propertyDefinitions().empty());
            
            const Model::ModelDefinitionList& models = static_cast<Model::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(2u, models.size());
        }

        TEST(FgdParserTest, parseDynamicModelProperty) {
            const String file =
            "@PointClass\n"
            "    model(pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\") = item_shells : \"Shells\" []\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            Model::EntityDefinitionList definitions = parser.parseDefinitions();
            ASSERT_EQ(1u, definitions.size());
            
            Model::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Model::EntityDefinition::PointEntity, definition->type());
            ASSERT_EQ(String("item_shells"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Shells"), definition->description());
            
            ASSERT_TRUE(definition->propertyDefinitions().empty());
            
            const Model::ModelDefinitionList& models = static_cast<Model::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(1u, models.size());
        }
    }
}
