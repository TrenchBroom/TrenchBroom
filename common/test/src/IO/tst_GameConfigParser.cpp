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

#include "EL/Expression.h"
#include "EL/Expressions.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/GameConfigParser.h"
#include "IO/Reader.h"
#include "Model/GameConfig.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("GameConfigParserTest.parseIncludedGameConfigs")
{
  const auto basePath = Disk::getCurrentWorkingDir() + Path("fixture/games/");
  const auto cfgFiles =
    Disk::findRecursively(basePath, makeExtensionPathMatcher({"cfg"}));

  for (const Path& path : cfgFiles)
  {
    CAPTURE(path);

    auto file = Disk::openFile(path);
    auto reader = file->reader().buffer();

    GameConfigParser parser(reader.stringView(), path);
    CHECK_NOTHROW(parser.parse());
  }
}

TEST_CASE("GameConfigParserTest.parseBlankConfig")
{
  const std::string config("   ");
  GameConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("GameConfigParserTest.parseEmptyConfig")
{
  const std::string config("  {  } ");
  GameConfigParser parser(config);
  CHECK_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("GameConfigParserTest.parseQuakeConfig")
{
  const std::string config(R"(
{
    "version": 7,
    "unexpectedKey": [],
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
        "root": "textures",
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

  CHECK(
    GameConfigParser(config).parse()
    == Model::GameConfig{
      "Quake",
      Path{},
      Path{"Icon.png"},
      false,
      {// map formats
       Model::MapFormatConfig{"Standard", Path{}},
       Model::MapFormatConfig{"Valve", Path{}}},
      Model::FileSystemConfig{Path{"id1"}, Model::PackageFormatConfig{{"pak"}, "idpak"}},
      Model::TextureConfig{
        Path{"textures"},
        Model::PackageFormatConfig{{"D"}, "idmip"},
        Path{"gfx/palette.lmp"},
        "wad",
        Path{},
        {}},
      Model::EntityConfig{
        {Path{"Quake.fgd"},
         Path{"Quoth2.fgd"},
         Path{"Rubicon2.def"},
         Path{"Teamfortress.fgd"}},
        Color{0.6f, 0.6f, 0.6f, 1.0f},
        {},
        false},
      Model::FaceAttribsConfig{},
      {
        Model::SmartTag{
          "Trigger",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::EntityClassNameTagMatcher>("trigger*", "")},
        Model::SmartTag{
          "Clip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("clip")},
        Model::SmartTag{
          "Skip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("skip")},
        Model::SmartTag{
          "Hint",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("hint*")},
        Model::SmartTag{
          "Liquid", {}, std::make_unique<Model::TextureNameTagMatcher>("\\**")},
      },            // smart tags
      std::nullopt, // soft map bounds
      {}            // compilation tools
    });
}

TEST_CASE("GameConfigParserTest.parseQuake2Config")
{
  const std::string config(R"%(
{
    "version": 7,
    "name": "Quake 2",
    "icon": "Icon.png",
    "fileformats": [ { "format": "Quake2" } ],
    "filesystem": {
        "searchpath": "baseq2",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "root": "textures",
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
            { "unused": true }, // 1 << 7
            { "unused": true }, // 1 << 8
            { "unused": true }, // 1 << 9
            { "unused": true }, // 1 << 10
            { "unused": true }, // 1 << 11
            { "unused": true }, // 1 << 12
            { "unused": true }, // 1 << 13
            { "unused": true }, // 1 << 14
            { "unused": true }, // 1 << 15
            {
                "name": "playerclip",
                "description": "Player cannot pass through the brush (other things can)"
            }, // 1 << 16
            {
                "name": "monsterclip",
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

  CHECK(
    GameConfigParser(config).parse()
    == Model::GameConfig{
      "Quake 2",
      Path{},
      Path{"Icon.png"},
      false,
      {Model::MapFormatConfig{"Quake2", Path{}}},
      Model::FileSystemConfig{
        Path{"baseq2"}, Model::PackageFormatConfig{{"pak"}, "idpak"}},
      Model::TextureConfig{
        Path{"textures"},
        Model::PackageFormatConfig{{"wal"}, "wal"},
        Path{"pics/colormap.pcx"},
        "_tb_textures",
        Path{},
        {}},
      Model::EntityConfig{{Path{"Quake2.fgd"}}, Color{0.6f, 0.6f, 0.6f, 1.0f}, {}, false},
      Model::FaceAttribsConfig{
        {{{"light",
           "Emit light from the surface, brightness is specified in the 'value' field",
           1 << 0},
          {"slick", "The surface is slippery", 1 << 1},
          {"sky",
           "The surface is sky, the texture will not be drawn, but the background sky "
           "box is used "
           "instead",
           1 << 2},
          {"warp", "The surface warps (like water textures do)", 1 << 3},
          {"trans33", "The surface is 33% transparent", 1 << 4},
          {"trans66", "The surface is 66% transparent", 1 << 5},
          {"flowing",
           "The texture wraps in a downward 'flowing' pattern (warp must also be set)",
           1 << 6},
          {"nodraw", "Used for non-fixed-size brush triggers and clip brushes", 1 << 7},
          {"hint", "Make a primary bsp splitter", 1 << 8},
          {"skip", "Completely ignore, allowing non-closed brushes", 1 << 9}}},
        {{{"solid", "Default for all brushes", 1 << 0},
          {"window", "Brush is a window (not really used)", 1 << 1},
          {"aux", "Unused by the engine", 1 << 2},
          {"lava", "The brush is lava", 1 << 3},
          {"slime", "The brush is slime", 1 << 4},
          {"water", "The brush is water", 1 << 5},
          {"mist", "The brush is non-solid", 1 << 6},
          {"playerclip",
           "Player cannot pass through the brush (other things can)",
           1 << 16},
          {"monsterclip",
           "Monster cannot pass through the brush (player and other things can)",
           1 << 17},
          {"current_0", "Brush has a current in direction of 0 degrees", 1 << 18},
          {"current_90", "Brush has a current in direction of 90 degrees", 1 << 19},
          {"current_180", "Brush has a current in direction of 180 degrees", 1 << 20},
          {"current_270", "Brush has a current in direction of 270 degrees", 1 << 21},
          {"current_up", "Brush has a current in the up direction", 1 << 22},
          {"current_dn", "Brush has a current in the down direction", 1 << 23},
          {"origin",
           "Special brush used for specifying origin of rotation for rotating brushes",
           1 << 24},
          {"monster", "Purpose unknown", 1 << 25},
          {"corpse", "Purpose unknown", 1 << 26},
          {"detail", "Detail brush", 1 << 27},
          {"translucent", "Use for opaque water that does not block vis", 1 << 28},
          {"ladder",
           "Brushes with this flag allow a player to move up and down a vertical surface",
           1 << 29}}},
        Model::BrushFaceAttributes{Model::BrushFaceAttributes::NoTextureName}},
      {
        Model::SmartTag{
          "Trigger",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::EntityClassNameTagMatcher>("trigger*", "trigger")},
        Model::SmartTag{
          "Clip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("clip")},
        Model::SmartTag{
          "Skip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("skip")},
        Model::SmartTag{
          "Hint",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("hint*")},
        Model::SmartTag{
          "Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27)},
        Model::SmartTag{
          "Liquid",
          {},
          std::make_unique<Model::ContentFlagsTagMatcher>(
            (1 << 3) | (1 << 4) | (1 << 5))},
        Model::SmartTag{
          "trans",
          {},
          std::make_unique<Model::SurfaceFlagsTagMatcher>((1 << 4) | (1 << 5))},
      },            // smart tags
      std::nullopt, // soft map bounds
      {}            // compilation tools
    });
}

TEST_CASE("GameConfigParserTest.parseExtrasConfig")
{
  const std::string config(R"%(
{
    "version": 7,
    "name": "Extras",
    "fileformats": [ { "format": "Quake3" } ],
    "filesystem": {
        "searchpath": "baseq3",
        "packageformat": { "extension": "pk3", "format": "zip" }
    },
    "textures": {
        "root": "textures",
        "format": { "extensions": [ "" ], "format": "q3shader" },
        "shaderSearchPath": "scripts", // this will likely change when we get a material system
        "attribute": "_tb_textures",
        "excludes": [
            "*_norm",
            "*_gloss"
        ]
    },
    "entities": {
        "definitions": [ "Extras.ent" ],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "md3" ],
        "scale": [ modelscale, modelscale_vec ]
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
                "match": "surfaceparm",
                "pattern": "playerclip"
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
            }
        ]
    },
    "faceattribs": {
        "defaults": {
            "textureName": "defaultTexture",
            "offset": [0, 0],
            "scale": [0.5, 0.5],
            "rotation": 0,
            "surfaceFlags": [ "slick" ],
            "surfaceContents": [ "solid" ],
            "surfaceValue": 0,
            "color": "1.0 1.0 1.0 1.0"
        },
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
            }, // 1
            {
                "name": "window",
                "description": "Brush is a window (not really used)"
            }, // 2
            {
                "name": "aux",
                "description": "Unused by the engine"
            }, // 4
            {
                "name": "lava",
                "description": "The brush is lava"
            }, // 8
            {
                "name": "slime",
                "description": "The brush is slime"
            }, // 16
            {
                "name": "water",
                "description": "The brush is water"
            }, // 32
            {
                "name": "mist",
                "description": "The brush is non-solid"
            }, // 64
            { "unused": true }, // 128
            { "unused": true }, // 256
            { "unused": true }, // 512
            { "unused": true }, // 1024
            { "unused": true }, // 2048
            { "unused": true }, // 4096
            { "unused": true }, // 8192
            { "unused": true }, // 16384
            { "unused": true }, // 32768
            {
                "name": "playerclip",
                "description": "Player cannot pass through the brush (other things can)"
            }, // 65536
            {
                "name": "monsterclip",
                "description": "Monster cannot pass through the brush (player and other things can)"
            }, // 131072
            {
                "name": "current_0",
                "description": "Brush has a current in direction of 0 degrees"
            },
            {
                "name": "current_90",
                "description": "Brush has a current in direction of 90 degrees"
            },
            {
                "name": "current_180",
                "description": "Brush has a current in direction of 180 degrees"
            },
            {
                "name": "current_270",
                "description": "Brush has a current in direction of 270 degrees"
            },
            {
                "name": "current_up",
                "description": "Brush has a current in the up direction"
            },
            {
                "name": "current_dn",
                "description": "Brush has a current in the down direction"
            },
            {
                "name": "origin",
                "description": "Special brush used for specifying origin of rotation for rotating brushes"
            },
            {
                "name": "monster",
                "description": "Purpose unknown"
            },
            {
                "name": "corpse",
                "description": "Purpose unknown"
            },
            {
                "name": "detail",
                "description": "Detail brush"
            },
            {
                "name": "translucent",
                "description": "Use for opaque water that does not block vis"
            },
            {
                "name": "ladder",
                "description": "Brushes with this flag allow a player to move up and down a vertical surface"
            }
        ]
    }
}
)%");

  Model::BrushFaceAttributes expectedBrushFaceAttributes("defaultTexture");
  expectedBrushFaceAttributes.setOffset(vm::vec2f(0.0f, 0.0f));
  expectedBrushFaceAttributes.setScale(vm::vec2f(0.5f, 0.5f));
  expectedBrushFaceAttributes.setRotation(0.0f);
  expectedBrushFaceAttributes.setSurfaceContents(1 << 0);
  expectedBrushFaceAttributes.setSurfaceFlags(1 << 1);
  expectedBrushFaceAttributes.setSurfaceValue(0.0f);
  expectedBrushFaceAttributes.setColor(Color(255, 255, 255, 255));

  CHECK(
    GameConfigParser(config).parse()
    == Model::GameConfig{
      "Extras",
      Path{},
      Path{},
      false,
      {Model::MapFormatConfig{"Quake3", Path{}}},
      Model::FileSystemConfig{Path{"baseq3"}, Model::PackageFormatConfig{{"pk3"}, "zip"}},
      Model::TextureConfig{
        Path{"textures"},
        Model::PackageFormatConfig{{""}, "q3shader"},
        Path{},
        "_tb_textures",
        Path{"scripts"},
        {"*_norm", "*_gloss"}},
      Model::EntityConfig{
        {Path{"Extras.ent"}},
        Color{0.6f, 0.6f, 0.6f, 1.0f},
        EL::Expression{
          EL::ArrayExpression{{
            // the line numbers are not checked
            EL::Expression{EL::VariableExpression{"modelscale"}, 0, 0},
            EL::Expression{EL::VariableExpression{"modelscale_vec"}, 0, 0},
          }},
          0,
          0},
        false},
      Model::FaceAttribsConfig{
        {{{"light",
           "Emit light from the surface, brightness is specified in the 'value' field",
           1 << 0},
          {"slick", "The surface is slippery", 1 << 1},
          {"sky",
           "The surface is sky, the texture will not be drawn, but the background sky "
           "box is used "
           "instead",
           1 << 2},
          {"warp", "The surface warps (like water textures do)", 1 << 3},
          {"trans33", "The surface is 33% transparent", 1 << 4},
          {"trans66", "The surface is 66% transparent", 1 << 5},
          {"flowing",
           "The texture wraps in a downward 'flowing' pattern (warp must also be set)",
           1 << 6},
          {"nodraw", "Used for non-fixed-size brush triggers and clip brushes", 1 << 7},
          {"hint", "Make a primary bsp splitter", 1 << 8},
          {"skip", "Completely ignore, allowing non-closed brushes", 1 << 9}}},
        {{{"solid", "Default for all brushes", 1 << 0},
          {"window", "Brush is a window (not really used)", 1 << 1},
          {"aux", "Unused by the engine", 1 << 2},
          {"lava", "The brush is lava", 1 << 3},
          {"slime", "The brush is slime", 1 << 4},
          {"water", "The brush is water", 1 << 5},
          {"mist", "The brush is non-solid", 1 << 6},
          {"playerclip",
           "Player cannot pass through the brush (other things can)",
           1 << 16},
          {"monsterclip",
           "Monster cannot pass through the brush (player and other things can)",
           1 << 17},
          {"current_0", "Brush has a current in direction of 0 degrees", 1 << 18},
          {"current_90", "Brush has a current in direction of 90 degrees", 1 << 19},
          {"current_180", "Brush has a current in direction of 180 degrees", 1 << 20},
          {"current_270", "Brush has a current in direction of 270 degrees", 1 << 21},
          {"current_up", "Brush has a current in the up direction", 1 << 22},
          {"current_dn", "Brush has a current in the down direction", 1 << 23},
          {"origin",
           "Special brush used for specifying origin of rotation for rotating brushes",
           1 << 24},
          {"monster", "Purpose unknown", 1 << 25},
          {"corpse", "Purpose unknown", 1 << 26},
          {"detail", "Detail brush", 1 << 27},
          {"translucent", "Use for opaque water that does not block vis", 1 << 28},
          {"ladder",
           "Brushes with this flag allow a player to move up and down a vertical surface",
           1 << 29}}},
        expectedBrushFaceAttributes},
      {
        Model::SmartTag{
          "Trigger",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::EntityClassNameTagMatcher>("trigger*", "trigger")},
        Model::SmartTag{
          "Clip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("clip")},
        Model::SmartTag{
          "Skip",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("skip")},
        Model::SmartTag{
          "Hint",
          {Model::TagAttribute{1u, "transparent"}},
          std::make_unique<Model::TextureNameTagMatcher>("hint*")},
        Model::SmartTag{
          "Detail", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1 << 27)},
        Model::SmartTag{
          "Liquid",
          {},
          std::make_unique<Model::ContentFlagsTagMatcher>(
            (1 << 3) | (1 << 4) | (1 << 5))},
      },            // smart tags
      std::nullopt, // soft map bounds
      {}            // compilation tools
    });
}

TEST_CASE("GameConfigParserTest.parseDuplicateTags")
{
  const std::string config(R"(
{
    "version": 7,
    "name": "Quake",
    "icon": "Icon.png",
    "fileformats": [
        { "format": "Standard" }
    ],
    "filesystem": {
        "searchpath": "id1",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "root": "textures",
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
                "name": "Trigger",
                "attribs": [ "transparent" ],
                "match": "texture",
                "pattern": "clip"
            }
        ]
    }
}
)");

  GameConfigParser parser(config);
  REQUIRE_THROWS_AS(parser.parse(), ParserException);
}

TEST_CASE("GameConfigParserTest.parseSetDefaultProperties")
{
  const std::string config(R"(
{
    "version": 7,
    "name": "Quake",
    "icon": "Icon.png",
    "fileformats": [
        { "format": "Standard" }
    ],
    "filesystem": {
        "searchpath": "id1",
        "packageformat": { "extension": "pak", "format": "idpak" }
    },
    "textures": {
        "root": "textures",
        "format": { "extension": "D", "format": "idmip" },
        "palette": "gfx/palette.lmp",
        "attribute": "wad"
    },
    "entities": {
        "definitions": [ "Quake.fgd", "Quoth2.fgd", "Rubicon2.def", "Teamfortress.fgd" ],
        "defaultcolor": "0.6 0.6 0.6 1.0",
        "modelformats": [ "mdl", "bsp" ],
        "setDefaultProperties": true
    }
}
)");

  CHECK(
    GameConfigParser(config).parse()
    == Model::GameConfig{
      "Quake",
      Path{},
      Path{"Icon.png"},
      false,
      {Model::MapFormatConfig{"Standard", Path{}}},
      Model::FileSystemConfig{Path{"id1"}, Model::PackageFormatConfig{{"pak"}, "idpak"}},
      Model::TextureConfig{
        Path{"textures"},
        Model::PackageFormatConfig{{"D"}, "idmip"},
        Path{"gfx/palette.lmp"},
        "wad",
        Path{},
        {}},
      Model::EntityConfig{
        {Path{"Quake.fgd"},
         Path{"Quoth2.fgd"},
         Path{"Rubicon2.def"},
         Path{"Teamfortress.fgd"}},
        Color{0.6f, 0.6f, 0.6f, 1.0f},
        {},
        true}, // setDefaultProperties
      Model::FaceAttribsConfig{},
      {},
      std::nullopt, // soft map bounds
      {}            // compilation tools
    });
}
} // namespace IO
} // namespace TrenchBroom
