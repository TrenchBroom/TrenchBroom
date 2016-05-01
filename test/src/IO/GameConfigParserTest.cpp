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
                                "  version=\"1\",\n"
                                "  name=\"Quake\",\n"
                                "  fileformats={\"Quake 1\",\"Valve\"},\n"
                                "  filesystem={\n"
                                "    searchpath=\"id1\",\n"
                                "    packageformat= {extension=\"pak\",format=\"idpak\"}\n"
                                "  },\n"
                                "  textures={\n"
                                "    package = { type = \"file\", format = { extension = \"wad\", format = \"wad\" } },"
                                "    attribute=\"wad\",\n"
                                "    palette = { type = \"builtin\", location = { path = \"Quake/palette.lmp\" } }\n"
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
            ASSERT_TRUE(VectorUtils::contains(gameConfig.fileFormats(), String("Quake 1")));
            ASSERT_TRUE(VectorUtils::contains(gameConfig.fileFormats(), String("Valve")));
            ASSERT_EQ(Path("id1"), gameConfig.fileSystemConfig().searchPath);
            ASSERT_EQ(String("pak"), gameConfig.fileSystemConfig().packageFormat.extension);
            ASSERT_EQ(String("idpak"), gameConfig.fileSystemConfig().packageFormat.format);
            ASSERT_EQ(Model::GameConfig::TexturePackageConfig::PT_File, gameConfig.textureConfig().package.type);
            ASSERT_EQ(String("wad"), gameConfig.textureConfig().package.format.extension);
            ASSERT_EQ(String("wad"), gameConfig.textureConfig().package.format.format);
            ASSERT_EQ(String("wad"), gameConfig.textureConfig().attribute);
            ASSERT_EQ(Model::GameConfig::PaletteConfig::LT_Builtin, gameConfig.textureConfig().palette.type);
            ASSERT_EQ(String("Quake/palette.lmp"), gameConfig.textureConfig().palette.path);
            ASSERT_TRUE(gameConfig.textureConfig().builtinTexturesSearchPath.isEmpty());
            ASSERT_EQ(Path("Quake1.fgd"), gameConfig.entityConfig().defFilePaths[0]);
            ASSERT_EQ(Path("Quoth2.fgd"), gameConfig.entityConfig().defFilePaths[1]);
            ASSERT_EQ(2u, gameConfig.entityConfig().modelFormats.size());
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.count("bsp"));
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.count("mdl"));
            ASSERT_EQ(Color(1.0f, 1.0f, 1.0f, 1.0f), gameConfig.entityConfig().defaultColor);
        }

        TEST(GameConfigParserTest, parseQuake2Config) {
            const String config("{\n"
                                "   version=\"1\",\n"
                                "	name = \"Quake 2\",\n"
                                "	icon = \"Quake2/Icon.png\",\n"
                                " 	fileformats = { \"Quake 2\" },\n"
                                "	filesystem = {\n"
                                "		searchpath = \"baseq2\",\n"
                                "		packageformat = { extension = \"pak\", format = \"idpak\" }\n"
                                "	},\n"
                                "	textures = {\n"
                                "   package = { type = \"directory\", format = { extension = \"wal\", format = \"idwal\" } },\n"
                                "    	attribute = \"_wal\",\n"
                                "       palette = { type = \"property\", location = { key = \"palette\", path = \"textures/${KEY}/colormap.bmp\" } },\n"
                                "		builtin = \"textures\"\n"
                                "	},\n"
                                "  	entities = {\n"
                                "		definitions = \"Quake2/Quake2.fgd\",\n"
                                "    	defaultcolor = \"1.0 1.0 1.0 1.0\",\n"
                                "		modelformats = { \"md2\" }\n"
                                "    },\n"
                                "    faceattribs = {\n"
                                "        surfaceflags = {\n"
                                "            {\n"
                                "                name = \"light\",\n"
                                "                description = \"Emit light from the surface, brightness is specified in the 'value' field\"\n"
                                "            },\n"
                                "            {\n"
                                "                name = \"slick\",\n"
                                "                description = \"The surface is slippery\"\n"
                                "            }\n"
                                "        },\n"
                                "        contentflags = {\n"
                                "            {\n"
                                "                name = \"solid\",\n"
                                "                description = \"Default for all brushes\"\n"
                                "            },\n"
                                "            {\n"
                                "                name = \"window\",\n"
                                "                description = \"Brush is a window (not really used)\"\n"
                                "            }\n"
                                "        }\n"
                                "    }\n"
                                "}\n");
            GameConfigParser parser(config);
            
            const Model::GameConfig gameConfig = parser.parse();
            ASSERT_EQ(String("Quake 2"), gameConfig.name());
            ASSERT_EQ(Path("Quake2/Icon.png"), gameConfig.icon());
            ASSERT_EQ(1u, gameConfig.fileFormats().size());
            ASSERT_TRUE(VectorUtils::contains(gameConfig.fileFormats(), String("Quake 2")));
            ASSERT_EQ(Path("baseq2"), gameConfig.fileSystemConfig().searchPath);
            ASSERT_EQ(String("pak"), gameConfig.fileSystemConfig().packageFormat.extension);
            ASSERT_EQ(String("idpak"), gameConfig.fileSystemConfig().packageFormat.format);
            ASSERT_EQ(Model::GameConfig::TexturePackageConfig::PT_Directory, gameConfig.textureConfig().package.type);
            ASSERT_EQ(String("wal"), gameConfig.textureConfig().package.format.extension);
            ASSERT_EQ(String("idwal"), gameConfig.textureConfig().package.format.format);
            ASSERT_EQ(String("_wal"), gameConfig.textureConfig().attribute);
            ASSERT_EQ(Model::GameConfig::PaletteConfig::LT_Property, gameConfig.textureConfig().palette.type);
            ASSERT_EQ(String("palette"), gameConfig.textureConfig().palette.property);
            ASSERT_EQ(String("textures/${KEY}/colormap.bmp"), gameConfig.textureConfig().palette.path);
            ASSERT_EQ(Path("textures"), gameConfig.textureConfig().builtinTexturesSearchPath);
            ASSERT_EQ(Path("Quake2/Quake2.fgd"), gameConfig.entityConfig().defFilePaths[0]);
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.size());
            ASSERT_EQ(1u, gameConfig.entityConfig().modelFormats.count("md2"));
            ASSERT_EQ(Color(1.0f, 1.0f, 1.0f, 1.0f), gameConfig.entityConfig().defaultColor);
            
            ASSERT_EQ(2u, gameConfig.faceAttribsConfig().surfaceFlags.flags.size());
            ASSERT_EQ(String("light"), gameConfig.faceAttribsConfig().surfaceFlags.flags[0].name);
            ASSERT_EQ(String("Emit light from the surface, brightness is specified in the 'value' field"), gameConfig.faceAttribsConfig().surfaceFlags.flags[0].description);
            ASSERT_EQ(String("slick"), gameConfig.faceAttribsConfig().surfaceFlags.flags[1].name);
            ASSERT_EQ(String("The surface is slippery"), gameConfig.faceAttribsConfig().surfaceFlags.flags[1].description);

            ASSERT_EQ(2u, gameConfig.faceAttribsConfig().contentFlags.flags.size());
            ASSERT_EQ(String("solid"), gameConfig.faceAttribsConfig().contentFlags.flags[0].name);
            ASSERT_EQ(String("Default for all brushes"), gameConfig.faceAttribsConfig().contentFlags.flags[0].description);
            ASSERT_EQ(String("window"), gameConfig.faceAttribsConfig().contentFlags.flags[1].name);
            ASSERT_EQ(String("Brush is a window (not really used)"), gameConfig.faceAttribsConfig().contentFlags.flags[1].description);
        }
    }
}
