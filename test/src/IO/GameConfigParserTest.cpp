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
#include "IO/DiskIO.h"
#include "IO/GameConfigParser.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        TEST(GameConfigParserTest, parseIncludedGameConfigs) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("data/GameConfig");
            const Path::List cfgFiles = Disk::findItems(basePath, [] (const Path& path, bool directory) {
                return !directory && StringUtils::caseInsensitiveEqual(path.extension(), "cfg");
            });
            
            for (const Path& path : cfgFiles) {
                MappedFile::Ptr file = Disk::openFile(path);
                GameConfigParser parser(file->begin(), file->end(), path);
                ASSERT_NO_THROW(parser.parse());
            }
        }
        
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
                                "    \"version\": 1,\n"
                                "	\"name\": \"Quake\",\n"
                                "	\"icon\": \"Quake/Icon.png\",\n"
                                " 	\"fileformats\": [ \"Standard\", \"Valve\" ],\n"
                                "	\"filesystem\": {\n"
                                "		\"searchpath\": \"id1\",\n"
                                "        \"packageformat\": { \"extension\": \"pak\", \"format\": \"idpak\" }\n"
                                "	},\n"
                                "	\"textures\": {\n"
                                "        \"package\": { \"type\": \"file\", \"format\": { \"extension\": \"wad\", \"format\": \"wad2\" } },\n"
                                "        \"format\": { \"extension\": \"D\", \"format\": \"idmip\" },\n"
                                "        \"palette\": \"gfx/palette.lmp\",\n"
                                "    	\"attribute\": \"wad\"\n"
                                "	},\n"
                                "  	\"entities\": {\n"
                                "		\"definitions\": [ \"Quake/Quake.fgd\", \"Quake/Quoth2.fgd\", \"Quake/Rubicon2.def\" ],\n"
                                "    	\"defaultcolor\": \"0.6 0.6 0.6 1.0\",\n"
                                "		\"modelformats\": [ \"mdl\", \"bsp\" ]\n"
                                "    },\n"
                                "    \"brushtypes\": [\n"
                                "        {\n"
                                "            \"name\": \"Clip brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"clip\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Skip brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"skip\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Hint brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"hint\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Liquid brushes\",\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"\\**\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Trigger brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"classname\",\n"
                                "            \"pattern\": \"trigger*\"\n"
                                "        }\n"
                                "    ]\n"
                                "}\n"
                                "\n");
            
            GameConfigParser parser(config);
            
            using Model::GameConfig;
            const GameConfig actual = parser.parse();
            
            using Model::BrushContentType;
            const GameConfig expected("Quake",
                                      Path(""),
                                      Path("Quake/Icon.png"),
                                      StringUtils::makeList(2, "Standard", "Valve"),
                                      GameConfig::FileSystemConfig(Path("id1"), GameConfig::PackageFormatConfig("pak", "idpak")),
                                      GameConfig::TextureConfig(GameConfig::TexturePackageConfig(GameConfig::PackageFormatConfig("wad", "wad2")),
                                                                GameConfig::PackageFormatConfig("D", "idmip"),
                                                                Path("gfx/palette.lmp"),
                                                                "wad"),
                                      GameConfig::EntityConfig(VectorUtils::create<Path>(Path("Quake/Quake.fgd"), Path("Quake/Quoth2.fgd"), Path("Quake/Rubicon2.def")),
                                                               StringUtils::makeSet(2, "mdl", "bsp"),
                                                               Color(0.6f, 0.6f, 0.6f, 1.0f)),
                                      GameConfig::FaceAttribsConfig(),
                                      BrushContentType::List());
            
            ASSERT_EQ(expected.name(), actual.name());
            ASSERT_EQ(expected.path(), actual.path());
            ASSERT_EQ(expected.icon(), actual.icon());
            ASSERT_EQ(expected.fileFormats(), actual.fileFormats());
            ASSERT_EQ(expected.fileSystemConfig(), actual.fileSystemConfig());
            ASSERT_EQ(expected.textureConfig(), actual.textureConfig());
            ASSERT_EQ(expected.entityConfig(), actual.entityConfig());
            ASSERT_EQ(expected.faceAttribsConfig(), actual.faceAttribsConfig());
            ASSERT_EQ(5u, actual.brushContentTypes().size());
        }

        TEST(GameConfigParserTest, parseQuake2Config) {
            const String config("{\n"
                                "    \"version\": 1,\n"
                                "    \"name\": \"Quake 2\",\n"
                                "    \"icon\": \"Quake2/Icon.png\",\n"
                                "    \"fileformats\": [ \"Quake2\" ],\n"
                                "    \"filesystem\": {\n"
                                "        \"searchpath\": \"baseq2\",\n"
                                "        \"packageformat\": { \"extension\": \"pak\", \"format\": \"idpak\" }\n"
                                "    },\n"
                                "    \"textures\": {\n"
                                "        \"package\": { \"type\": \"directory\", \"root\": \"textures\" },\n"
                                "        \"format\": { \"extension\": \"wal\", \"format\": \"idwal\" },\n"
                                "        \"palette\": \"pics/colormap.pcx\",\n"
                                "        \"attribute\": \"_tb_textures\"\n"
                                "    },\n"
                                "    \"entities\": {\n"
                                "        \"definitions\": [ \"Quake2/Quake2.fgd\" ],\n"
                                "        \"defaultcolor\": \"0.6 0.6 0.6 1.0\",\n"
                                "        \"modelformats\": [ \"md2\" ]\n"
                                "    },\n"
                                "    \"brushtypes\": [\n"
                                "        {\n"
                                "            \"name\": \"Clip brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"clip\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Skip brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"skip\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Hint brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"texture\",\n"
                                "            \"pattern\": \"hint\"\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Detail brushes\",\n"
                                "            \"match\": \"contentflag\",\n"
                                "            \"flags\": [ \"detail\" ]\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Liquid brushes\",\n"
                                "            \"match\": \"contentflag\",\n"
                                "            \"flags\": [ \"lava\", \"slime\", \"water\" ]\n"
                                "        },\n"
                                "        {\n"
                                "            \"name\": \"Trigger brushes\",\n"
                                "            \"attribs\": [ \"transparent\" ],\n"
                                "            \"match\": \"classname\",\n"
                                "            \"pattern\": \"trigger*\"\n"
                                "        }\n"
                                "    ],\n"
                                "    \"faceattribs\": {\n"
                                "        \"surfaceflags\": [\n"
                                "            {\n"
                                "                \"name\": \"light\",\n"
                                "                \"description\": \"Emit light from the surface, brightness is specified in the 'value' field\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"slick\",\n"
                                "                \"description\": \"The surface is slippery\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"sky\",\n"
                                "                \"description\": \"The surface is sky, the texture will not be drawn, but the background sky box is used instead\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"warp\",\n"
                                "                \"description\": \"The surface warps (like water textures do)\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"trans33\",\n"
                                "                \"description\": \"The surface is 33% transparent\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"trans66\",\n"
                                "                \"description\": \"The surface is 66% transparent\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"flowing\",\n"
                                "                \"description\": \"The texture wraps in a downward 'flowing' pattern (warp must also be set)\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"nodraw\",\n"
                                "                \"description\": \"Used for non-fixed-size brush triggers and clip brushes\"\n"
                                "            }\n"
                                "        ],\n"
                                "        \"contentflags\": [\n"
                                "            {\n"
                                "                \"name\": \"solid\",\n"
                                "                \"description\": \"Default for all brushes\"\n"
                                "            }, // 1\n"
                                "            {\n"
                                "                \"name\": \"window\",\n"
                                "                \"description\": \"Brush is a window (not really used)\"\n"
                                "            }, // 2\n"
                                "            {\n"
                                "                \"name\": \"aux\",\n"
                                "                \"description\": \"Unused by the engine\"\n"
                                "            }, // 4\n"
                                "            {\n"
                                "                \"name\": \"lava\",\n"
                                "                \"description\": \"The brush is lava\"\n"
                                "            }, // 8\n"
                                "            {\n"
                                "                \"name\": \"slime\",\n"
                                "                \"description\": \"The brush is slime\"\n"
                                "            }, // 16\n"
                                "            {\n"
                                "                \"name\": \"water\",\n"
                                "                \"description\": \"The brush is water\"\n"
                                "            }, // 32\n"
                                "            {\n"
                                "                \"name\": \"mist\",\n"
                                "                \"description\": \"The brush is non-solid\"\n"
                                "            }, // 64\n"
                                "            { \"name\": \"unused\" }, // 128\n"
                                "            { \"name\": \"unused\" }, // 256\n"
                                "            { \"name\": \"unused\" }, // 512\n"
                                "            { \"name\": \"unused\" }, // 1024\n"
                                "            { \"name\": \"unused\" }, // 2048\n"
                                "            { \"name\": \"unused\" }, // 4096\n"
                                "            { \"name\": \"unused\" }, // 8192\n"
                                "            { \"name\": \"unused\" }, // 16384\n"
                                "            { \"name\": \"unused\" }, // 32768\n"
                                "            {\n"
                                "                \"name\": \"playerclip\",\n"
                                "                \"description\": \"Player cannot pass through the brush (other things can)\"\n"
                                "            }, // 65536\n"
                                "            {\n"
                                "                \"name\": \"mosterclip\",\n"
                                "                \"description\": \"Monster cannot pass through the brush (player and other things can)\"\n"
                                "            }, // 131072\n"
                                "            {\n"
                                "                \"name\": \"current_0\",\n"
                                "                \"description\": \"Brush has a current in direction of 0 degrees\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"current_90\",\n"
                                "                \"description\": \"Brush has a current in direction of 90 degrees\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"current_180\",\n"
                                "                \"description\": \"Brush has a current in direction of 180 degrees\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"current_270\",\n"
                                "                \"description\": \"Brush has a current in direction of 270 degrees\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"current_up\",\n"
                                "                \"description\": \"Brush has a current in the up direction\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"current_dn\",\n"
                                "                \"description\": \"Brush has a current in the down direction\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"origin\",\n"
                                "                \"description\": \"Special brush used for specifying origin of rotation for rotating brushes\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"monster\",\n"
                                "                \"description\": \"Purpose unknown\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"corpse\",\n"
                                "                \"description\": \"Purpose unknown\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"detail\",\n"
                                "                \"description\": \"Detail brush\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"translucent\",\n"
                                "                \"description\": \"Use for opaque water that does not block vis\"\n"
                                "            },\n"
                                "            {\n"
                                "                \"name\": \"ladder\",\n"
                                "                \"description\": \"Brushes with this flag allow a player to move up and down a vertical surface\"\n"
                                "            }\n"
                                "        ]\n"
                                "    }\n"
                                "}\n"
                                "\n");
            GameConfigParser parser(config);
            
            using Model::GameConfig;
            const GameConfig actual = parser.parse();
            
            GameConfig::FlagConfigList surfaceFlags;
            surfaceFlags.push_back(GameConfig::FlagConfig("light", "Emit light from the surface, brightness is specified in the 'value' field"));
            surfaceFlags.push_back(GameConfig::FlagConfig("slick", "The surface is slippery"));
            surfaceFlags.push_back(GameConfig::FlagConfig("sky", "The surface is sky, the texture will not be drawn, but the background sky box is used instead"));
            surfaceFlags.push_back(GameConfig::FlagConfig("warp", "The surface warps (like water textures do)"));
            surfaceFlags.push_back(GameConfig::FlagConfig("trans33", "The surface is 33% transparent"));
            surfaceFlags.push_back(GameConfig::FlagConfig("trans66", "The surface is 66% transparent"));
            surfaceFlags.push_back(GameConfig::FlagConfig("flowing", "The texture wraps in a downward 'flowing' pattern (warp must also be set)"));
            surfaceFlags.push_back(GameConfig::FlagConfig("nodraw", "Used for non-fixed-size brush triggers and clip brushes"));
            
            GameConfig::FlagConfigList contentFlags;
            contentFlags.push_back(GameConfig::FlagConfig("solid", "Default for all brushes"));
            contentFlags.push_back(GameConfig::FlagConfig("window", "Brush is a window (not really used)"));
            contentFlags.push_back(GameConfig::FlagConfig("aux", "Unused by the engine"));
            contentFlags.push_back(GameConfig::FlagConfig("lava", "The brush is lava"));
            contentFlags.push_back(GameConfig::FlagConfig("slime", "The brush is slime"));
            contentFlags.push_back(GameConfig::FlagConfig("water", "The brush is water"));
            contentFlags.push_back(GameConfig::FlagConfig("mist", "The brush is non-solid"));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("unused", ""));
            contentFlags.push_back(GameConfig::FlagConfig("playerclip", "Player cannot pass through the brush (other things can)"));
            contentFlags.push_back(GameConfig::FlagConfig("mosterclip", "Monster cannot pass through the brush (player and other things can)"));
            contentFlags.push_back(GameConfig::FlagConfig("current_0", "Brush has a current in direction of 0 degrees"));
            contentFlags.push_back(GameConfig::FlagConfig("current_90", "Brush has a current in direction of 90 degrees"));
            contentFlags.push_back(GameConfig::FlagConfig("current_180", "Brush has a current in direction of 180 degrees"));
            contentFlags.push_back(GameConfig::FlagConfig("current_270", "Brush has a current in direction of 270 degrees"));
            contentFlags.push_back(GameConfig::FlagConfig("current_up", "Brush has a current in the up direction"));
            contentFlags.push_back(GameConfig::FlagConfig("current_dn", "Brush has a current in the down direction"));
            contentFlags.push_back(GameConfig::FlagConfig("origin", "Special brush used for specifying origin of rotation for rotating brushes"));
            contentFlags.push_back(GameConfig::FlagConfig("monster", "Purpose unknown"));
            contentFlags.push_back(GameConfig::FlagConfig("corpse", "Purpose unknown"));
            contentFlags.push_back(GameConfig::FlagConfig("detail", "Detail brush"));
            contentFlags.push_back(GameConfig::FlagConfig("translucent", "Use for opaque water that does not block vis"));
            contentFlags.push_back(GameConfig::FlagConfig("ladder", "Brushes with this flag allow a player to move up and down a vertical surface"));

            using Model::BrushContentType;
            const GameConfig expected("Quake 2",
                                      Path(""),
                                      Path("Quake2/Icon.png"),
                                      StringUtils::makeList(1, "Quake2"),
                                      GameConfig::FileSystemConfig(Path("baseq2"), GameConfig::PackageFormatConfig("pak", "idpak")),
                                      GameConfig::TextureConfig(GameConfig::TexturePackageConfig(Path("textures")),
                                                                GameConfig::PackageFormatConfig("wal", "idwal"),
                                                                Path("pics/colormap.pcx"),
                                                                "_tb_textures"),
                                      GameConfig::EntityConfig(Path::List(1, Path("Quake2/Quake2.fgd")),
                                                               StringUtils::makeSet(1, "md2"),
                                                               Color(0.6f, 0.6f, 0.6f, 1.0f)),
                                      GameConfig::FaceAttribsConfig(surfaceFlags, contentFlags),
                                      BrushContentType::List());
            
            ASSERT_EQ(expected.name(), actual.name());
            ASSERT_EQ(expected.path(), actual.path());
            ASSERT_EQ(expected.icon(), actual.icon());
            ASSERT_EQ(expected.fileFormats(), actual.fileFormats());
            ASSERT_EQ(expected.fileSystemConfig(), actual.fileSystemConfig());
            ASSERT_EQ(expected.textureConfig(), actual.textureConfig());
            ASSERT_EQ(expected.entityConfig(), actual.entityConfig());
            ASSERT_EQ(expected.faceAttribsConfig(), actual.faceAttribsConfig());
            ASSERT_EQ(6u, actual.brushContentTypes().size());
        }
    }
}
