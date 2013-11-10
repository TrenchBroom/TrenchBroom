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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "IO/GameConfigParser.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        TEST(GameConfigParserTest, parseBlankConfig) {
            GameConfigParser parser("   ");
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(GameConfigParserTest, parseEmptyConfig) {
            GameConfigParser parser("  {  } ");
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(GameConfigParserTest, parseQuakeConfig) {
            GameConfigParser parser("{"
                                    "  name=\"Quake\","
                                    "  textureformat={"
                                    "    type=\"wad\","
                                    "    palette=\"quake/palette.lmp\""
                                    "  },"
                                    "  modelformats={"
                                    "    \"mdl\", \"bsp\""
                                    "  }"
                                    "}");
            
            const Model::GameConfig config = parser.parse();
            ASSERT_EQ(String("Quake"), config.name());
            ASSERT_EQ(Model::GameConfig::TextureFormat::TWad, config.textureFormat().type);
            ASSERT_EQ(IO::Path("quake/palette.lmp"), config.textureFormat().palette);
            ASSERT_EQ(2u, config.modelFormats().size());
            ASSERT_TRUE(config.modelFormats().count("mdl") > 0);
            ASSERT_TRUE(config.modelFormats().count("bsp") > 0);
        }
    }
}
