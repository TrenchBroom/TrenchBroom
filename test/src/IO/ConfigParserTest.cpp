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

#include "StringUtils.h"
#include "ConfigTypes.h"
#include "IO/ConfigParser.h"
#include "Model/GameConfig.h"

namespace TrenchBroom {
    namespace IO {
        TEST(ConfigParserTest, testParseEmptyConfig) {
            const String config = "";

            ConfigParser parser(config);
            ASSERT_EQ(ConfigEntry::Ptr(), parser.parse());
        }

        TEST(ConfigParserTest, testParseBlankConfig) {
            const String config = "    \n  ";
            
            ConfigParser parser(config);
            ASSERT_EQ(ConfigEntry::Ptr(), parser.parse());
        }
        
        TEST(ConfigParserTest, testParseOneValue) {
            const String config = "\"asdf\"";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr value = parser.parse();
            
            ASSERT_TRUE(value != NULL);
            ASSERT_TRUE(value->type() == ConfigEntry::TValue);
            ASSERT_EQ(String("asdf"), static_cast<const String&>(*value));
        }
        
        TEST(ConfigParserTest, testParseEmptyList) {
            const String config = "  { } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TList);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(0u, list.count());
        }
        
        TEST(ConfigParserTest, testParseList) {
            const String config = "  { \"first\", \"\", \"third\" } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TList);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::TValue == list[0].type());
            ASSERT_TRUE(ConfigEntry::TValue == list[1].type());
            ASSERT_TRUE(ConfigEntry::TValue == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String(""), static_cast<const String&>(list[1]));
            ASSERT_EQ(String("third"), static_cast<const String&>(list[2]));
        }
        
        TEST(ConfigParserTest, testParseTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = \"\" } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TTable);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::TValue == table["first"].type());
            ASSERT_TRUE(ConfigEntry::TValue == table["second"].type());
            ASSERT_TRUE(ConfigEntry::TValue == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));
            ASSERT_EQ(String(""), static_cast<const String&>(table["third"]));
            
            const StringSet& keys = table.keys();
            ASSERT_EQ(3u, keys.size());
            ASSERT_TRUE(keys.count("first") == 1);
            ASSERT_TRUE(keys.count("second") == 1);
            ASSERT_TRUE(keys.count("third") == 1);
        }
        
        TEST(ConfigParserTest, testParseListNestedInList) {
            const String config = "  { \"first\", {\"second\", \"third\"}, \"fourth\" } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TList);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::TValue == list[0].type());
            ASSERT_TRUE(ConfigEntry::TList == list[1].type());
            ASSERT_TRUE(ConfigEntry::TValue == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(list[2]));

            const ConfigList& nested = list[1];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested[0].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested[1].type());
            ASSERT_EQ(String("second"), static_cast<const String&>(nested[0]));
            ASSERT_EQ(String("third"), static_cast<const String&>(nested[1]));
        }
        
        TEST(ConfigParserTest, testParseTableNestedInList) {
            const String config = "  { \"first\", {second=\"second\", third=\"third\"}, \"fourth\" } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TList);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(3u, list.count());
            ASSERT_TRUE(ConfigEntry::TValue == list[0].type());
            ASSERT_TRUE(ConfigEntry::TTable == list[1].type());
            ASSERT_TRUE(ConfigEntry::TValue == list[2].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(list[0]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(list[2]));
            
            const ConfigTable& nested = list[1];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested["second"].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested["third"].type());
            ASSERT_EQ(String("second"), static_cast<const String&>(nested["second"]));
            ASSERT_EQ(String("third"), static_cast<const String&>(nested["third"]));
        }
        
        
        TEST(GameConfigParserTest, testParseTablesNestedInList) {
            const String config = "  { {first = \"first\", second=\"second\"}, {third=\"third\", fourth = \"fourth\"} } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TList);
            
            const ConfigList& list = *entry;
            ASSERT_EQ(2u, list.count());
            ASSERT_TRUE(ConfigEntry::TTable == list[0].type());
            ASSERT_TRUE(ConfigEntry::TTable == list[1].type());
            
            const ConfigTable& nested1 = list[0];
            ASSERT_EQ(2u, nested1.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested1["first"].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested1["second"].type());
            ASSERT_EQ(String("first"), static_cast<const String&>(nested1["first"]));
            ASSERT_EQ(String("second"), static_cast<const String&>(nested1["second"]));
            
            const ConfigTable& nested2 = list[1];
            ASSERT_EQ(2u, nested2.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested2["third"].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested2["fourth"].type());
            ASSERT_EQ(String("third"), static_cast<const String&>(nested2["third"]));
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested2["fourth"]));
        }

        TEST(ConfigParserTest, testParseListNestedInTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = {\"fourth\",\"fifth\"} } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TTable);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::TValue == table["first"].type());
            ASSERT_TRUE(ConfigEntry::TValue == table["second"].type());
            ASSERT_TRUE(ConfigEntry::TList == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));

            const ConfigList& nested = table["third"];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested[0].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested[1].type());
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested[0]));
            ASSERT_EQ(String("fifth"), static_cast<const String&>(nested[1]));
        }
        
        TEST(ConfigParserTest, testParseTableNestedInTable) {
            const String config = "  { first = \"firstValue\", second=\"secondValue\", third = {fourth=\"fourth\",fifth=\"fifth\"} } ";
            
            ConfigParser parser(config);
            const ConfigEntry::Ptr entry = parser.parse();
            
            ASSERT_TRUE(entry != NULL);
            ASSERT_TRUE(entry->type() == ConfigEntry::TTable);
            
            const ConfigTable& table = *entry;
            ASSERT_EQ(3u, table.count());
            ASSERT_TRUE(ConfigEntry::TValue == table["first"].type());
            ASSERT_TRUE(ConfigEntry::TValue == table["second"].type());
            ASSERT_TRUE(ConfigEntry::TTable == table["third"].type());
            ASSERT_EQ(String("firstValue"), static_cast<const String&>(table["first"]));
            ASSERT_EQ(String("secondValue"), static_cast<const String&>(table["second"]));
            
            const ConfigTable& nested = table["third"];
            ASSERT_EQ(2u, nested.count());
            ASSERT_TRUE(ConfigEntry::TValue == nested["fourth"].type());
            ASSERT_TRUE(ConfigEntry::TValue == nested["fifth"].type());
            ASSERT_EQ(String("fourth"), static_cast<const String&>(nested["fourth"]));
            ASSERT_EQ(String("fifth"), static_cast<const String&>(nested["fifth"]));
        }
    }
}
