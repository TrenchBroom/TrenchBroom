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

#include "CollectionUtils.h"
#include "IO/CompilationConfigParser.h"
#include "IO/Path.h"
#include "Model/CompilationConfig.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        TEST(CompilationConfigParserTest, parseBlankConfig) {
            const String config("   ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(CompilationConfigParserTest, parseEmptyConfig) {
            const String config("  {  } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseMissingProfiles) {
            const String config("  { version = \"1\" } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseMissingVersion) {
            const String config("  { profiles = {} } ");
            CompilationConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(CompilationConfigParserTest, parseEmptyProfiles) {
            const String config("  { version = \"1\", profiles = {} } ");
            CompilationConfigParser parser(config);
            
            Model::CompilationConfig result = parser.parse();
            ASSERT_EQ(0u, result.profileCount());
        }
    }
}
