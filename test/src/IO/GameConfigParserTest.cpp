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
            const String config("{"
                                "   version = \"1\","
                                "	name = \"Quake\","
                                "	icon = \"Quake/Icon.png\","
                                " 	fileformats = { \"Standard\", \"Valve\" },"
                                "	filesystem = {"
                                "		searchpath = \"id1\","
                                "        packageformat = { extension = \"pak\", format = \"idpak\" }"
                                "	},"
                                "	textures = { "
                                "        package = { type = \"file\", format = { extension = \"wad\", format = \"wad2\" } },"
                                "        format = { extension = \"D\", format = \"idmip\" },"
                                "        palette = \"gfx/palette.lmp\","
                                "    	attribute = \"wad\""
                                "	},"
                                "  	entities = {"
                                "		definitions = { \"Quake/Quake.fgd\", \"Quake/Quoth2.fgd\", \"Quake/Rubicon2.def\" },"
                                "    	defaultcolor = \"0.6 0.6 0.6 1.0\","
                                "		modelformats = { \"mdl\", \"bsp\" }"
                                "    },"
                                "    brushtypes = {"
                                "        {"
                                "            name = \"Clip brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"clip\""
                                "        },"
                                "        {"
                                "            name = \"Skip brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"skip\""
                                "        },"
                                "        {"
                                "            name = \"Hint brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"hint\""
                                "        },"
                                "        {"
                                "            name = \"Liquid brushes\","
                                "            match = \"texture\","
                                "            pattern = \"\\**\""
                                "        },"
                                "        {"
                                "            name = \"Trigger brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"classname\","
                                "            pattern = \"trigger*\""
                                "        }"
                                "    }"
                                "}");
            
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
                                                                GameConfig::PackageFormatConfig("mip", "idmip"),
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
            const String config("{"
                                "    version = \"1\","
                                "    name = \"Quake 2\","
                                "    icon = \"Quake2/Icon.png\","
                                "    fileformats = { \"Quake2\" },"
                                "    filesystem = {"
                                "        searchpath = \"baseq2\","
                                "        packageformat = { extension = \"pak\", format = \"idpak\" }"
                                "    },"
                                "    textures = {"
                                "        package = { type = \"directory\", root = \"textures\" },"
                                "        format = { extension = \"wal\", format = \"idwal\" },"
                                "        palette = \"pics/colormap.pcx\","
                                "        attribute = \"_tb_textures\","
                                "    },"
                                "    entities = {"
                                "        definitions = \"Quake2/Quake2.fgd\","
                                "        defaultcolor = \"0.6 0.6 0.6 1.0\","
                                "        modelformats = { \"md2\" }"
                                "    },"
                                "    brushtypes = {"
                                "        {"
                                "            name = \"Clip brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"clip\""
                                "        },"
                                "        {"
                                "            name = \"Skip brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"skip\""
                                "        },"
                                "        {"
                                "            name = \"Hint brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"texture\","
                                "            pattern = \"hint\""
                                "        },"
                                "        {"
                                "            name = \"Detail brushes\","
                                "            match = \"contentflag\","
                                "            flags = { \"detail\" }"
                                "        },"
                                "        {"
                                "            name = \"Liquid brushes\","
                                "            match = \"contentflag\","
                                "            flags = { \"lava\", \"slime\", \"water\" }"
                                "        },"
                                "        {"
                                "            name = \"Trigger brushes\","
                                "            attribs = { \"transparent\" },"
                                "            match = \"classname\","
                                "            pattern = \"trigger*\""
                                "        }"
                                "    },"
                                "    faceattribs = {"
                                "        surfaceflags = {"
                                "            {"
                                "                name = \"light\","
                                "                description = \"Emit light from the surface, brightness is specified in the 'value' field\""
                                "            },"
                                "            {"
                                "                name = \"slick\","
                                "                description = \"The surface is slippery\""
                                "            },"
                                "            {"
                                "                name = \"sky\","
                                "                description = \"The surface is sky, the texture will not be drawn, but the background sky box is used instead\""
                                "            },"
                                "            {"
                                "                name = \"warp\","
                                "                description = \"The surface warps (like water textures do)\""
                                "            },"
                                "            {"
                                "                name = \"trans33\","
                                "                description = \"The surface is 33% transparent\""
                                "            },"
                                "            {"
                                "                name = \"trans66\","
                                "                description = \"The surface is 66% transparent\""
                                "            },"
                                "            {"
                                "                name = \"flowing\","
                                "                description = \"The texture wraps in a downward 'flowing' pattern (warp must also be set)\""
                                "            },"
                                "            {"
                                "                name = \"nodraw\","
                                "                description = \"Used for non-fixed-size brush triggers and clip brushes\""
                                "            }"
                                "        },"
                                "        contentflags = {"
                                "            {"
                                "                name = \"solid\","
                                "                description = \"Default for all brushes\""
                                "            }, // 1"
                                "            {"
                                "                name = \"window\","
                                "                description = \"Brush is a window (not really used)\""
                                "            }, // 2"
                                "            {"
                                "                name = \"aux\","
                                "                description = \"Unused by the engine\""
                                "            }, // 4"
                                "            {"
                                "                name = \"lava\","
                                "                description = \"The brush is lava\""
                                "            }, // 8"
                                "            {"
                                "                name = \"slime\","
                                "                description = \"The brush is slime\""
                                "            }, // 16"
                                "            {"
                                "                name = \"water\","
                                "                description = \"The brush is water\""
                                "            }, // 32"
                                "            {"
                                "                name = \"mist\","
                                "                description = \"The brush is non-solid\""
                                "            }, // 64"
                                "            { name = \"unused\" }, // 128"
                                "            { name = \"unused\" }, // 256"
                                "            { name = \"unused\" }, // 512"
                                "            { name = \"unused\" }, // 1024"
                                "            { name = \"unused\" }, // 2048"
                                "            { name = \"unused\" }, // 4096"
                                "            { name = \"unused\" }, // 8192"
                                "            { name = \"unused\" }, // 16384"
                                "            { name = \"unused\" }, // 32768"
                                "            {"
                                "                name = \"playerclip\","
                                "                description = \"Player cannot pass through the brush (other things can)\""
                                "            }, // 65536"
                                "            {"
                                "                name = \"mosterclip\","
                                "                description = \"Monster cannot pass through the brush (player and other things can)\""
                                "            }, // 131072"
                                "            {"
                                "                name = \"current_0\","
                                "                description = \"Brush has a current in direction of 0 degrees\""
                                "            },"
                                "            {"
                                "                name = \"current_90\","
                                "                description = \"Brush has a current in direction of 90 degrees\""
                                "            },"
                                "            {"
                                "                name = \"current_180\","
                                "                description = \"Brush has a current in direction of 180 degrees\""
                                "            },"
                                "            {"
                                "                name = \"current_270\","
                                "                description = \"Brush has a current in direction of 270 degrees\""
                                "            },"
                                "            {"
                                "                name = \"current_up\","
                                "                description = \"Brush has a current in the up direction\""
                                "            },"
                                "            {"
                                "                name = \"current_dn\","
                                "                description = \"Brush has a current in the down direction\""
                                "            },"
                                "            {"
                                "                name = \"origin\","
                                "                description = \"Special brush used for specifying origin of rotation for rotating brushes\""
                                "            },"
                                "            {"
                                "                name = \"monster\","
                                "                description = \"Purpose unknown\""
                                "            },"
                                "            {"
                                "                name = \"corpse\","
                                "                description = \"Purpose unknown\""
                                "            },"
                                "            {"
                                "                name = \"detail\","
                                "                description = \"Detail brush\""
                                "            },"
                                "            {"
                                "                name = \"translucent\","
                                "                description = \"Use for opaque water that does not block vis\""
                                "            },"
                                "            {"
                                "                name = \"ladder\","
                                "                description = \"Brushes with this flag allow a player to move up and down a vertical surface\""
                                "            }"
                                "        }"
                                "    }"
                                "}\n");
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
                                      StringUtils::makeList(2, "Quake2"),
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
            ASSERT_EQ(5u, actual.brushContentTypes().size());

        }
    }
}
