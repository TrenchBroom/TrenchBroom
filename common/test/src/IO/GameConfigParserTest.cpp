/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/GameConfigParser.h"
#include "IO/Reader.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"

#include <string>

namespace TrenchBroom {
    namespace IO {
        TEST(GameConfigParserTest, parseIncludedGameConfigs) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
            const Path::List cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("cfg"));

            for (const Path& path : cfgFiles) {
                auto file = Disk::openFile(path);
                auto reader = file->reader().buffer();

                GameConfigParser parser(std::begin(reader), std::end(reader), path);
                try {
                    parser.parse();
                } catch (const std::exception& e) {
                    FAIL() << "Parsing game config " << path.asString() << " failed: " << e.what();
                }
            }
        }

        TEST(GameConfigParserTest, parseBlankConfig) {
            const std::string config("   ");
            GameConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(GameConfigParserTest, parseEmptyConfig) {
            const std::string config("  {  } ");
            GameConfigParser parser(config);
            ASSERT_THROW(parser.parse(), ParserException);
        }

        TEST(GameConfigParserTest, parseQuakeConfig) {
            const std::string config(R"(
{
    "version": 3,
    "name": "Quake",
    "icon": "Icon.png",
    "fileformats": [
        { "format": "Standard" },
        { "format": "Valve" }
    ],
    "filesystem": {
        "searchpath": "id1",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "package": { "type": "file", "format": { "extension": "wad", "format": "wad2" } },
        "format": { "extension": "D", "format": "idmip" },
        "palette": "gfx/palette.lmp",
        "attribute": "wad"
    },
    "entities": {
        "definitions": [ "Quake.fgd", "Quoth2.fgd", "Rubicon2.def", "Teamfortress.fgd" ],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "mdl", "bsp" ]
    },
    "tags": {
        "brush": [
            {
                "name": "Trigger",
                "attribs": [ "transparent" ],
                "match": "classname",
                "pattern": "trigger*"
            }
        ],
        "brushface": [
            {
                "name": "Clip",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "clip"
            },
            {
                "name": "Skip",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "skip"
            },
            {
                "name": "Hint",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "hint*"
            },
            {
                "name": "Liquid",
                "match": "texture",
                "pattern": "\**"
            }
        ]
    }
}
)");

            GameConfigParser parser(config);

            using Model::GameConfig;
            const GameConfig actual = parser.parse();

            const GameConfig expected("Quake",
                Path(),
                Path("Icon.png"),
                false,
                { // map formats
                    GameConfig::MapFormatConfig("Standard", Path()),
                    GameConfig::MapFormatConfig("Valve", Path())
                },
                GameConfig::FileSystemConfig(Path("id1"), GameConfig::PackageFormatConfig("pak", "idpak")),
                GameConfig::TextureConfig(
                    GameConfig::TexturePackageConfig(GameConfig::PackageFormatConfig("wad", "wad2")),
                    GameConfig::PackageFormatConfig("D", "idmip"),
                    Path("gfx/palette.lmp"),
                    "wad",
                    Path()),
                GameConfig::EntityConfig(
                    { Path("Quake.fgd"), Path("Quoth2.fgd"), Path("Rubicon2.def"), Path("Teamfortress.fgd") },
                    { "bsp", "mdl" },
                    Color(0.6f, 0.6f, 0.6f, 1.0f)),
                GameConfig::FaceAttribsConfig(),
                {
                  Model::SmartTag("Trigger", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::EntityClassNameTagMatcher>("trigger*", "")),
                  Model::SmartTag("Clip", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("clip")),
                  Model::SmartTag("Skip", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("skip")),
                  Model::SmartTag("Hint", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("hint*")),
                  Model::SmartTag("Liquid", {}, std::make_unique<Model::TextureNameTagMatcher>("\\**")),
                } // smart tags
            );

            ASSERT_EQ(expected.name(), actual.name());
            ASSERT_EQ(expected.path(), actual.path());
            ASSERT_EQ(expected.icon(), actual.icon());
            ASSERT_EQ(expected.experimental(), actual.experimental());
            ASSERT_EQ(expected.fileFormats(), actual.fileFormats());
            ASSERT_EQ(expected.fileSystemConfig(), actual.fileSystemConfig());
            ASSERT_EQ(expected.textureConfig(), actual.textureConfig());
            ASSERT_EQ(expected.entityConfig(), actual.entityConfig());
            ASSERT_EQ(expected.faceAttribsConfig(), actual.faceAttribsConfig());
            ASSERT_EQ(expected.smartTags(), actual.smartTags());
        }

        TEST(GameConfigParserTest, parseQuake2Config) {
            const std::string config(R"%(
{
    "version": 3,
    "name": "Quake 2",
    "icon": "Icon.png",
    "fileformats": [ { "format": "Quake2" } ],
    "filesystem": {
        "searchpath": "baseq2",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "package": { "type": "directory", "root": "textures" },
        "format": { "extension": "wal", "format": "wal" },
        "palette": "pics/colormap.pcx",
        "attribute": "_tb_textures"
    },
    "entities": {
        "definitions": [ "Quake2.fgd" ],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "md2" ]
    },
    "tags": {
        "brush": [
            {
                "name": "Trigger",
                "attribs": [ "transparent" ],
                "match": "classname",
                "pattern": "trigger*",
                "texture": "trigger"
            }
        ],
        "brushface": [
            {
                "name": "Clip",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "clip"
            },
            {
                "name": "Skip",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "skip"
            },
            {
                "name": "Hint",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "hint*"
            },
            {
                "name": "Detail",
                "match": "contentflag",
                "flags": [ "detail" ]
            },
            {
                "name": "Liquid",
                "match": "contentflag",
                "flags": [ "lava", "slime", "water" ]
            },
            {
                "name": "trans",
                "attribs": [ "transparent" ],
                "match": "surfaceflag",
                "flags": [ "trans33", "trans66" ]
            }
        ]
    },
    "faceattribs": {
        "surfaceflags": [
            {
                "name": "light",
                "description": "Emit light from the surface, brightness is specified in the 'value' field"
            },
            {
                "name": "slick",
                "description": "The surface is slippery"
            },
            {
                "name": "sky",
                "description": "The surface is sky, the texture will not be drawn, but the background sky box is used instead"
            },
            {
                "name": "warp",
                "description": "The surface warps (like water textures do)"
            },
            {
                "name": "trans33",
                "description": "The surface is 33% transparent"
            },
            {
                "name": "trans66",
                "description": "The surface is 66% transparent"
            },
            {
                "name": "flowing",
                "description": "The texture wraps in a downward 'flowing' pattern (warp must also be set)"
            },
            {
                "name": "nodraw",
                "description": "Used for non-fixed-size brush triggers and clip brushes"
            },
            {
                "name": "hint",
                "description": "Make a primary bsp splitter"
            },
            {
                "name": "skip",
                "description": "Completely ignore, allowing non-closed brushes"
            }
        ],
        "contentflags": [
            {
                "name": "solid",
                "description": "Default for all brushes"
            }, // 1 << 0
            {
                "name": "window",
                "description": "Brush is a window (not really used)"
            }, // 1 << 1
            {
                "name": "aux",
                "description": "Unused by the engine"
            }, // 1 << 2
            {
                "name": "lava",
                "description": "The brush is lava"
            }, // 1 << 3
            {
                "name": "slime",
                "description": "The brush is slime"
            }, // 1 << 4
            {
                "name": "water",
                "description": "The brush is water"
            }, // 1 << 5
            {
                "name": "mist",
                "description": "The brush is non-solid"
            }, // 1 << 6
            { "name": "unused" }, // 1 << 7
            { "name": "unused" }, // 1 << 8
            { "name": "unused" }, // 1 << 9
            { "name": "unused" }, // 1 << 10
            { "name": "unused" }, // 1 << 11
            { "name": "unused" }, // 1 << 12
            { "name": "unused" }, // 1 << 13
            { "name": "unused" }, // 1 << 14
            { "name": "unused" }, // 1 << 15
            {
                "name": "playerclip",
                "description": "Player cannot pass through the brush (other things can)"
            }, // 1 << 16
            {
                "name": "mosterclip",
                "description": "Monster cannot pass through the brush (player and other things can)"
            }, // 1 << 17
            {
                "name": "current_0",
                "description": "Brush has a current in direction of 0 degrees"
            }, // 1 << 18
            {
                "name": "current_90",
                "description": "Brush has a current in direction of 90 degrees"
            }, // 1 << 19
            {
                "name": "current_180",
                "description": "Brush has a current in direction of 180 degrees"
            }, // 1 << 20
            {
                "name": "current_270",
                "description": "Brush has a current in direction of 270 degrees"
            }, // 1 << 21
            {
                "name": "current_up",
                "description": "Brush has a current in the up direction"
            }, // 1 << 22
            {
                "name": "current_dn",
                "description": "Brush has a current in the down direction"
            }, // 1 << 23
            {
                "name": "origin",
                "description": "Special brush used for specifying origin of rotation for rotating brushes"
            }, // 1 << 24
            {
                "name": "monster",
                "description": "Purpose unknown"
            }, // 1 << 25
            {
                "name": "corpse",
                "description": "Purpose unknown"
            }, // 1 << 26
            {
                "name": "detail",
                "description": "Detail brush"
            }, // 1 << 27
            {
                "name": "translucent",
                "description": "Use for opaque water that does not block vis"
            }, // 1 << 28
            {
                "name": "ladder",
                "description": "Brushes with this flag allow a player to move up and down a vertical surface"
            } // 1 << 29
        ]
    }
}
)%");

            GameConfigParser parser(config);

            using Model::GameConfig;
            const GameConfig actual = parser.parse();

            const GameConfig expected(
                "Quake 2",
                Path(),
                Path("Icon.png"),
                false,
                GameConfig::MapFormatConfig::List({
                    GameConfig::MapFormatConfig("Quake2", Path())
                }),
                GameConfig::FileSystemConfig(Path("baseq2"), GameConfig::PackageFormatConfig("pak", "idpak")),
                GameConfig::TextureConfig(
                    GameConfig::TexturePackageConfig(Path("textures")),
                    GameConfig::PackageFormatConfig("wal", "wal"),
                    Path("pics/colormap.pcx"),
                    "_tb_textures",
                    Path()),
                GameConfig::EntityConfig(
                    { Path("Quake2.fgd") },
                    { "md2" },
                    Color(0.6f, 0.6f, 0.6f, 1.0f)),
                GameConfig::FaceAttribsConfig(
                    {
                        { "light", "Emit light from the surface, brightness is specified in the 'value' field" },
                        { "slick", "The surface is slippery" },
                        { "sky", "The surface is sky, the texture will not be drawn, but the background sky box is used instead" },
                        { "warp", "The surface warps (like water textures do)" },
                        { "trans33", "The surface is 33% transparent" },
                        { "trans66", "The surface is 66% transparent" },
                        { "flowing", "The texture wraps in a downward 'flowing' pattern (warp must also be set)" },
                        { "nodraw", "Used for non-fixed-size brush triggers and clip brushes" },
                        { "hint", "Make a primary bsp splitter" },
                        { "skip", "Completely ignore, allowing non-closed brushes" }
                    },
                    {
                        { "solid", "Default for all brushes" },
                        { "window", "Brush is a window (not really used)" },
                        { "aux", "Unused by the engine" },
                        { "lava", "The brush is lava" },
                        { "slime", "The brush is slime" },
                        { "water", "The brush is water" },
                        { "mist", "The brush is non-solid" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "unused", "" },
                        { "playerclip", "Player cannot pass through the brush (other things can)" },
                        { "mosterclip", "Monster cannot pass through the brush (player and other things can)" },
                        { "current_0", "Brush has a current in direction of 0 degrees" },
                        { "current_90", "Brush has a current in direction of 90 degrees" },
                        { "current_180", "Brush has a current in direction of 180 degrees" },
                        { "current_270", "Brush has a current in direction of 270 degrees" },
                        { "current_up", "Brush has a current in the up direction" },
                        { "current_dn", "Brush has a current in the down direction" },
                        { "origin", "Special brush used for specifying origin of rotation for rotating brushes" },
                        { "monster", "Purpose unknown" },
                        { "corpse", "Purpose unknown" },
                        { "detail", "Detail brush" },
                        { "translucent", "Use for opaque water that does not block vis" },
                        { "ladder", "Brushes with this flag allow a player to move up and down a vertical surface" }
                    }),
                {
                    Model::SmartTag("Trigger", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::EntityClassNameTagMatcher>("trigger*", "trigger")),
                    Model::SmartTag("Clip", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("clip")),
                    Model::SmartTag("Skip", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("skip")),
                    Model::SmartTag("Hint", { Model::TagAttribute(1u, "transparent") }, std::make_unique<Model::TextureNameTagMatcher>("hint*")),
                    Model::SmartTag("Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27)),
                    Model::SmartTag("Liquid", {}, std::make_unique<Model::ContentFlagsTagMatcher>((1 << 3) | (1 << 4) | (1 << 5))),
                    Model::SmartTag("trans", {}, std::make_unique<Model::SurfaceFlagsTagMatcher>((1 << 4) | (1 << 5))),
                } // smart tags
            );

            ASSERT_EQ(expected.name(), actual.name());
            ASSERT_EQ(expected.path(), actual.path());
            ASSERT_EQ(expected.icon(), actual.icon());
            ASSERT_EQ(expected.experimental(), actual.experimental());
            ASSERT_EQ(expected.fileFormats(), actual.fileFormats());
            ASSERT_EQ(expected.fileSystemConfig(), actual.fileSystemConfig());
            ASSERT_EQ(expected.textureConfig(), actual.textureConfig());
            ASSERT_EQ(expected.entityConfig(), actual.entityConfig());
            ASSERT_EQ(expected.faceAttribsConfig(), actual.faceAttribsConfig());
            ASSERT_EQ(expected.smartTags(), actual.smartTags());
        }
    }
}
