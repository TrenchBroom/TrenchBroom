/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "StringUtils.h"
#include "ConfigTypes.h"
#include "IO/ConfigFileParser.h"
#include "Model/GameConfig.h"

namespace TrenchBroom {
    namespace IO {
        TEST(ConfigFileParserTest, testParseEmptyConfig) {
            const String config = "";

            ConfigFileParser parser(config);
            ASSERT_EQ(NULL, parser.parse());
        }

        TEST(ConfigFileParserTest, testParseBlankConfig) {
            const String config = "    \n  ";
            
            ConfigFileParser parser(config);
            ASSERT_EQ(NULL, parser.parse());
        }
        
        TEST(ConfigFileParserTest, testParseOneValue) {
            const String config = "\"asdf\"";
            
            ConfigFileParser parser(config);
            const ConfigEntry* value = parser.parse();
            
            ASSERT_TRUE(value != NULL);
            ASSERT_TRUE(value->type() == ConfigEntry::Type_Value);
            ASSERT_EQ(String("asdf"), static_cast<const String&>(*value));
            
            delete value;
        }
        
        TEST(ConfigFileParserTest, testParseEmptyList) {
            const String config = "  { } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_List);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(0u, list.count());
            
            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseList) {
            const String config = "  { \"first\", \"\", \"third\" } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_List);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[1].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String(""), static_cast<const String&>(list[1]));
            ASSERT_EQ(String("third"), static_cast<const String&>(list[2]));

            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = \"\" } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_Table);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["first"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["second"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));
            ASSERT_EQ(String(""), static_cast<const String&>(table["third"]));
            
            const StringSet& keys = table.keys();
            ASSERT_EQ(3u, keys.size());
            ASSERT_TRUE(keys.count("first") == 1);
            ASSERT_TRUE(keys.count("second") == 1);
            ASSERT_TRUE(keys.count("third") == 1);
            
            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseListNestedInList) {
            const String config = "  { \"first\", {\"second\", \"third\"}, \"fourth\" } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_List);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[0].type());
            ASSERT_TRUE(ConfigEntry::Type_List == list[1].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(list[2]));

            const ConfigList& nested = list[1];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[1].type());
            ASSERT_EQ(String("second"), static_cast<const String&>(nested[0]));
            ASSERT_EQ(String("third"), static_cast<const String&>(nested[1]));
            
            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseTableNestedInList) {
            const String config = "  { \"first\", {second=\"second\", third=\"third\"}, \"fourth\" } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_List);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Table == list[1].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(list[2]));
            
            const ConfigTable& nested = list[1];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested["second"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested["third"].type());
            ASSERT_EQ(String("second"), static_cast<const String&>(nested["second"]));
            ASSERT_EQ(String("third"), static_cast<const String&>(nested["third"]));
            
            delete entry;
        }
        
        
        TEST(GameConfigParserTest, testParseTablesNestedInList) {
            const String config = "  { {first = \"first\", second=\"second\"}, {third=\"third\", fourth = \"fourth\"} } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_List);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(2u, list.count());
            ASSERT_TRUE(ConfigEntry::Type_Table == list[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Table == list[1].type());
            
            const ConfigTable& nested1 = list[0];
            ASSERT_EQ(2u, nested1.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested1["first"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested1["second"].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(nested1["first"]));
            ASSERT_EQ(String("second"), static_cast<const String&>(nested1["second"]));
            
            const ConfigTable& nested2 = list[1];
            ASSERT_EQ(2u, nested2.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested2["third"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested2["fourth"].type());
            ASSERT_EQ(String("third"), static_cast<const String&>(nested2["third"]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested2["fourth"]));
            
            delete entry;
        }

        TEST(ConfigFileParserTest, testParseListNestedInTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = {\"fourth\",\"fifth\"} } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_Table);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["first"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["second"].type());
            ASSERT_TRUE(ConfigEntry::Type_List == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));

            const ConfigList& nested = table["third"];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[1].type());
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested[0]));
            ASSERT_EQ(String("fifth"), static_cast<const String&>(nested[1]));
            
            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseTableNestedInTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = {fourth=\"fourth\",fifth=\"fifth\"} } ";
            
            ConfigFileParser parser(config);
            const ConfigEntry* entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::Type_Table);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["first"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["second"].type());
            ASSERT_TRUE(ConfigEntry::Type_Table == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));
            
            const ConfigTable& nested = table["third"];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested["fourth"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested["fifth"].type());
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested["fourth"]));
            ASSERT_EQ(String("fifth"), static_cast<const String&>(nested["fifth"]));
            
            delete entry;
        }
        
        TEST(ConfigFileParserTest, testParseSerializedConfig) {
            const ConfigEntry* config = ConfigFileParser("  { first = \"firstValue\", second=\"secondValue\", third = {\"fourth\",\"fifth\"} } ").parse();
            const String serialized = config->asString();
            const ConfigEntry* deserialized = ConfigFileParser(serialized).parse();
            
            ASSERT_TRUE(deserialized != NULL);
            ASSERT_TRUE(deserialized->type() == ConfigEntry::Type_Table);
            
            const ConfigTable& table = *deserialized;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["first"].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == table["second"].type());
            ASSERT_TRUE(ConfigEntry::Type_List == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));
            
            const ConfigList& nested = table["third"];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[0].type());
            ASSERT_TRUE(ConfigEntry::Type_Value == nested[1].type());
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested[0]));
            ASSERT_EQ(String("fifth"), static_cast<const String&>(nested[1]));
            
            delete deserialized;
        }
    }
}
