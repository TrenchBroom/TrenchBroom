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

#include "Logger.h"
#include "Observer.h"
#include "mdl/EntityNode.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("MapDocument")
{
  auto logger = NullLogger{};
  const auto environmentConfig = mdl::EnvironmentConfig{};
  auto taskManager = createTestTaskManager();

  SECTION("createDocument")
  {
    MapDocument::createDocument(
      environmentConfig,
      mdl::Quake2GameInfo,
      mdl::MapFormat::Valve,
      vm::bbox3d{8192.0},
      *taskManager)
      | kdl::transform([&](auto document) {
          SECTION("creates a new map with the given game")
          {
            CHECK(&document->map().gameInfo() == &mdl::Quake2GameInfo);
          }
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("create")
  {
    auto document = MapDocument::createDocument(
                      environmentConfig,
                      mdl::Quake2GameInfo,
                      mdl::MapFormat::Valve,
                      vm::bbox3d{8192.0},
                      *taskManager)
                    | kdl::value();

    auto documentWasLoaded = Observer<void>{document->documentWasLoadedNotifier};

    const auto* previousMap = &document->map();

    document->create(
      environmentConfig,
      mdl::QuakeGameInfo,
      mdl::MapFormat::Daikatana,
      vm::bbox3d{8192.0})
      | kdl::transform([&]() {
          SECTION("creates a new map with the given game")
          {
            CHECK(&document->map() != previousMap);
            CHECK(&document->map().gameInfo() == &mdl::QuakeGameInfo);
          }

          SECTION("calls notifiers")
          {
            CHECK(documentWasLoaded.called);
          }
        })
      | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }


  SECTION("loadDocument")
  {
    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Map/emptyValveMap.map";

    MapDocument::loadDocument(
      environmentConfig,
      mdl::Quake2GameInfo,
      mdl::MapFormat::Valve,
      vm::bbox3d{8192.0},
      path,
      *taskManager)
      | kdl::transform([&](auto document) {
          SECTION("loads map at given path")
          {
            CHECK(document->map().path() == path);
            CHECK(&document->map().gameInfo() == &mdl::Quake2GameInfo);
          }
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }

  SECTION("load")
  {
    auto document = MapDocument::createDocument(
                      environmentConfig,
                      mdl::Quake2GameInfo,
                      mdl::MapFormat::Valve,
                      vm::bbox3d{8192.0},
                      *taskManager)
                    | kdl::value();

    auto documentWasLoaded = Observer<void>{document->documentWasLoadedNotifier};

    const auto* previousMap = &document->map();

    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Map/emptyValveMap.map";

    document->load(
      environmentConfig,
      mdl::QuakeGameInfo,
      mdl::MapFormat::Unknown,
      vm::bbox3d{8192.0},
      path)
      | kdl::transform([&]() {
          SECTION("loads map at given path")
          {
            CHECK(&document->map() != previousMap);
            CHECK(document->map().path() == path);
            CHECK(&document->map().gameInfo() == &mdl::QuakeGameInfo);
          }

          SECTION("calls notifiers")
          {
            CHECK(documentWasLoaded.called);
          }
        })
      | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }

  SECTION("reload")
  {
    auto document = MapDocument::createDocument(
                      environmentConfig,
                      mdl::Quake2GameInfo,
                      mdl::MapFormat::Valve,
                      vm::bbox3d{8192.0},
                      *taskManager)
                    | kdl::value();

    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Map/emptyValveMap.map";

    REQUIRE(document->load(
      environmentConfig,
      mdl::QuakeGameInfo,
      mdl::MapFormat::Unknown,
      vm::bbox3d{8192.0},
      path));

    REQUIRE(document->map().path() == path);

    auto* transientEntityNode = new mdl::EntityNode{{}};
    addNodes(
      document->map(), {{mdl::parentForNodes(document->map()), {transientEntityNode}}});
    REQUIRE(
      document->map().worldNode().defaultLayer()->children()
      == std::vector<mdl::Node*>{transientEntityNode});
    REQUIRE(document->map().modified());

    auto documentWasLoaded = Observer<void>{document->documentWasLoadedNotifier};

    const auto* previousMap = &document->map();

    document->reload() | kdl::transform([&]() {
      SECTION("reloads map")
      {
        CHECK(&document->map() != previousMap);
        CHECK(document->map().path() == path);
        CHECK(&document->map().gameInfo() == &mdl::QuakeGameInfo);
        CHECK(!document->map().modified());
      }

      SECTION("calls notifiers")
      {
        CHECK(documentWasLoaded.called);
      }
    }) | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }
}

} // namespace tb::ui
