/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/GameConfigFixture.h"

namespace tb::mdl
{
namespace
{

GameInfo makeGameInfoFixture(GameConfig gameConfig, std::filesystem::path gamePath)
{
  auto gameInfo = makeGameInfo(std::move(gameConfig));
  gameInfo.gamePathPreference =
    Preference<std::filesystem::path>{gameInfo.gamePathPreference.path, gamePath};

  return gameInfo;
}

} // namespace

const GameInfo DefaultGameInfo = makeGameInfoFixture(
  {
    .name = "Test",
    .path = {},
    .icon = {},
    .experimental = false,
    .fileFormats = {},
    .fileSystemConfig = {},
    .materialConfig =
      {"textures", {".D"}, "fixture/test/mdl/DefaultGameInfo/palette.lmp", {}, "", {}},
    .entityConfig = {},
    .faceAttribsConfig = {},
    .smartTags = {},
    .softMapBounds = {},
    .compilationTools = {},
    .forceEmptyNewMap = true,
  },
  std::filesystem::current_path());

const GameInfo QuakeGameInfo = makeGameInfoFixture(
  {
    .name = "Quake",
    .path = {},
    .icon = "Icon.png",
    .experimental = false,
    .fileFormats =
      {
        {.format = "Valve", .initialMap = ""},
        {.format = "Standard", .initialMap = ""},
      },
    .fileSystemConfig =
      {
        .searchPath = "id1",
        .packageFormat = {.extensions = {".pak"}, .format = "idpak"},
      },
    .materialConfig =
      {
        .root = "textures",
        .extensions = {".D"},
        .palette = "gfx/palette.lmp",
        .property = "wad",
        .shaderSearchPath = "",
        .excludes = {},
      },
    .entityConfig =
      {
        .defFilePaths = {"Quake.fgd", "Quoth2.fgd", "Rubicon2.def", "Teamfortress.fgd"},
        .defaultColor = RgbaF{0.6f, 0.6f, 0.6f, 1.0f},
        .scaleExpression = std::nullopt,
      },
    .faceAttribsConfig = {},
    .smartTags = {},
    .softMapBounds = vm::bbox3d{4096.0},
    .compilationTools = {},
    .forceEmptyNewMap = true,
  },
  std::filesystem::current_path() / "fixture" / "test" / "mdl" / "Game" / "Quake");

const GameInfo Quake2GameInfo = makeGameInfoFixture(
  {
    .name = "Quake 2",
    .path = {},
    .icon = "Icon.png",
    .experimental = false,
    .fileFormats =
      {
        {.format = "Quake2", .initialMap = ""},
        {.format = "Quake2 (Valve)", .initialMap = ""},
      },
    .fileSystemConfig =
      {
        .searchPath = "baseq2",
        .packageFormat = {.extensions = {".pak"}, .format = "idpak"},
      },
    .materialConfig =
      {
        .root = "textures",
        .extensions = {".wal"},
        .palette = "pics/colormap.pcx",
        .property = std::nullopt,
        .shaderSearchPath = "",
        .excludes = {},
      },
    .entityConfig =
      {
        .defFilePaths = {"Quake2.fgd"},
        .defaultColor = RgbaF{0.6f, 0.6f, 0.6f, 1.0f},
        .scaleExpression = std::nullopt,
      },
    .faceAttribsConfig = {},
    .smartTags = {},
    .softMapBounds = {},
    .compilationTools = {},
    .forceEmptyNewMap = true,
  },
  std::filesystem::current_path() / "fixture" / "test" / "mdl" / "Game" / "Quake2");

} // namespace tb::mdl
