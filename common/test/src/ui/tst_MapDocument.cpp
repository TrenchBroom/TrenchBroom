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
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("MapDocument")
{
  auto logger = NullLogger{};
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();

  SECTION("create")
  {
    auto documentWasLoaded = Observer<void>{document.documentWasLoadedNotifier};

    const auto* previousMap = &document.map();

    document.create(mdl::MapFormat::Daikatana, mdl::QuakeGameInfo, vm::bbox3d{8192.0})
      | kdl::transform([&]() {
          SECTION("creates a new map with the given game")
          {
            CHECK(&document.map() != previousMap);
            CHECK(&document.map().gameInfo() == &mdl::QuakeGameInfo);
          }

          SECTION("calls notifiers")
          {
            CHECK(documentWasLoaded.called);
          }
        })
      | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }

  SECTION("load")
  {
    auto documentWasLoaded = Observer<void>{document.documentWasLoadedNotifier};

    const auto* previousMap = &document.map();

    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Map/emptyValveMap.map";


    document.load(path, mdl::MapFormat::Unknown, mdl::QuakeGameInfo, vm::bbox3d{8192.0})
      | kdl::transform([&]() {
          SECTION("loads map at given path")
          {
            CHECK(&document.map() != previousMap);
            CHECK(document.map().path() == path);
            CHECK(&document.map().gameInfo() == &mdl::QuakeGameInfo);
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
    const auto path =
      std::filesystem::current_path() / "fixture/test/mdl/Map/emptyValveMap.map";

    REQUIRE(document.load(
      path, mdl::MapFormat::Unknown, mdl::QuakeGameInfo, vm::bbox3d{8192.0}));

    REQUIRE(document.map().path() == path);

    auto* transientEntityNode = new mdl::EntityNode{{}};
    addNodes(
      document.map(), {{mdl::parentForNodes(document.map()), {transientEntityNode}}});
    REQUIRE(
      document.map().worldNode().defaultLayer()->children()
      == std::vector<mdl::Node*>{transientEntityNode});
    REQUIRE(document.map().modified());

    auto documentWasLoaded = Observer<void>{document.documentWasLoadedNotifier};

    const auto* previousMap = &document.map();

    document.reload() | kdl::transform([&]() {
      SECTION("reloads map")
      {
        CHECK(&document.map() != previousMap);
        CHECK(document.map().path() == path);
        CHECK(&document.map().gameInfo() == &mdl::QuakeGameInfo);
        CHECK(!document.map().modified());
      }

      SECTION("calls notifiers")
      {
        CHECK(documentWasLoaded.called);
      }
    }) | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }
}

} // namespace tb::ui
