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

#include "MapFixture.h"
#include "MockGame.h"
#include "io/MapHeader.h"
#include "io/TestEnvironment.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Persistence")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  SECTION("load")
  {
    SECTION("Format detection")
    {
      auto gameConfig = MockGameConfig{};
      gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Standard", {}},
        {"Valve", {}},
        {"Quake3", {}},
      };

      SECTION("Detect Valve Format Map")
      {
        fixture.load(
          "fixture/test/ui/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
          {.game = MockGameFixture{std::move(gameConfig)}});

        CHECK(map.world()->mapFormat() == mdl::MapFormat::Valve);
        CHECK(map.world()->defaultLayer()->childCount() == 1);
      }

      SECTION("Detect Standard Format Map")
      {
        fixture.load(
          "fixture/test/ui/MapDocumentTest/standardFormatMapWithoutFormatTag.map",
          {.game = MockGameFixture{std::move(gameConfig)}});

        CHECK(map.world()->mapFormat() == mdl::MapFormat::Standard);
        CHECK(map.world()->defaultLayer()->childCount() == 1);
      }

      SECTION("detectEmptyMap")
      {
        fixture.load(
          "fixture/test/ui/MapDocumentTest/emptyMapWithoutFormatTag.map",
          {.game = LoadGameFixture{"Quake"}});

        // an empty map detects as Valve because Valve is listed first in the Quake game
        // config
        CHECK(map.world()->mapFormat() == mdl::MapFormat::Valve);
        CHECK(map.world()->defaultLayer()->childCount() == 0);
      }

      SECTION("mixedFormats")
      {
        // map has both Standard and Valve brushes
        CHECK_THROWS_AS(
          fixture.load(
            "fixture/test/ui/MapDocumentTest/mixedFormats.map",
            {.game = LoadGameFixture{"Quake"}}),
          std::runtime_error);
      }
    }
  }

  SECTION("saveAs")
  {
    SECTION("Writing map header")
    {
      fixture.load(
        "fixture/test/ui/MapDocumentTest/valveFormatMapWithoutFormatTag.map",
        {.game = LoadGameFixture{"Quake"}});
      REQUIRE(map.world()->mapFormat() == mdl::MapFormat::Valve);

      auto env = io::TestEnvironment{};

      const auto newDocumentPath = std::filesystem::path{"test.map"};
      map.saveAs(env.dir() / newDocumentPath);
      REQUIRE(env.fileExists(newDocumentPath));

      const auto newDocumentContent = env.loadFile(newDocumentPath);
      auto istr = std::istringstream{newDocumentContent};

      CHECK(
        io::readMapHeader(istr)
        == Result<std::pair<std::optional<std::string>, mdl::MapFormat>>{
          std::pair{"Quake", mdl::MapFormat::Valve}});
    }
  }

  SECTION("exportAs")
  {
    auto env = io::TestEnvironment{};

    SECTION("omit layers from export")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      {
        fixture.create({.game = LoadGameFixture{"Quake"}});

        auto layer = mdl::Layer{"Layer"};
        layer.setOmitFromExport(true);

        auto* layerNode = new mdl::LayerNode{std::move(layer)};
        addNodes(map, {{map.world(), {layerNode}}});

        REQUIRE(
          map.exportAs(io::MapExportOptions{env.dir() / newDocumentPath}).is_success());
        REQUIRE(env.fileExists(newDocumentPath));
      }

      fixture.load(
        env.dir() / newDocumentPath,
        {.mapFormat = MapFormat::Standard, .game = LoadGameFixture{"Quake"}});
      CHECK(map.world()->customLayers().empty());
    }
  }
}

} // namespace tb::mdl
