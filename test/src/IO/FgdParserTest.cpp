/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "IO/FgdParser.h"
#include "Model/ModelTypes.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "IO/TestParserStatus.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        TEST(FgdParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseCommentsFile) {
            const String file = "// asdfasdfasdf\n//kj3k4jkdjfkjdf\n";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseEmptyFlagDescription) {
            const String file =
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
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_BrushEntity, definition->type());
            ASSERT_EQ(String("worldspawn"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("World entity"), definition->description());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(6u, attributes.size());
            
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());

            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());

            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(5u, attributes.size());
            
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(9u, attributes.size());

            VectorUtils::clearAndDelete(definitions);
        }
        
        TEST(FgdParserTest, parseType_TargetSourceAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	targetname(target_source) : \"Source\" : : \"A long description\" \n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());
            
            Assets::AttributeDefinitionPtr attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinition::Type_TargetSourceAttribute, attribute->type());
            ASSERT_EQ(String("targetname"), attribute->name());
            ASSERT_EQ(String("Source"), attribute->shortDescription());
            ASSERT_EQ(String("A long description"), attribute->longDescription());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseType_TargetDestinationAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "	target(target_destination) : \"Target\" \n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            const Assets::AttributeDefinitionList& attributes = definition->attributeDefinitions();
            ASSERT_EQ(1u, attributes.size());
            
            Assets::AttributeDefinitionPtr attribute = attributes[0];
            ASSERT_EQ(Assets::AttributeDefinition::Type_TargetDestinationAttribute, attribute->type());
            ASSERT_EQ(String("target"), attribute->name());
            ASSERT_EQ(String("Target"), attribute->shortDescription());
            ASSERT_EQ(String(""), attribute->longDescription());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseStringAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   message(string) : \"Text on entering the world\" : : \"Long description 1\"\n"
            "   message2(string) : \"With a default value\" : \"DefaultValue\" : \"Long description 2\"\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->attributeDefinitions().size());
            
            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("message");
            ASSERT_TRUE(attribute1 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_StringAttribute, attribute1->type());

            const Assets::StringAttributeDefinition* stringAttribute1 = static_cast<const Assets::StringAttributeDefinition*>(attribute1);
            ASSERT_EQ(String("message"), stringAttribute1->name());
            ASSERT_EQ(String("Text on entering the world"), stringAttribute1->shortDescription());
            ASSERT_EQ(String("Long description 1"), stringAttribute1->longDescription());
            ASSERT_FALSE(stringAttribute1->hasDefaultValue());
            
            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("message2");
            ASSERT_TRUE(attribute2 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_StringAttribute, attribute2->type());
            
            const Assets::StringAttributeDefinition* stringAttribute2 = static_cast<const Assets::StringAttributeDefinition*>(attribute2);
            ASSERT_EQ(String("message2"), stringAttribute2->name());
            ASSERT_EQ(String("With a default value"), stringAttribute2->shortDescription());
            ASSERT_EQ(String("Long description 2"), stringAttribute2->longDescription());
            ASSERT_TRUE(stringAttribute2->hasDefaultValue());
            ASSERT_EQ(String("DefaultValue"), stringAttribute2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseIntegerAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   sounds(integer) : \"CD track to play\" : : \"Longer description\"\n"
            "   sounds2(integer) : \"CD track to play with default\" : 2 : \"Longer description\"\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->attributeDefinitions().size());
            
            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("sounds");
            ASSERT_TRUE(attribute1 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_IntegerAttribute, attribute1->type());
            
            const Assets::IntegerAttributeDefinition* intAttribute1 = static_cast<const Assets::IntegerAttributeDefinition*>(attribute1);
            ASSERT_EQ(String("sounds"), intAttribute1->name());
            ASSERT_EQ(String("CD track to play"), intAttribute1->shortDescription());
            ASSERT_EQ(String("Longer description"), intAttribute1->longDescription());
            ASSERT_FALSE(intAttribute1->hasDefaultValue());
            
            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("sounds2");
            ASSERT_TRUE(attribute2 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_IntegerAttribute, attribute2->type());
            
            const Assets::IntegerAttributeDefinition* intAttribute2 = static_cast<const Assets::IntegerAttributeDefinition*>(attribute2);
            ASSERT_EQ(String("sounds2"), intAttribute2->name());
            ASSERT_EQ(String("CD track to play with default"), intAttribute2->shortDescription());
            ASSERT_EQ(String("Longer description"), intAttribute2->longDescription());
            ASSERT_TRUE(intAttribute2->hasDefaultValue());
            ASSERT_EQ(2, intAttribute2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseFloatAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   test(float) : \"Some test attribute\" : : \"Longer description 1\"\n"
            "   test2(float) : \"Some test attribute with default\" : \"2.7\" : \"Longer description 2\"\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->attributeDefinitions().size());
            
            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("test");
            ASSERT_TRUE(attribute1 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_FloatAttribute, attribute1->type());
            
            const Assets::FloatAttributeDefinition* floatAttribute1 = static_cast<const Assets::FloatAttributeDefinition*>(attribute1);
            ASSERT_EQ(String("test"), floatAttribute1->name());
            ASSERT_EQ(String("Some test attribute"), floatAttribute1->shortDescription());
            ASSERT_EQ(String("Longer description 1"), floatAttribute1->longDescription());
            ASSERT_FALSE(floatAttribute1->hasDefaultValue());
            
            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("test2");
            ASSERT_TRUE(attribute2 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_FloatAttribute, attribute2->type());
            
            const Assets::FloatAttributeDefinition* floatAttribute2 = static_cast<const Assets::FloatAttributeDefinition*>(attribute2);
            ASSERT_EQ(String("test2"), floatAttribute2->name());
            ASSERT_EQ(String("Some test attribute with default"), floatAttribute2->shortDescription());
            ASSERT_EQ(String("Longer description 2"), floatAttribute2->longDescription());
            ASSERT_TRUE(floatAttribute2->hasDefaultValue());
            ASSERT_FLOAT_EQ(2.7f, floatAttribute2->defaultValue());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseChoiceAttribute) {
            const String file =
            "@PointClass = info_notnull : \"Wildcard entity\" // I love you\n"
            "[\n"
            "   worldtype(choices) : \"Ambience\" : : \"Long description 1\" =\n"
            "   [\n"
            "       0 : \"Medieval\"\n"
            "       1 : \"Metal (runic)\"\n"
            "       2 : \"Base\"\n"
            "   ]\n"
            "   worldtype2(choices) : \"Ambience with default\" : 1 : \"Long description 2\" =\n"
            "   [\n"
            "       0 : \"Medieval\"\n"
            "       1 : \"Metal (runic)\"\n"
            "   ]\n"
            "]\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(2u, definition->attributeDefinitions().size());
            
            const Assets::AttributeDefinition* attribute1 = definition->attributeDefinition("worldtype");
            ASSERT_TRUE(attribute1 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_ChoiceAttribute, attribute1->type());
            
            const Assets::ChoiceAttributeDefinition* choiceAttribute1 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute1);
            ASSERT_EQ(String("worldtype"), choiceAttribute1->name());
            ASSERT_EQ(String("Ambience"), choiceAttribute1->shortDescription());
            ASSERT_EQ(String("Long description 1"), choiceAttribute1->longDescription());
            ASSERT_FALSE(choiceAttribute1->hasDefaultValue());
            
            const Assets::ChoiceAttributeOption::List& options1 = choiceAttribute1->options();
            ASSERT_EQ(3u, options1.size());
            ASSERT_EQ(String("0"), options1[0].value());
            ASSERT_EQ(String("Medieval"), options1[0].description());
            ASSERT_EQ(String("1"), options1[1].value());
            ASSERT_EQ(String("Metal (runic)"), options1[1].description());
            ASSERT_EQ(String("2"), options1[2].value());
            ASSERT_EQ(String("Base"), options1[2].description());
            
            const Assets::AttributeDefinition* attribute2 = definition->attributeDefinition("worldtype2");
            ASSERT_TRUE(attribute2 != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_ChoiceAttribute, attribute2->type());
            
            const Assets::ChoiceAttributeDefinition* choiceAttribute2 = static_cast<const Assets::ChoiceAttributeDefinition*>(attribute2);
            ASSERT_EQ(String("worldtype2"), choiceAttribute2->name());
            ASSERT_EQ(String("Ambience with default"), choiceAttribute2->shortDescription());
            ASSERT_EQ(String("Long description 2"), choiceAttribute2->longDescription());
            ASSERT_TRUE(choiceAttribute2->hasDefaultValue());
            ASSERT_EQ(1u, choiceAttribute2->defaultValue());
            
            const Assets::ChoiceAttributeOption::List& options2 = choiceAttribute1->options();
            ASSERT_EQ(3u, options2.size());
            ASSERT_EQ(String("0"), options2[0].value());
            ASSERT_EQ(String("Medieval"), options2[0].description());
            ASSERT_EQ(String("1"), options2[1].value());
            ASSERT_EQ(String("Metal (runic)"), options2[1].description());
            
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(FgdParserTest, parseFlagsAttribute) {
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("info_notnull"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Wildcard entity"), definition->description());
            
            ASSERT_EQ(1u, definition->attributeDefinitions().size());
            
            const Assets::AttributeDefinition* attribute = definition->attributeDefinition("spawnflags");
            ASSERT_TRUE(attribute != NULL);
            ASSERT_EQ(Assets::AttributeDefinition::Type_FlagsAttribute, attribute->type());
            
            const Assets::FlagsAttributeDefinition* flagsAttribute = static_cast<const Assets::FlagsAttributeDefinition*>(attribute);
            ASSERT_EQ(String("spawnflags"), flagsAttribute->name());
            ASSERT_EQ(String(""), flagsAttribute->shortDescription());
            ASSERT_EQ(2560, flagsAttribute->defaultValue());
            
            const Assets::FlagsAttributeOption::List& options = flagsAttribute->options();
            ASSERT_EQ(4u, options.size());
            ASSERT_EQ(256, options[0].value());
            ASSERT_EQ(String("Not on Easy"), options[0].shortDescription());
            ASSERT_FALSE(options[0].isDefault());
            ASSERT_EQ(512, options[1].value());
            ASSERT_EQ(String("Not on Normal"), options[1].shortDescription());
            ASSERT_TRUE(options[1].isDefault());
            ASSERT_EQ(1024, options[2].value());
            ASSERT_EQ(String("Not on Hard"), options[2].shortDescription());
            ASSERT_FALSE(options[2].isDefault());
            ASSERT_EQ(2048, options[3].value());
            ASSERT_EQ(String("Not in Deathmatch"), options[3].shortDescription());
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
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("item_shells"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Shells"), definition->description());
            
            ASSERT_TRUE(definition->attributeDefinitions().empty());
            
            const Assets::ModelDefinitionList& models = static_cast<Assets::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(2u, models.size());
        }

        TEST(FgdParserTest, parseDynamicModelAttribute) {
            const String file =
            "@PointClass\n"
            "    model(pathKey = \"model\" skinKey = \"skin\" frameKey = \"frame\") = item_shells : \"Shells\" []\n";
            
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            FgdParser parser(file, defaultColor);
            
            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size());
            
            Assets::EntityDefinition* definition = definitions[0];
            ASSERT_EQ(Assets::EntityDefinition::Type_PointEntity, definition->type());
            ASSERT_EQ(String("item_shells"), definition->name());
            ASSERT_VEC_EQ(defaultColor, definition->color());
            ASSERT_EQ(String("Shells"), definition->description());
            
            ASSERT_TRUE(definition->attributeDefinitions().empty());
            
            const Assets::ModelDefinitionList& models = static_cast<Assets::PointEntityDefinition*>(definition)->modelDefinitions();
            ASSERT_EQ(1u, models.size());
        }
    }
}
