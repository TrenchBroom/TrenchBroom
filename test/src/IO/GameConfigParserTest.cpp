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
            const String config("   ");
            GameConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(GameConfigParserTest, parseEmptyConfig) {
            const String config("  {  } ");
            GameConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }
        
        TEST(GameConfigParserTest, parseQuakeConfig) {
            const String config("{\n"
                             "  name=\"Quake\",\n"
                             "  fileformats={\"Quake 1\",\"Valve\"},\n"
                             "  filesystem={\n"
                             "    searchpath=\"id1\",\n"
                             "    packageformat=\"pak\"\n"
                             "  },\n"
                             "  textures={\n"
                             "    type=\"wad\",\n"
                             "    property=\"wad\",\n"
                             "    palette=\"palette.lmp\"\n"
                             "  },\n"
                             "  entities={\n"
                             "    definitions={ \"Quake1.fgd\", \"Quoth2.fgd\" },\n"
                             "    defaultcolor=\"1.0 1.0 1.0 1.0\",\n"
                             "    modelformats={\"bsp\", \"mdl\"}\n"
                             "  }\n"
                             "}\n");
            GameConfigParser parser(config);
            
            const Model::GameConfig gameConfig = parser.parse();
            ASSERT_EQ(String("Quake"), gameConfig.name());
            ASSERT_EQ(2u, gameConfig.fileFormats().size());
            ASSERT_EQ(1u, gameConfig.fileFormats().count("Quake 1"));
            ASSERT_EQ(1u, gameConfig.fileFormats().count("Valve"));
            ASSERT_EQ(Path("id1"), gameConfig.fileSystemConfig().searchPath);
            ASSERT_EQ(String("pak"), gameConfig.fileSystemConfig().packageFormat);
            ASSERT_EQ(String("wad"), gameConfig.textureConfig().type);
            ASSERT_EQ(String("wad"), gameConfig.textureConfig().property);
            ASSERT_EQ(Path("palette.lmp"), gameConfig.textureConfig().palette);
            ASSERT_TRUE(gameConfig.textureConfig().builtinTexturesSearchPath.isEmpty());
            ASSERT_EQ(Path("Quake1.fgd"), gameConfig.entityConfig().defFilePaths[0]);
            ASSERT_EQ(Path("Quoth2.fgd"), gameConfig.entityConfig().defFilePaths[1]);
            ASSERT_EQ(2u, gameConfig.entityConfig().modelFormats.size());
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.count("bsp"));
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.count("mdl"));
            ASSERT_EQ(Color(1.0f, 1.0f, 1.0f, 1.0f), gameConfig.entityConfig().defaultColor);
        }
    }
}
